/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// Trigger includes
#include "TrigEventSelectorByteStream.h"
#include "TrigKernel/HltExceptions.h"

// Athena includes
#include "AthenaKernel/EventContextClid.h"
#include "ByteStreamCnvSvc/ByteStreamInputSvc.h"
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "ByteStreamData/RawEvent.h"
#include "EventInfo/EventInfo.h"
#include "StoreGate/StoreGateSvc.h"

// TDAQ includes
#include "hltinterface/DataCollector.h"

// =============================================================================
// Standard constructor
// =============================================================================
TrigEventSelectorByteStream::TrigEventSelectorByteStream(const std::string& name, ISvcLocator* svcLoc)
: base_class(name, svcLoc),
  m_eventSource("ByteStreamInputSvc", name),
  m_evtStore("StoreGateSvc", name) {
  declareProperty("ByteStreamInputSvc", m_eventSource);
}

// =============================================================================
// Standard destructor
// =============================================================================
TrigEventSelectorByteStream::~TrigEventSelectorByteStream() {}

// =============================================================================
// Implementation of Service::initialize
// =============================================================================
StatusCode TrigEventSelectorByteStream::initialize()
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);

  ATH_CHECK(m_eventSource.retrieve());
  ATH_CHECK(m_evtStore.retrieve());

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of Service::finalize
// =============================================================================
StatusCode TrigEventSelectorByteStream::finalize()
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);
  if (m_eventSource.release().isFailure()) {
    ATH_MSG_WARNING("Cannot release the event source service");
  }
  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of IEvtSelector::next(Context&)
// There is actually no event selection here, we process all events online
// =============================================================================
StatusCode TrigEventSelectorByteStream::next(IEvtSelector::Context& /*c*/) const
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);

  // RawEvent is a typedef for FullEventFragment
  const RawEvent* ptrRawEvent = nullptr;

  // Try to get the next event from the event source
  try {
    ptrRawEvent = m_eventSource->nextEvent();
  }
  catch (const hltonl::Exception::NoMoreEvents& e) {
    ATH_MSG_INFO(e.what());
    throw; // rethrow NoMoreEvents
  }
  catch (const std::exception& e) {
    ATH_MSG_ERROR("Failed to get next event from the event source, std::exception caught: " << e.what());
    return StatusCode::FAILURE;
  }
  catch (...) {
    ATH_MSG_ERROR("Failed to get next event from the event source, unhandled exception caught");
    return StatusCode::FAILURE;
  }

  // Check if something was returned
  if (ptrRawEvent == nullptr) {
    ATH_MSG_ERROR("Failed to get next event from the event source, nullptr returned");
    return StatusCode::FAILURE;
  }

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of IEvtSelector::createContext(Context*&)
// =============================================================================
StatusCode TrigEventSelectorByteStream::createContext(IEvtSelector::Context*& c) const
{
  c = new TrigEventSelectorByteStream::Context(this);
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of IEvtSelector::releaseContext(Context*&)
// =============================================================================
StatusCode TrigEventSelectorByteStream::releaseContext(IEvtSelector::Context*& /*c*/) const
{
  // this does nothing
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of IEvtSelector::createAddress(Context&,IOpaqueAddress*&)
// =============================================================================
StatusCode TrigEventSelectorByteStream::createAddress(const IEvtSelector::Context& /*c*/, IOpaqueAddress*& iop) const
{
  ATH_MSG_VERBOSE("start of " << __FUNCTION__);

  // Get the EventContext via event store because the interface doesn't allow passing it explicitly as an argument
  // and we don't want to use ThreadLocalContext. Don't use ReadHandle here because it calls ThreadLocalContext if
  // not given a context (which we want to retrieve).
  const EventContext* eventContext = nullptr;
  ATH_CHECK(m_evtStore->retrieve(eventContext));

  // Perhaps the name shouldn't be hard-coded
  ByteStreamAddress* addr = new ByteStreamAddress(ClassID_traits<EventInfo>::ID(), "ByteStreamEventInfo", "");
  addr->setEventContext(*eventContext);
  iop = static_cast<IOpaqueAddress*>(addr);
  ATH_CHECK(m_evtStore->recordAddress("ByteStreamEventInfo",iop));

  ATH_MSG_DEBUG("Recorded new ByteStreamAddress for EventInfo with event context " << *eventContext);

  ATH_MSG_VERBOSE("end of " << __FUNCTION__);
  return StatusCode::SUCCESS;
}

// =============================================================================
// Unimplemented methods of IEvtSelector
// =============================================================================
#define TRIGEVENTSELECTORBYTESTREAM_UNIMPL \
  ATH_MSG_FATAL("Misconfiguration - the method " << __FUNCTION__ << " cannot be used online"); \
  return StatusCode::FAILURE;

StatusCode TrigEventSelectorByteStream::next(IEvtSelector::Context& /*c*/, int /*jump*/) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

StatusCode TrigEventSelectorByteStream::previous(IEvtSelector::Context& /*c*/) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

StatusCode TrigEventSelectorByteStream::previous(IEvtSelector::Context& /*c*/, int /*jump*/) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

StatusCode TrigEventSelectorByteStream::last(IEvtSelector::Context& /*refContext*/) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

StatusCode TrigEventSelectorByteStream::rewind(IEvtSelector::Context& /*c*/) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

StatusCode TrigEventSelectorByteStream::resetCriteria(const std::string& /*cr*/, IEvtSelector::Context& /*c*/) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

// =============================================================================
// Context implementation
// =============================================================================
TrigEventSelectorByteStream::Context::Context(const IEvtSelector* selector)
: m_evtSelector(selector) {}

TrigEventSelectorByteStream::Context::Context(const TrigEventSelectorByteStream::Context& other)
: m_evtSelector(other.m_evtSelector) {}

TrigEventSelectorByteStream::Context::~Context() {}

void* TrigEventSelectorByteStream::Context::identifier() const
{
  return (void*)(m_evtSelector);
}

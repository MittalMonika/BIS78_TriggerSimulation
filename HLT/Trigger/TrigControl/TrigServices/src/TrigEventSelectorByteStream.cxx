/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigEventSelectorByteStream.h"

#include "ByteStreamData/RawEvent.h"
#include "ByteStreamCnvSvc/ByteStreamInputSvc.h"
#include "EventInfo/EventInfo.h"
#include "StoreGate/StoreGateSvc.h"
#include "TrigKernel/HltExceptions.h"

#include "hltinterface/DataCollector.h"

// =============================================================================
// Standard constructor
// =============================================================================
TrigEventSelectorByteStream::TrigEventSelectorByteStream(const std::string& name, ISvcLocator* svcLoc)
: Service(name, svcLoc),
  m_eventSource("ByteStreamInputSvc", name),
  m_evtStore("StoreGateSvc", name) {
  declareProperty("ByteStreamInputSvc", m_eventSource);
}

// =============================================================================
// Standard destructor
// =============================================================================
TrigEventSelectorByteStream::~TrigEventSelectorByteStream() {}

// =============================================================================
// Implementation of IInterface::queryInterface
// =============================================================================
StatusCode TrigEventSelectorByteStream::queryInterface(const InterfaceID& riid, void** ppvInterface)
{
  verbose() << "start of " << __FUNCTION__ << endmsg;

  if(IEvtSelector::interfaceID().versionMatch(riid))
    *ppvInterface = static_cast<IEvtSelector*>(this);
  else
    return Service::queryInterface(riid, ppvInterface);

  addRef();
  verbose() << "end of " << __FUNCTION__ << endmsg;
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of Service::initialize
// =============================================================================
StatusCode TrigEventSelectorByteStream::initialize()
{
  verbose() << "start of " << __FUNCTION__ << endmsg;
  StatusCode sc = StatusCode::SUCCESS;

  sc = m_eventSource.retrieve();
  if (sc.isFailure()) {
    error() << "Failed to retrieve the event source service" << endmsg;
    return sc;
  }

  sc = m_evtStore.retrieve();
  if (sc.isFailure()) {
    error() << "Failed to retrieve the event store service" << endmsg;
    return sc;
  }

  verbose() << "end of " << __FUNCTION__ << endmsg;
  return sc;
}

// =============================================================================
// Implementation of Service::finalize
// =============================================================================
StatusCode TrigEventSelectorByteStream::finalize()
{
  verbose() << "start of " << __FUNCTION__ << endmsg;
  if (m_eventSource.release().isFailure()) {
    warning() << "Cannot release the event source service" << endmsg;
  }
  verbose() << "end of " << __FUNCTION__ << endmsg;
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of IEvtSelector::next(Context&)
// There is actually no event selection here, we process all events online
// =============================================================================
StatusCode TrigEventSelectorByteStream::next(IEvtSelector::Context& /*c*/) const
{
  verbose() << "start of " << __FUNCTION__ << endmsg;

  // RawEvent is a typedef for FullEventFragment
  const RawEvent* ptrRawEvent = nullptr;

  // Try to get the next event from the event source
  try {
    ptrRawEvent = m_eventSource->nextEvent();
  }
  catch (hltonl::Exception::NoMoreEvents e) {
    info() << e.what() << endmsg;
    throw; // rethrow NoMoreEvents
  }
  catch (...) { // what can we actually catch here?
    error() << "Failed to get next event from the event source" << endmsg;
    return StatusCode::FAILURE;
  }

  // Check if something was returned
  if (ptrRawEvent == nullptr) {
    error() << "Event source failed to read next event" << endmsg;
    return StatusCode::FAILURE;
  }

  // In online implementation, this creates an EventInfo address for use in event processing
  if (m_eventSource->generateDataHeader().isFailure()) {
    error() << "Failed to record ByteStream DataHeader / EventInfo in StoreGate" << endmsg;
    return StatusCode::FAILURE;
  }

  verbose() << "end of " << __FUNCTION__ << endmsg;
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
// Unlike offline, we do not provide a DataHeader proxy here, but an EventInfo proxy
// =============================================================================
StatusCode TrigEventSelectorByteStream::createAddress(const IEvtSelector::Context& c, IOpaqueAddress*& iop) const
{
  verbose() << "start of " << __FUNCTION__ << endmsg;
  SG::DataProxy* proxy = m_evtStore->proxy(ClassID_traits<EventInfo>::ID(),"ByteStreamEventInfo");
  if (proxy !=0) {
    iop = proxy->address();
    verbose() << "end of " << __FUNCTION__ << endmsg;
    return StatusCode::SUCCESS;
  } else {
    iop = 0;
    verbose() << "end of " << __FUNCTION__ << endmsg;
    return StatusCode::FAILURE;
  }
}

// =============================================================================
// Unimplemented methods of IEvtSelector
// =============================================================================
#define TRIGEVENTSELECTORBYTESTREAM_UNIMPL \
  fatal() << "Misconfiguration - the method " << __FUNCTION__ << " cannot be used online" << endmsg; \
  return StatusCode::FAILURE;

StatusCode TrigEventSelectorByteStream::next(IEvtSelector::Context& c, int jump) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

StatusCode TrigEventSelectorByteStream::previous(IEvtSelector::Context& c) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

StatusCode TrigEventSelectorByteStream::previous(IEvtSelector::Context& c, int jump) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

StatusCode TrigEventSelectorByteStream::last(IEvtSelector::Context& refContext) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

StatusCode TrigEventSelectorByteStream::rewind(IEvtSelector::Context& c) const
{
  TRIGEVENTSELECTORBYTESTREAM_UNIMPL
}

StatusCode TrigEventSelectorByteStream::resetCriteria(const std::string& cr, IEvtSelector::Context& c) const
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
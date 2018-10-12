/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SCTRawDataProviderTool.cxx
//   Implementation file for class SCTRawDataProviderTool
///////////////////////////////////////////////////////////////////

#include "SCTRawDataProviderTool.h"
#include "StoreGate/ReadHandle.h"

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

// -------------------------------------------------------
// Constructor

SCTRawDataProviderTool::SCTRawDataProviderTool
(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type, name, parent),
  m_robIdSet{},
  m_decodeErrCount{0},
  m_mutex{}
{
}

// -------------------------------------------------------
// Initialize

StatusCode SCTRawDataProviderTool::initialize()
{
  StatusCode sc{AlgTool::initialize()};
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Failed to init baseclass");
    return StatusCode::FAILURE;
  }
   
  ATH_CHECK(m_decoder.retrieve());

  return StatusCode::SUCCESS;
}

// -------------------------------------------------------
// Convert method

StatusCode SCTRawDataProviderTool::convert(std::vector<const ROBFragment*>& vecRobs,
                                           ISCT_RDO_Container& rdoIdc,
                                           InDetBSErrContainer* errs,
                                           SCT_ByteStreamFractionContainer* bsFracCont) const
{
  if (vecRobs.empty()) return StatusCode::SUCCESS;
  ATH_MSG_DEBUG("SCTRawDataProviderTool::convert()");
  
  StatusCode sc{StatusCode::SUCCESS};

  // loop over the ROB fragments

  std::set<uint32_t> tmpRobIdSet;

  for (const ROBFragment* rob_it : vecRobs) {
    // get the ID of this ROB/ROD
    uint32_t robid{(rob_it)->rod_source_id()};
    // check if this ROBFragment was already decoded (EF case in ROIs)
    if (m_robIdSet.count(robid) or tmpRobIdSet.count(robid)) {
      ATH_MSG_DEBUG(" ROB Fragment with ID  "
                    << std::hex<<robid << std::dec
                    << " already decoded, skip");
      continue;
    }

    // Insert the new ROBID to the set.
    tmpRobIdSet.insert(robid);

    sc = m_decoder->fillCollection(*rob_it, rdoIdc, errs, bsFracCont);
    if (sc==StatusCode::FAILURE) {
      if (m_decodeErrCount <= 100) {
        if (100 == m_decodeErrCount) {
          ATH_MSG_ERROR("Too many Problem with SCT Decoding messages, turning message off.");
        }
        else {
          ATH_MSG_ERROR("Problem with SCT ByteStream Decoding!");
        }
        m_decodeErrCount++;
      }
    }
  }

  m_mutex.lock();
  m_robIdSet.insert(tmpRobIdSet.begin(), tmpRobIdSet.end());
  m_mutex.unlock();

  if (sc==StatusCode::FAILURE) {
    ATH_MSG_ERROR("There was a problem with SCT ByteStream conversion");
    return sc;
  }

  return sc;
}

// -------------------------------------------------------
// beginNewEvent method

void SCTRawDataProviderTool::beginNewEvent() const {
  // reset list of known robIds
  std::lock_guard<std::mutex> lock(m_mutex);
  m_robIdSet.clear();
}

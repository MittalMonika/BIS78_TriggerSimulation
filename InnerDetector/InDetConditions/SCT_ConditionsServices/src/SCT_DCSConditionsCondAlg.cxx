/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_DCSConditionsCondAlg.h"

#include "GaudiKernel/EventIDRange.h"

SCT_DCSConditionsCondAlg::SCT_DCSConditionsCondAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : ::AthAlgorithm(name, pSvcLocator)
  , m_readKeyState{"/SCT/DCS/CHANSTAT"}
  , m_writeKeyState{"SCT_DCSStatCondData", "SCT_DCSStatCondData"}
  , m_condSvc{"CondSvc", name}
{
  declareProperty("ReadKeyState", m_readKeyState);
  declareProperty("WriteKeyState", m_writeKeyState);
}

SCT_DCSConditionsCondAlg::~SCT_DCSConditionsCondAlg()
{
}

StatusCode SCT_DCSConditionsCondAlg::initialize()
{
  ATH_MSG_DEBUG("initialize " << name());

  // CondSvc
  ATH_CHECK(m_condSvc.retrieve());

  // Read Cond Handle
  ATH_CHECK(m_readKeyState.initialize());

  // Write Cond Handle
  ATH_CHECK(m_writeKeyState.initialize());
  // Register write handle
  if(m_condSvc->regHandle(this, m_writeKeyState, m_writeKeyState.dbKey()).isFailure()) {
    ATH_MSG_ERROR("unable to register WriteCondHandle " << m_writeKeyState.fullKey() << " with CondSvc");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode SCT_DCSConditionsCondAlg::execute()
{
  ATH_MSG_DEBUG("execute " << name());

  // Write Cond Handle
  SG::WriteCondHandle<SCT_DCSStatCondData> writeHandle{m_writeKeyState};

  // Do we have a valid Write Cond Handle for current time?
  if(writeHandle.isValid()) {
    // in theory this should never be called in MT
    writeHandle.updateStore();
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                  << ". In theory this should not be called, but may happen"
                  << " if multiple concurrent events are being processed out of order."
                  << " Forcing update of Store contents");
    return StatusCode::SUCCESS;
  }

  // Construct the output Cond Object and fill it in
  SCT_DCSStatCondData* writeCdo{new SCT_DCSStatCondData()};

  // Read Cond Handle
  SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKeyState};
  const CondAttrListCollection* readCdo{*readHandle};
  if(readCdo==nullptr) {
    ATH_MSG_ERROR("Null pointer to the read conditions object");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Size of CondAttrListCollection readCdo->size()= " << readCdo->size());

  // Fill Write Cond Handle
  static const unsigned int defectListIndex{7};
  CondAttrListCollection::const_iterator iter{readCdo->begin()};
  CondAttrListCollection::const_iterator last{readCdo->end()};
  for(; iter!=last; ++iter) {
    // const AthenaAttributeList& list{iter->second};
    // if(list.size()>defectListIndex) {
    //   writeCdo->insert(iter->first, list[defectListIndex].data<std::string>());
    // }
  }

  // Define validity of the output cond obbject and record it
  EventIDRange rangeW;
  if(not readHandle.range(rangeW)) {
    ATH_MSG_ERROR("Failed to retrieve validity range for " << readHandle.key());
    return StatusCode::FAILURE;
  }
  if(writeHandle.record(rangeW, writeCdo).isFailure()) {
    ATH_MSG_ERROR("Could not record SCT_DCSStatCondData " << writeHandle.key()
                  << " with EventRange " << rangeW
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new CDO " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
}

StatusCode SCT_DCSConditionsCondAlg::finalize()
{
  ATH_MSG_DEBUG("finalize " << name());
  return StatusCode::SUCCESS;
}

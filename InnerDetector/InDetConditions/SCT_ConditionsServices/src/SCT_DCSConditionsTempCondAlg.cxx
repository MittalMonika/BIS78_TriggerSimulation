/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_DCSConditionsTempCondAlg.h"

#include "Identifier/IdentifierHash.h"
#include "SCT_Cabling/SCT_OnlineId.h"

#include "GaudiKernel/EventIDRange.h"

SCT_DCSConditionsTempCondAlg::SCT_DCSConditionsTempCondAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : ::AthAlgorithm(name, pSvcLocator)
  , m_readKey{"/SCT/DCS/MODTEMP"}
  , m_writeKey{{"SCT_DCSTemp0CondData"}, {"SCT_DCSTemp1CondData"}}
  , m_condSvc{"CondSvc", name}
  , m_returnHVTemp{true}
{
  declareProperty("ReturnHVTemp", m_returnHVTemp);
  
  declareProperty("ReadKey", m_readKey, "Key of input (raw) temperature conditions folder");
  declareProperty("WriteKey0", m_writeKey[0], "Key of output (derived) temperature conditions folder");
  declareProperty("WriteKey1", m_writeKey[1], "Key of output (derived) temperature conditions folder");
}

SCT_DCSConditionsTempCondAlg::~SCT_DCSConditionsTempCondAlg() {
}

StatusCode SCT_DCSConditionsTempCondAlg::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  // CondSvc
  ATH_CHECK(m_condSvc.retrieve());

  if (m_returnHVTemp) {
    // Read Cond Handle
    ATH_CHECK(m_readKey.initialize());
    // Write Cond Handles
    for (unsigned int side{0}; side<2; side++) {
      ATH_CHECK(m_writeKey[side].initialize());
      if(m_condSvc->regHandle(this, m_writeKey[side], m_writeKey[side].dbKey()).isFailure()) {
        ATH_MSG_FATAL("unable to register WriteCondHandle " << m_writeKey[side].fullKey() << " with CondSvc");
        return StatusCode::FAILURE;
      }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode SCT_DCSConditionsTempCondAlg::execute() {
  ATH_MSG_DEBUG("execute " << name());

  if (not m_returnHVTemp) {
    return StatusCode::SUCCESS;
  }

  // Write Cond Handle
  SG::WriteCondHandle<SCT_DCSFloatCondData> writeHandle[2]{m_writeKey[0], m_writeKey[1]};
  // Do we have a valid Write Cond Handle for current time?
  for (unsigned int side{2}; side<2; side++) {
    if (writeHandle[side].isValid()) {
      // in theory this should never be called in MT
      writeHandle[side].updateStore();
      if (side==0) writeHandle[1].updateStore();
      ATH_MSG_DEBUG("CondHandle " << writeHandle[side].fullKey() << " is already valid."
                    << ". In theory this should not be called, but may happen"
                    << " if multiple concurrent events are being processed out of order."
                    << " Forcing update of Store contents");
      return StatusCode::SUCCESS; 
    }
  }

  // Read Cond Handle
  SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey};
  const CondAttrListCollection* readCdo{*readHandle}; 
  if (readCdo==nullptr) {
    ATH_MSG_FATAL("Null pointer to the read conditions object");
    return StatusCode::FAILURE;
  }
  // Get the validitiy range
  EventIDRange rangeW;
  if (not readHandle.range(rangeW)) {
    ATH_MSG_FATAL("Failed to retrieve validity range for " << readHandle.key());
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
  ATH_MSG_INFO("Range of input is " << rangeW);
  
  // Construct the output Cond Object and fill it in
  SCT_DCSFloatCondData* writeCdo[2]{new SCT_DCSFloatCondData(), new SCT_DCSFloatCondData()};

  // Read temperature info
  std::string param[2]{"MOCH_TM0_RECV", "MOCH_TM1_RECV"};
  CondAttrListCollection::const_iterator attrList{readCdo->begin()};
  CondAttrListCollection::const_iterator end{readCdo->end()};
  // CondAttrListCollection doesn't support C++11 type loops, no generic 'begin'
  for (; attrList!=end; ++attrList) {
    // A CondAttrListCollection is a map of ChanNum and AttributeList
    CondAttrListCollection::ChanNum channelNumber{attrList->first};
    CondAttrListCollection::AttributeList payload{attrList->second};
    for (unsigned int side{2}; side<2; side++) {
      if (payload.exists(param[side])) {
        float val{payload[param[side]].data<float>()};
        writeCdo[side]->setValue(channelNumber, val);
      } else {
        ATH_MSG_WARNING(param[side] << " does not exist for ChanNum " << channelNumber);
      }
    }
  }

  // Record the output cond object
  for(unsigned int side{2}; side<2; side++) {
    if (writeHandle[side].record(rangeW, writeCdo[side]).isFailure()) {
      ATH_MSG_FATAL("Could not record SCT_DCSFloatCondData " << writeHandle[side].key() 
                    << " with EventRange " << rangeW
                    << " into Conditions Store");
      delete writeCdo[side];
      if (side==0) delete writeCdo[1];
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new CDO " << writeHandle[side].key() << " with range " << rangeW << " into Conditions Store");
  }

  return StatusCode::SUCCESS;
}

StatusCode SCT_DCSConditionsTempCondAlg::finalize()
{
  ATH_MSG_DEBUG("finalize " << name());
  return StatusCode::SUCCESS;
}

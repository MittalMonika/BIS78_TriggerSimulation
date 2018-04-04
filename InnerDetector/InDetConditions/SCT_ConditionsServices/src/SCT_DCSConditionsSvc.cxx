/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// New SCT_DCSConditions Service, based on existing tool in SCT_ConditionsAlgs
// A. R-Veronneau 26/02/08

#include "SCT_DCSConditionsSvc.h"
#include "SCT_ConditionsServices/SCT_SlhcIdConverter.h"
#include "InDetIdentifier/SCT_ID.h"

#include "GaudiKernel/ThreadLocalContext.h"

using SCT_ConditionsServices::castId;

const Identifier SCT_DCSConditionsSvc::s_invalidId;
const float SCT_DCSConditionsSvc::s_defaultHV{-30.}; 
const float SCT_DCSConditionsSvc::s_defaultTemperature{-40.};

SCT_DCSConditionsSvc::SCT_DCSConditionsSvc(const std::string& name,
                                           ISvcLocator* pSvcLocator) :
  AthService(name, pSvcLocator),
  m_detStore{"DetectorStore", name},
  m_readAllDBFolders{true},
  m_returnHVTemp{true},
  m_barrel_correction{-3.7},
  m_ecInner_correction{-13.1},
  m_ecOuter_correction{-15.5},
  m_mutex{},
  m_cacheState{},
  m_cacheHV{},
  m_cacheTemp0{},
  m_pBadModules{},
  m_pModulesHV{},
  m_pModulesTemp0{},
  m_condKeyState{"SCT_DCSStatCondData"},
  m_condKeyHV{"SCT_DCSHVCondData"},
  m_condKeyTemp0{"SCT_DCSTemp0CondData"},
  m_pHelper{nullptr}
{ 
    //declare variables which will be filled by jobOptions
    declareProperty("DetectorStore", m_detStore);
    declareProperty("ReadAllDBFolders", m_readAllDBFolders);
    declareProperty("ReturnHVTemp", m_returnHVTemp);
    declareProperty("TempBarrelCorrection", m_barrel_correction);
    declareProperty("TempEcInnerCorrection", m_ecInner_correction);
    declareProperty("TempEcOuterCorrection", m_ecOuter_correction);
}

StatusCode SCT_DCSConditionsSvc::initialize() {
  if (AthService::initialize().isFailure()) return StatusCode::FAILURE;
  if (m_detStore.retrieve().isFailure()) {
    ATH_MSG_ERROR(" Cannot retrieve detector store ");
    return StatusCode::FAILURE;
  }
  if (m_detStore->retrieve(m_pHelper,"SCT_ID").isFailure()) {
    ATH_MSG_ERROR("SCT helper failed to retrieve ");
    return StatusCode::FAILURE;
  }
  
  // Read Cond Handle Keys
  if ((m_readAllDBFolders and m_returnHVTemp) or m_returnHVTemp) {
    ATH_CHECK(m_condKeyHV.initialize());
    ATH_CHECK(m_condKeyTemp0.initialize());
  }
  if ((m_readAllDBFolders and m_returnHVTemp) or (not m_readAllDBFolders and not m_returnHVTemp)) {
    ATH_CHECK(m_condKeyState.initialize());
  }

  return StatusCode::SUCCESS;
}

StatusCode SCT_DCSConditionsSvc::finalize() { 
  return StatusCode::SUCCESS;
}

//////////////////////////////
// From s.binet
// Query the interfaces.
//   Input: riid, Requested interface ID
//          ppvInterface, Pointer to requested interface
//   Return: StatusCode indicating SUCCESS or FAILURE.
// N.B. Don't forget to release the interface after use!!!
StatusCode SCT_DCSConditionsSvc::queryInterface(const InterfaceID& riid, void** ppvInterface) {
  if (ISCT_DCSConditionsSvc::interfaceID().versionMatch(riid)) {
    *ppvInterface = this;
  } else if (ISCT_ConditionsSvc::interfaceID().versionMatch(riid)) {
    *ppvInterface = dynamic_cast<ISCT_ConditionsSvc*>(this);
  } else {
    // Interface is not directly available : try out a base class
    return AthService::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}

//returns if fillData worked correctly
bool SCT_DCSConditionsSvc::filled() const {
  return true;
}

//Can report about the module as a whole or the wafer
bool SCT_DCSConditionsSvc::canReportAbout(InDetConditions::Hierarchy h) {
  return (h==InDetConditions::SCT_MODULE or h==InDetConditions::SCT_SIDE or h==InDetConditions::SCT_STRIP);
}   

//returns the module ID (int), or returns -1 if not able to report
Identifier SCT_DCSConditionsSvc::getModuleID(const Identifier& elementId, InDetConditions::Hierarchy h) {
  if (not canReportAbout(h)) return s_invalidId;  

  Identifier moduleId;

  if (h==InDetConditions::SCT_MODULE) {
    moduleId = elementId;
  } else if (h==InDetConditions::SCT_SIDE) {
    moduleId = m_pHelper->module_id(elementId); 
  } else if (h==InDetConditions::SCT_STRIP) {
    Identifier waferId = m_pHelper->wafer_id(elementId);
    moduleId = m_pHelper->module_id(waferId); 
  }
  return moduleId;
}

//Returns if element Id is good or bad
bool SCT_DCSConditionsSvc::isGood(const Identifier& elementId, InDetConditions::Hierarchy h) {
  Identifier moduleId=getModuleID(elementId, h);
  if (not moduleId.is_valid()) return true; // not canreportabout

  if ((m_readAllDBFolders and m_returnHVTemp) or (not m_readAllDBFolders and not m_returnHVTemp)) {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    const SCT_DCSStatCondData* condDataState{getCondDataState(ctx)};
    if (!condDataState) return false; // no cond data
    else if (condDataState->output(castId(moduleId))==0) return true; //No params are listed as bad
    else return false;
  } else {
    return true;
  }
}

//Does the same for hashIds
bool SCT_DCSConditionsSvc::isGood(const IdentifierHash& hashId) {
  Identifier waferId = m_pHelper->wafer_id(hashId);
  Identifier moduleId = m_pHelper->module_id(waferId);
  return isGood(moduleId, InDetConditions::SCT_MODULE);
}

/////////////////////////////////// 

// some lame helper methods: 
// returns HV (s_defaultHV(-30) if there is no information)
float SCT_DCSConditionsSvc::modHV(const Identifier& elementId, InDetConditions::Hierarchy h) {
  Identifier moduleId = getModuleID(elementId, h);
  if (not moduleId.is_valid()) return s_defaultHV; // not canreportabout, return s_defaultHV(-30)

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const SCT_DCSFloatCondData* condDataHV{getCondDataHV(ctx)};
  if (!condDataHV) return s_defaultHV; // no cond data

  float hvval{s_defaultHV};
  if (condDataHV->getValue(castId(moduleId), hvval) and isGood(elementId, h)) {
    return hvval;
  }
  return s_defaultHV; //didn't find the module, return s_defaultHV(-30)
}

//Does the same for hashIds
float SCT_DCSConditionsSvc::modHV(const IdentifierHash& hashId) {
  Identifier waferId = m_pHelper->wafer_id(hashId);
  Identifier moduleId = m_pHelper->module_id(waferId);
  return modHV(moduleId,InDetConditions::SCT_MODULE);
} 

//Returns temp0 (s_defaultTemperature(-40) if there is no information)
float SCT_DCSConditionsSvc::hybridTemperature(const Identifier& elementId, InDetConditions::Hierarchy h) {
  Identifier moduleId = getModuleID(elementId, h);
  if (not moduleId.is_valid()) return s_defaultTemperature; // not canreportabout

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const SCT_DCSFloatCondData* condDataTemp0{getCondDataTemp0(ctx)};
  if (!condDataTemp0) return s_defaultTemperature; // no cond data

  float temperature{s_defaultTemperature};
  if (condDataTemp0->getValue(castId(moduleId), temperature) and isGood(elementId, h)) {
    return temperature;
  }
  return s_defaultTemperature;//didn't find the module, return -40. 
} 

//Does the same for hashIds
float SCT_DCSConditionsSvc::hybridTemperature(const IdentifierHash& hashId) {
  Identifier waferId = m_pHelper->wafer_id(hashId);
  Identifier moduleId = m_pHelper->module_id(waferId);
  return hybridTemperature(moduleId, InDetConditions::SCT_MODULE);
}

//Returns temp0 + correction for Lorentz angle calculation (s_defaultTemperature(-40) if there is no information)
float SCT_DCSConditionsSvc::sensorTemperature(const Identifier& elementId, InDetConditions::Hierarchy h) {
  Identifier moduleId = getModuleID(elementId, h);
  if (not moduleId.is_valid()) return s_defaultTemperature; // not canreportabout

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const SCT_DCSFloatCondData* condDataTemp0{getCondDataTemp0(ctx)};
  if (!condDataTemp0) return s_defaultTemperature; // no cond data

  float temperature{s_defaultTemperature};
  if (condDataTemp0->getValue(castId(moduleId), temperature) and isGood(elementId, h)) {
    int bec{m_pHelper->barrel_ec(moduleId)};
    if (bec==0) { // Barrel
      return ( temperature + m_barrel_correction);  //return the temp+correction
    } else { // Endcaps
      int modeta{m_pHelper->eta_module(moduleId)};
      if (modeta==2) {
        return (temperature + m_ecInner_correction);  //return the temp+correction
      } else {
        return (temperature + m_ecOuter_correction);  //return the temp+correction
      }
    }
  }
  return s_defaultTemperature;  //didn't find the module, return s_defaultTemperature(-40).
} 

//Does the same for hashIds
float SCT_DCSConditionsSvc::sensorTemperature(const IdentifierHash& hashId) {
  Identifier waferId = m_pHelper->wafer_id(hashId);
  Identifier moduleId = m_pHelper->module_id(waferId);
  return sensorTemperature(moduleId, InDetConditions::SCT_MODULE);
}

///////////////////////////////////

// This is kept for SiLorentzAngleSvc callback of SCT_SiliconConditionsSvc and SCT_DCSConditionsSvc
// (2017-09-29)
StatusCode SCT_DCSConditionsSvc::fillData(int& /* i */, std::list<std::string>& /*keys*/) {
  ATH_MSG_FATAL("SCT_DCSConditionsSvc::fillData should not be used anymore!");
  return StatusCode::FAILURE;
}

const SCT_DCSStatCondData*
SCT_DCSConditionsSvc::getCondDataState(const EventContext& ctx) const {
  static const EventContext::ContextEvt_t invalidValue{EventContext::INVALID_CONTEXT_EVT};
  EventContext::ContextID_t slot{ctx.slot()};
  EventContext::ContextEvt_t evt{ctx.evt()};
  std::lock_guard<std::mutex> lock{m_mutex};
  if (slot>=m_cacheState.size()) {
    m_cacheState.resize(slot+1, invalidValue); // Store invalid values in order to go to the next IF statement.
  }
  if (m_cacheState[slot]!=evt) {
    SG::ReadCondHandle<SCT_DCSStatCondData> condData{m_condKeyState};
    if (not condData.isValid()) {
      ATH_MSG_ERROR("Failed to get " << m_condKeyState.key());
    }
    m_pBadModules.set(*condData);
    m_cacheState[slot] = evt;
  }
  return m_pBadModules.get();
}

const SCT_DCSFloatCondData*
SCT_DCSConditionsSvc::getCondDataHV(const EventContext& ctx) const {
  static const EventContext::ContextEvt_t invalidValue{EventContext::INVALID_CONTEXT_EVT};
  EventContext::ContextID_t slot{ctx.slot()};
  EventContext::ContextEvt_t evt{ctx.evt()};
  std::lock_guard<std::mutex> lock{m_mutex};
  if (slot>=m_cacheHV.size()) {
    m_cacheHV.resize(slot+1, invalidValue); // Store invalid values in order to go to the next IF statement.
  }
  if (m_cacheHV[slot]!=evt) {
    SG::ReadCondHandle<SCT_DCSFloatCondData> condData{m_condKeyHV};
    if (not condData.isValid()) {
      ATH_MSG_ERROR("Failed to get " << m_condKeyHV.key());
    }
    m_pModulesHV.set(*condData);
    m_cacheHV[slot] = evt;
  }
  return m_pModulesHV.get();
}

const SCT_DCSFloatCondData*
SCT_DCSConditionsSvc::getCondDataTemp0(const EventContext& ctx) const {
  static const EventContext::ContextEvt_t invalidValue{EventContext::INVALID_CONTEXT_EVT};
  EventContext::ContextID_t slot{ctx.slot()};
  EventContext::ContextEvt_t evt{ctx.evt()};
  std::lock_guard<std::mutex> lock{m_mutex};
  if (slot>=m_cacheTemp0.size()) {
    m_cacheTemp0.resize(slot+1, invalidValue); // Store invalid values in order to go to the next IF statement.
  }
  if (m_cacheTemp0[slot]!=evt) {
    SG::ReadCondHandle<SCT_DCSFloatCondData> condData{m_condKeyTemp0};
    if (not condData.isValid()) {
      ATH_MSG_ERROR("Failed to get " << m_condKeyTemp0.key());
    }
    m_pModulesTemp0.set(*condData);
    m_cacheTemp0[slot] = evt;
  }
  return m_pModulesTemp0.get();
}

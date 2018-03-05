/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_DCSConditionsSvc_h
#define SCT_DCSConditionsSvc_h
/**
 * @file SCT_DCSConditionsSvc.h
 *
 * @brief Header file for the SCT_DCSConditionsSvc class 
 *  in package SCT_ConditionsServices
 *
 * @author A. R-Veronneau 26/02/07, Shaun Roe 4/4/2008
 **/

//
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ContextSpecificPtr.h"
//
#include "InDetConditionsSummaryService/InDetHierarchy.h"
#include "SCT_ConditionsServices/ISCT_DCSConditionsSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "SCT_ConditionsData/SCT_DCSFloatCondData.h"
#include "SCT_ConditionsData/SCT_DCSStatCondData.h"
//STL
#include <list>
#include <string>
#include <map>
#include <mutex>

class SCT_ID;

/**
 * Class to provide DCS information about modules from the COOL database
 **/
class SCT_DCSConditionsSvc: virtual public ISCT_DCSConditionsSvc, virtual public AthService {
  
public:
  SCT_DCSConditionsSvc(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~SCT_DCSConditionsSvc() = default;
  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual StatusCode queryInterface(const InterfaceID& riid, void** ppvInterface);
  static const InterfaceID& interfaceID();
  
  /// @name Methods to be implemented from virtual baseclass methods, when introduced
  //@{
  ///Return whether this service can report on the hierarchy level (e.g. module, chip...)
  virtual bool canReportAbout(InDetConditions::Hierarchy h);
  //returns the module ID (int), or returns 9999 (not a valid module number) if not able to report
  virtual  Identifier getModuleID(const Identifier& elementId, InDetConditions::Hierarchy h);
  ///Summarise the result from the service as good/bad
  virtual bool isGood(const Identifier& elementId, InDetConditions::Hierarchy h=InDetConditions::DEFAULT);
  ///is it good?, using wafer hash
  virtual bool isGood(const IdentifierHash& hashId);
  //Returns HV (0 if there is no information)
  virtual float modHV(const Identifier& elementId, InDetConditions::Hierarchy h=InDetConditions::DEFAULT);
  //Does the same for hashIds
  virtual float modHV(const IdentifierHash& hashId);
  //Returns temp0 (0 if there is no information)
  virtual float hybridTemperature(const Identifier& elementId, InDetConditions::Hierarchy h=InDetConditions::DEFAULT);
  //Does the same for hashIds
  virtual float hybridTemperature(const IdentifierHash& hashId);
  //Returns temp0 + correction for Lorentz angle calculation (0 if there is no information)
  virtual float sensorTemperature(const Identifier& elementId, InDetConditions::Hierarchy h=InDetConditions::DEFAULT);
  //Does the same for hashIds
  virtual float sensorTemperature(const IdentifierHash& hashId);
  virtual StatusCode fillData(int& i, std::list<std::string>& keys);
  ///Manually get the data in the structure before proceding
  virtual StatusCode fillData() { return StatusCode::FAILURE; }
  virtual bool filled() const;
  ///Need to access cool on every callback, so can't fill during initialize
  virtual bool canFillDuringInitialize() { return false; }
  //@}
    
private:
  //Declare Storegate container
  ServiceHandle<StoreGateSvc> m_detStore;
  //Key for DataHandle
  BooleanProperty m_readAllDBFolders;
  BooleanProperty m_returnHVTemp;
  float m_barrel_correction;
  float m_ecInner_correction;
  float m_ecOuter_correction;
  // Mutex to protect the contents.
  mutable std::mutex m_mutex;
  // Cache to store events for slots
  mutable std::vector<EventContext::ContextEvt_t> m_cacheState;
  mutable std::vector<EventContext::ContextEvt_t> m_cacheHV;
  mutable std::vector<EventContext::ContextEvt_t> m_cacheTemp0;
  // Pointer of SCT_DCSStatCondData
  mutable Gaudi::Hive::ContextSpecificPtr<const SCT_DCSStatCondData> m_pBadModules;
  // Pointers of SCT_DCSFloatCondData
  mutable Gaudi::Hive::ContextSpecificPtr<const SCT_DCSFloatCondData> m_pModulesHV;
  mutable Gaudi::Hive::ContextSpecificPtr<const SCT_DCSFloatCondData> m_pModulesTemp0;
  SG::ReadCondHandleKey<SCT_DCSStatCondData> m_condKeyState;
  SG::ReadCondHandleKey<SCT_DCSFloatCondData> m_condKeyHV;
  SG::ReadCondHandleKey<SCT_DCSFloatCondData> m_condKeyTemp0;
  const SCT_ID* m_pHelper;
  static const Identifier s_invalidId;
  static const float s_defaultHV;
  static const float s_defaultTemperature;
  const SCT_DCSStatCondData* getCondDataState(const EventContext& ctx) const;
  const SCT_DCSFloatCondData* getCondDataHV(const EventContext& ctx) const;
  const SCT_DCSFloatCondData* getCondDataTemp0(const EventContext& ctx) const;
};

#endif // SCT_DCSConditionsSvc_h 

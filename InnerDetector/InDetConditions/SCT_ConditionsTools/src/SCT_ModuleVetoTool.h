/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_ModuleVetoTool.h
 * header file for tool allowing one to declare modules as bad
 * @author shaun.roe@cern.ch
 **/

#ifndef SCT_ModuleVetoTool_h
#define SCT_ModuleVetoTool_h

//STL includes
#include <set>
#include <vector>
#include <mutex>

//Interface include
#include "SCT_ConditionsTools/ISCT_ConditionsTool.h"

//Gaudi includes
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ContextSpecificPtr.h"

//Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetConditionsSummaryService/InDetHierarchy.h"
#include "SCT_ConditionsData/SCT_ModuleVetoCondData.h"

//forward declarations
class SCT_ID;

/**
 * @class SCT_ModuleVetoTool
 * Tool allowing one to manually declare detector elements as 'bad' in the joboptions file
 **/
class SCT_ModuleVetoTool: public extends<AthAlgTool, ISCT_ConditionsTool> {
 public:
  //@name Tool methods
  //@{
  SCT_ModuleVetoTool(const std::string &type, const std::string &name, const IInterface *parent);
  virtual ~SCT_ModuleVetoTool() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  //@}
  
  ///Can the service report about the given component? (chip, module...)
  virtual bool canReportAbout(InDetConditions::Hierarchy h) const override;
  
  ///Is the detector element good?
  virtual bool isGood(const Identifier& elementId, InDetConditions::Hierarchy h=InDetConditions::DEFAULT) const override;
  
  ///is it good?, using wafer hash
  virtual bool isGood(const IdentifierHash& hashId) const override;

 private:
  StringArrayProperty m_badElements; //list of bad detector elements (= module sides)
  SCT_ModuleVetoCondData m_localCondData;
  const SCT_ID* m_pHelper;
  bool m_useDatabase;
  bool m_maskLayers;
  int m_maskSide;
  std::vector<int> m_layersToMask; 
  std::vector<int> m_disksToMask; 

  // Mutex to protect the contents.
  mutable std::mutex m_mutex;
  // Cache to store events for slots
  mutable std::vector<EventContext::ContextEvt_t> m_cache;
  // Pointer of SCT_ModuleVetoCondData
  mutable Gaudi::Hive::ContextSpecificPtr<const SCT_ModuleVetoCondData> m_condData;
  // ReadCondHandleKey
  SG::ReadCondHandleKey<SCT_ModuleVetoCondData> m_condKey;
  // Fill data from m_badElements
  StatusCode fillData();
  // Provides SCT_ModuleVetoCondData pointer
  const SCT_ModuleVetoCondData* getCondData(const EventContext& ctx) const;

};

#endif // SCT_ModuleVetoTool_h

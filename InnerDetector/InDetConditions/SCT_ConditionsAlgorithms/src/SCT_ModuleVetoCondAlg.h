/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/ 

#ifndef SCT_MODULEVETOCONDALG
#define SCT_MODULEVETOCONDALG

#include "AthenaBaseComps/AthAlgorithm.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"

#include "StoreGate/WriteCondHandleKey.h"
#include "SCT_ConditionsData/SCT_ModuleVetoCondData.h"

#include "GaudiKernel/ICondSvc.h"
#include "GaudiKernel/Property.h"

class SCT_ModuleVetoCondAlg : public AthAlgorithm 
{  
 public:
  SCT_ModuleVetoCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~SCT_ModuleVetoCondAlg() = default;
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

 private:
  SG::ReadCondHandleKey<AthenaAttributeList> m_readKey{this, "ReadKey", "/SCT/Manual/BadModules", "Key of input (raw) bad module conditions folder"};
  SG::WriteCondHandleKey<SCT_ModuleVetoCondData> m_writeKey{this, "WriteKey", "SCT_ModuleVetoCondData", "Key of output (derived) bad module conditions data"};

  ServiceHandle<ICondSvc> m_condSvc;
};

#endif // SCT_MODULEVETOCONDALG

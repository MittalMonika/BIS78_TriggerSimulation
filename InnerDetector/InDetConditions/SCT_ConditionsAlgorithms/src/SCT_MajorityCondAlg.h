/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/ 

#ifndef SCT_MAJORITYCONDALG
#define SCT_MAJORITYCONDALG

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "SCT_ConditionsData/SCT_MajorityCondData.h"
#include "GaudiKernel/ICondSvc.h"

class SCT_MajorityCondAlg : public AthAlgorithm 
{  
 public:
  SCT_MajorityCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~SCT_MajorityCondAlg() = default;
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

 private:
  SG::ReadCondHandleKey<CondAttrListCollection> m_readKey;
  SG::WriteCondHandleKey<SCT_MajorityCondData> m_writeKey;
  ServiceHandle<ICondSvc> m_condSvc; 
};

#endif // SCT_MAJORITYCONDALG

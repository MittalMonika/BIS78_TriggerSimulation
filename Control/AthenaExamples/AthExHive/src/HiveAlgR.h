/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXHIVE_HIVEALGR_H
#define ATHEXHIVE_HIVEALGR_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "AthExHive/HiveDataObj.h"
#include "EventInfo/EventInfo.h"

class HiveAlgR  :  public AthReentrantAlgorithm {
  
public:
  
  // Standard Algorithm Constructor:
  
  HiveAlgR (const std::string& name, ISvcLocator* pSvcLocator);
  ~HiveAlgR ();
  
  // Define the initialize, execute and finalize methods:
  
  StatusCode initialize();
  StatusCode execute_r(const EventContext&) const;
  StatusCode finalize();
  
private:

  SG::ReadHandleKey<EventInfo> m_evt{this, "EvtInfo", "McEventInfo", "event info key"};

  SG::WriteHandleKey<HiveDataObj> m_wrh1 {this, "Key_W1", "ar1", "write handle key"};
  
};
#endif

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARROD_LARFEBERRORSUMMARYMAKER
#define LARROD_LARFEBERRORSUMMARYMAKER


/********************************************************************

NAME:     LArFebSummaryMaker
          Algorithm that makes LArFebSummary 

********************************************************************/

#include <AthenaBaseComps/AthAlgorithm.h>
#include <GaudiKernel/ServiceHandle.h>
#include <GaudiKernel/ToolHandle.h>

#include "LArRawEvent/LArFebHeaderContainer.h"
#include "LArRawEvent/LArFebErrorSummary.h"
#include "LArRecConditions/LArBadChannelCont.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <vector>
#include <set>

class LArOnlineID; 

class LArFebErrorSummaryMaker : public AthAlgorithm
{

 public:

  LArFebErrorSummaryMaker(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~LArFebErrorSummaryMaker();

  virtual StatusCode initialize() override final;
  virtual StatusCode execute() override final;
  virtual StatusCode finalize() override final;

 private:

  int m_missingFebsWarns; //counter for missing FEB warnings
  std::vector<int> m_errors;  //error types accumulator
  std::set<unsigned int> m_all_febs ; 
  bool m_isHec;
  bool m_isFcal;
  bool m_isEmb;
  bool m_isEmec;
  bool m_isEmPS;
  bool m_isAside;
  bool m_isCside;

  // properties:
  Gaudi::Property<int> m_warnLimit{ this, "warnLimit", 10, "Limit the number of warning messages for missing input" };
  Gaudi::Property<bool> m_checkAllFeb{ this, "CheckAllFEB", true, "Check all FEBS ?" };
  Gaudi::Property<std::string> m_partition{ this, "PartitionId", "", "Should contain DAQ partition (+ eventually the EventBuilder)" };
  Gaudi::Property<std::vector<unsigned int> > m_knownEvtId{ this, "MaskFebEvtId", {}, "ignore these FEBs for EvtId" };
  Gaudi::Property<std::vector<unsigned int> > m_knownSCACStatus{ this, "MaskFebScacStatus", {}, "ignore these FEBs for ScacStatus" };
  Gaudi::Property<std::vector<unsigned int> > m_knownZeroSample{ this, "MaskFebZeroSample", {}, "ignore these FEBs for ZeroSample" };

  const LArOnlineID* m_onlineHelper;

  SG::ReadCondHandleKey<LArBadFebCont> m_bfKey;
  SG::ReadHandleKey<LArFebHeaderContainer> m_readKey;  
  SG::WriteHandleKey<LArFebErrorSummary> m_writeKey;

  // methods:
  bool masked (unsigned int hid, const std::vector<unsigned int>& v_feb) const; 
};
#endif











/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGSTEERMONITOR_TRIGSIGNATUREMONIMT_H
#define TRIGSTEERMONITOR_TRIGSIGNATUREMONIMT_H 1

#include <string>
#include <mutex>
#include <TH2.h>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ITHistSvc.h"
#include "DecisionHandling/TrigCompositeUtils.h"
#include "xAODEventInfo/EventInfo.h"
#include "DecisionCollectorTool.h"

/**
 * @class Algorithm implementing monitoring of the HLT decision in the MT framework
 * @brief 
 **/


class TrigSignatureMoniMT : public ::AthAlgorithm
{ 
 public: 

  TrigSignatureMoniMT( const std::string& name, ISvcLocator* pSvcLocator );


  virtual StatusCode  initialize() override;
  virtual StatusCode  execute() override;
  virtual StatusCode  finalize() override;
 
 private: 
  TrigSignatureMoniMT();
  SG::ReadHandleKey<TrigCompositeUtils::DecisionContainer> m_l1DecisionsKey{ this, "L1Decisions", "HLTChainsResult", "Chains activated after the L1" };

    
  SG::ReadHandleKey<TrigCompositeUtils::DecisionContainer> m_finalDecisionKey{ this, "FinalDecisionKey", "HLTFinalDecisions", "Final stage of all decisions" };


  Gaudi::Property<std::vector<std::string> > m_allChains{ this, "ChainsList", {}, "List of all configured chains" };
 
  std::map<unsigned int, int> m_chainIDToBinMap;
  
  ServiceHandle<ITHistSvc> m_histSvc{ this, "THistSvc", "THistSvc/THistSvc", "Histogramming svc" };
  Gaudi::Property<std::string> m_bookingPath{ this, "HistParh", "/EXPERT/TrigSteer_HLT", "Booking path for the histogram"};

  TH2* m_outputHistogram;

  ToolHandleArray<DecisionCollectorTool> m_collectorTools{ this, "CollectorTools", {}, "Tools that collect decisions for steps" };
  
  int nBinsX() const { return m_allChains.size() +1; }
  int nBinsY() const;
  StatusCode initHist();
  StatusCode fillChains(const TrigCompositeUtils::DecisionIDContainer& dc, int row);
}; 


#endif //> !TRIGSTEERMONITOR_TRIGSIGNATUREMONIMT_H

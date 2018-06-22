/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETHYPOTOOLConfig_ETAET_H
#define TRIGJETHYPOTOOLConfig_ETAET_H
/********************************************************************
 *
 * NAME:     TrigJetHypoToolConfig_EtaEtTool.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 *
 *********************************************************************/


#include "ITrigJetHypoToolConfig.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/ConditionsDefs.h"
#include "DecisionHandling/HLTIdentifier.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "DecisionHandling/TrigCompositeUtils.h"
#include "AthenaMonitoring/GenericMonitoringTool.h"

#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/ConditionsDefs.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/ICleaner.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJetGrouper.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/CleanerBridge.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/ConditionsDefs.h"

class TrigJetHypoToolConfig_EtaEt:
public extends<AthAlgTool, ITrigJetHypoToolConfig> {

 public:
  
  TrigJetHypoToolConfig_EtaEt(const std::string& type,
                          const std::string& name,
                          const IInterface* parent);
  virtual ~TrigJetHypoToolConfig_EtaEt();

  virtual StatusCode initialize() override;
  virtual std::vector<std::shared_ptr<ICleaner>> getCleaners() const override;
  virtual std::unique_ptr<IJetGrouper> getJetGrouper() const override;
  virtual Conditions getConditions() const override;

 private:
  
  Gaudi::Property<std::vector<double>>
    m_EtThresholds{this, "EtThresholds", {}, "Etthresholds by eta region"};
  
  Gaudi::Property<std::vector<double>>
    m_etaMins{this, "eta_mins", {}, "Eta min for eta regions"};
  
  Gaudi::Property<std::vector<double>>
    m_etaMaxs{this, "eta_maxs", {}, "Eta max for eta regions"};

  Gaudi::Property<std::vector<int>>
    m_asymmetricEtas{this, "asymmetricEtas", {}, "Apply asym. eta cuts"};
  
  Gaudi::Property<std::string>
    m_matchingAlg{this, "matchingAlg", {}, "matcher alg"};
  



  StatusCode checkVals()  const;
 
 // Monitored variables...
 /*
  declareMonitoredVariable("NJet", m_njet);
  declareMonitoredVariable("Et", m_et);
  declareMonitoredVariable("Eta", m_eta);
  declareMonitoredVariable("Phi", m_phi);
*/


};
#endif

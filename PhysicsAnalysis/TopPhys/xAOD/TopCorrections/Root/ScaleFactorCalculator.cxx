/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TopCorrections/ScaleFactorCalculator.h"

#include <string>

#include "TopConfiguration/TopConfig.h"
#include "TopEvent/EventTools.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthEventContainer.h"

namespace top {

ScaleFactorCalculator::ScaleFactorCalculator(const std::string& name) :
  asg::AsgTool(name),
  m_config(nullptr),

  m_photonSF(nullptr),
  m_electronSF(nullptr),
  m_muonSF(nullptr),
  m_tauSF(nullptr),
  m_jetSF(nullptr),
  m_btagSF(nullptr),
  m_pileupSF(nullptr),
  m_sherpa_22_reweight_tool("PMGSherpa22VJetsWeightTool"),
  m_globalLeptonTriggerSF(nullptr),
  m_pmg_truth_weight_tool("PMGTruthWeightTool"){
  declareProperty("config", m_config);
}

StatusCode ScaleFactorCalculator::initialize() {
  ATH_MSG_INFO(" top::ScaleFactorCalculator initialize");

  m_photonSF = std::make_unique<top::PhotonScaleFactorCalculator>("top::PhotonScaleFactorCalculator");
  m_electronSF = std::make_unique<top::ElectronScaleFactorCalculator>("top::ElectronScaleFactorCalculator");
  m_muonSF = std::make_unique<top::MuonScaleFactorCalculator>("top::MuonScaleFactorCalculator");
  m_tauSF = std::make_unique<top::TauScaleFactorCalculator>("top::TauScaleFactorCalculator");
  m_jetSF = std::make_unique<top::JetScaleFactorCalculator>("top::JetScaleFactorCalculator");
  m_btagSF = std::make_unique<top::BTagScaleFactorCalculator>("top::BTagScaleFactorCalculator");
  m_pileupSF = std::make_unique<top::PileupScaleFactorCalculator>("top::PileupScaleFactorCalculator");
  m_globalLeptonTriggerSF = std::make_unique<top::GlobalLeptonTriggerCalculator>("top::GlobalLeptonTriggerCalculator");

  if (m_config->isMC()) {
    if (m_config->usePhotons()) {
      top::check(m_photonSF->setProperty("config", m_config), "Failed to setProperty");
      top::check(m_photonSF->initialize(), "Failed to initialize");
    }

    if (m_config->useElectrons()) {
      top::check(m_electronSF->setProperty("config", m_config), "Failed to setProperty");
      // m_electronSF->msg().setLevel(MSG::DEBUG);
      top::check(m_electronSF->initialize(), "Failed to initialize");
    }

    if (m_config->useMuons() && !m_config->isTruthDxAOD()) {
      top::check(m_muonSF->setProperty("config", m_config), "Failed to setProperty");
      // m_muonSF->msg().setLevel(MSG::DEBUG);
      top::check(m_muonSF->initialize(), "Failed to initialize");
    }

    if (m_config->useTaus()) {
      top::check(m_tauSF->setProperty("config", m_config), "Failed to setProperty");
      top::check(m_tauSF->initialize(), "Failed to initialize");
    }

    if (m_config->useJets()) {
      top::check(m_jetSF->setProperty("config", m_config), "Failed to setProperty");
      top::check(m_jetSF->initialize(), "Failed to initialize");

      top::check(m_btagSF->setProperty("config", m_config), "Failed to setProperty");
      top::check(m_btagSF->initialize(), "Failed to initialize");
    }

    if (m_config->isSherpa22Vjets())
      top::check(m_sherpa_22_reweight_tool.retrieve(),
                 "Failed to retrieve PMGSherpa22VJetsWeightTool");
    
    if ( (m_config->useElectrons() || m_config->useMuons()) && m_config->useGlobalTrigger() ){
      top::check(m_globalLeptonTriggerSF->setProperty("config", m_config), "Failed to setProperty");
      // m_globalLeptonTriggerSF->msg().setLevel(MSG::DEBUG); 
      top::check(m_globalLeptonTriggerSF->initialize(), "Failed to initalize");
    }
    
    top::check(m_pmg_truth_weight_tool.retrieve(), "Failed to retrieve PMGTruthWeightTool");

  }

  if (m_config->doPileupReweighting()) {
    top::check(m_pileupSF->setProperty("config", m_config), "Failed to add config to pileup SF calculator");
    top::check(m_pileupSF->initialize(), "Failed to initialize pileup SF calculator");
  }

  return StatusCode::SUCCESS;
}

StatusCode ScaleFactorCalculator::execute() {
  if (m_config->isMC()) {
    if (m_config->usePhotons())
      top::check(m_photonSF->execute(), "Failed to execute photon SF");
    if (m_config->useElectrons())
      top::check(m_electronSF->execute(), "Failed to execute electron SF");
    if (m_config->useMuons() && !m_config->isTruthDxAOD())
      top::check(m_muonSF->execute(), "Failed to execute muon SF");
    if (m_config->useTaus())
      top::check(m_tauSF->execute(), "Failed to execute tau SF");
    if (m_config->useJets())
      top::check(m_jetSF->execute(), "Failed to execute jet SF");
    if (m_config->useJets()) {
      top::check(m_btagSF->execute(), "Failed to execute btag SF");
    }
    // Add Sherpa 22 weights directly here, if we get more
    // PMG tools for reweighting then we should consider making
    // a m_PMG_SF class, as with other corrections
    if (m_config->isSherpa22Vjets()) {
      const xAOD::EventInfo* eventInfo(nullptr);
      top::check(evtStore()->retrieve(eventInfo, m_config->sgKeyEventInfo()),
                 "Failed to retrieve EventInfo");
      double sherpa_weight = m_sherpa_22_reweight_tool->getWeight();
      eventInfo->auxdecor<double>("Sherpa22VJetsWeight") = sherpa_weight;
    }
    if ( (m_config->useElectrons() || m_config->useMuons()) && m_config->useGlobalTrigger() ){
      top::check(m_globalLeptonTriggerSF->execute(), "Failed to exectute global trigger SF");
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode ScaleFactorCalculator::executePileup() {
  if (m_config->doPileupReweighting())
    top::check(m_pileupSF->execute(), "Failed to execute pileup SF");
  return StatusCode::SUCCESS;
}

float ScaleFactorCalculator::pileupWeight() const {
  float sf(1.);
  if (!m_config->isMC()) {
    return sf;
  }

  const xAOD::EventInfo* eventInfo(nullptr);
  top::check(evtStore()->retrieve(eventInfo, m_config->sgKeyEventInfo()),
             "Failed to retrieve EventInfo");

  if (eventInfo->isAvailable<float>("PileupWeight"))
    sf = eventInfo->auxdataConst<float>("PileupWeight");

  return sf;
}

float ScaleFactorCalculator::mcEventWeight() const {
  float sf(1.);
  if (!m_config->isMC()) {
    return sf;
  } 

  ///-- Start using the PMG tool to get the nominal event weights --///
  ///-- But nominal weight name seems to vary so we try to test   --///
  const std::vector<std::string> nominal_weight_names = {" nominal ", "nominal"};

  for(auto weight_name : nominal_weight_names){
    ///-- Check whether this weight name does exist --///
    if(m_pmg_truth_weight_tool->hasWeight(weight_name)){
      sf = m_pmg_truth_weight_tool->getWeight( weight_name );
      ///-- Return from here if we find it --///
      return sf;
    } 
  }
  ///-- If we reach here, no name was found, so use the old method --///
  ///-- If not, we can default to retrieving the nominal weight assuming it is in the 0th position --///
  const xAOD::EventInfo* eventInfo(nullptr);
  top::check(evtStore()->retrieve(eventInfo, m_config->sgKeyEventInfo()), "Failed to retrieve EventInfo");
  const xAOD::TruthEventContainer* truthEventContainer(nullptr);
  top::check( evtStore()->retrieve(truthEventContainer, m_config->sgKeyTruthEvent()) , "Failed to retrieve truth PDF info" );
    
  // Old method which was buggy due to issues in metadata
  // sf = eventInfo->mcEventWeight();
  
  // Temporary bug fix due to the above problem
  sf = truthEventContainer->at(0)->weights()[0];

  return sf;
}
}  // namespace top

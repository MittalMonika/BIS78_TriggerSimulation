/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

///Local include(s)
#include "TrigMultiVarHypo/TrigL2CaloRingerFex.h"
#include "TrigMultiVarHypo/preproc/TrigRingerPreprocessor.h"
#include "TrigMultiVarHypo/tools/MultiLayerPerceptron.h"

///xAOD include(s)
#include "xAODTrigCalo/TrigEMCluster.h"
#include "xAODTrigRinger/TrigRNNOutput.h"
#include "xAODTrigRinger/TrigRNNOutputContainer.h"
#include "xAODTrigRinger/TrigRingerRings.h"
#include "xAODTrigRinger/TrigRingerRingsContainer.h"

///std include(s)
#include <new>
#include <cmath>

#define SIZEOF_NODES            3 
#define SIZEOF_RINGSETS         7

#include <iostream>
using namespace std;

//!===============================================================================================
TrigL2CaloRingerFex::TrigL2CaloRingerFex(const std::string& name, ISvcLocator* pSvcLocator):
  HLT::FexAlgo(name, pSvcLocator), m_lumiBlockMuTool("LumiBlockMuTool/LumiBlockMuTool"),
  m_reader("TrigL2CaloRingerReader")
{  
  declareProperty("HltFeature"        , m_hlt_feature = "TrigRingerNeuralFex" );  
  declareProperty("Feature"           , m_feature = "TrigT2CaloEgamma"        );  
  declareProperty("NormalisationRings", m_normRings                           );
  declareProperty("SectionRings"      , m_sectionRings                        );
  declareProperty("NRings"            , m_nRings                              );
  declareProperty("CalibPath"         , m_calibPath = ""                      );
  declareProperty("LuminosityTool"    , m_lumiBlockMuTool, "Luminosity Tool"  );
  declareMonitoredVariable("NeuralNetworkOutput", m_output                    );
  
  m_reader.setMsgStream(msg());
  m_useLumiVar=false;
  m_useEtaVar=false;
  m_key       = "";
}

//!===============================================================================================
HLT::ErrorCode TrigL2CaloRingerFex::hltInitialize() 
{
  // Retrieve the NeuralNetwork list
  if(!m_calibPath.empty()){
    if(!m_reader.retrieve(m_calibPath, m_discriminators)){
      msg() << MSG::ERROR << "Can not retrieve all information from " << m_calibPath <<  endmsg;
      return HLT::BAD_JOB_SETUP;
    }
    m_useLumiVar = m_reader.useLumiVar();
    m_useEtaVar  = m_reader.useEtaVar();
  }

  if(m_nRings.size() != m_normRings.size()){
    msg() << MSG::ERROR << "Preproc nRings list dont match with the number of discriminators found" << endmsg;
    return HLT::BAD_JOB_SETUP;
  }

  if(m_sectionRings.size() != m_normRings.size()){
    msg() << MSG::ERROR << "Preproc section rings list dont match with the number of discriminators found" << endmsg;
    return HLT::BAD_JOB_SETUP;
  }

  ///TODO: This is default for now, apply this into the conf file for future
  ///Initialize all discriminators
  for(unsigned i=0; i<m_discriminators.size(); ++i)
  {
    TrigRingerPreprocessor *preproc;
    try{
      preproc = new TrigRingerPreprocessor(m_nRings,m_normRings,m_sectionRings);
    }catch(const std::bad_alloc& xa){
      msg() << MSG::ERROR << "Bad alloc for TrigRingerPrepoc." << endmsg;
      return HLT::BAD_JOB_SETUP;
    }
    ///Hold the pointer configuration
    m_preproc.push_back(preproc);
  }///Loop over discriminators
  

  ///Monitoring hitograms
  if(doTiming()){
    m_normTimer     = addTimer("Normalization");
    m_decisionTimer = addTimer("Decision");
    m_storeTimer    = addTimer("StoreOutput");
  }///Only if time is set on python config

  m_useLumiTool=false;
  if (m_lumiBlockMuTool.retrieve().isFailure()) {
    msg() << MSG::WARNING << "Unable to retrieve Luminosity Tool" << endmsg;
  } else {
    m_useLumiTool=true; // Tell to the execute that the LumiTool is available...
    msg() << MSG::INFO << "Successfully retrieved Luminosity Tool" << endmsg;
  }
 
  msg() << MSG::INFO << "Using the Luminosity tool? " <<  (m_useLumiTool ? "Yes":"No")  << endmsg;
  msg() << MSG::INFO << "Using lumiVar?             " <<  (m_useLumiVar ? "Yes":"No")  << endmsg;
  msg() << MSG::INFO << "Using etaVar?              " <<  (m_useEtaVar ? "Yes":"No")  << endmsg;
  
  msg() << MSG::INFO <<  "TrigL2CaloRingerHypo initialization completed successfully." << endmsg;
  msg() << MSG::INFO << "TrigL2CaloRingerHypo initialization completed successfully." << endmsg;

  return HLT::OK;
}
//!===============================================================================================
HLT::ErrorCode TrigL2CaloRingerFex::hltFinalize() {  
  ///release memory
  for(unsigned i=0; i<m_discriminators.size();++i){
    if(m_preproc.at(i))         delete m_preproc.at(i);
    if(m_discriminators.at(i))  delete m_discriminators.at(i);
  }///Loop over all discriminators and prepoc objects
  msg() << MSG::INFO <<  "TrigL2CaloRingerHypo finalization completed successfully." << endmsg;

  return HLT::OK;
}
//!===============================================================================================
HLT::ErrorCode TrigL2CaloRingerFex::hltExecute(const HLT::TriggerElement* /*inputTE*/, HLT::TriggerElement* outputTE){

  // For now, this must be [avgmu, rnnOutputWithTansig, rnnOutputWithoutTansig] 
  m_output=-999;
  std::vector<float> output;

  ///Retrieve rings pattern information
  const xAOD::TrigRingerRings *ringerShape = get_rings(outputTE);
  if(!ringerShape){
    msg() << MSG::WARNING <<  "Can not retrieve xADO::TrigRingerRings from storegate."  << endmsg;
    return HLT::OK;
  }///protection

  ///Retrieve cluster information
  const xAOD::TrigEMCluster *emCluster = ringerShape->emCluster();

  ///Check if emCluster link exist
  if(!emCluster){
    msg() << MSG::WARNING <<  "Can not found the xAOD::TrigEMCluster link"  << endmsg;
    return HLT::OK;
  }///protection

  msg() << MSG::DEBUG <<  "Event with roiword: 0x" << std::hex << emCluster->RoIword() << std::dec  << endmsg; 


  ///It's ready to select the correct eta/et bin
  MultiLayerPerceptron    *discr  = nullptr;
  TrigRingerPreprocessor  *preproc = nullptr;

  float eta = std::fabs(emCluster->eta());
  float et  = emCluster->et()*1e-3; ///in GeV 
  float avgmu, mu = 0.0;

  if(m_useLumiTool){
    if(m_lumiBlockMuTool){
      mu = m_lumiBlockMuTool->actualInteractionsPerCrossing(); // (retrieve mu for the current BCID)
      avgmu = m_lumiBlockMuTool->averageInteractionsPerCrossing();
      msg() << MSG::DEBUG << "Retrieved Mu Value : " << mu<< endmsg;
      msg() << MSG::DEBUG << "Average Mu Value   : " << avgmu<< endmsg;   
    }
  }

  // Fix eta range
  if(eta>2.50) eta=2.50;///fix for events out of the ranger
  // Add avgmu!
  output.push_back(avgmu);

  if(doTiming())  m_decisionTimer->start();
  
  if(m_discriminators.size() > 0){
      
    for(unsigned i=0; i<m_discriminators.size(); ++i){
      if(avgmu > m_discriminators[i]->mumin() && avgmu <= m_discriminators[i]->mumax()){
        if(et > m_discriminators[i]->etmin() && et <= m_discriminators[i]->etmax()){
          if(eta > m_discriminators[i]->etamin() && eta <= m_discriminators[i]->etamax()){
            discr   = m_discriminators[i];
            preproc = m_preproc[i];
            break;
          }// mu conditions
        }///eta conditions
      }///Et conditions
    }///Loop over discriminators

    msg() << MSG::DEBUG <<  "Et = " << et << " GeV, |eta| = " << eta << endmsg;
    
    ///Apply the discriminator
    if(discr){

      const std::vector<float> rings = ringerShape->rings();
      std::vector<float> refRings(rings.size());
      refRings.assign(rings.begin(), rings.end());

      ///pre-processing ringer shape (default is Norm1...)
      if(doTiming())  m_normTimer->start();
      if(preproc)     preproc->ppExecute(refRings);
      if(doTiming())  m_normTimer->stop();


      float eta_norm=0.0;
      float avgmu_norm=0.0;

      // Add extra variables in this order! Do not change this!!!
      if(m_useEtaVar){
        if(preproc){
          eta_norm = preproc->normalize_eta(emCluster->eta(), discr->etamin(), discr->etamax());
          refRings.push_back(eta_norm);
        }
      }

      if(m_useLumiVar){
        if(preproc){
          avgmu_norm = preproc->normalize_mu(avgmu, m_lumiCut);
          refRings.push_back(avgmu_norm);
        }
      }

      m_output=discr->propagate(refRings);
      output.push_back(m_output);
      output.push_back(discr->getOutputBeforeTheActivationFunction());
    }// has discr?
  }else{
    msg() << MSG::DEBUG <<  "There is no discriminator into this Fex." << endmsg;
  }

  if(doTiming())  m_decisionTimer->stop();

  msg() << MSG::DEBUG <<  "Et = " << et << " GeV, |eta| = " << eta << " and rnnoutput = " << m_output << endmsg;

  if(doTiming())  m_storeTimer->start();
  ///Store outout information for monitoring and studys
  xAOD::TrigRNNOutput *rnnOutput = new xAOD::TrigRNNOutput();
  rnnOutput->makePrivateStore(); 
  rnnOutput->setRnnDecision(output);

  ///Get the ringer link to store into TrigRNNOuput  
  HLT::ErrorCode hltStatus;
  ElementLink<xAOD::TrigRingerRingsContainer> ringer_link;
  hltStatus = getFeatureLink<xAOD::TrigRingerRingsContainer,xAOD::TrigRingerRings>(outputTE, ringer_link);

  if( (hltStatus != HLT::OK) || (!ringer_link.isValid())){
    msg() << MSG::WARNING <<  "Failed to access ElementLink to TrigRingerRings"  << endmsg;
  }else{
    rnnOutput->setRingerLink( ringer_link );
  }
  hltStatus = recordAndAttachFeature<xAOD::TrigRNNOutput>(outputTE, rnnOutput, m_key, m_hlt_feature);
  if(doTiming())  m_storeTimer->stop();

  if (hltStatus != HLT::OK) {
    msg() << MSG::WARNING <<  "Failed to record xAOD::TrigRNNOutput to StoreGate."  << endmsg;
    return HLT::OK;
  }

  return HLT::OK;
}
//!===============================================================================================

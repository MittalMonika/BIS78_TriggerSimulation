/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArGainThresholds2Ntuple.h"
#include "LArIdentifier/LArOnlineID.h"

LArGainThresholds2Ntuple::LArGainThresholds2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator), m_febConfigReader("LArFEBConfigReader") {

  declareProperty("FebConfigReader",m_febConfigReader);
  m_ntTitle="Gain Thresholds";
  m_ntpath="/NTUPLES/FILE1/GAINTH";
}


StatusCode LArGainThresholds2Ntuple::initialize() {

  if (m_febConfigReader.retrieve().isFailure()) {
    msg(MSG::ERROR) << "Failed to retrieve tool " << m_febConfigReader << endreq;
    return StatusCode::FAILURE;
  }

  return LArCond2NtupleBase::initialize();
}

LArGainThresholds2Ntuple::~LArGainThresholds2Ntuple() 
{}

StatusCode LArGainThresholds2Ntuple::stop() {
  StatusCode sc;
  NTuple::Item<long> lower;
  NTuple::Item<long> upper;
 
   sc=m_nt->addItem("lower",lower,-1000,5000);
   if (sc!=StatusCode::SUCCESS) {
     msg(MSG::ERROR) << "addItem 'lower' failed" << endreq;
     return StatusCode::FAILURE;
   }
   
   sc=m_nt->addItem("upper",upper,-1000.,5000.);
   if (sc!=StatusCode::SUCCESS) {
     msg(MSG::ERROR) << "addItem 'upper' failed" << endreq;
     return StatusCode::FAILURE;
   }
   
   std::vector<HWIdentifier>::const_iterator itOnId = m_onlineId->channel_begin();
   std::vector<HWIdentifier>::const_iterator itOnIdEnd = m_onlineId->channel_end();
   for(; itOnId!=itOnIdEnd;++itOnId){
     const HWIdentifier hwid = *itOnId;
     lower=m_febConfigReader->lowerGainThreshold(hwid);
     upper=m_febConfigReader->upperGainThreshold(hwid);
     
     fillFromIdentifier(hwid);
     
     sc=ntupleSvc()->writeRecord(m_nt);      
     if (sc!=StatusCode::SUCCESS) {
       (*m_log) << MSG::ERROR << "writeRecord failed" << endreq;
       return StatusCode::FAILURE;
     }
   }
 
   msg(MSG::INFO) << "LArGainThresholds2Ntuple has finished." << endreq;
   return StatusCode::SUCCESS;
   
}// end finalize-method.
   

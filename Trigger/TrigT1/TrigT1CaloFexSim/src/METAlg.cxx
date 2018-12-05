/*
 *   Copyright (C) 2002-2018 or the benefit of the ATLAS collaboration
*/
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "AthAnalysisBaseComps/AthAnalysisAlgorithm.h"
#include "TrigT1CaloFexSim/METAlg.h"
#include "TrigT1CaloFexSim/JGTower.h"

//MET algorithms
#include "TrigT1CaloFexSim/Rho.h"
#include "TrigT1CaloFexSim/Softkiller.h"
#include "TrigT1CaloFexSim/JwoJ.h"
#include "TrigT1CaloFexSim/Pufit.h"

//------------------------------------------------------------------------------------------------
StatusCode METAlg::BuildMET(const xAOD::JGTowerContainer*towers, METAlg::MET* met, std::vector<float> noise, bool useNegTowers){
  float met_x=0;
  float met_y=0;
  
  for(unsigned t=0; t<towers->size(); t++){
    const xAOD::JGTower* tower = towers->at(t);
    if(tower->et()<noise.at(t)) continue;
    float phi=tower->phi();
    float et =tower->et();
    if(!useNegTowers) et = TMath::Abs(et);
    met_x -= et*cos(phi);
    met_y -= et*sin(phi);
  }
  
  float et_met = sqrt(met_x*met_x+met_y*met_y);
  float phi_met=TMath::ACos(met_x/et_met);
  if (met_y<0) phi_met = -phi_met;
  met->phi=phi_met;
  met->et = et_met;
  
  return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------------------------
StatusCode METAlg::SubtractRho_MET(const xAOD::JGTowerContainer* towers, METAlg::MET* met, bool useRMS, bool useMedian, bool useNegTowers){
  
  float EtMiss = 0;
  float Ex = 0, Ey = 0, Ex_ = 0, Ey_ = 0;
  float threshold  = 0;
  
  //can calculate rho as either the average or median gTower energy in the barrel
  float rho = Rho_bar(towers,0);
  if(useMedian) rho = Rho_med(towers, useNegTowers);
  
  unsigned int size = towers->size();

  TH1F* h_Et = new TH1F("h_Et", "", 50, 0, 5000);
  if(useRMS){
    for(int t = 0; t < size; t++){
      const xAOD::JGTower* tower = towers->at(t);
      float Et = tower->et();
      if(!useNegTowers) Et = TMath::Abs(Et);
      h_Et->Fill(Et);
    }
    threshold = h_Et->GetRMS();
  }
  for(int t = 0; t < size; t++){
    const xAOD::JGTower* tower = towers->at(t);
    float Et = tower->et();
    float phi = tower->phi();
    float eta = TMath::Abs(tower->eta());
    
    float Et_sub = 0;
    if(eta < 2.4) Et_sub = TMath::Abs(Et) - rho;
    else Et_sub = TMath::Abs(Et) - 4*rho;
    
    if(Et_sub < threshold) continue;
    
    //if tower originally had negative Et, keep Et_sub negative
    if(useNegTowers && Et < 0) Et_sub = -Et_sub;
    
    Ex_ = Et_sub*TMath::Cos(phi);
    Ey_ = Et_sub*TMath::Sin(phi);
    
    Ex += Ex_;
    Ey += Ey_;
  }
  
  EtMiss = TMath::Sqrt(Ex*Ex + Ey*Ey);
  float phi_met=TMath::ACos(Ex/EtMiss);
  met->et = EtMiss;
  met->phi = phi_met;
  //ATH_MSG_INFO("Dumping event rho: " << rho ); 
  //std::cout << "Calculating MET for rho =  " << rho << std::endl;
  //std::cout << "MET = " << EtMiss << std::endl;
  
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------------------------
StatusCode METAlg::Softkiller_MET(const xAOD::JGTowerContainer* towers, METAlg::MET* met, bool useNegTowers){
  float median = 0;
  unsigned int size = towers->size();
  
  TH2F* grid = ConstructGrid(towers, useNegTowers);
  
  //compress grid into 1D vector of towers
  std::vector<float> EtMaxPerPatch = FlattenHistogram(grid);
  
  //find the true median of the tower energies
  median = Et_median_true(&EtMaxPerPatch);
  
  float rho = Rho_bar(towers, 0);
  float Ex = 0, Ey = 0, Ex_ = 0, Ey_ = 0;
  
  for(unsigned int t = 0; t < size; t++){
    const xAOD::JGTower* tower = towers->at(t);
    float eta = TMath::Abs(tower->eta());
    float phi = tower->phi();
    float Et = tower->et();
    
    float Et_sub = 0;
    
    if(eta < 2.4){
      Et_sub = TMath::Abs(Et) - rho;
      if(Et_sub < median) continue;
    }else{
      Et_sub = TMath::Abs(Et) - 4*rho;
      if(Et_sub < 4*median) continue;
    }
    
    //if the tower originally had negative Et, keep Et_sub negative
    if(useNegTowers && Et < 0) Et_sub = -Et_sub;
    Ex_ = Et_sub*TMath::Cos(phi);
    Ey_ = Et_sub*TMath::Sin(phi);
    
    Ex += Ex_;
    Ey += Ey_;
  }
  
  float EtMiss = TMath::Sqrt(Ex*Ex + Ey*Ey);
  float phi_met = TMath::ACos(Ex/EtMiss);
  
  met->et = EtMiss;
  met->phi = phi_met;
  
  //std::cout<<"Calculating MET with pileup subtracted SoftKiller: "<<std::endl;
  //std::cout<<"MET = "<<EtMiss<<std::endl;
  return StatusCode::SUCCESS;
}

StatusCode METAlg::JwoJ_MET(const xAOD::JGTowerContainer* towers, METAlg::MET* met, float pTcone_cut, bool useNegTowers){
  
  unsigned int size = towers->size();
  
  std::vector<float> Et_values = Run_JwoJ(towers, pTcone_cut,  useNegTowers);
  
  //set fit parameters for calculating MET
  //Look up table for parameters a,b,c depending on scalar sumEt
  //Optimized for resolution in external framework
  float a;
  float b;
  float c;
  
  if(Et_values[0] <= 500){
    a = 0.6;
    b = 0.6;
    c = 20.;
  }
  else if(Et_values[0] <= 700){
    a = 0.55;
    b = 0.7;
    c = 15.;
  }
  else if(Et_values[0] <= 900){
    a = 0.65;
    b = 0.6;
    c = 13.;
  }
  else if(Et_values[0] <= 1100){
    a = 0.75;
    b = 0.55;
    c = 10.;
  }
  else if(Et_values[0] <= 1300){
    a = 0.75;
    b = 0.45;
    c = 5.;
  }
  else if(Et_values[0] <= 1500){
    a = 0.8;
    b = 0.35;
    c = 0.;
  }
  else if(Et_values[0] <= 1700){
    a = 0.75;
    b = 0.4;
    c = -5.;
  }
  else{
    a = 0.7;
    b = 0.45;
    c = -10.;
  }
  
  float EtMiss = a*Et_values[1] + b*Et_values[2] + c;
  met->et = EtMiss*Gaudi::Units::GeV;
  
  //std::cout<<"Calculating MET with optimized Jets Without Jets: "<<std::endl;
  //std::cout<<"MET = "<<EtMiss<<std::endl;
  return StatusCode::SUCCESS;
}

StatusCode METAlg::Pufit_MET(const xAOD::JGTowerContainer*towers, METAlg::MET* met, bool useNegTowers){
  float EtMiss = Run_PUfit(towers, 3, useNegTowers);
  met->et = EtMiss;
  //std::cout<<"Calculating MET with PUfit:"<<std::endl;
  //std::cout<<"MET = "<<EtMiss<<std::endl;
  return StatusCode::SUCCESS;
}



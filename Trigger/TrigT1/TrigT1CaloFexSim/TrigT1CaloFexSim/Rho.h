/*

 *   Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

Rho.h: Calculates energy density rho to subtract as pileup
Author: Ava Myers (amyers@cern.ch)
Methods:
Rho_bar(): Calculates rho as the average tower energy density in the barrel region of the calorimeter, to be scaled with area in more forward regions. An upper threshold of 3 GeV is applied to eliminate bias from hard scatter events

Rho_med(): Calculates rho as the median tower energy density in the barrel, again to be scaled with area

 */

#ifndef TRIGT1CALOFEXSIM_RHO_H
#define TRIGT1CALOFEXSIM_RHO_H

/*
 * Class   : Rho
 *
 * Author  : Myers, Ava (amyers@cern.ch)
 *
 *Calculates energy density rho either as a median or an average to subract as pileup
 */

#include "CaloIdentifier/GTower_ID.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODTrigL1Calo/JGTower.h"
#include "xAODTrigL1Calo/JGTowerContainer.h"
#include "xAODTrigL1Calo/JGTowerAuxContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetInterface/IJetUpdateJvt.h"
#include "TrigAnalysisInterfaces/IBunchCrossingTool.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "Identifier/IdentifierHash.h"
#include <algorithm>
#include <vector>
#include "TMath.h"

/**
 *@brief Calculates rho as the average tower energy density in the barrel region of the calorimeter, to be scaled with are in more forward regions. An upper threshold of 10 GeV is applied to eliminate bias from hard scatter events
 *@return @c float
 */
float Rho_bar(const xAOD::JGTowerContainer* towers, const bool useEtaBins, int fpga, const bool useNegTowers){
  //fpga = 0 ->entire barrel
  //fpga = 1 -> FPGA_A
  //fpga = 2 -> FPGA_B
  //fpga = 3 -> FPGA_C

  float rho = 0;
  float et_max = 10*Gaudi::Units::GeV; //an upper threshold such that the average rho is not biased by hard scatter events                                                                                                                     
  const unsigned int size = towers->size();
  int length = 0;
  float area_a = 0;
  float area_b = 0;
  float area_c = 0;

 for(unsigned int i = 0; i < size; i++){
   const xAOD::JGTower* tower = towers->at(i);

   float eta = TMath::Abs(tower->eta());
   float Et = tower->et();

   if(!useNegTowers && Et < 0) continue;

   if(eta < 0 && eta > -2.5){ //FPGA A bounds
     if(eta > -2.4) area_a += 1;
     else area_a += 0.5;
   }
   if(eta < 2.5 && eta > 0){
     if(eta < 2.4) area_b += 1.;
     else area_b += 0.5;
   }
   if(fabs(eta) > 2.5){
     if(eta < 3.2) area_c += 1.;
     else area_c += 4.;
   }

   if(useEtaBins){
     if(Et < et_max){
       rho += Et;
       //length++;
     }
   }else{
     if(eta < 2.4){
       if(Et < et_max){
	 rho += Et;
	 length++;
       }
     }
   }
 }
 float rho_bar = rho/length;
 if(fpga == 1) rho_bar = rho/area_a;
 if(fpga == 2) rho_bar = rho/area_b;
 if(fpga ==3) rho_bar = rho/area_c;

 return rho_bar;
}

/**
 *@brief Calculates rho as the median tower energy density in the barrel region of the calorimeter, to be scaled with area in more forward regions
 *@return @c float
 */
float Rho_med(const xAOD::JGTowerContainer* towers, const bool useNegTowers){
  float rho = 0;
  const unsigned int size = towers->size();
  std::vector<float> Et_in;

  for(unsigned int i = 0; i < size; i++){
    const xAOD::JGTower* tower = towers->at(i);
    float Et = tower->et();
    float eta = TMath::Abs(tower->eta());

    if(!useNegTowers && Et < 0) continue;

    if(eta < 2.4) Et_in.push_back(Et);
  }
  std::sort(Et_in.begin(), Et_in.end());
 
  if(Et_in.size() == 0) return 0;
  if(Et_in.size() == 1) rho = Et_in[0];
  else if(Et_in.size() % 2 == 0) rho = (Et_in[Et_in.size()/2] + Et_in[Et_in.size()/2 -1])/2;
  else rho = Et_in[Et_in.size()/2];

  return rho;
}

#endif

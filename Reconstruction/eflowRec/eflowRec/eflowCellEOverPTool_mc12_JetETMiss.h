/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWCELLEOVERPTOOL_MC12_JETETMISS_H
#define EFLOWCELLEOVERPTOOL_MC12_JETETMISS_H

/********************************************************************

NAME:     eflowCellEOverPTool_mc12_JetETMiss.h
PACKAGE:  offline/Reconstruction/eflowRec

AUTHORS:  M.Hodgkinson
CREATED:  March 2014

Description: New EM e/p for DC14 - JetMET ONLY - this is because taus already tuned PanTau on eflowRec, and hence prefer no updates for DC14

********************************************************************/

#include "eflowRec/IEFlowCellEOverPTool.h"

class eflowBaseParameters;

class eflowCellEOverPTool_mc12_JetETMiss : public IEFlowCellEOverPTool {

 public:

  eflowCellEOverPTool_mc12_JetETMiss(const std::string& type,const std::string& name,const IInterface* parent);
  
  ~eflowCellEOverPTool_mc12_JetETMiss() {};

  StatusCode initialize();
  StatusCode execute(eflowEEtaBinnedParameters *binnedParameters) ;
  StatusCode finalize() ;

 private:

  std::vector<double>  m_eBinValues;
  std::vector<double> m_etaBinBounds;
  std::vector<std::string> m_eBinLabels;
  std::vector<std::string> m_etaBinLabels;

};
#endif

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWCELLEOVERPTOOL_MC12_HLLHC_H
#define EFLOWCELLEOVERPTOOL_MC12_HLLHC_H

#include "eflowRec/IEFlowCellEOverPTool.h"

class eflowBaseParameters;

/**
Class to store reference e/p mean and widths, as well as reference energy density radial profile fit parameters. The data is input to an eflowEEtaBinnedParameters object in the execute method. Stores data at the uncalibrated (EM) scale and is for HLLHC setup. This inherits from IEFlowCellEOverPTool. The values are derived using single pions with the AMI tags e3895_s3020_r9063.
*/
class eflowCellEOverPTool_mc12_HLLHC : public IEFlowCellEOverPTool {

 public:

  eflowCellEOverPTool_mc12_HLLHC(const std::string& type,const std::string& name,const IInterface* parent);
  
  ~eflowCellEOverPTool_mc12_HLLHC() {};

  StatusCode initialize();
  StatusCode execute(eflowEEtaBinnedParameters *binnedParameters) ;
  StatusCode finalize() ;

 private:

  std::vector<std::vector<std::vector<std::vector<std::vector<double> > > > > m_theEnergyEtaFirstIntLayerShapeParams;
  std::vector<std::vector<double> > m_theLayerShapeParams;
  std::vector<std::vector<std::vector<double> > > m_theEnergyEtaRingThicknesses;
  std::vector<std::vector<double> > m_theRingThicknesses;
  std::vector<double> m_theEOverPMeans;
  std::vector<double> m_theEOverPStdDevs;
  std::vector<std::vector<std::vector<double> > >  m_theEnergyEtaFirstIntLayerEOverPMeans;
  std::vector<std::vector<std::vector<double> > >  m_theEnergyEtaFirstIntLayerEOverPStandardDeviations;
  std::vector<std::vector<double> > m_test2;

  //const int m_nEBins;
  //const int m_nEtaBins;
  //const int m_nFirstIntLayerBins;
  //const int m_nCaloRegionBins;

  enum E_BINS        { E001bin = 0, E003point5bin, E010bin, E020bin, E032point5bin, E040bin };
    
  enum ETA_BINS      { eta050bin = 0, eta100bin, eta150bin, eta250bin, eta350bin, eta450bin};

  enum SHAPE_PARAMS  { NORM1 = 0, WIDTH1, NORM2, WIDTH2 };

  std::vector<double>  m_eBinValues;
  std::vector<double> m_etaBinBounds;
  std::vector<std::string> m_eBinLabels;
  std::vector<std::string> m_etaBinLabels;

};
#endif

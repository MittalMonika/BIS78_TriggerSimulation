/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWLCCALIBTOOL_H
#define EFLOWLCCALIBTOOL_H

/********************************************************************

NAME:     eflowLCCalibTool.h
PACKAGE:  offline/Reconstruction/eflowRec

AUTHORS:  T. Velz, M.Hodgkinson
CREATED:  Jan 2014

********************************************************************/

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloRec/CaloClusterCollectionProcessor.h"
#include "eflowRec/eflowBaseAlgTool.h"
#include "eflowRec/IEFlowClusterCollectionTool.h"

class eflowCaloObjectContainer;

static const InterfaceID IID_eflowLCCalibTool("eflowLCCalibTool", 1, 0);

class eflowLCCalibTool : virtual public eflowBaseAlgTool, public AthAlgTool {

  public:
  
  eflowLCCalibTool(const std::string& type,const std::string& name,const IInterface* parent);

  ~eflowLCCalibTool() {};

  static const InterfaceID& interfaceID();

  StatusCode initialize();
  void execute(eflowCaloObjectContainer* theEflowCaloObjectContainer);
  StatusCode finalize();

 private:

  void applyLocalHadronCalibration(xAOD::CaloCluster* cluster);
  void apply(ToolHandle<CaloClusterCollectionProcessor>& calibTool, xAOD::CaloCluster* cluster);
  void debugPrintCluster(xAOD::CaloCluster* cluster, std::string prefix, bool printMoments = false);

  /** Tool to put all clusters into a temporary container - then we use this to calculate moments, some of which depend on configuration of nearby clusters */
  ToolHandle<IEFlowClusterCollectionTool> m_clusterCollectionTool;

  /* Tool for applying local hadronc calibration weights to cells */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterLocalCalibTool;

  /* Tool to deal with out of cluster corrections */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterLocalCalibOOCCTool;

  /* Tool to do Pi0 corrections */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterLocalCalibOOCCPi0Tool;

  /* Tool for correcting clusters at cell level for dead materia */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterLocalCalibDMTool;

  /** Turns on and off usage of the calibration tools - needed in case we only want to calculate the new cluster moments */
  //bool m_runCalibrationTools;

};

inline const InterfaceID& eflowLCCalibTool::interfaceID()
{ 
  return IID_eflowLCCalibTool;
}

#endif

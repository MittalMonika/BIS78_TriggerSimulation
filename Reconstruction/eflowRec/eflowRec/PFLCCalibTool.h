/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFLCCALIBTOOL_H
#define PFLCCALIBTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloRec/CaloClusterCollectionProcessor.h"
#include "eflowRec/IPFBaseTool.h"
#include "eflowRec/IPFClusterCollectionTool.h"

#include <cassert>

class eflowCaloObjectContainer;
class eflowRecCluster;

class PFLCCalibTool : public extends<AthAlgTool, IPFBaseTool> {

  public:
  
  PFLCCalibTool(const std::string& type,const std::string& name,const IInterface* parent);

  ~PFLCCalibTool() {}

  StatusCode initialize();
  void execute(const eflowCaloObjectContainer& theEflowCaloObjectContainer, xAOD::CaloClusterContainer& theCaloClusterContainer);
  StatusCode finalize();

 private:

  void apply(ToolHandle<CaloClusterCollectionProcessor>& calibTool, xAOD::CaloCluster* cluster);
  void applyLocal(ToolHandle<CaloClusterCollectionProcessor>& calibTool, eflowRecCluster *cluster);
  void applyLocalWeight(eflowRecCluster* theEFRecCluster);

  /** Tool to put all clusters into a temporary container - then we use this to calculate moments, some of which depend on configuration of nearby clusters */
  ToolHandle<IPFClusterCollectionTool> m_clusterCollectionTool;

  /* Tool for applying local hadronc calibration weights to cells */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterLocalCalibTool;

  /* Tool to deal with out of cluster corrections */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterLocalCalibOOCCTool;

  /* Tool to do Pi0 corrections */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterLocalCalibOOCCPi0Tool;

  /* Tool for correcting clusters at cell level for dead materia */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterLocalCalibDMTool;

  bool m_useLocalWeight;

};

#endif

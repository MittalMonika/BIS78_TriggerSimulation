/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFMOMENTCALCULATORTOOL_H
#define PFMOMENTCALCULATORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloRec/CaloClusterCollectionProcessor.h"
#include "eflowRec/IPFBaseTool.h"
#include "eflowRec/IPFClusterCollectionTool.h"

#include <cassert>

class eflowCaloObjectContainer;

static const InterfaceID IID_PFMomentCalculatorTool("PFMomentCalculatorTool", 1, 0);

class PFMomentCalculatorTool : virtual public IPFBaseTool, public AthAlgTool {

  public:
  
  PFMomentCalculatorTool(const std::string& type,const std::string& name,const IInterface* parent);

  ~PFMomentCalculatorTool() {}

  static const InterfaceID& interfaceID();

  StatusCode initialize();
  void execute(eflowCaloObjectContainer* theEflowCaloObjectContainer,xAOD::CaloClusterContainer& theCaloClusterContainer);
  StatusCode finalize();

 private:

  /** Tool to put all clusters into a temporary container - then we use this to calculate moments, some of which depend on configuration of nearby clusters */
  ToolHandle<IPFClusterCollectionTool> m_clusterCollectionTool;
  
  /* Tool to calculate cluster moments */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterMomentsMaker;

  /** Bool to toggle whether we are in LC mode - false by default */
  bool m_LCMode;

};

inline const InterfaceID& PFMomentCalculatorTool::interfaceID()
{ 
  return IID_PFMomentCalculatorTool;
}

#endif

/*
 * Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
 */

#ifndef IDPERFMON_ELECTRONSELECTOR_H
#define IDPERFMON_ELECTRONSELECTOR_H

//==============================================================================
// Include files...
//==============================================================================
#include "InDetPerformanceMonitoring/EventAnalysis.h"

#include <map>
#include "TH1.h"

#include "xAODMuon/Muon.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"

#include "GaudiKernel/ServiceHandle.h"
#include "AsgTools/ToolHandle.h" 

#include "ElectronPhotonSelectorTools/AsgElectronLikelihoodTool.h"
//==============================================================================
// Forward class declarations...
//==============================================================================
class ElectronSelector : public EventAnalysis
{
 public:
  ElectronSelector();
  ~ElectronSelector();


  //  void setDebug(bool debug){m_doDebug = debug;}
  //  ToolHandle<CP::IMuonSelectionTool> m_muonSelectionTool;
 
  // Override functions from EventAnalysis
  void  Init();
  // virtual bool Reco();

 protected:
  // virtual void BookHistograms();

 private:
  typedef EventAnalysis PARENT;

  static unsigned int s_uNumInstances;

  // message stream
  MsgStream * m_msgStream;

  // Class variables
  const xAOD::Muon*   m_pxElectron;

  // 
  bool m_debug;

  // Electron likelihood tool:
  AsgElectronLikelihoodTool* m_LHTool2015; //!
};

#endif

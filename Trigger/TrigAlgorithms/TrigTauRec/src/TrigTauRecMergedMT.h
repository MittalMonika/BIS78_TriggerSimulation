/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//
/********************************************************************
 *
 * NAME:     TrigTauRecMergedMT.h
 * PACKAGE:  Trigger/TrigAlgorithms/TrigTauRecMerged
 *
 * AUTHOR:   R. Soluk (based on tauBuilder)
 * CREATED:  Nov 21 2005
 * MODIFIED: Dec 14 2006 S.Xella
 *           Mar 02 2001 ccuenca
 *********************************************************************/

#ifndef TRIGTAURECMERGEDMT_H
#define TRIGTAURECMERGEDMT_H

// general athena stuff
//#include "GaudiKernel/IToolSvc.h"
#include "TrigTimeAlgs/TrigTimerSvc.h"
//#include "TrigTimeAlgs/TrigTimer.h"

//Gaudi 
#include "GaudiKernel/ToolHandle.h" 
#include "GaudiKernel/ServiceHandle.h"

// Base class
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthenaMonitoring/GenericMonitoringTool.h"

#include <vector>
#include "tauRecTools/ITauToolBase.h"

#include "LumiBlockComps/ILuminosityTool.h" 
#include "BeamSpotConditionsData/BeamSpotData.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"

namespace HLT {
  class TriggerElement;
}

class ILumiBlockMuTool;

class TrigTauRecMergedMT: public AthAlgorithm {

 public:

  TrigTauRecMergedMT(const std::string& name, ISvcLocator* pSvcLocator);
  ~TrigTauRecMergedMT();

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode execute() override;


 private:

  TauEventData m_tauEventData;

  void setEmptyTauTrack( xAOD::TauJet* &tauJet,
			 xAOD::TauTrackContainer* &tauTrackCont);

  enum TAUEFCALOMON{
    NoROIDescr=0,
    NoCellCont=1,
    EmptyCellCont=2,
    NoClustCont=3,
    NoClustKey=4,
    EmptyClustCont=5,
    NoJetAttach=6,
    NoHLTtauAttach=7,
    NoHLTtauDetAttach=8,
    NoHLTtauXdetAttach=9
  };

  enum TAUEFTRKMON{
    NoTrkCont=0,
    NoVtxCont=1
  };


  /** internal tool store */
  ToolHandleArray<ITauToolBase>  m_tools;

  /** internal tool store */
  ToolHandleArray<ITauToolBase>  m_endtools;

  /** Luminosity Tool */
  ToolHandle<ILumiBlockMuTool> m_lumiBlockMuTool;

  /** Beam spot Object */
  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

  /** vector of Timers */
  std::vector<TrigTimer* > m_mytimers;

  // Monitoring tool
  ToolHandle< GenericMonitoringTool > m_monTool { this, "MonTool", "", "Monitoring tool" };

  SG::ReadHandleKey< TrigRoiDescriptorCollection > m_roIInputKey {this,"RoIInputKey","Undefined",""};
  SG::ReadHandleKey< TrigRoiDescriptorCollection > m_L1RoIKey {this, "L1RoIKey","Undefined",""};
  SG::ReadHandleKey< xAOD::CaloClusterContainer > m_clustersKey { this, "clustersKey", "caloclusters", "caloclusters in view" };
  SG::ReadHandleKey< xAOD::TrackParticleContainer > m_tracksKey { this, "Key_trackPartInputContainer", "InDetTrackParticles", "input track particle container key"};
  SG::ReadHandleKey< xAOD::VertexContainer> m_vertexKey {this,"Key_vertexInputContainer", "PrimaryVertices", "input vertex container key"};
  SG::ReadHandleKey< xAOD::TauJetContainer> m_trigTauJetKey { this, "TrigTauJet", "", "" };
  SG::WriteHandleKey< xAOD::JetContainer > m_trigtauSeedOutKey {this,"TrigTauJetOutputKey","seed tau jet","Key for output jets which are seed for tau jets"};
  SG::WriteHandleKey< xAOD::TauJetContainer > m_trigtauRecOutKey {this,"TrigTauRecOutputKey","output tau jet object","Output taujet container"};
  SG::WriteHandleKey< xAOD::TauTrackContainer > m_trigtauTrkOutKey {this,"TrigTauTrkOutputKey","output tau track object","Output tautrack container"};

  Gaudi::Property< std::string > m_outputName {this,"OutputCollection","TrigTauRecMerged","Name of output collection"};
  Gaudi::Property< float > m_maxeta {this,"maxeta",2.5,"max eta for tau"};
  Gaudi::Property< float > m_minpt {this,"minpt",10000.0,"min pt for tau"};
  Gaudi::Property< std::string > m_beamType {this,"BeamType","collisions","Beam type"};
  Gaudi::Property< float > m_trkcone {this,"trkcone",0.2,"max distance track seed from roi center"};

};
#endif

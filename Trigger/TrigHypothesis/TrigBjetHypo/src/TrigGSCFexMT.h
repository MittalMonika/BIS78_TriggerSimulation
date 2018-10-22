/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// ************************************************
//
// NAME:     TrigGSCFex.h
// PACKAGE:  Trigger/TrigHypothesis/TrigBjetHypo
//
// AUTHOR:   Carlo Varni
// EMAIL:    carlo.varni@ge.infn.it
//
// ************************************************

#ifndef TRIGGSCFEX_MT_H
#define TRIGGSCFEX_MT_H

#include <vector>
#include <string>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "GaudiKernel/ToolHandle.h"

#include "TrigInterfaces/FexAlgo.h"
#include "JetCalibTools/IJetCalibrationTool.h"


class MsgStream;

/**
 * @brief FEX class for the b-jet weight construction using offline tools
 *
 * @author John Alison <johnda @ uchicago.edu>
 *
 * This is the base feature extraction class for the HLT jet slice.
 * It computes applies the GSC calibration based on b-jet tracks
 */


class TrigGSCFexMT : public AthAlgorithm {
 public:
  /** @brief Constructor */
  TrigGSCFexMT(const std::string&, ISvcLocator*);
  /** @brief Destructor */
  ~TrigGSCFexMT();

  StatusCode initialize();
  StatusCode finalize();
  StatusCode execute();

 private:
  bool trkIsGood(const xAOD::TrackParticle *trk);

 private:
  // TOOLS
  /** @brief Offline BTag tool */
  PublicToolHandle< IJetCalibrationTool > m_jetGSCCalib_tool {this,"JetGSCCalibrationTool","","JetCalibrationTool for applying GSC correction"};

  // PROPERTIES
  /** @brief Switch for offline tools */
  Gaudi::Property< bool > m_setupOfflineTools {this,"setupOfflineTools",false,"Switch for offline tools"};

 private :
  SG::ReadHandleKey< xAOD::JetContainer > m_JetContainerKey {this,"JetKey","SplitJet","Key for input jets to TrigGSCFexMT"};
  SG::ReadHandleKey< xAOD::VertexContainer > m_VertexContainerKey {this,"PriVtxKey","xPrimVx","Key of primary vertexes to TrigGSCFexMT"};
  SG::ReadHandleKey< xAOD::TrackParticleContainer > m_TrackParticleContainerKey {this,"TrackKey","InDetTrigTrackingxAODCnv_Bjet_IDTrig","Key for track particle container to TrigGSCFexMT"};

  SG::WriteHandleKey< xAOD::JetContainer > m_jetOutputKey {this,"JetOutputKey","GSCJet","Key for output jets of TrigGSCFexMT"};
};

#endif

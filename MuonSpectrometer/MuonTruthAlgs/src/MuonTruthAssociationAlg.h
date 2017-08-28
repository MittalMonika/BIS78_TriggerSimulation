/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MuonTruthAssociationAlg.h
//   Header file for class MuonTruthAssociationAlg
///////////////////////////////////////////////////////////////////

#ifndef TRUTHPARTICLEALGS_MUONTRUTHASSOCIATIONALG_H
#define TRUTHPARTICLEALGS_MUONTRUTHASSOCIATIONALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "MuonIdHelpers/MuonIdHelperTool.h"
#include "MuonIdHelpers/MuonStationIndex.h"
#include "StoreGate/WriteDecorHandleKey.h"

class MuonTruthAssociationAlg : public AthAlgorithm  {

public:
  // Constructor with parameters:
  MuonTruthAssociationAlg(const std::string &name,ISvcLocator *pSvcLocator);

  // Basic algorithm methods:
  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();

private:
  Gaudi::Property<SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> >m_muonTruthParticleContainerName{this,"MuonTruthParticleContainerName","MuonTruthParticles","container name for muon truth particles"};
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_muonTruthParticleLink;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_muonTruthParticleType;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_muonTruthParticleOrigin;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_muonTruthParticleNPrecMatched;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_muonTruthParticleNPhiMatched;
  SG::WriteDecorHandleKey<xAOD::MuonContainer> m_muonTruthParticleNTrigEtaMatched;
  std::string m_muonName;

  bool m_associateWithInDetTP;
  int m_barcodeOffset;

  ToolHandle<Muon::MuonIdHelperTool>    m_idHelper;
};


#endif //TRUTHPARTICLEALGS_MUONTRUTHDECORATIONALG_H

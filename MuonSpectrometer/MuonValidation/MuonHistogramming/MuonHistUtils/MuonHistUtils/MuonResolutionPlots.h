/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHISTUTILS_MUONRESOLUTIONPLOTS_H
#define MUONHISTUTILS_MUONRESOLUTIONPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "TrkValHistUtils/ResolutionPlots.h"

#include "xAODMuon/Muon.h"
#include "xAODTruth/TruthParticle.h"

namespace Muon{
  
class MuonResolutionPlots:public PlotBase {
 public:
  MuonResolutionPlots(PlotBase* pParent, std::string sDir, std::string sType="", bool doBinnedResolutions=false);
  void fill(const xAOD::TrackParticle& muontp, const xAOD::TruthParticle& truthprt);

  Trk::ResolutionPlots m_oResolutionPlots;
  TH2* Res_pT_vs_pT;
  TH2* Res_pT_vs_eta;
  TH2* Res_pT_vs_phi;
  
  TProfile *ProfRes_pT_vs_pT;
  TProfile *ProfRes_pT_vs_eta;
  TProfile *ProfRes_pT_vs_phi;

 private:
  std::string m_sType;
  bool m_doBinnedResolutionPlots;

};


}

#endif

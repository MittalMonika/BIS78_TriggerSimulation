/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/BTagJetAugmenter.h"

#include <cmath>
#include <cstddef>

#include "xAODJet/Jet.h"

#include "TVector3.h"

BTagJetAugmenter::BTagJetAugmenter():
  pt_uncalib("pt_uncalib"),
  eta_uncalib("eta_uncalib"),
  abs_eta_uncalib("abs_eta_uncalib"),
  ip2d_weightBOfTracks("IP2D_weightBofTracks"),
  ip2d_nTrks("IP2D_nTrks"),
  ip2d_pu("IP2D_pu"),
  ip2d_pc("IP2D_pc"),
  ip2d_pb("IP2D_pb"),
  ip2d_isDefaults("IP2D_isDefaults"),
  ip2d_cu("IP2D_cu"),
  ip2d_bu("IP2D_bu"),
  ip2d_bc("IP2D_bc"),
  ip3d_weightBOfTracks("IP3D_weightBofTracks"),
  ip3d_nTrks("IP3D_nTrks"),
  ip3d_pu("IP3D_pu"),
  ip3d_pc("IP3D_pc"),
  ip3d_pb("IP3D_pb"),
  ip3d_isDefaults("IP3D_isDefaults"),
  ip3d_cu("IP3D_cu"),
  ip3d_bu("IP3D_bu"),
  ip3d_bc("IP3D_bc"),
  jf_deltaEta("JetFitter_deltaeta"),
  jf_deltaPhi("JetFitter_deltaphi"),
  jf_fittedPosition("JetFitter_fittedPosition"),
  jf_vertices("JetFitter_JFvertices"),
  jf_nVtx("JetFitter_nVTX"),
  jf_nSingleTracks("JetFitter_nSingleTracks"),
  jf_isDefaults("JetFitter_isDefaults"),
  jf_deltaR("JetFitter_deltaR"),
  sv1_vertices("SV1_vertices"),
  sv1_nVtx("SV1_nVtx"),
  sv1_isDefaults("SV1_isDefaults"),
  jet_track_links("BTagTrackToJetAssociator"),
  secondaryVtx_isDefaults("secondaryVtx_isDefaults"),
  secondaryVtx_nTrks("secondaryVtx_nTrks"),
  secondaryVtx_m("secondaryVtx_m"),
  secondaryVtx_E("secondaryVtx_E"),
  secondaryVtx_EFrac("secondaryVtx_EFrac"),
  secondaryVtx_L3d("secondaryVtx_L3d"),
  secondaryVtx_Lxy("secondaryVtx_Lxy"),
  secondaryVtx_min_trk_flightDirRelEta("secondaryVtx_min_trk_flightDirRelEta"),
  secondaryVtx_max_trk_flightDirRelEta("secondaryVtx_max_trk_flightDirRelEta"),
  secondaryVtx_avg_trk_flightDirRelEta("secondaryVtx_avg_trk_flightDirRelEta"),
  min_trk_flightDirRelEta("min_trk_flightDirRelEta"),
  max_trk_flightDirRelEta("max_trk_flightDirRelEta"),
  avg_trk_flightDirRelEta("avg_trk_flightDirRelEta"),
  smt_mu_pt("SMT_mu_pt"),
  smt_isDefaults("SMT_isDefaults"),
  rnnip_pbIsValid("rnnip_pbIsValid"),
  rnnip_isDefaults("rnnip_isDefaults") {}

void BTagJetAugmenter::augment(const xAOD::Jet &jet, const xAOD::Jet &uncalibrated_jet) {

  const xAOD::BTagging& btag = *jet.btagging();

  pt_uncalib(btag) = uncalibrated_jet.pt();
  eta_uncalib(btag) = uncalibrated_jet.eta();
  abs_eta_uncalib(btag) = std::abs(uncalibrated_jet.eta());

  // pass off to calibrated jet function
  augment(jet);
}

void BTagJetAugmenter::augment(const xAOD::Jet &jet) {
  const xAOD::BTagging& btag = *jet.btagging();

  if (ip2d_weightBOfTracks(btag).size() > 0) {
    ip2d_isDefaults(btag) = 0;
  } else {
    ip2d_isDefaults(btag) = 1;
  }
  ip2d_nTrks(btag) = ip2d_weightBOfTracks(btag).size();

  ip2d_cu(btag) = std::log(ip2d_pc(btag) / ip2d_pu(btag));
  ip2d_bu(btag) = std::log(ip2d_pb(btag) / ip2d_pu(btag));
  ip2d_bc(btag) = std::log(ip2d_pb(btag) / ip2d_pc(btag));

  if (ip3d_weightBOfTracks(btag).size() > 0) {
    ip3d_isDefaults(btag) = 0;
  } else {
    ip3d_isDefaults(btag) = 1;
  }

  ip3d_cu(btag) = std::log(ip3d_pc(btag) / ip3d_pu(btag));
  ip3d_bu(btag) = std::log(ip3d_pb(btag) / ip3d_pu(btag));
  ip3d_bc(btag) = std::log(ip3d_pb(btag) / ip3d_pc(btag));
  ip3d_nTrks(btag) = ip3d_weightBOfTracks(btag).size();

  if (jf_vertices(btag).size() > 0 && (jf_nVtx(btag) > 0 || jf_nSingleTracks(btag) > 0)) {
    jf_isDefaults(btag) = 0;
    jf_deltaR(btag) = std::hypot(jf_deltaEta(btag), jf_deltaPhi(btag));
  } else {
    jf_isDefaults(btag) = 1;
    jf_deltaR(btag) = NAN;
  }


  if (sv1_vertices(btag).size() > 0) {
    sv1_isDefaults(btag) = 0;
  } else {
    sv1_isDefaults(btag) = 1;
  }

  TVector3 flightDir(NAN, NAN, NAN);
  float jf_phi = NAN;
  float jf_theta = NAN;

  int secondary_jf_vtx_index = -1;
  int secondaryVtx_track_number = -1;
  double secondaryVtx_track_flightDirRelEta_total = NAN;
  float min_jf_vtx_L3d = NAN;

  if (jf_fittedPosition(btag).size() > 4) {
    jf_phi = jf_fittedPosition(btag).at(3);
    jf_theta = jf_fittedPosition(btag).at(4);
    flightDir.SetMagThetaPhi(1, jf_theta, jf_phi);

    for (std::size_t jf_vtx_index = 0; jf_vtx_index < jf_vertices(btag).size() && jf_vtx_index < jf_fittedPosition(btag).size() - 5; jf_vtx_index++) {
      float jf_vtx_L3d = jf_fittedPosition(btag).at(jf_vtx_index + 5);
      if (jf_vtx_L3d > 0 && (jf_vtx_L3d < min_jf_vtx_L3d || std::isnan(min_jf_vtx_L3d))) {
        secondary_jf_vtx_index = jf_vtx_index;
        min_jf_vtx_L3d = jf_vtx_L3d;
      }
    }

    if (secondary_jf_vtx_index >= 0) {
      secondaryVtx_track_number = 0;
      secondaryVtx_track_flightDirRelEta_total = 0;
      secondaryVtx_isDefaults(btag) = 0;
    } else {
      secondaryVtx_isDefaults(btag) = 1;
    }
  } else {
    secondaryVtx_isDefaults(btag) = 1;
  }
  secondaryVtx_L3d(btag) = min_jf_vtx_L3d;
  secondaryVtx_Lxy(btag) = min_jf_vtx_L3d * sinf(jf_theta);

  const float track_mass = 139.57; // Why?

  unsigned track_number = 0;
  double track_E_total = 0;
  double min_track_flightDirRelEta = NAN;
  double max_track_flightDirRelEta = NAN;
  double track_flightDirRelEta_total = 0;

  TLorentzVector secondaryVtx_4momentum_total;
  double secondaryVtx_min_track_flightDirRelEta = NAN;
  double secondaryVtx_max_track_flightDirRelEta = NAN;

  for (const auto &jet_track_link : jet_track_links(btag)) {
    const xAOD::TrackParticle &track_particle = **jet_track_link;

    uint8_t n_pixel_hits;
    uint8_t n_sct_hits;
    track_particle.summaryValue(n_pixel_hits, xAOD::numberOfPixelHits);
    track_particle.summaryValue(n_sct_hits, xAOD::numberOfSCTHits);
    if (n_pixel_hits + n_sct_hits < 2) continue;

    track_number++;
    track_E_total += track_particle.e();

    TVector3 track_flightDirRelVect = track_particle.p4().Vect();
    track_flightDirRelVect.SetTheta(track_flightDirRelVect.Angle(flightDir));

    double track_flightDirRelEta = NAN;
    if (track_flightDirRelVect.Perp() != 0) {
      track_flightDirRelEta = track_flightDirRelVect.PseudoRapidity();
    }

    track_flightDirRelEta_total += track_flightDirRelEta;
    if (track_flightDirRelEta < min_track_flightDirRelEta || std::isnan(min_track_flightDirRelEta)) {
      min_track_flightDirRelEta = track_flightDirRelEta;
    }
    if (track_flightDirRelEta > max_track_flightDirRelEta || std::isnan(max_track_flightDirRelEta)) {
      max_track_flightDirRelEta = track_flightDirRelEta;
    }
    if (secondary_jf_vtx_index >= 0) {
      for (const ElementLink<xAOD::TrackParticleContainer> vertex_track_particle : (**jf_vertices(btag).at(secondary_jf_vtx_index)).track_links()) {
        if (*vertex_track_particle == &track_particle) {
          secondaryVtx_track_number++;
          TLorentzVector track_fourVector;
          track_fourVector.SetVectM(track_particle.p4().Vect(), track_mass);
          secondaryVtx_4momentum_total += track_fourVector;
          secondaryVtx_track_flightDirRelEta_total += track_flightDirRelEta;
          if (track_flightDirRelEta < secondaryVtx_min_track_flightDirRelEta || std::isnan(secondaryVtx_min_track_flightDirRelEta)) {
            secondaryVtx_min_track_flightDirRelEta = track_flightDirRelEta;
          }
          if (track_flightDirRelEta > secondaryVtx_max_track_flightDirRelEta || std::isnan(secondaryVtx_max_track_flightDirRelEta)) {
            secondaryVtx_max_track_flightDirRelEta = track_flightDirRelEta;
          }
        }
      }
    }
  }

  secondaryVtx_nTrks(btag) = secondaryVtx_track_number;
  if (secondaryVtx_track_number >= 0) {
    secondaryVtx_m(btag) = secondaryVtx_4momentum_total.M();
    secondaryVtx_E(btag) = secondaryVtx_4momentum_total.E();
    secondaryVtx_EFrac(btag) = secondaryVtx_4momentum_total.E() / track_E_total;
  } else {
    secondaryVtx_m(btag) = NAN;
    secondaryVtx_E(btag) = NAN;
    secondaryVtx_EFrac(btag) = NAN;
  }
  secondaryVtx_min_trk_flightDirRelEta(btag) = secondaryVtx_min_track_flightDirRelEta;
  secondaryVtx_max_trk_flightDirRelEta(btag) = secondaryVtx_max_track_flightDirRelEta;
  secondaryVtx_avg_trk_flightDirRelEta(btag) = secondaryVtx_track_flightDirRelEta_total / secondaryVtx_track_number;

  min_trk_flightDirRelEta(btag) = min_track_flightDirRelEta;
  max_trk_flightDirRelEta(btag) = max_track_flightDirRelEta;
  avg_trk_flightDirRelEta(btag) = track_flightDirRelEta_total / track_number;

  if (smt_mu_pt(btag) > 0) {
    smt_isDefaults(btag) = 0;
  }  else {
    smt_isDefaults(btag) = 1;
  }

  if (! rnnip_pbIsValid(btag)) {
    rnnip_isDefaults(btag) = 1;
  }  else {
    rnnip_isDefaults(btag) = 0;
  }
}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetBasicPlot.cxx
 * @author shaun roe
 **/

#include "InDetBasicPlot.h"


InDetBasicPlot::InDetBasicPlot(InDetPlotBase *pParent, const std::string &sDir) :
  InDetPlotBase(pParent, sDir),
  m_paramNames{"d0", "z0", "phi", "theta", "eta", "qOverP"},
  m_truthParamNames{"z0st", "prodR", "prodZ"},
  m_basicTruthPlots{nullptr},
  m_extraTruthPlots{nullptr},
  m_basicTrackPlots{nullptr},
  m_d0IsExactlyZeroInTrackCounter(0),
  m_d0IsExactlyZeroInTruthCounter(0),
  m_numCallsToFillTruth(0),
  m_numCallsToFillTrack(0) {
}

void
InDetBasicPlot::initializePlots() {
  unsigned int i(0);
  const std::string prefix("basic");
  const std::string truthPrefix("truth");

  for (const auto &p:m_paramNames) {
    const std::string particleHistoId = prefix + p;
    const std::string truthHistoId = truthPrefix + p;
    book(m_basicTruthPlots[i], truthHistoId);
    book(m_basicTrackPlots[i], particleHistoId);
    ++i;
  }
  i = 0;
  for (const auto &p:m_truthParamNames) {
    const std::string truthHistoId = truthPrefix + p;
    book(m_extraTruthPlots[i], truthHistoId);
    ++i;
  }
}

void
InDetBasicPlot::fill(const xAOD::TruthParticle &particle) {
  unsigned int i(0);

  ++m_numCallsToFillTruth;
  for (const auto &p:m_paramNames) {
    if (particle.isAvailable<float>(p)) {
      const auto thisParameterValue = particle.auxdata< float >(p);
      if ((i == 0) and thisParameterValue == 0.) {
        ++m_d0IsExactlyZeroInTruthCounter;
      }
      fillHisto(m_basicTruthPlots[i],thisParameterValue);
    }
    ++i;
  }
  i = 0;
  for (const auto &p:m_truthParamNames) {
    if (particle.isAvailable<float>(p)) {
      const auto thisParameterValue = particle.auxdata< float >(p);
      fillHisto(m_extraTruthPlots[i],thisParameterValue);
    }
    ++i;
  }
}

void
InDetBasicPlot::fill(const xAOD::TrackParticle &particle) {
  ++m_numCallsToFillTrack;
  float trkParticleParams[NPARAMS];
  trkParticleParams[D0] = particle.d0();
  if (trkParticleParams[D0] == 0.) {
    ++m_d0IsExactlyZeroInTrackCounter;
  }
  trkParticleParams[Z0] = particle.z0();
  trkParticleParams[PHI] = particle.phi0();
  trkParticleParams[THETA] = particle.theta();
  trkParticleParams[ETA] = particle.eta();
  trkParticleParams[QOVERP] = particle.qOverP();
  for (unsigned int i(0); i < NPARAMS; ++i) {
    const auto &thisParameterValue = trkParticleParams[i];
    fillHisto(m_basicTrackPlots[i],thisParameterValue);
  }
}

void
InDetBasicPlot::finalizePlots() {
  ATH_MSG_INFO(
    "Number of exact zero values for d0 in tracks: " << m_d0IsExactlyZeroInTrackCounter << " out of " <<
    m_numCallsToFillTrack);
  ATH_MSG_INFO(
    "Number of exact zero values for d0 in truth: " << m_d0IsExactlyZeroInTruthCounter << " out of " <<
    m_numCallsToFillTruth);
}

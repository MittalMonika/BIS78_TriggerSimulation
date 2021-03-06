/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_TRUTHJETWEIGHTFILTER_H
#define GENERATORFILTERS_TRUTHJETWEIGHTFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "TRandom3.h"
#include <vector>

// File generated by ROOT TMVA containing the MLP response
#include "GeneratorFilters/TruthJetWeightFilter_loose_MLP.class.C"

class TruthJetWeightFilter : public GenFilter {
public:

  TruthJetWeightFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode filterInitialize();
  virtual StatusCode filterEvent();

private:

  int    m_weightAlgorithm;

  double m_rescaleEm;
  double m_rescaleHad;

  double m_minPtLeadUpweight;

  double m_minPt;
  double m_maxPt;
  double m_minEfficiency;
  double m_lowPtTune;
  double m_maxEtaRange;

  int    m_samplingRandomSeed;
  bool   m_deactivateFilter;

  std::string m_TruthJetContainerName;

  ReadMLP* m_MLPReader;
  TRandom3* m_rand;

  StatusCode saveWeight(double weight);
  double getRescaledJetPt(const CLHEP::HepLorentzVector& jet);
  void getLeadTruthParts(const CLHEP::HepLorentzVector& jet,
                         CLHEP::HepLorentzVector& lead,
                         CLHEP::HepLorentzVector& sublead,
                         CLHEP::HepLorentzVector& leadchg,
                         CLHEP::HepLorentzVector& highm5,
                         CLHEP::HepLorentzVector& highm10);

  CLHEP::HepLorentzVector getJetSubcomponent(const CLHEP::HepLorentzVector& axis,
                                             int idcomp = 1,
                                             double maxdr = 0.4,
                                             double ptcutoff = 0.5);
  void getJetSubinfo(const CLHEP::HepLorentzVector& axis,
                     std::vector<double>& jetvalues);

  StatusCode getEffJetPt(double& prob);
  StatusCode getEffJetSubinfo(double& prob);

};

#endif

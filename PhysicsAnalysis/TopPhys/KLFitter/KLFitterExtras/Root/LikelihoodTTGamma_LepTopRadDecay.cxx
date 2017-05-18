/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "KLFitterExtras/LikelihoodTTGamma_LepTopRadDecay.h" 
#include "KLFitter/LikelihoodTopLeptonJets.h" 

// --------------------------------------------------------- 
KLFitter::LikelihoodTTGamma_LepTopRadDecay::LikelihoodTTGamma_LepTopRadDecay()
{
  // calls base class constructor
}

// --------------------------------------------------------- 
KLFitter::LikelihoodTTGamma_LepTopRadDecay::~LikelihoodTTGamma_LepTopRadDecay()
{
  // calls base class destructor
}

// --------------------------------------------------------- 
int KLFitter::LikelihoodTTGamma_LepTopRadDecay::CalculateLorentzVectors(std::vector <double> const& parameters)
{
  KLFitter::LikelihoodTTGamma::CalculateLorentzVectors(parameters);

  // leptonic top 
  *(fParticlesModel->Parton(5)) += *(fParticlesModel->Photon(0));

  // no error 
  return 1; 
}

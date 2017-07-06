/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "KLFitterExtras/LikelihoodTTGamma_HadTopRadDecay.h" 
#include "KLFitter/LikelihoodTopLeptonJets.h" 

// --------------------------------------------------------- 
KLFitter::LikelihoodTTGamma_HadTopRadDecay::LikelihoodTTGamma_HadTopRadDecay()
{
  // calls base class constructor
}

// --------------------------------------------------------- 
KLFitter::LikelihoodTTGamma_HadTopRadDecay::~LikelihoodTTGamma_HadTopRadDecay()
{
  // calls base class destructor
}

// --------------------------------------------------------- 
int KLFitter::LikelihoodTTGamma_HadTopRadDecay::CalculateLorentzVectors(std::vector <double> const& parameters)
{
  KLFitter::LikelihoodTTGamma::CalculateLorentzVectors(parameters);

  // hadronic top 
  *(fParticlesModel->Parton(4)) += *(fParticlesModel->Photon(0));

  // no error 
  return 1; 
}

/*
   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

#include "egammaUtils/egammaEnergyPositionAllSamples.h"
#include "xAODCaloEvent/CaloCluster.h"
namespace {
/*if both in barrel and end-cap then have to
 * rely on energy deposition
 * be careful to test 0 precisely
 * As 0 (no deposit) > noise (which is negative)*/
bool isCrackBarrel(const xAOD::CaloCluster &cluster, 
                   const xAOD::CaloCluster::CaloSample barrel,
                   const xAOD::CaloCluster::CaloSample endCap) {
  if (cluster.eSample(barrel) == 0. && cluster.eSample(endCap) != 0.) {
    return false; // endcap
  }
  if (cluster.eSample(barrel) != 0. && cluster.eSample(endCap) == 0.) {
    return true; // barrel
  }
  if (cluster.eSample(barrel) >= cluster.eSample(endCap)) {
    return true; // barrel
  }  
  return false; // endcap;
}
} // namespace

double egammaEnergyPositionAllSamples::e(const xAOD::CaloCluster &cluster) {
  return (e0(cluster) + e1(cluster) + e2(cluster) + e3(cluster));
}

double egammaEnergyPositionAllSamples::e0(const xAOD::CaloCluster &cluster) {
  return (cluster.eSample(CaloSampling::PreSamplerB) + cluster.eSample(CaloSampling::PreSamplerE));
}

double egammaEnergyPositionAllSamples::e1(const xAOD::CaloCluster &cluster) {
  return (cluster.eSample(CaloSampling::EMB1) + cluster.eSample(CaloSampling::EME1));
}

double egammaEnergyPositionAllSamples::e2(const xAOD::CaloCluster &cluster) {
  return (cluster.eSample(CaloSampling::EMB2) + cluster.eSample(CaloSampling::EME2));
}

double egammaEnergyPositionAllSamples::e3(const xAOD::CaloCluster &cluster) {
  return (cluster.eSample(CaloSampling::EMB3) + cluster.eSample(CaloSampling::EME3));
}

bool egammaEnergyPositionAllSamples::inBarrel(const xAOD::CaloCluster &cluster, const int sampling) {

  // from cluster position and energy define
  // if we are in barrel or end-cap
  // In particular it uses energy deposit in 2nd sampling
  // to check close to the crack region
  // sampling (0=presampler,1=strips,2=middle,3=back)
  //
  if (cluster.inBarrel() && !cluster.inEndcap()) {
    return true; // barrel
  } else if (!cluster.inBarrel() && cluster.inEndcap()) {
    return false; // endcap
  } else if (cluster.inBarrel() && cluster.inEndcap()) {
    switch (sampling) {
    case 0: {
      return isCrackBarrel(cluster,CaloSampling::PreSamplerB, CaloSampling::PreSamplerE);
    }
    case 1: {
      return isCrackBarrel(cluster,CaloSampling::EMB1, CaloSampling::EME1);
    }
    case 2: {
      return isCrackBarrel(cluster,CaloSampling::EMB2, CaloSampling::EME2);
    }
    case 3: {
      return isCrackBarrel(cluster,CaloSampling::EMB3, CaloSampling::EME3);
    }
    default: {
      return true; // barrel default
    }
    }
  }

  return true; // barrel defautl
}


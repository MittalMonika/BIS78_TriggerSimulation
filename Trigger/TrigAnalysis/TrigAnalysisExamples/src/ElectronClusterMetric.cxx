/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: ElectronClusterMetric.cxx 770492 2016-08-28 16:52:40Z rwhite $

// Local include(s):
#include "ElectronClusterMetric.h"

// The only function that a metric need implement
// is calculateDistance(const T *t, const R *r)
// where T is the trigger type you are matching
// and r is the reco type you are matching.

// There are a few rules you must follow when you
// implement the distance function:
//
// (1) The distance must be greater than or equal to
//     zero.  Anything less than 0 will be treated as
//     infinitely far away.
// (2) Smaller distances mean closer.  So, for example,
//     if you measure the distance between two things
//     as the number of hits they share, you should
//     return one over that number.
// (3) The actual scale you set with the distance does
//     not matter, as long as you set the cutoff
//     appropriately.  Numbers between 0 and 100
//     work just as well numbers between 0 and 1

float ElectronClusterMetric::calculateDistance(
    const xAOD::TrigElectron *trig,
    const xAOD::Electron *reco) const {

  // test for valid parameters
  if(!trig || !reco)
    return -1.;

  // We can instantiate the default deltaR distance functor to
  // calculate deltaR for us
  //DeltaRDistanceFunctor<xAOD::CaloCluster,xAOD::TrigEMCluster> deltaRMetric;
  //float dist = deltaRMetric(reco->cluster(), trig->cluster());
  float dist=0.;
  return dist;
}

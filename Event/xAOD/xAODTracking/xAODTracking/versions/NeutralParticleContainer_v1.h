/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRACKING_VERSIONS_NEUTRALPARTICLECONTAINER_V1_H
#define XAODTRACKING_VERSIONS_NEUTRALPARTICLECONTAINER_V1_H
 
#include <stdint.h>
 
// Core include(s):
#include "AthContainers/DataVector.h"
 
// Local include(s):
#include "xAODTracking/versions/NeutralParticle_v1.h"
#include "xAODTracking/NeutralParticleContainerFwd.h"

// To complete the DATAVECTOR_BASE macro:
template struct DataVector_detail::DVLEltBaseInit< xAOD::NeutralParticle_v1 >;

namespace xAOD {
   /// The container is a simple typedef for now
   typedef DataVector< xAOD::NeutralParticle_v1 > NeutralParticleContainer_v1;
}
 

#endif // XAODTRACKING_VERSIONS_NEUTRALPARTICLECONTAINER_V1_H

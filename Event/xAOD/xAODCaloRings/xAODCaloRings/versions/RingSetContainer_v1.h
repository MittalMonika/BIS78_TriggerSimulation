/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: RingSetContainer_v1.h 707323 2015-11-12 02:45:01Z wsfreund $
#ifndef XAODCALORINGS_VERSIONS_RINGSETCONTAINER_V1_H
#define XAODCALORINGS_VERSIONS_RINGSETCONTAINER_V1_H

// Core include(s):
#include "AthContainers/DataVector.h"
#include "AthLinks/ElementLink.h"

// Local include(s):
#include "xAODCaloRings/versions/RingSet_v1.h"

namespace xAOD {
/// The container is a simple typedef for now
typedef DataVector< xAOD::RingSet_v1 > RingSetContainer_v1;
/// Define EL vector for this version:
typedef std::vector< ElementLink< RingSetContainer_v1 > >
  RingSetLinks_v1;
} // namespace xAOD

#endif // XAODCALORINGS_VERSIONS_RINGSETCONTAINER_V1_H

// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: CPMHitsAuxContainer_v1.h 631511 2014-11-27 13:17:07Z gwatts $
#ifndef XAODTRIGL1CALO_VERSIONS_CPMHITSAUXCONTAINER_V1_H
#define XAODTRIGL1CALO_VERSIONS_CPMHITSAUXCONTAINER_V1_H

// STL include(s):
#include <vector>

// EDM include(s):
#include "xAODCore/AuxContainerBase.h"

namespace xAOD{
  
  /// AuxContainer for CPMHits_v1
  ///
  /// @author John Morris <john.morris@cern.ch>
  ///
  /// $Revision: 631511 $
  /// $Date: 2014-11-27 14:17:07 +0100 (Thu, 27 Nov 2014) $  

  class CPMHitsAuxContainer_v1 : public AuxContainerBase {
  public:
    // Default constructor
    CPMHitsAuxContainer_v1();

  private:
    std::vector<int> crate;
    std::vector<int> module;
    std::vector<int> peak;
    std::vector<std::vector<unsigned int> > hitsVec0;
    std::vector<std::vector<unsigned int> > hitsVec1;  
    
  }; // class CPMHitsAuxContainer_v1 
} // namespace xAOD

// Set up a CLID and StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
  SG_BASE( xAOD::CPMHitsAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODTRIGL1CALO_VERSIONS_CPMHITSAUXCONTAINER_V1_H

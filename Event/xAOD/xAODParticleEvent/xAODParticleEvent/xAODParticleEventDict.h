// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: xAODParticleEventDict.h 599909 2014-06-02 14:25:44Z kkoeneke $
#ifndef XAODPARTICLEEVENT_XAODPARTICLEEVENTDICT_H
#define XAODPARTICLEEVENT_XAODPARTICLEEVENTDICT_H


// STL
#include <vector>

// EDM include(s):
#include "AthLinks/DataLink.h"
#include "AthLinks/ElementLink.h"
#include "AthLinks/ElementLinkVector.h"

// Local include(s):
#include "xAODParticleEvent/versions/IParticleLinkContainer_v1.h"
#include "xAODParticleEvent/versions/ParticleContainer_v1.h"
#include "xAODParticleEvent/versions/ParticleAuxContainer_v1.h"
#include "xAODParticleEvent/versions/CompositeParticleContainer_v1.h"
#include "xAODParticleEvent/versions/CompositeParticleAuxContainer_v1.h"


namespace {
  struct GCCXML_DUMMY_INSTANTIATION_XAODPARTICLEEVENT {
    xAOD::IParticleLinkContainer_v1                                              ipl_c1;
    DataLink< xAOD::IParticleLinkContainer_v1 >                                  ipl_l1;
    ElementLink< xAOD::IParticleLinkContainer_v1 >                               ipl_l2;
    ElementLinkVector< xAOD::IParticleLinkContainer_v1 >                         ipl_l3;
    std::vector< DataLink< xAOD::IParticleLinkContainer_v1 > >                   ipl_l4;
    std::vector< ElementLink< xAOD::IParticleLinkContainer_v1 > >                ipl_l5;
    std::vector< ElementLinkVector< xAOD::IParticleLinkContainer_v1 > >          ipl_l6;
    std::vector< std::vector< ElementLink< xAOD::IParticleLinkContainer_v1 > > > ipl_l7;

    xAOD::ParticleContainer_v1                                              p_c1;
    DataLink< xAOD::ParticleContainer_v1 >                                  p_l1;
    ElementLink< xAOD::ParticleContainer_v1 >                               p_l2;
    ElementLinkVector< xAOD::ParticleContainer_v1 >                         p_l3;
    std::vector< DataLink< xAOD::ParticleContainer_v1 > >                   p_l4;
    std::vector< ElementLink< xAOD::ParticleContainer_v1 > >                p_l5;
    std::vector< ElementLinkVector< xAOD::ParticleContainer_v1 > >          p_l6;
    std::vector< std::vector< ElementLink< xAOD::ParticleContainer_v1 > > > p_l7;

    xAOD::CompositeParticleContainer_v1                                              cp_c1;
    DataLink< xAOD::CompositeParticleContainer_v1 >                                  cp_l1;
    ElementLink< xAOD::CompositeParticleContainer_v1 >                               cp_l2;
    ElementLinkVector< xAOD::CompositeParticleContainer_v1 >                         cp_l3;
    std::vector< DataLink< xAOD::CompositeParticleContainer_v1 > >                   cp_l4;
    std::vector< ElementLink< xAOD::CompositeParticleContainer_v1 > >                cp_l5;
    std::vector< ElementLinkVector< xAOD::CompositeParticleContainer_v1 > >          cp_l6;
    std::vector< std::vector< ElementLink< xAOD::CompositeParticleContainer_v1 > > > cp_l7;

    // // Smart pointers needed for the correct generation of the auxiliary
    // // class dictionaries:
    // std::vector< xAOD::IParticleLinkContainer > el1;
    // ElementLink< xAOD::IParticleContainer > el7;
    // std::vector< ElementLink< xAOD::IParticleContainer > > el8;
    // std::vector< std::vector< ElementLink< xAOD::IParticleContainer > > > el9;

  };
}

#endif // XAODPARTICLEEVENT_XAODPARTICLEEVENTDICT_H

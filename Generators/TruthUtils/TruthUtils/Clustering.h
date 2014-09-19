// -*- C++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//
// This file is part of MCUtils -- https://bitbucket.org/andybuckley/mcutils
// Copyright (C) 2013-2014 Andy Buckley <andy.buckley@cern.ch>
//
// Embedding of MCUtils code in other projects is permitted provided this
// notice is retained and the MCUtils namespace and include path are changed.
//
#pragma once

/// @file FastJet clustering on HepMC events
/// @author Andy Buckley <andy.buckley@cern.ch>

#include "TruthUtils/HepMCEventUtils.h"
#include "TruthUtils/FastJet.h"

namespace MC {


  /// @name Converters between HepMC and FastJet momentum types
  //@{

  /// Convert a HepMC FourVector to a FastJet PseudoJet
  inline fastjet::PseudoJet mk_pseudojet(const HepMC::FourVector& p4) {
    return fastjet::PseudoJet(p4.px(), p4.py(), p4.pz(), p4.e());
  }

  /// Convert a HepMC GenParticle to a FastJet PseudoJet
  inline fastjet::PseudoJet mk_pseudojet(const HepMC::GenParticle* gp) {
    fastjet::PseudoJet pj = mk_pseudojet(gp->momentum());
    //pj.set_user_info(new HepMCInfo(p));
    return pj;
  }

  /// Convert a vector of HepMC GenParticles to a vector of FastJet PseudoJets
  inline std::vector<fastjet::PseudoJet> mk_pseudojets(const std::vector<const HepMC::GenParticle*>& gps) {
    std::vector<fastjet::PseudoJet> pjs;
    BOOST_FOREACH (const HepMC::GenParticle* gp, gps) {
      pjs.push_back( mk_pseudojet(gp) );
    }
    return pjs;
  }

  //@}


  /// @name Jet builders from HepMC input types
  //@{

  /// Construct pT-sorted jets using the @a alg measure with jet @a R parameter, and min pT @a ptmin (in MeV)
  inline std::vector<fastjet::PseudoJet> get_jets(const GenParticlesC& particles, double R, double ptmin,
                                                  fastjet::JetAlgorithm alg=fastjet::antikt_algorithm) {
    /// @todo Convert to use filter_jet_inputs
    return get_jets(mk_pseudojets(filter_stable(particles)), R, ptmin, alg);
  }


  /// Construct pT-sorted jets using the @a alg measure with jet @a R parameter, and min pT @a ptmin (in MeV)
  inline std::vector<fastjet::PseudoJet> get_jets(const HepMC::GenEvent* evt, double R, double ptmin,
                                                  fastjet::JetAlgorithm alg=fastjet::antikt_algorithm) {
    /// @todo Convert to use get_jet_inputs
    return get_jets(mk_pseudojets(get_stable(evt)), R, ptmin, alg);
  }

  //@}


  /// @todo Make safe photon clustering for charged leptons

  /// @todo Make tau, B, top and W/Z finders (both safe and direct versions)


}

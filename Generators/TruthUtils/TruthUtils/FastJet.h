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

/// @file FastJet helper routines
/// @author Andy Buckley <andy.buckley@cern.ch>

#include "TruthUtils/Utils.h"
#include "TruthUtils/Vectors.h"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"

namespace MC {


  /// @name Converters between MCUtils and FastJet momentum types
  //@{

  /// @todo Enable... conditionally on FJ version?
  // /// For attaching the GenParticle provenance info to a PseudoJet
  // struct HepMCInfo : public PseudoJet::UserInfoBase {
  //     HepMCInfo(const HepMC::GenParticle* gp) : genparticle(gp) { }
  //     const HepMC::GenParticle* genparticle;
  // };
  //
  // // Usage: const GenParticle* gp = pj.user_info<HepMCInfo>().genparticle;


  /// Convert a P4 to a FastJet PseudoJet
  inline fastjet::PseudoJet mk_pseudojet(const P4& p) {
    return fastjet::PseudoJet(p.px(), p.py(), p.pz(), p.E());
  }


  /// Convert a FastJet PseudoJet to a P4
  inline P4 mk_p4(const fastjet::PseudoJet& p) {
    const double m = p.m();
    assert(m > -1e-3 && "Negative mass vector from FastJet");
    return P4::mkXYZM(p.px(), p.py(), p.pz(), (m >= 0) ? m : 0);
  }

  //@}


  /// @name Jet builders
  //@{

  /// Construct pT-sorted jets using the @a alg measure with jet @a R parameter, and min pT @a ptmin (in MeV)
  inline std::vector<fastjet::PseudoJet> get_jets(const std::vector<fastjet::PseudoJet>& particles, double R, double ptmin,
                                                  fastjet::JetAlgorithm alg=fastjet::antikt_algorithm) {
    const fastjet::JetDefinition jet_def(alg, R);
    /// @todo Add area definition?
    fastjet::ClusterSequence cseq(particles, jet_def); //< @todo Need new + auto_ptr?
    return sorted_by_pt(cseq.inclusive_jets(ptmin));
  }

  //@}


}

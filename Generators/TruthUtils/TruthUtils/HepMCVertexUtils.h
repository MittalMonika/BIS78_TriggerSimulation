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

/// @file Functions for filtering and classifying HepMC GenVertex objects
/// @author Andy Buckley <andy.buckley@cern.ch>

#include "HepMC/GenEvent.h"
#include "boost/function.hpp" //< Replace with std::function when possible
#include <vector>

#include "TruthUtils/HepMCEventUtils.h"
#include "TruthUtils/Vectors.h"
#include "TruthUtils/Utils.h"

namespace MC {


  /// @name Friendlier HepMC type typedefs
  //@{
  typedef std::vector<HepMC::GenVertex*> GenVertices;
  typedef std::vector<const HepMC::GenVertex*> GenVerticesC;
  //@}


  /// @name Vertex constness converters
  ///
  /// @note These are just syntactic sugar: this is already not so inconvenient!
  //@{
  inline const HepMC::GenVertex* mk_const(HepMC::GenVertex* gv) { return const_cast<const HepMC::GenVertex*>(gv); }
  inline HepMC::GenVertex* mk_unconst(const HepMC::GenVertex* const_gv) { return const_cast<HepMC::GenVertex*>(const_gv); }
  //@}


  /// @name Container constness converters
  ///
  /// @note Pointers in containers will still point to the same objects after
  /// const conversion!
  ///
  /// @note Due to constness rules, new vectors have to be made and
  /// populated. So this is not super-efficient, but seems to be necessary.
  //@{

  inline GenVerticesC mk_const(const GenVertices& gvs) {
    GenVerticesC rtn;
    rtn.reserve(gvs.size());
    BOOST_FOREACH (const HepMC::GenVertex* gv, gvs) {
      rtn.push_back(gv);
    }
    return rtn;
  }

  inline GenVertices mk_unconst(const GenVerticesC& const_gps) {
    GenVertices rtn;
    rtn.reserve(const_gps.size());
    BOOST_FOREACH (const HepMC::GenVertex* const_gp, const_gps) {
      rtn.push_back(const_cast<HepMC::GenVertex*>(const_gp));
    }
    return rtn;
  }

  //@}


}

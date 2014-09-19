/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// JetTrimmer.cxx

#include "JetRec/JetTrimmer.h"
#include <iomanip>
#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/Selector.hh"
#include "fastjet/tools/Filter.hh"
#include "JetEDM/PseudoJetVector.h"

using std::setw;
using fastjet::PseudoJet;
using xAOD::JetContainer;

//**********************************************************************

JetTrimmer::JetTrimmer(std::string name)
: AsgTool(name), m_bld("") {
  declareProperty("RClus", m_rclus =0.3);
  declareProperty("PtFrac", m_ptfrac =0.03);
  declareProperty("JetBuilder", m_bld);
}

//**********************************************************************

JetTrimmer::~JetTrimmer() {
}

//**********************************************************************

StatusCode JetTrimmer::initialize() {
  if ( m_rclus < 0.0 || m_rclus > 10.0 ) {
    ATH_MSG_WARNING("Invalid value for RClus " << m_rclus);
  }
  if ( m_ptfrac < 0.0 || m_ptfrac > 1.0 ) {
    ATH_MSG_WARNING("Invalid value for PtFrac " << m_ptfrac);
  }
  if ( m_bld.empty() ) {
    ATH_MSG_ERROR("Unable top retrieve jet builder.");
  }
  return StatusCode::SUCCESS;
}

//**********************************************************************

int JetTrimmer::groom(const xAOD::Jet& jin, xAOD::JetContainer& jets) const {
  const PseudoJet* ppjin = jin.getPseudoJet();
  if ( ppjin == 0 ) {
    ATH_MSG_ERROR("Jet does not have a pseudojet.");
  }
  // Trim.
  fastjet::Filter trimmer(fastjet::JetDefinition(fastjet::kt_algorithm, m_rclus),
                          fastjet::SelectorPtFractionMin(m_ptfrac));
  PseudoJet pjtrim = trimmer(*ppjin);
  ATH_MSG_VERBOSE("   Input cluster sequence: " << ppjin->associated_cluster_sequence());
  ATH_MSG_VERBOSE(" Trimmed cluster sequence: " << pjtrim.associated_cluster_sequence());
  int nptrim = pjtrim.pieces().size();
  // Add jet to collection.
  xAOD::Jet* pjet = m_bld->add(pjtrim, jets, &jin);
  pjet->setAttribute<int>("TransformType", xAOD::JetTransform::Trim);
  pjet->setAttribute("RClus", m_rclus);
  pjet->setAttribute("PtFrac", m_ptfrac);
  ATH_MSG_DEBUG("Properties after trimming:");
  ATH_MSG_DEBUG("   ncon: " << pjtrim.constituents().size() << "/"
                            << ppjin->constituents().size());
  ATH_MSG_DEBUG("   nsub: " << nptrim);
  if ( pjet == 0 ) {
    ATH_MSG_ERROR("Unable to add jet to container");
  } else {
    ATH_MSG_DEBUG("Added jet to container.");
  }
  return 0;
}

//**********************************************************************

void JetTrimmer::print() const {
  ATH_MSG_INFO("     Recluster R: " << m_rclus);
  ATH_MSG_INFO("  pT faction min: " << m_ptfrac);
  ATH_MSG_INFO("  Jet builder: " << m_bld.name());
}

//**********************************************************************

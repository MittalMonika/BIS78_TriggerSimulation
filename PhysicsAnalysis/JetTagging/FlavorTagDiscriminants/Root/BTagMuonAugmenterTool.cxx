/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/BTagMuonAugmenterTool.h"
#include "FlavorTagDiscriminants/BTagMuonAugmenter.h"

namespace FlavorTagDiscriminants {

  BTagMuonAugmenterTool::BTagMuonAugmenterTool(const std::string& name):
    asg::AsgTool(name),
    m_aug(nullptr)
  {
    declareProperty("MuonAssociationName", m_muonAssociationName="Muons");
    declareProperty("MuonMinDR", m_muonMinDR=defaults::MUON_MIN_DR);
    declareProperty("MuonMinpT", m_muonMinpT=defaults::MUON_MIN_PT);
    declareProperty("schema", m_schema = "WINTER_2018");
    declareProperty("flipTagConfig", m_flipTagConfig = "STANDARD");
  }
  BTagMuonAugmenterTool::~BTagMuonAugmenterTool() {}

  StatusCode BTagMuonAugmenterTool::initialize() {
    m_aug.reset(new BTagMuonAugmenter(
                  m_muonAssociationName,
                  m_muonMinDR,
                  m_muonMinpT,
                  enumFromString(m_schema),
                  flipTagConfigFromString(m_flipTagConfig)));
    return StatusCode::SUCCESS;
  }

  int BTagMuonAugmenterTool::modifyJet(xAOD::Jet& jet) const {
    m_aug->augment(jet);
    return 0; // 0 means success
  }

}
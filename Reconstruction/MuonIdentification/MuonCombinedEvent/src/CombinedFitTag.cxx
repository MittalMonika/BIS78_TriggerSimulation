/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCombinedEvent/CombinedFitTag.h"
#include "TrkTrack/Track.h"
#include <iostream>

namespace MuonCombined {

  CombinedFitTag::CombinedFitTag( xAOD::Muon::Author author, const MuonCandidate& muonCandidate, 
				  const Trk::TrackScore& score ) : 
    TagBase(author,TagBase::Type::Combined), m_muonCandidate(&muonCandidate), 
    m_trackScore(score),
    m_MELink(ElementLink<TrackCollection>()),
    m_matchChi2(0.0), 
    m_matchDoF(0), 
    m_matchProb(0.0),
    m_momentumBalanceSignificance(0.0)
  {}

  CombinedFitTag::~CombinedFitTag() {
  }

  std::string CombinedFitTag::toString() const { 
    std::ostringstream sout;
    sout << name() << " TrackScore " << trackScore() << " matchChi2 " << matchChi2(); 
    return sout.str();
  }

}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

#ifndef MUONDECAYTRUTHTRAJECTORYBUILDER_H
#define MUONDECAYTRUTHTRAJECTORYBUILDER_H

#include "TrkToolInterfaces/ITruthTrajectoryBuilder.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "AtlasHepMC/GenVertex.h"

namespace Muon {
  
  class MuonDecayTruthTrajectoryBuilder: virtual public Trk::ITruthTrajectoryBuilder,public AthAlgTool {
  public:
    MuonDecayTruthTrajectoryBuilder(const std::string& type, const std::string& name, const IInterface* parent);

    virtual StatusCode initialize();
    
    void buildTruthTrajectory(TruthTrajectory *result, const HepMC::GenParticle *input) const;
    
    /** Returns an umambiguous mother of the truth particle on a TruthTrajectory, or 0.
     * Implemented via truthTrajectoryCuts().
     */
    const HepMC::GenParticle* getMother(const HepMC::GenParticle* particle) const;

    /** Returns an umambiguous daughter of the truth particle on a TruthTrajectory, or 0.
     * This assumes no one mother can correspond to no more than one daughter.
     * Implemented via truthTrajectoryCuts().
     */
    const HepMC::GenParticle* getDaughter(const HepMC::GenParticle* particle) const;

  private:

    /** Return type for the next method */
    typedef std::pair<const HepMC::GenParticle*, const HepMC::GenParticle*> MotherDaughter;

    /** 
     * Decides if the vertex connects two particles on the same
     * TruthTrajectory.  Returns the (mother,daughter) pair,
     * where either of the two can be NULL.
     *
     * This interface implies that there is one-to-one correspondence
     * between mother and daughter particles; i.e. TruthTrajectories
     * are not allowed to branch.
     */
    MotherDaughter truthTrajectoryCuts(const HepMC::GenVertex *vtx) const;

    // mutable bool m_isDecayIntoTwoMuons; // Really don't think this does anything? EJWM.
  };
  
} // end namespace Trk

#endif

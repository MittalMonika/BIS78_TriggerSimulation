/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_GEANT4EVENT_ISFG4HELPER_H
#define ISF_GEANT4EVENT_ISFG4HELPER_H

// Barcode includes
#include "BarcodeEvent/Barcode.h"

// Generators
#include "GeneratorObjects/HepMcParticleLink.h"

// ISF Includes
#include "ISF_Event/ISFParticle.h"

// MCTruth includes
#include "MCTruth/VTrackInformation.h"

#include <memory>

// forward declarations
namespace ISF {
  class TruthBinding;
}
class VTrackInformation;
class TrackInformation;
class EventInformation;
class G4Track;

namespace iGeant4 {

  class ISFG4Helper {

  public:
    ISFG4Helper() = delete;

    /** convert the given G4Track into an ISFParticle */
    static std::unique_ptr<ISF::ISFParticle> convertG4TrackToISFParticle(const G4Track& aTrack,
                                                                         const ISF::ISFParticle& parent,
                                                                         ISF::TruthBinding* truth = nullptr,
                                                                         const HepMcParticleLink * partLink = nullptr);

    /** get the ParticleBarcode corresponding to the given G4Track */
    static Barcode::ParticleBarcode getParticleBarcode(const G4Track &aTrack);

    /** return a valid UserInformation object of the G4Track for use within the ISF */
    static VTrackInformation* getISFTrackInfo(const G4Track& aTrack);

    /** link the given G4Track to the given ISFParticle */
    static void setG4TrackInfoFromBaseISFParticle( G4Track& aTrack,
                                                   const ISF::ISFParticle& baseIsp,
                                                   bool setReturnToISF=false );

    /** attach a new TrackInformation object to the given new (!) G4Track
     *  (the G4Track must not have a UserInformation object attached to it) */
    static TrackInformation* attachTrackInfoToNewG4Track( G4Track& aTrack,
                                                          const ISF::ISFParticle& baseIsp,
                                                          TrackClassification classification,
                                                          HepMC::GenParticle *nonRegeneratedTruthParticle = nullptr);

    /** return pointer to current EventInformation */
    static EventInformation* getEventInformation();

  private:

  };
}

#endif // ISF_GEANT4EVENT_ISFG4HELPER_H

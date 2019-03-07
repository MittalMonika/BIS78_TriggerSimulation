#include "G4UserActions/G4SimTimerTool.h"
#include "G4UserActions/G4TrackCounterTool.h"
#include "G4UserActions/LooperKillerTool.h"
// This is a weird one... Geant4 declares 'times' as 'otimes' in some of its
// headers. That can have a catastrophic interference with how Boost uses the
// `times` variable itself. This next line just gets rid of one such
// catastrophic interference. Though this is super fragile...
#undef times
#include "G4UserActions/CosmicPerigeeActionTool.h"
#include "G4UserActions/MomentumConservationTool.h"
#include "G4UserActions/FastIDKillerTool.h"
#include "G4UserActions/HIPKillerTool.h"
#include "G4UserActions/LengthIntegratorTool.h"
#include "G4UserActions/HIPLArVolumeAcceptTool.h"
#include "G4UserActions/HitWrapperTool.h"
#include "G4UserActions/PhotonKillerTool.h"
#include "G4UserActions/ScoringVolumeTrackKillerTool.h"
#include "G4UserActions/StoppedParticleActionTool.h"
#include "G4UserActions/FluxRecorderTool.h"
#include "G4UserActions/ScoringPlaneTool.h"
#include "G4UserActions/RadiationMapsMakerTool.h"
#include "G4UserActions/RadLengthActionTool.h"
#include "G4UserActions/LooperThresholdSetTool.h"
#include "../TestActionTool.h"

DECLARE_COMPONENT( G4UA::G4SimTimerTool )
DECLARE_COMPONENT( G4UA::G4TrackCounterTool )
DECLARE_COMPONENT( G4UA::LooperKillerTool )
DECLARE_COMPONENT( G4UA::CosmicPerigeeActionTool )
DECLARE_COMPONENT( G4UA::MomentumConservationTool )
DECLARE_COMPONENT( G4UA::FastIDKillerTool )
DECLARE_COMPONENT( G4UA::HIPKillerTool )
DECLARE_COMPONENT( G4UA::LengthIntegratorTool )
DECLARE_COMPONENT( G4UA::HIPLArVolumeAcceptTool )
DECLARE_COMPONENT( G4UA::HitWrapperTool )
DECLARE_COMPONENT( G4UA::PhotonKillerTool )
DECLARE_COMPONENT( G4UA::ScoringVolumeTrackKillerTool )
DECLARE_COMPONENT( G4UA::StoppedParticleActionTool )
DECLARE_COMPONENT( G4UA::FluxRecorderTool )
DECLARE_COMPONENT( G4UA::ScoringPlaneTool )
DECLARE_COMPONENT( G4UA::RadiationMapsMakerTool )
DECLARE_COMPONENT( G4UA::RadLengthActionTool )
DECLARE_COMPONENT( G4UA::LooperThresholdSetTool )
DECLARE_COMPONENT( G4UA::TestActionTool )

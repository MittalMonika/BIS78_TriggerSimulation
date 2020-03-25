#include "GaudiKernel/DeclareFactoryEntries.h"
//#include "ParticleJetTools/JetTrackTruthMatching.h"
#include "ParticleJetTools/JetQuarkLabel.h"
#include "ParticleJetTools/JetConeLabeling.h"
//#include "ParticleJetTools/JetQGPartonLabel.h"
#include "ParticleJetTools/ParticleToJetAssociator.h"
//#include "ParticleJetTools/FindLeptonTruth.h"
#include "ParticleJetTools/CopyFlavorLabelTruthParticles.h"
#include "ParticleJetTools/CopyBosonTopLabelTruthParticles.h"
#include "ParticleJetTools/CopyTruthPartons.h"
#include "ParticleJetTools/JetPartonTruthLabel.h"
#include "ParticleJetTools/CopyTruthJetParticles.h"
#include "ParticleJetTools/ParticleJetDeltaRLabelTool.h"
#include "ParticleJetTools/JetParticleAssociationAlg.h"
#include "ParticleJetTools/JetParticleShrinkingConeAssociation.h"
#include "ParticleJetTools/JetParticleCenterOfMassAssociation.h"
#include "src/JetModifierAlg.h"
#include "src/JetAssocConstAlg.h"
#include "ParticleJetTools/JetParticleFixedConeAssociation.h"

using namespace Analysis;

/* DECLARE_NAMESPACE_TOOL_FACTORY( Analysis, JetTrackTruthMatching ) */
DECLARE_NAMESPACE_TOOL_FACTORY( Analysis, JetQuarkLabel )
DECLARE_NAMESPACE_TOOL_FACTORY( Analysis, JetConeLabeling )
DECLARE_NAMESPACE_TOOL_FACTORY( Analysis, JetPartonTruthLabel )
/* DECLARE_NAMESPACE_TOOL_FACTORY( Analysis, JetQGPartonLabel ) */
DECLARE_NAMESPACE_TOOL_FACTORY( Analysis, ParticleToJetAssociator )
/* DECLARE_NAMESPACE_TOOL_FACTORY( Analysis, FindLeptonTruth ) */
/// @todo Convert to namespace, tool, etc?
DECLARE_TOOL_FACTORY( CopyFlavorLabelTruthParticles )
DECLARE_TOOL_FACTORY( CopyBosonTopLabelTruthParticles )
DECLARE_TOOL_FACTORY( CopyTruthPartons )
DECLARE_TOOL_FACTORY( CopyTruthJetParticles )
DECLARE_TOOL_FACTORY( ParticleJetDeltaRLabelTool )
DECLARE_TOOL_FACTORY( JetParticleShrinkingConeAssociation )
DECLARE_TOOL_FACTORY( JetParticleCenterOfMassAssociation )
DECLARE_TOOL_FACTORY( JetParticleFixedConeAssociation )
DECLARE_ALGORITHM_FACTORY( JetParticleAssociationAlg )
DECLARE_ALGORITHM_FACTORY( JetModifierAlg )
DECLARE_ALGORITHM_FACTORY( JetAssocConstAlg )

/** factory entries need to have the name of the package */
DECLARE_FACTORY_ENTRIES( ParticleJetTools ) {
/*    DECLARE_NAMESPACE_TOOL( Analysis, JetTrackTruthMatching ) */
    DECLARE_NAMESPACE_TOOL( Analysis, JetQuarkLabel )
    DECLARE_NAMESPACE_TOOL( Analysis, JetConeLabeling )
    DECLARE_NAMESPACE_TOOL( Analysis, JetPartonTruthLabel )
/*    DECLARE_NAMESPACE_TOOL( Analysis, JetQGPartonLabel ) */
    DECLARE_NAMESPACE_TOOL( Analysis, ParticleToJetAssociator )
/*    DECLARE_NAMESPACE_TOOL( Analysis, FindLeptonTruth ) */
    /// @todo Convert to namespace, tool, etc?
    DECLARE_TOOL( CopyFlavorLabelTruthParticles )
    DECLARE_TOOL( CopyBosonTopLabelTruthParticles )
    DECLARE_TOOL( CopyTruthPartons )
    DECLARE_TOOL( CopyTruthJetParticles )
    DECLARE_TOOL( ParticleJetDeltaRLabelTool )
    DECLARE_TOOL( JetParticleShrinkingConeAssociation )
    DECLARE_TOOL( JetParticleCenterOfMassAssociation )
    DECLARE_TOOL( JetParticleFixedConeAssociation )
    DECLARE_ALGORITHM( JetParticleAssociationAlg )
    DECLARE_ALGORITHM( JetModifierAlg )
    DECLARE_ALGORITHM( JetAssocConstAlg )
}

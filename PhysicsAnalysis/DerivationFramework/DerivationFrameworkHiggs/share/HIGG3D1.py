#********************************************************************
# HIGG3D1.py
# reductionConf flag HIGG3D1 in Reco_tf.py
#********************************************************************

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkFlavourTag.FlavourTagCommon import *
from DerivationFrameworkFlavourTag.HbbCommon import *
from DerivationFrameworkCore.WeightMetadata import *

from JetRec.JetRecStandard import jtm

from DerivationFrameworkHiggs.TruthCategories import *

from AthenaCommon.GlobalFlags import globalflags

#====================================================================
# SET UP STREAM
#====================================================================
streamName = derivationFlags.WriteDAOD_HIGG3D1Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_HIGG3D1Stream )
HIGG3D1Stream = MSMgr.NewPoolRootStream( streamName, fileName )
HIGG3D1Stream.AcceptAlgs(["HIGG3D1Kernel_skimming"])

## Prepare thinning service and add trigger chains for TrigNavigation thinning
from DerivationFrameworkCore.ThinningHelper import ThinningHelper
HIGG3D1ThinningHelper = ThinningHelper( "HIGG3D1ThinningHelper" )
HIGG3D1ThinningHelper.TriggerChains = "HLT_mu.* | HLT_e.* | HLT_2e.* | HLT_2mu.*"
HIGG3D1ThinningHelper.AppendToStream( HIGG3D1Stream )

#====================================================================
# THINNING TOOLS
#====================================================================

thinningTools=[]

# MET/Jet tracks
thinning_expression = "(InDetTrackParticles.pt > 0.5*GeV) && (InDetTrackParticles.numberOfPixelHits > 0) && (InDetTrackParticles.numberOfSCTHits > 5) && (abs(DFCommonInDetTrackZ0AtPV) < 1.5)"
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
HIGG3D1MetTPThinningTool = DerivationFramework__TrackParticleThinning(name                   = "HIGG3D1MetTPThinningTool",
                                                                      ThinningService        = HIGG3D1ThinningHelper.ThinningSvc(),
                                                                      SelectionString        = thinning_expression,
                                                                      InDetTrackParticlesKey = "InDetTrackParticles",
                                                                      ApplyAnd               = True)
ToolSvc += HIGG3D1MetTPThinningTool
thinningTools.append(HIGG3D1MetTPThinningTool)

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__JetTrackParticleThinning
HIGG3D1JetTPThinningTool = DerivationFramework__JetTrackParticleThinning( name          = "HIGG3D1JetTPThinningTool",
                                                                ThinningService         = HIGG3D1ThinningHelper.ThinningSvc(),
                                                                JetKey                  = "AntiKt4EMTopoJets",
                                                                InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                ApplyAnd                = True)
ToolSvc += HIGG3D1JetTPThinningTool
thinningTools.append(HIGG3D1JetTPThinningTool)

# Tracks associated with Muons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
HIGG3D1MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(name                    = "HIGG3D1MuonTPThinningTool",
                                                                           ThinningService         = HIGG3D1ThinningHelper.ThinningSvc(),
                                                                           MuonKey                 = "Muons",
                                                                           InDetTrackParticlesKey  = "InDetTrackParticles")

ToolSvc += HIGG3D1MuonTPThinningTool
thinningTools.append(HIGG3D1MuonTPThinningTool)

# Tracks associated with Electrons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
HIGG3D1ElectronTPThinningTool = DerivationFramework__EgammaTrackParticleThinning(name                    = "HIGG3D1ElectronTPThinningTool",
                                                                                 ThinningService         = HIGG3D1ThinningHelper.ThinningSvc(),
                                                                                 SGKey                   = "Electrons",
                                                                                 InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += HIGG3D1ElectronTPThinningTool
thinningTools.append(HIGG3D1ElectronTPThinningTool)

# Tracks themselves
HIGG3D1TPThinningTool = DerivationFramework__TrackParticleThinning(name                    = "HIGG3D1TPThinningTool",
                                                                   ThinningService         = HIGG3D1ThinningHelper.ThinningSvc(),
                                                                   SelectionString         = "abs( DFCommonInDetTrackZ0AtPV * sin(InDetTrackParticles.theta)) < 3.0",
                                                                   InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += HIGG3D1TPThinningTool
thinningTools.append(HIGG3D1TPThinningTool)

# Truth particles
truth_cond1 = "((abs(TruthParticles.pdgId) >= 23) && (abs(TruthParticles.pdgId) <= 25))" # W, Z and Higgs
truth_cond2 = "((abs(TruthParticles.pdgId) >= 11) && (abs(TruthParticles.pdgId) <= 16))" # Leptons
truth_cond3 = "(abs(TruthParticles.pdgId) == 6)"  # Top quark
truth_cond4 = "((abs(TruthParticles.pdgId) == 22) && (TruthParticles.pt > 5*GeV))" # Photon

truth_expression = truth_cond1+' || '+truth_cond2 +' || '+truth_cond3 +' || '+truth_cond4

from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__GenericTruthThinning
HIGG3D1TruthThinningTool = DerivationFramework__GenericTruthThinning(name                    = "HIGG3D1TruthThinningTool", 
                                                                     ThinningService         = HIGG3D1ThinningHelper.ThinningSvc(),
                                                                     ParticleSelectionString = truth_expression,
                                                                     PreserveDescendants     = True,
                                                                     PreserveAncestors      = True,
                                                                     SimBarcodeOffset = DerivationFrameworkSimBarcodeOffset)

if globalflags.DataSource()=='geant4':
    ToolSvc += HIGG3D1TruthThinningTool                                                                      
    thinningTools.append(HIGG3D1TruthThinningTool)


# Calo cluster thinning
from DerivationFrameworkCalo.DerivationFrameworkCaloConf import DerivationFramework__CaloClusterThinning
from DerivationFrameworkCalo.DerivationFrameworkCaloConf import DerivationFramework__JetCaloClusterThinning

# Calo Clusters associated with Electrons
HIGG3D1ElectronCCThinningTool = DerivationFramework__CaloClusterThinning( name                  = "HIGG3D1ElectronCCThinningTool",
                                                                          ThinningService       = HIGG3D1ThinningHelper.ThinningSvc(),
                                                                          SGKey                 = "Electrons",
                                                                          TopoClCollectionSGKey = "CaloCalTopoClusters",
                                                                          SelectionString       = "Electrons.pt > 7*GeV",
                                                                          ConeSize              = 0.2)
ToolSvc += HIGG3D1ElectronCCThinningTool
thinningTools.append(HIGG3D1ElectronCCThinningTool)

# Calo Clusters associated with Muons
HIGG3D1MuonCCThinningTool = DerivationFramework__CaloClusterThinning( name                  = "HIGG3D1MuonCCThinningTool",
                                                                      ThinningService       = HIGG3D1ThinningHelper.ThinningSvc(),
                                                                      SGKey                 = "Muons",
                                                                      TopoClCollectionSGKey = "CaloCalTopoClusters",
                                                                      SelectionString       = "Muons.pt > 6*GeV",
                                                                      ConeSize              = 0.2)
ToolSvc += HIGG3D1MuonCCThinningTool
thinningTools.append(HIGG3D1MuonCCThinningTool)

# Seems to no longer be needed/causes errors

# Calo Clusters associated with AntiKt4EM Jets
#HIGG3D1AntiKt4EMCCThinningTool = DerivationFramework__JetCaloClusterThinning(name                    = "HIGG3D1AntiKt4EMCCThinningTool",
#                                                                             ThinningService         = HIGG3D1ThinningHelper.ThinningSvc(),
#                                                                             SGKey                   = "AntiKt4EMTopoJets",
#                                                                             TopoClCollectionSGKey   = "CaloCalTopoClusters",
#                                                                             SelectionString         = "AntiKt4EMTopoJets.DFCommonJets_Calib_pt > 15*GeV")
#ToolSvc += HIGG3D1AntiKt4EMCCThinningTool
#thinningTools.append(HIGG3D1AntiKt4EMCCThinningTool)

###############################################################

#====================================================================
# SKIMMING TOOL 
#====================================================================
electronIDRequirements = '(Electrons.DFCommonElectronsLHVeryLoose)'
electronRequirements = '(Electrons.pt > 7*GeV) && (abs(Electrons.eta) < 2.6) && '+electronIDRequirements
leadElectron = electronRequirements + ' && (Electrons.pt > 17*GeV)'
muonsRequirements = '(Muons.pt > 7*GeV) && (Muons.DFCommonGoodMuon)'
leadMuon = muonsRequirements + ' && (Muons.pt > 17*GeV)'

eeSelection = '((count('+electronRequirements+') >= 2) && (count('+leadElectron+') >= 1))'
mmSelection = '((count('+muonsRequirements+') >= 2) && (count('+leadMuon+') >= 1))'

emSelection = '(((count('+electronRequirements+') >= 1) && (count('+muonsRequirements+') >= 1)) && ((count('+leadElectron+') >= 1) || (count('+leadMuon+') >= 1)))'

expression = eeSelection+' || '+mmSelection+' || '+emSelection

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
HIGG3D1SkimmingTool = DerivationFramework__xAODStringSkimmingTool( name = "HIGG3D1SkimmingTool1",
                                                                    expression = expression)
ToolSvc += HIGG3D1SkimmingTool

higg3d1Seq = CfgMgr.AthSequencer("HIGG3d1Sequence")
higg3d1PreSeq = CfgMgr.AthSequencer("HIGG3d1PreSelectionSequence")

#====================================================================
# RESTORE JET COLLECTIONS REMOVED BETWEEN r20 AND r21
#====================================================================
OutputJets["HIGG3D1"] = ["AntiKt4EMPFlowJets",
                         "AntiKtVR30Rmax4Rmin02TrackJets"]

reducedJetList = ["AntiKt2PV0TrackJets",
                  "AntiKt4PV0TrackJets",
                  "AntiKt10LCTopoJets" # Needed for Ghost association
                 ]
if jetFlags.useTruth:
   reducedJetList += ["AntiKt4TruthJets", "AntiKt4TruthWZJets"]
replaceAODReducedJets(reducedJetList, higg3d1Seq,"HIGG3D1")

addDefaultTrimmedJets(higg3d1Seq,"HIGG3D1")

#====================================================================
# Create variable-R trackjets and dress AntiKt10LCTopo with ghost VR-trkjet
#====================================================================
addVRJets(higg3d1Seq)

#addVRJets(higg3d1Seq, "AntiKtVR30Rmax4Rmin02Track", "GhostVR30Rmax4Rmin02TrackJet",
#          VRJetAlg="AntiKt", VRJetRadius=0.4, VRJetInputs="pv0track",
#          ghostArea = 0 , ptmin = 2000, ptminFilter = 7000,
#          variableRMinRadius = 0.02, variableRMassScale = 30000, calibOpt = "none")

#===================================================================
# B-TAGGING
#===================================================================
from BTagging.BTaggingFlags import BTaggingFlags

# alias for VR
BTaggingFlags.CalibrationChannelAliases += ["AntiKtVR30Rmax4Rmin02Track->AntiKtVR30Rmax4Rmin02Track,AntiKt4EMTopo"]

#===================================================================
# Hbb tagger
#===================================================================
addHbbTagger(higg3d1Seq, ToolSvc)

# Add flavor tagging to the PFlow Jet collections
FlavorTagInit(JetCollections = ['AntiKt4EMPFlowJets'], Sequencer = higg3d1Seq)

#====================================================================
# Add non-prompt lepton tagging
#====================================================================
# import the JetTagNonPromptLepton config and add to the private sequence
import JetTagNonPromptLepton.JetTagNonPromptLeptonConfig as JetTagConfig
higg3d1Seq += JetTagConfig.GetDecoratePromptLeptonAlgs()


#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM
#=======================================
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
higg3d1PreSeq += CfgMgr.DerivationFramework__DerivationKernel("HIGG3D1Kernel_skimming",
                                                           SkimmingTools = [HIGG3D1SkimmingTool],
                                                           )


DerivationFrameworkJob += higg3d1PreSeq
higg3d1PreSeq += higg3d1Seq

higg3d1Seq += CfgMgr.DerivationFramework__DerivationKernel("HIGG3D1Kernel_thinning",
                                                           ThinningTools = thinningTools
                                                           )


applyJetCalibration_xAODColl("AntiKt4EMTopo", higg3d1Seq)

#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
HIGG3D1SlimmingHelper = SlimmingHelper("HIGG3D1SlimmingHelper")
from DerivationFrameworkHiggs.HIGG3D1ExtraContent import *

HIGG3D1SlimmingHelper.SmartCollections = ["Electrons",
                                          "Photons",
                                          "Muons",
                                          "TauJets",
                                          "MET_Reference_AntiKt4EMTopo",
                                          "MET_Reference_AntiKt4LCTopo",
                                          "AntiKt4EMTopoJets",
                                          "AntiKt4EMPFlowJets",
                                          "AntiKt4LCTopoJets",
                                          "BTagging_AntiKt4EMTopo",
                                          "BTagging_AntiKt4EMPFlow",
                                          "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                          "InDetTrackParticles",
                                          "PrimaryVertices"]

HIGG3D1SlimmingHelper.ExtraVariables = list(HIGG3D1ExtraVariables)
HIGG3D1SlimmingHelper.AllVariables = list(HIGG3D1ExtraContainers)
HIGG3D1SlimmingHelper.ExtraVariables += JetTagConfig.GetExtraPromptVariablesForDxAOD()
HIGG3D1SlimmingHelper.AppendToDictionary = {'BTagging_AntiKt4EMPFlow':'xAOD::BTaggingContainer',
                                            'BTagging_AntiKt4EMPFlowAux':'xAOD::BTaggingAuxContainer',
                                            'AntiKtVR30Rmax4Rmin02Track':'xAOD::JetContainer',
                                            'AntiKtVR30Rmax4Rmin02TrackAux':'xAOD::JetAuxContainer',
                                            'BTagging_AntiKtVR30Rmax4Rmin02Track':'xAOD::BTaggingContainer',
                                            'BTagging_AntiKtVR30Rmax4Rmin02TrackAux':'xAOD::BTaggingAuxContainer'
                                           }

if globalflags.DataSource()=='geant4':
    HIGG3D1SlimmingHelper.SmartCollections += ["AntiKt4TruthJets",
                                               "AntiKt4TruthWZJets"]

    # Add special truth containers
    from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents
    addStandardTruthContents()
    HIGG3D1SlimmingHelper.StaticContent = [ "xAOD::TruthParticleContainer#TruthMuons",
                                            "xAOD::TruthParticleAuxContainer#TruthMuonsAux.",
                                            "xAOD::TruthParticleContainer#TruthElectrons",
                                            "xAOD::TruthParticleAuxContainer#TruthElectronsAux.",
                                            "xAOD::TruthParticleContainer#TruthPhotons",
                                            "xAOD::TruthParticleAuxContainer#TruthPhotonsAux.",
                                            "xAOD::TruthParticleContainer#TruthNeutrinos",
                                            "xAOD::TruthParticleAuxContainer#TruthNeutrinosAux." ]
    HIGG3D1SlimmingHelper.AllVariables += list(HIGG3D1ExtraTruthContainers)
    HIGG3D1SlimmingHelper.ExtraVariables += list(HIGG3D1ExtraTruthVariables)

# Add Trigger content
HIGG3D1SlimmingHelper.IncludeMuonTriggerContent = True
HIGG3D1SlimmingHelper.IncludeEGammaTriggerContent = True

# Add MET to output stream
addMETOutputs(HIGG3D1SlimmingHelper, ["AntiKt4EMPFlow", "Track"], ["AntiKt4EMPFlow", "AntiKt4EMTopo"])
addJetOutputs(HIGG3D1SlimmingHelper, ["HIGG3D1"],
                                     [], # smart collections list
                                     ["AntiKt2PV0TrackJets",
                                      "AntiKt4TruthJets",
                                      "AntiKt4TruthWZJets",
                                      "AntiKt4EMPFlowJets",
                                      "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
                                      "AntiKt10LCTopoJets",
                                      "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"] #veto list
                                     )

HIGG3D1SlimmingHelper.AppendContentToStream(HIGG3D1Stream)

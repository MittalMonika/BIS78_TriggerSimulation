#====================================================================
# FTAG2.py
# It requires the reductionConf flag FTAG2 in Reco_tf.py
#====================================================================

# Set up common services and job object.
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkJetEtMiss.PFlowCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
if globalflags.DataSource()!='data': from DerivationFrameworkMCTruth.MCTruthCommon import *

from DerivationFrameworkCore.ThinningHelper import ThinningHelper
from DerivationFrameworkExotics.JetDefinitions import *
from JetRec.JetRecStandard import jtm
from TrkVertexFitterUtils.TrkVertexFitterUtilsConf import Trk__TrackToVertexIPEstimator
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
from DerivationFrameworkFlavourTag.HbbCommon import *
from DerivationFrameworkFlavourTag.FlavourTagCommon import *
from DerivationFrameworkFlavourTag.FlavourTagAllVariables import FTAllVars_bjetTriggerVtx
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackToVertexWrapper
from DerivationFrameworkJetEtMiss.AntiKt4EMTopoJetsCPContent import AntiKt4EMTopoJetsCPContent

#====================================================================
# SKIMMING TOOLS
#====================================================================
FTAG2StringSkimmingTool = DerivationFramework__xAODStringSkimmingTool(name = "FTAG2StringSkimmingTool",
                          expression = 'count( (Muons.pt > 18*GeV) && (Muons.DFCommonGoodMuon) ) + count(( Electrons.pt > 18*GeV) && ((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))) >= 2 ')

ToolSvc += FTAG2StringSkimmingTool
print FTAG2StringSkimmingTool

#====================================================================                                                                                                                   
# AUGMENTATION TOOLS
# ( AUGMENTATION = adding information to the output DxAOD that is not found in the input file )
#====================================================================  

#make IPE tool for TrackToVertexWrapper
FTAG2IPETool = Trk__TrackToVertexIPEstimator(name = "FTAG2IPETool")
ToolSvc += FTAG2IPETool
print FTAG2IPETool

#Add unbiased track parameters to track particles
FTAG2TrackToVertexWrapper= DerivationFramework__TrackToVertexWrapper(name = "FTAG2TrackToVertexWrapper",
        TrackToVertexIPEstimator = FTAG2IPETool,
        DecorationPrefix = "FTAG2",
        ContainerName = "InDetTrackParticles")
ToolSvc += FTAG2TrackToVertexWrapper
print FTAG2TrackToVertexWrapper

#====================================================================
# CREATE PRIVATE SEQUENCE
#====================================================================

FTAG2Seq = CfgMgr.AthSequencer("FTAG2Sequence");
DerivationFrameworkJob += FTAG2Seq

#====================================================================
# Special Jets
#====================================================================
addStandardJets("AntiKt", 0.4, "PV0Track", 2000, mods="track_ungroomed", algseq=FTAG2Seq, outputGroup="FTAG2")
addStandardJets("AntiKt", 0.2, "PV0Track", 2000, mods="track_ungroomed", algseq=FTAG2Seq, outputGroup="FTAG2")
addStandardJets("AntiKt", 1.0, "LCTopo", mods="lctopo_ungroomed", ptmin=40000, ptminFilter=50000, calibOpt="none", algseq=FTAG2Seq, outputGroup="FTAG2")
if globalflags.DataSource()!='data': addStandardJets("AntiKt", 0.4, "Truth", 5000, mods="truth_ungroomed", algseq=FTAG2Seq, outputGroup="FTAG2")
addDefaultTrimmedJets(FTAG2Seq,"FTAG2",dotruth=True)
# Create variable-R trackjets and dress AntiKt10LCTopo with ghost VR-trkjet 
#addVRJets(FTAG2Seq, "AntiKtVR30Rmax4Rmin02Track", "GhostVR30Rmax4Rmin02TrackJet",
#          VRJetAlg="AntiKt", VRJetRadius=0.4, VRJetInputs="pv0track",
#          ghostArea = 0 , ptmin = 2000, ptminFilter = 7000,
#          variableRMinRadius = 0.02, variableRMassScale = 30000, calibOpt = "none")
OutputJets["FTAG2"] = ["AntiKt4TruthJets",
                       "AntiKt4EMTopoJets",
                       #"AntiKtVR30Rmax4Rmin02TrackJets",
                       "AntiKt4PV0TrackJets",
                       "AntiKt2PV0TrackJets",
                       "AntiKt10LCTopoJets",
                       "AntiKt4EMPFlowJets"]


#===================================================================
# Run b-tagging
#===================================================================

# alias for VR
BTaggingFlags.CalibrationChannelAliases += ["AntiKtVR30Rmax4Rmin02Track->AntiKtVR30Rmax4Rmin02Track,AntiKt4EMTopo"]

FlavorTagInit(JetCollections  = ['AntiKt4EMPFlowJets',
                                 'AntiKt4EMTopoJets',
                                 'AntiKt2PV0TrackJets',
                                 #'AntiKtVR30Rmax4Rmin02TrackJets'
                                 'AntiKt4PV0TrackJets'], Sequencer = FTAG2Seq)


#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS
#====================================================================

FTAG2Seq += CfgMgr.DerivationFramework__DerivationKernel("FTAG2Kernel",
                                                         SkimmingTools = [FTAG2StringSkimmingTool],
                                                         AugmentationTools = [FTAG2TrackToVertexWrapper]
                                                         )


#====================================================================
# SET UP STREAM
#====================================================================

# The base name (DAOD_FTAG2 here) must match the string in
streamName = derivationFlags.WriteDAOD_FTAG2Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_FTAG2Stream )
FTAG2Stream = MSMgr.NewPoolRootStream( streamName, fileName )
# Only events that pass the filters listed below are written out.
# Name must match that of the kernel above
# AcceptAlgs  = logical OR of filters
# RequireAlgs = logical AND of filters
FTAG2Stream.AcceptAlgs(["FTAG2Kernel"])

#add PFlow jets
applyPFOAugmentation(FTAG2Seq)
#ReTag(['IP2D', 'IP3D', 'SV0', 'MultiSVbb1', 'MultiSVbb2', 'SV1', 'JetFitterNN', 'MV2c00', 'MV2c10', 'MV2c20', 'MV2c100', 'MV2m'],['AntiKt4EMPFlowJets'], FTAG2Seq)

FTAG2SlimmingHelper = SlimmingHelper("FTAG2SlimmingHelper")

# nb: BTagging_AntiKt4EMTopo smart collection includes both AntiKt4EMTopoJets and BTagging_AntiKt4EMTopo 
# container variables. Thus BTagging_AntiKt4EMTopo is needed in SmartCollections as well as AllVariables
FTAG2SlimmingHelper.SmartCollections = ["Electrons","Muons",
                                        "InDetTrackParticles",
                                        "AntiKt4EMTopoJets","BTagging_AntiKt4EMTopo",
                                        "MET_Reference_AntiKt4EMTopo",
                                        "AntiKt4EMPFlowJets",
                                        "MET_Reference_AntiKt4EMPFlow"]

FTAG2SlimmingHelper.AllVariables = ["AntiKt4EMTopoJets",
                                    "BTagging_AntiKt4EMTopo",
                                    "BTagging_AntiKt4EMTopoJFVtx",
                                    "BTagging_AntiKt4EMPFlow",
                                    "BTagging_AntiKt4EMPFlowJFVtx",
                                    "BTagging_AntiKt2Track",
                                    "BTagging_AntiKt2TrackJFVtx", 
                                    "BTagging_AntiKt4Track", 
                                    "BTagging_AntiKt4TrackJFVtx",
                                    "PrimaryVertices",
                                    "TruthEvents",
                                    "TruthParticles",
                                    "TruthVertices",
                                    "MET_Truth",
                                    "MET_TruthRegions"]

for FT2_bjetTriggerVtx in FTAllVars_bjetTriggerVtx:
    FTAG2SlimmingHelper.AllVariables.append(FT2_bjetTriggerVtx)

FTAG2SlimmingHelper.ExtraVariables += [AntiKt4EMTopoJetsCPContent[1].replace("AntiKt4EMTopoJetsAux","AntiKt10LCTopoJets"),
                                       "InDetTrackParticles.truthMatchProbability.x.y.z.vx.vy.vz",
                                       "InDetTrackParticles.numberOfContribPixelLayers.numberOfTRTHits.numberOfInnermostPixelLayerSharedHits.numberOfNextToInnermostPixelLayerSharedHits",
                                       "InDetTrackParticles.numberOfPixelSplitHits.numberOfInnermostPixelLayerSplitHits.numberOfNextToInnermostPixelLayerSplitHits", 
                                       "InDetTrackParticles.FTAG2_unbiased_d0.FTAG2_unbiased_z0.FTAG2_unbiased_d0Sigma.FTAG2_unbiased_z0Sigma",
                                       "InDetForwardTrackParticles.phi.qOverP.theta",
                                       "BTagging_AntiKt4EMTopoSecVtx.-vxTrackAtVertex",
                                       "BTagging_AntiKt4TrackSecVtx.-vxTrackAtVertex",
                                       "BTagging_AntiKt4EMPFlow.MV1_discriminant.MV1c_discriminant.SV1_pb.SV1_pu.IP3D_pb.IP3D_pu.MV2c10_discriminant",
                                       "AntiKt4EMPFlowJets.EMFrac.HECFrac.LArQuality.HECQuality.FracSamplingMax.NegativeE.AverageLArQF.FracSamplingMaxIndex.HadronConeExclTruthLabelID.GhostTrack",
                                       "AntiKt4EMPFlowJets.Jvt.JvtRpt.JvtJvfcorr",
                                       "AntiKt4EMPFlowJets.NumTrkPt1000.NumTrkPt500.SumPtTrkPt500.SumPtTrkPt1000"]


#----------------------------------------------------------------------
# Add needed dictionary stuff
FTAG2SlimmingHelper.AppendToDictionary = {      
  "BTagging_AntiKt4EMPFlow"                     : "xAOD::BTaggingContainer", 
  "BTagging_AntiKt4EMPFlowAux"                  : "xAOD::BTaggingAuxContainer", 
  "AntiKtVR30Rmax4Rmin02TrackJets"              :   "xAOD::JetContainer"        ,
  "AntiKtVR30Rmax4Rmin02TrackJetsAux"           :   "xAOD::JetAuxContainer"     ,
  "BTagging_AntiKtVR30Rmax4Rmin02Track"         :   "xAOD::BTaggingContainer"   ,
  "BTagging_AntiKtVR30Rmax4Rmin02TrackAux"      :   "xAOD::BTaggingAuxContainer",
  "BTagging_AntiKt2Track"                      :   "xAOD::BTaggingContainer"   ,
  "BTagging_AntiKt2TrackAux"                   :   "xAOD::BTaggingAuxContainer",
  "BTagging_AntiKt4Track"                      :   "xAOD::BTaggingContainer"   ,
  "BTagging_AntiKt4TrackAux"                   :   "xAOD::BTaggingAuxContainer",
  "BTagging_AntiKt2TrackJFVtx"                 :   "xAOD::BTagVertexContainer"   ,
  "BTagging_AntiKt2TrackJFVtxAux"              :   "xAOD::BTagVertexAuxContainer",
  "BTagging_AntiKt4TrackJFVtx"                 :   "xAOD::BTagVertexContainer"   ,
  "BTagging_AntiKt4TrackJFVtxAux"              :   "xAOD::BTagVertexAuxContainer",
  "BTagging_AntiKt2TrackSecVtx"                :   "xAOD::VertexContainer"   ,
  "BTagging_AntiKt2TrackSecVtxAux"             :   "xAOD::VertexAuxContainer",
  "BTagging_AntiKt4TrackSecVtx"                :   "xAOD::VertexContainer"   ,
  "BTagging_AntiKt4TrackSecVtxAux"             :   "xAOD::VertexAuxContainer",
}
#----------------------------------------------------------------------

addJetOutputs(FTAG2SlimmingHelper,["FTAG2"])

FTAG2SlimmingHelper.IncludeMuonTriggerContent = True
FTAG2SlimmingHelper.IncludeEGammaTriggerContent = True
FTAG2SlimmingHelper.IncludeJetTriggerContent = True
FTAG2SlimmingHelper.IncludeEtMissTriggerContent = True
FTAG2SlimmingHelper.IncludeBJetTriggerContent = True

#FTAG2 TrigNav Thinning
FTAG2ThinningHelper = ThinningHelper( "FTAG2ThinningHelper" )
FTAG2ThinningHelper.TriggerChains = 'HLT_.*b.*perf.*|HLT_mu.*|HLT_e.*'
FTAG2ThinningHelper.AppendToStream( FTAG2Stream )

FTAG2SlimmingHelper.AppendContentToStream(FTAG2Stream)

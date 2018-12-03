#====================================================================
# TOPQ1
# SINGLE TOP SELECTION
#   >=1 electron(pT>20GeV) OR
#   >=1 muon(pT>20GeV)
# reductionConf flag TOPQ1 in Reco_tf.py
#====================================================================

#====================================================================
# IMPORTS - Order Matters
#====================================================================
from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
from AthenaCommon.GlobalFlags import globalflags
DFisMC = (globalflags.DataSource()=='geant4')

# no truth info for data xAODs
if DFisMC:
  from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents
  addStandardTruthContents()

#====================================================================
# SET UP STREAM
#====================================================================
streamName = derivationFlags.WriteDAOD_TOPQ1Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_TOPQ1Stream )
TOPQ1Stream = MSMgr.NewPoolRootStream( streamName, fileName )
# Accept the most selective kernel (last one in sequence; later in derivation)
TOPQ1Stream.AcceptAlgs(["TOPQ1Kernel"])

#====================================================================
# PDF Weight Metadata
#====================================================================
if DFisMC:
  from DerivationFrameworkCore.WeightMetadata import *

#====================================================================
# TRIGGER NAVIGATION THINNING
#====================================================================
from DerivationFrameworkCore.ThinningHelper import ThinningHelper
import DerivationFrameworkTop.TOPQCommonThinning
TOPQ1ThinningHelper = ThinningHelper("TOPQ1ThinningHelper")
TOPQ1ThinningHelper.TriggerChains =  DerivationFrameworkTop.TOPQCommonThinning.TOPQTriggerChains('leptonicTriggers' if globalflags.DataSource()!='geant4' else 'allTriggers')
TOPQ1ThinningHelper.AppendToStream(TOPQ1Stream)

#====================================================================
# SKIMMING TOOLS
#====================================================================
import DerivationFrameworkTop.TOPQCommonSelection
skimmingTools_lep = DerivationFrameworkTop.TOPQCommonSelection.setup_lep('TOPQ1', ToolSvc)
skimmingTools_jet = DerivationFrameworkTop.TOPQCommonSelection.setup_jet('TOPQ1', ToolSvc)

#====================================================================
# THINNING TOOLS
#====================================================================
import DerivationFrameworkTop.TOPQCommonThinning
thinningTools = DerivationFrameworkTop.TOPQCommonThinning.setup('TOPQ1',TOPQ1ThinningHelper.ThinningSvc(), ToolSvc)

#====================================================================
# Add K_S0->pi+pi- reconstruction (TOPQDERIV-69)
#====================================================================
doSimpleV0Finder = False
if doSimpleV0Finder:
  include("DerivationFrameworkBPhys/configureSimpleV0Finder.py")
else:
  include("DerivationFrameworkBPhys/configureV0Finder.py")

TOPQ1_V0FinderTools = BPHYV0FinderTools("TOPQ1")
print TOPQ1_V0FinderTools

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_V0Finder
TOPQ1_Reco_V0Finder   = DerivationFramework__Reco_V0Finder(
    name                   = "TOPQ1_Reco_V0Finder",
    V0FinderTool           = TOPQ1_V0FinderTools.V0FinderTool,
    OutputLevel            = INFO,
    V0ContainerName        = "TOPQ1RecoV0Candidates",
    KshortContainerName    = "TOPQ1RecoKshortCandidates",
    LambdaContainerName    = "TOPQ1RecoLambdaCandidates",
    LambdabarContainerName = "TOPQ1RecoLambdabarCandidates",
    CheckVertexContainers  = ["PrimaryVertices"]
)

ToolSvc += TOPQ1_Reco_V0Finder
print TOPQ1_Reco_V0Finder

#====================================================================
# CREATE THE KERNEL(S)
#====================================================================
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel

# Create the private sequence
TOPQ1Sequence = CfgMgr.AthSequencer("TOPQ1Sequence")

# First skim on leptons
TOPQ1Sequence += CfgMgr.DerivationFramework__DerivationKernel("TOPQ1SkimmingKernel_lep", SkimmingTools = skimmingTools_lep)

#============
# add Jets
#============
#====================================================================
# Special jets
#====================================================================
# Create TCC objects (see JETM1.py)
from DerivationFrameworkJetEtMiss.TCCReconstruction import runTCCReconstruction
# Set up geometry and BField
import AthenaCommon.AtlasUnixStandardJob
include("RecExCond/AllDet_detDescr.py")
runTCCReconstruction(TOPQ1Sequence, ToolSvc, "LCOriginTopoClusters", "InDetTrackParticles")

from DerivationFrameworkTop.TOPQCommonJets import addStandardJetsForTop
from DerivationFrameworkTop.TOPQCommonJets import addSoftDropJetsForTop
from DerivationFrameworkTop.TOPQCommonJets import addTCCTrimmedJetsForTop
from DerivationFrameworkTop.TOPQCommonJets import addVRJetsForTop
from DerivationFrameworkTop.TOPQCommonJets import addExKtDoubleTagVariables
from DerivationFrameworkTop.TOPQCommonJets import addMSVVariables
from DerivationFrameworkTop.TOPQCommonJets import applyTOPQJetCalibration
# add fat/trimmed jets
addStandardJetsForTop(TOPQ1Sequence,'TOPQ1')
# add SoftDrop jets
addSoftDropJetsForTop(TOPQ1Sequence, "TOPQ1")
# add TTC jets
addTCCTrimmedJetsForTop(TOPQ1Sequence, "TOPQ1")
# add VR jets
addVRJetsForTop(TOPQ1Sequence, "TOPQ1")
# apply jet calibration
applyTOPQJetCalibration("AntiKt4EMTopo",DerivationFrameworkJob)
applyTOPQJetCalibration("AntiKt10LCTopoTrimmedPtFrac5SmallR20",TOPQ1Sequence)

# Then skim on the newly created fat jets and calibrated jets
TOPQ1Sequence += CfgMgr.DerivationFramework__DerivationKernel("TOPQ1SkimmingKernel_jet", SkimmingTools = skimmingTools_jet)

# Retagging to get BTagging_AntiKt4EMPFlow Collection (not present in primary AOD)
from BTagging.BTaggingFlags import BTaggingFlags
BTaggingFlags.CalibrationChannelAliases += [ "AntiKt4EMPFlow->AntiKt4EMTopo" ]
TaggerList = BTaggingFlags.StandardTaggers
from DerivationFrameworkFlavourTag.FlavourTagCommon import ReTag
ReTag(TaggerList,['AntiKt4EMPFlowJets'],TOPQ1Sequence)
# for VR
#BTaggingFlags.CalibrationChannelAliases += ["AntiKtVR30Rmax4Rmin02Track->AntiKtVR30Rmax4Rmin02Track,AntiKt4EMTopo"]
#ReTag(TaggerList,['AntiKtVR30Rmax4Rmin02Track'],TOPQ1Sequence)

# Then apply truth tools in the form of aumentation
if DFisMC:
  from DerivationFrameworkTop.TOPQCommonTruthTools import *
  TOPQ1Sequence += TOPQCommonTruthKernel

# add MSV variables
addMSVVariables("AntiKt4EMTopoJets", TOPQ1Sequence, ToolSvc)
# add ExKtDoubleTagVariables (TOPQDERIV-62)
addExKtDoubleTagVariables(TOPQ1Sequence, ToolSvc)

# Then apply thinning 
#AugmentationTool for TOPQDERIV-69
TOPQ1Sequence += CfgMgr.DerivationFramework__DerivationKernel("TOPQ1Kernel", ThinningTools = thinningTools, AugmentationTools = [TOPQ1_Reco_V0Finder])

#====================================================================
# JetTagNonPromptLepton decorations
#====================================================================
import JetTagNonPromptLepton.JetTagNonPromptLeptonConfig as JetTagConfig

# Build AntiKt4PV0TrackJets and run b-tagging
JetTagConfig.ConfigureAntiKt4PV0TrackJets(TOPQ1Sequence, 'TOPQ1')

# Add BDT decoration algs
TOPQ1Sequence += JetTagConfig.GetDecoratePromptLeptonAlgs()
TOPQ1Sequence += JetTagConfig.GetDecoratePromptTauAlgs()


# Finally, add the private sequence to the main job
DerivationFrameworkJob += TOPQ1Sequence

#====================================================================
# SLIMMING
#====================================================================
import DerivationFrameworkTop.TOPQCommonSlimming
DerivationFrameworkTop.TOPQCommonSlimming.setup('TOPQ1', TOPQ1Stream)

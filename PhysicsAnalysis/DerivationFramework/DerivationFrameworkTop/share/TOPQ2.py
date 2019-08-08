#====================================================================
# TOPQ2
# DILEPTON SELECTION
#  >=2 leptons(pT>15GeV) OR
#  >=2 leptons(pT>10GeV) && >=1 lepton(pT>20GeV)
# reductionConf flag TOPQ2 in Reco_tf.py
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
streamName = derivationFlags.WriteDAOD_TOPQ2Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_TOPQ2Stream )
TOPQ2Stream = MSMgr.NewPoolRootStream( streamName, fileName )
# Accept the most selective kernel (last one in sequence; later in derivation)
TOPQ2Stream.AcceptAlgs(["TOPQ2Kernel"])

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
TOPQ2ThinningHelper = ThinningHelper("TOPQ2ThinningHelper")
TOPQ2ThinningHelper.TriggerChains =  DerivationFrameworkTop.TOPQCommonThinning.TOPQTriggerChains('leptonicTriggers' if globalflags.DataSource()!='geant4' else 'allTriggers')
TOPQ2ThinningHelper.AppendToStream(TOPQ2Stream)

#====================================================================
# SKIMMING TOOLS
#====================================================================
import DerivationFrameworkTop.TOPQCommonSelection
skimmingTools_lep = DerivationFrameworkTop.TOPQCommonSelection.setup_lep('TOPQ2', ToolSvc)
skimmingTools_jet = DerivationFrameworkTop.TOPQCommonSelection.setup_jet('TOPQ2', ToolSvc)

#====================================================================
# THINNING TOOLS
#====================================================================
import DerivationFrameworkTop.TOPQCommonThinning
thinningTools = DerivationFrameworkTop.TOPQCommonThinning.setup('TOPQ2',TOPQ2ThinningHelper.ThinningSvc(), ToolSvc)

#====================================================================
# CREATE THE KERNEL(S)
#====================================================================
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel

# Create the private sequence
TOPQ2Sequence = CfgMgr.AthSequencer("TOPQ2Sequence")

# First skim on leptons
TOPQ2Sequence += CfgMgr.DerivationFramework__DerivationKernel("TOPQ2SkimmingKernel_lep", SkimmingTools = skimmingTools_lep)

# add fat/trimmed jets
from DerivationFrameworkTop.TOPQCommonJets import addStandardJetsForTop
addStandardJetsForTop(TOPQ2Sequence,'TOPQ2')

# apply jet calibration
from DerivationFrameworkTop.TOPQCommonJets import applyTOPQJetCalibration
applyTOPQJetCalibration("AntiKt4EMTopo",DerivationFrameworkJob)
applyTOPQJetCalibration("AntiKt10LCTopoTrimmedPtFrac5SmallR20",TOPQ2Sequence)

# Then skim on the newly created fat jets and calibrated jets
TOPQ2Sequence += CfgMgr.DerivationFramework__DerivationKernel("TOPQ2SkimmingKernel_jet", SkimmingTools = skimmingTools_jet)

# Retagging to get BTagging_AntiKt4EMPFlow Collection (not present in primary AOD)
from BTagging.BTaggingFlags import BTaggingFlags
BTaggingFlags.CalibrationChannelAliases += [ "AntiKt4EMPFlow->AntiKt4EMTopo" ]

TaggerList = BTaggingFlags.StandardTaggers
from DerivationFrameworkFlavourTag.FlavourTagCommon import FlavorTagInit
FlavorTagInit(JetCollections  = ['AntiKt4EMPFlowJets'], Sequencer = TOPQ2Sequence)

# Then apply truth tools in the form of aumentation
if DFisMC:
  from DerivationFrameworkTop.TOPQCommonTruthTools import *
  TOPQ2Sequence += TOPQCommonTruthKernel

# add MSV variables
from DerivationFrameworkTop.TOPQCommonJets import addMSVVariables
addMSVVariables("AntiKt4EMTopoJets", TOPQ2Sequence, ToolSvc)

# Then apply thinning
TOPQ2Sequence += CfgMgr.DerivationFramework__DerivationKernel("TOPQ2Kernel", ThinningTools = thinningTools)

#====================================================================
# JetTagNonPromptLepton decorations
#====================================================================
import JetTagNonPromptLepton.JetTagNonPromptLeptonConfig as JetTagConfig

# Build AntiKt4PV0TrackJets and run b-tagging
JetTagConfig.ConfigureAntiKt4PV0TrackJets(TOPQ2Sequence, 'TOPQ2')

# Add BDT decoration algs
TOPQ2Sequence += JetTagConfig.GetDecoratePromptLeptonAlgs()
TOPQ2Sequence += JetTagConfig.GetDecoratePromptTauAlgs()

# Finally, add the private sequence to the main job
DerivationFrameworkJob += TOPQ2Sequence

#====================================================================
# SLIMMING
#====================================================================
import DerivationFrameworkTop.TOPQCommonSlimming
DerivationFrameworkTop.TOPQCommonSlimming.setup('TOPQ2', TOPQ2Stream)

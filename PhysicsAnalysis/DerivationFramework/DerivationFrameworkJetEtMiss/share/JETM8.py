#====================================================================
# JETM8.py 
# reductionConf flag JETM8 in Reco_tf.py   
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkFlavourTag.HbbCommon import *
from DerivationFrameworkFlavourTag.FlavourTagCommon import *

#====================================================================
# SKIMMING TOOL 
#====================================================================

from DerivationFrameworkJetEtMiss import TriggerLists
jetTriggers = TriggerLists.jetTrig()

# For first data
jetSelection = '(count( AntiKt10LCTopoJets.pt > 150.*GeV && abs(AntiKt10LCTopoJets.eta) < 2.5 ) >=1)'
jetSelection += '||(count( AntiKt10TrackCaloClusterJets.pt > 150.*GeV && abs(AntiKt10TrackCaloClusterJets.eta) < 2.5 ) >=1)'
jetSelection += '||(count( AntiKt10UFOCSSKJets.pt > 150.*GeV && abs(AntiKt10UFOCSSKJets.eta) < 2.5 ) >=1)'

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
JETM8TrigSkimmingTool = DerivationFramework__TriggerSkimmingTool(   name           = "JETM8TrigSkimmingTool",
                                                                    TriggerListOR  = jetTriggers )
ToolSvc += JETM8TrigSkimmingTool

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
JETM8OfflineSkimmingTool = DerivationFramework__xAODStringSkimmingTool( name = "JETM8OfflineSkimmingTool",
                                                                        expression = jetSelection)
ToolSvc += JETM8OfflineSkimmingTool

#====================================================================
# THINNING TOOLS 
#====================================================================

thinningTools = []

#====================================================================
# Thin TruthParticles for truth jet constituents
#====================================================================

if DerivationFrameworkIsMonteCarlo:
  from DerivationFrameworkJetEtMiss.DerivationFrameworkJetEtMissConf import DerivationFramework__ViewContainerThinning
  JETM8TruthJetInputThin = DerivationFramework__ViewContainerThinning( name = "JETM8ViewContThinning",
                                                                       ThinningService        = "JETM8ThinningSvc",
                                                                       SourceContainer = "TruthParticles",
                                                                       ViewContainer = "JetInputTruthParticles",
                                                                       ParticleType = 201, # truthParticles
                                                                       ApplyAnd = False)

  ToolSvc += JETM8TruthJetInputThin
  thinningTools.append(JETM8TruthJetInputThin)


#=======================================
# CREATE PRIVATE SEQUENCE
#=======================================

jetm8Seq = CfgMgr.AthSequencer("JETM8Sequence")
DerivationFrameworkJob += jetm8Seq

#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM   
#=======================================

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
jetm8Seq += CfgMgr.DerivationFramework__DerivationKernel(	name = "JETM8TrigKernel", 
									SkimmingTools = [JETM8TrigSkimmingTool],
									ThinningTools = [])

#=======================================
# BUILD PFO, TCC, AND UFO INPUTS 
#=======================================

# Add PFlow constituents
from JetRecTools.ConstModHelpers import getConstModSeq, xAOD
pflowCSSKSeq = getConstModSeq(["CS","SK"], "EMPFlow")

from JetRec.JetRecConf import JetAlgorithm
clustSeqAlg = JetAlgorithm("ClusterModifiers", Tools = [pflowCSSKSeq])
jetm8Seq += clustSeqAlg

# Add TCC constituents
from TrackCaloClusterRecTools.TrackCaloClusterConfig import runTCCReconstruction
import AthenaCommon.AtlasUnixStandardJob
include("RecExCond/AllDet_detDescr.py")
runTCCReconstruction(jetm8Seq, ToolSvc, "LCOriginTopoClusters", "InDetTrackParticles", outputTCCName="TrackCaloClustersCombinedAndNeutral")

# Add UFO constituents
from TrackCaloClusterRecTools.TrackCaloClusterConfig import runUFOReconstruction
emcsskufoAlg = runUFOReconstruction(jetm8Seq, ToolSvc, PFOPrefix="CSSK")

#=======================================
# RESTORE AOD-REDUCED JET COLLECTIONS
#=======================================
reducedJetList = ["AntiKt2PV0TrackJets",
                  "AntiKt4PV0TrackJets",
                  "AntiKt2LCTopoJets",
                  "AntiKt2TruthJets",
                  "AntiKt4TruthJets",
                  "AntiKt4TruthWZJets",
                  "AntiKt10TruthJets",
                  "AntiKt10LCTopoJets",
                  "AntiKt10TrackCaloClusterJets",
                  "AntiKt10UFOCSSKJets"]
replaceAODReducedJets(reducedJetList,jetm8Seq,"JETM8")

jetm8Seq += CfgMgr.DerivationFramework__DerivationKernel( name = "JETM8MainKernel", 
                                                          SkimmingTools = [JETM8OfflineSkimmingTool],
                                                          ThinningTools = thinningTools)

#====================================================================
# Jets for tagging
#====================================================================

OutputJets["JETM8"] = []

# AntiKt10*PtFrac5Rclus20
addDefaultTrimmedJets(jetm8Seq,"JETM8")

# Add VR track jets for b-tagging
addVRJets(jetm8Seq)
addVRJets(jetm8Seq, do_ghost=True)
addVRJets(jetm8Seq, training='201903')

addTCCTrimmedJets(jetm8Seq,"JETM8")

if DerivationFrameworkIsMonteCarlo:
  addSoftDropJets('AntiKt', 1.0, 'Truth', beta=1.0, zcut=0.1, mods="truth_groomed", algseq=jetm8Seq, outputGroup="JETM8", writeUngroomed=False)
  addRecursiveSoftDropJets('AntiKt', 1.0, 'Truth', beta=1.0, zcut=0.05, N=-1,  mods="truth_groomed", algseq=jetm8Seq, outputGroup="JETM8", writeUngroomed=False)
  addBottomUpSoftDropJets('AntiKt', 1.0, 'Truth', beta=1.0, zcut=0.05, mods="truth_groomed", algseq=jetm8Seq, outputGroup="JETM8", writeUngroomed=False)

addTrimmedJets("AntiKt", 1.0, "UFOCSSK", rclus=0.2, ptfrac=0.05, algseq=jetm8Seq, outputGroup="JETM8", writeUngroomed=False, mods="tcc_groomed")
addSoftDropJets("AntiKt", 1.0, "UFOCSSK", beta=1.0, zcut=0.1, algseq=jetm8Seq, outputGroup="JETM8", writeUngroomed=False, mods="tcc_groomed")
addRecursiveSoftDropJets('AntiKt', 1.0, 'UFOCSSK', beta=1.0, zcut=0.05, N=-1,  mods="tcc_groomed", algseq=jetm8Seq, outputGroup="JETM8", writeUngroomed=False)
addBottomUpSoftDropJets('AntiKt', 1.0, 'UFOCSSK', beta=1.0, zcut=0.05, mods="tcc_groomed", algseq=jetm8Seq, outputGroup="JETM8", writeUngroomed=False)

#====================================================================
# Set up b-tagging
#====================================================================

# use alias for VR jets
from BTagging.BTaggingFlags import BTaggingFlags
BTaggingFlags.CalibrationChannelAliases += ["AntiKtVR30Rmax4Rmin02Track->AntiKtVR30Rmax4Rmin02Track,AntiKt4EMTopo"]

#tag pFlow jets
FlavorTagInit(scheduleFlipped = False, JetCollections  = ['AntiKt4EMPFlowJets'], Sequencer = jetm8Seq)

#====================================================================
# Add truth information
#====================================================================

if DerivationFrameworkIsMonteCarlo:
  from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents,addBosonsAndDownstreamParticles,addTopQuarkAndDownstreamParticles,addHFAndDownstreamParticles
  addStandardTruthContents()
  addBosonsAndDownstreamParticles()
  addTopQuarkAndDownstreamParticles()
  addHFAndDownstreamParticles(addB=True, addC=False, generations=0)

#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_JETM8Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_JETM8Stream )
JETM8Stream = MSMgr.NewPoolRootStream( streamName, fileName )
JETM8Stream.AcceptAlgs(["JETM8MainKernel"])
# for thinning
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()
svcMgr += createThinningSvc( svcName="JETM8ThinningSvc", outStreams=[evtStream] )

#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
JETM8SlimmingHelper = SlimmingHelper("JETM8SlimmingHelper")
JETM8SlimmingHelper.SmartCollections = ["Electrons", "Photons", "Muons",
                                        "InDetTrackParticles", "PrimaryVertices",
                                        "AntiKt2TruthJets",
                                        "AntiKt2LCTopoJets",
                                        "AntiKt4TruthJets",
                                        "AntiKt4TruthWZJets",
                                        "AntiKt4EMPFlowJets",
                                        "AntiKt4EMPFlowJets_BTagging201810",
                                        "AntiKt4EMPFlowJets_BTagging201903",
                                        "AntiKt10TruthJets",
                                        "AntiKt10TruthWZJets",
                                        "AntiKt10TruthTrimmedPtFrac5SmallR20Jets",
                                        "AntiKt10TruthSoftDropBeta100Zcut10Jets",
                                        "AntiKt10TruthBottomUpSoftDropBeta100Zcut5Jets",
                                        "AntiKt10TruthRecursiveSoftDropBeta100Zcut5NinfJets",
                                        "AntiKt10LCTopoJets",
                                        "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                        "AntiKt10TrackCaloClusterJets",
                                        "AntiKt10TrackCaloClusterTrimmedPtFrac5SmallR20Jets",
                                        "AntiKt10UFOCSSKJets",
                                        "AntiKt10UFOCSSKTrimmedPtFrac5SmallR20Jets",
                                        "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                        "AntiKt10UFOCSSKBottomUpSoftDropBeta100Zcut5Jets",
                                        "AntiKt10UFOCSSKRecursiveSoftDropBeta100Zcut5NinfJets",
                                        "BTagging_AntiKt4EMPFlow_201810",
                                        "BTagging_AntiKt4EMPFlow_201903",
                                        "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                        ]
JETM8SlimmingHelper.AllVariables = ["CaloCalTopoClusters",
                                    "CSSKUFO",
                                    "TruthParticles",
                                    "Kt4EMPFlowEventShape"]

JETM8SlimmingHelper.AppendToDictionary["CSSKUFO"] = "xAOD::TrackCaloClusterContainer"
JETM8SlimmingHelper.AppendToDictionary["CSSKUFOAux"] = "xAOD::TrackCaloClusterAuxContainer"
JETM8SlimmingHelper.ExtraVariables += [ "CSSKUFO.pt.eta.phi.taste" ]

# Add the jet containers to the stream
addJetOutputs(JETM8SlimmingHelper,["LargeR",
                                   "AntiKt4EMPFlowJets",
                                   "AntiKt4TruthJets",
                                   "AntiKt4TruthWZJets",
                                   "AntiKt2LCTopoJets",
                                   "AntiKt2TruthJets",
                                   "JETM8"],JETM8SlimmingHelper.SmartCollections)

for truthc in [
  "TruthTopQuark",
  "TruthBoson",
  "TruthHF"
  ]:
  JETM8SlimmingHelper.StaticContent.append("xAOD::TruthParticleContainer#"+truthc+"WithDecayParticles")
  JETM8SlimmingHelper.StaticContent.append("xAOD::TruthParticleAuxContainer#"+truthc+"WithDecayParticlesAux.")
  JETM8SlimmingHelper.StaticContent.append("xAOD::TruthVertexContainer#"+truthc+"WithDecayVertices")
  JETM8SlimmingHelper.StaticContent.append("xAOD::TruthVertexAuxContainer#"+truthc+"WithDecayVerticesAux.")

for truthc in [
  "TruthMuons",
  "TruthElectrons",
  "TruthPhotons",
  "TruthBottom",
  "TruthBSM"
  ]:
  JETM8SlimmingHelper.StaticContent.append("xAOD::TruthParticleContainer#"+truthc)
  JETM8SlimmingHelper.StaticContent.append("xAOD::TruthParticleAuxContainer#"+truthc+"Aux.")

print JETM8SlimmingHelper.AppendToDictionary

JETM8SlimmingHelper.AppendContentToStream(JETM8Stream)
JETM8Stream.RemoveItem("xAOD::TrigNavigation#*")
JETM8Stream.RemoveItem("xAOD::TrigNavigationAuxInfo#*")

def removeVars(coll, vars):
    """Terrible hack to force removing a limited set of variables.
    Intended to remove definingParametersCovMatrix if possible.. """
    items = JETM8Stream.GetItems()
    origS = None
    for i in items:
        if coll in i and '.' in i:
            origS = i
            break
    if origS is None:
        return
    existV = origS.split('.')
    cleanedV = []
    for v in existV:
        if v in vars:
            continue
        cleanedV.append(v)
    newS = '.'.join(cleanedV)
    JETM8Stream.RemoveItem( origS ) 
    JETM8Stream.AddItem( newS ) 
removeVars('InDetTrackParticles', ['definingParametersCovMatrix',])

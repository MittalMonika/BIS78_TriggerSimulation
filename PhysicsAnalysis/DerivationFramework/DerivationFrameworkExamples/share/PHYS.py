#====================================================================
# DAOD_PHYS.py
# This defines DAOD_PHYS, which is a prototype for a possible new 
# unskimmed format for Run 3
# It requires the reductionConf flag PHYS in Reco_tf.py   
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkEGamma.ElectronsCPDetailedContent import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkCore.WeightMetadata import *
from DerivationFrameworkFlavourTag.FlavourTagCommon import FlavorTagInit
from DerivationFrameworkFlavourTag.HbbCommon import *
from TriggerMenu.api.TriggerAPI import TriggerAPI
from TriggerMenu.api.TriggerEnums import TriggerPeriod, TriggerType
from DerivationFrameworkTrigger.TriggerMatchingHelper import TriggerMatchingHelper


#====================================================================
# TRIGGER CONTENT   
#====================================================================
# See https://twiki.cern.ch/twiki/bin/view/Atlas/TriggerAPI
# Get single and multi mu, e, photon triggers
# Jet, tau, multi-object triggers not available in the matching code
allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
trig_el  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el,  livefraction=0.8)
trig_mu  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu,  livefraction=0.8)
trig_g   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.g,   livefraction=0.8)
trig_tau = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.tau, livefraction=0.8)
# Add cross-triggers for some sets
trig_em = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.mu,  livefraction=0.8)
trig_et = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.tau, livefraction=0.8)
trig_mt = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu, additionalTriggerType=TriggerType.tau, livefraction=0.8)
# Note that this seems to pick up both isolated and non-isolated triggers already, so no need for extra grabs

# Merge and remove duplicates
trigger_names_full = list(set(trig_el+trig_mu+trig_g+trig_tau+trig_em+trig_et+trig_mt))

# Now reduce the list...
from RecExConfig.InputFilePeeker import inputFileSummary
trigger_names = []
for trig_item in inputFileSummary['metadata']['/TRIGGER/HLT/Menu']:
    if not 'ChainName' in trig_item: continue
    if trig_item['ChainName'] in trigger_names_full: trigger_names += [ trig_item['ChainName'] ]

# Create trigger matching decorations
trigmatching_helper = TriggerMatchingHelper(
        trigger_list = trigger_names, add_to_df_job=True)

#====================================================================
# SET UP STREAM   
#====================================================================

streamName = derivationFlags.WriteDAOD_PHYSStream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_PHYSStream )
PHYSStream = MSMgr.NewPoolRootStream( streamName, fileName )
PHYSStream.AcceptAlgs(["PHYSKernel"])

#====================================================================
# INNER DETECTOR TRACK THINNING
#====================================================================

# See recommedations here: 
# https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations

from DerivationFrameworkCore.ThinningHelper import ThinningHelper
PHYSThinningHelper = ThinningHelper( "PHYSThinningHelper" )
PHYSThinningHelper.AppendToStream( PHYSStream )

# Inner detector group recommendations for indet tracks in analysis
PHYS_thinning_expression = "InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm && InDetTrackParticles.pt > 10*GeV"
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
PHYSTrackParticleThinningTool = DerivationFramework__TrackParticleThinning(name                    = "PHYSTrackParticleThinningTool",
                                                                           ThinningService         = PHYSThinningHelper.ThinningSvc(),
                                                                           SelectionString         = PHYS_thinning_expression,
                                                                           InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                           ApplyAnd                = False)

ToolSvc += PHYSTrackParticleThinningTool

# Include inner detector tracks associated with muons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
PHYSMuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(name                    = "PHYSMuonTPThinningTool",
                                                                        ThinningService         = PHYSThinningHelper.ThinningSvc(),
                                                                        MuonKey                 = "Muons",
                                                                        InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                        ApplyAnd = False)
ToolSvc += PHYSMuonTPThinningTool

# TauJets thinning
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__GenericObjectThinning
PHYSTauJetsThinningTool = DerivationFramework__GenericObjectThinning(name            = "PHYSTauJetsThinningTool",
                                                                     ThinningService = PHYSThinningHelper.ThinningSvc(),
                                                                     ContainerName   = "TauJets",
                                                                     SelectionString = "(TauJets.ptFinalCalib >= 13.*GeV) && (TauJets.nTracks<6)")
ToolSvc += PHYSTauJetsThinningTool

# Only keep tau tracks (and associated ID tracks) classified as charged tracks
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TauTrackParticleThinning
PHYSTauTPThinningTool = DerivationFramework__TauTrackParticleThinning(name                   = "PHYSTauTPThinningTool",
                                                                      ThinningService        = PHYSThinningHelper.ThinningSvc(),
                                                                      TauKey                 = "TauJets",
                                                                      InDetTrackParticlesKey = "InDetTrackParticles",
                                                                      SelectionString        = "(TauJets.ptFinalCalib >= 13.*GeV) && (TauJets.nTracks<6)",
                                                                      ApplyAnd               = False,
                                                                      DoTauTracksThinning    = True,
                                                                      TauTracksKey           = "TauTracks")
ToolSvc += PHYSTauTPThinningTool

# ID tracks associated with high-pt di-tau
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__DiTauTrackParticleThinning
PHYSDiTauTPThinningTool = DerivationFramework__DiTauTrackParticleThinning(name                    = "PHYSDiTauTPThinningTool",
                                                                          ThinningService         = PHYSThinningHelper.ThinningSvc(),
                                                                          DiTauKey                = "DiTauJets",
                                                                          InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += PHYSDiTauTPThinningTool

#====================================================================
# JET/MET   
#====================================================================

OutputJets["PHYS"] = ["AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"]
reducedJetList = ["AntiKt2PV0TrackJets","AntiKt4PV0TrackJets"]

if (DerivationFrameworkIsMonteCarlo):
   OutputJets["PHYS"].append("AntiKt10TruthTrimmedPtFrac5SmallR20Jets")
   reducedJetList.append("AntiKt4TruthWZJets")
   reducedJetList.append("AntiKt4TruthJets")  
replaceAODReducedJets(reducedJetList,DerivationFrameworkJob,"PHYS")
addDefaultTrimmedJets(DerivationFrameworkJob,"PHYS",dotruth=DerivationFrameworkIsMonteCarlo)

addQGTaggerTool(jetalg="AntiKt4EMTopo",sequence=DerivationFrameworkJob,algname="QGTaggerToolAlg")
addQGTaggerTool(jetalg="AntiKt4EMPFlow",sequence=DerivationFrameworkJob,algname="QGTaggerToolPFAlg")

# fJVT
getPFlowfJVT(jetalg='AntiKt4EMPFlow',sequence=DerivationFrameworkJob, algname='PHYSJetForwardPFlowJvtToolAlg')

#====================================================================
# Tau   
#====================================================================

# Schedule low-pt di-tau reconstruction (needs AntiKt2PV0TrackJets)
from DerivationFrameworkTau.TauCommon import addDiTauLowPt
addDiTauLowPt()

# Low-pt di-tau thinning
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__GenericObjectThinning
PHYSDiTauLowPtThinningTool = DerivationFramework__GenericObjectThinning(name            = "PHYSDiTauLowPtThinningTool",
                                                                        ThinningService = PHYSThinningHelper.ThinningSvc(),
                                                                        ContainerName   = "DiTauJetsLowPt",
                                                                        SelectionString = "DiTauJetsLowPt.nSubjets > 1")
ToolSvc += PHYSDiTauLowPtThinningTool

# ID tracks associated with low-pt ditau
PHYSDiTauLowPtTPThinningTool = DerivationFramework__DiTauTrackParticleThinning(name                    = "PHYSDiTauLowPtTPThinningTool",
                                                                               ThinningService         = PHYSThinningHelper.ThinningSvc(),
                                                                               DiTauKey                = "DiTauJetsLowPt",
                                                                               InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                               SelectionString         = "DiTauJetsLowPt.nSubjets > 1")
ToolSvc += PHYSDiTauLowPtTPThinningTool

#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM   
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel("PHYSKernel",
                                                                       ThinningTools = [PHYSTrackParticleThinningTool,PHYSMuonTPThinningTool,
                                                                                        PHYSTauJetsThinningTool,PHYSTauTPThinningTool,
                                                                                        PHYSDiTauTPThinningTool,PHYSDiTauLowPtThinningTool,PHYSDiTauLowPtTPThinningTool])


#====================================================================
# FLAVOUR TAGGING   
#====================================================================
# Create variable-R trackjets and dress AntiKt10LCTopo with ghost VR-trkjet 
addVRJets(DerivationFrameworkJob)
#addVRJetsTCC(DerivationFrameworkJob, "AntiKtVR30Rmax4Rmin02Track", "GhostVR30Rmax4Rmin02TrackJet",
#             VRJetAlg="AntiKt", VRJetRadius=0.4, VRJetInputs="pv0track",
#             ghostArea = 0 , ptmin = 2000, ptminFilter = 2000,
#             variableRMinRadius = 0.02, variableRMassScale = 30000, calibOpt = "none")
# add xbb taggers
from DerivationFrameworkFlavourTag.HbbCommon import addRecommendedXbbTaggers
addRecommendedXbbTaggers(DerivationFrameworkJob, ToolSvc)

FlavorTagInit(JetCollections  = [ 'AntiKt4EMTopoJets','AntiKt4EMPFlowJets'], Sequencer = DerivationFrameworkJob)

#====================================================================
# TC-LVT Vertices 
#====================================================================

from SoftBVrtClusterTool.SoftBVrtConfig import addSoftBVrt
addSoftBVrt(DerivationFrameworkJob,'Loose')
addSoftBVrt(DerivationFrameworkJob,'Medium')
addSoftBVrt(DerivationFrameworkJob,'Tight')

#====================================================================
# Truth collections
#====================================================================
if (DerivationFrameworkIsMonteCarlo):
   from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents,addMiniTruthCollectionLinks,addHFAndDownstreamParticles,addPVCollection
   import DerivationFrameworkHiggs.TruthCategories 
   addStandardTruthContents()
   addMiniTruthCollectionLinks()
   addHFAndDownstreamParticles()
   addPVCollection()
   from DerivationFrameworkSUSY.DecorateSUSYProcess import IsSUSYSignal
   if IsSUSYSignal():
      from DerivationFrameworkSUSY.SUSYWeightMetadata import *
   # Add sumOfWeights metadata for LHE3 multiweights =======
   from DerivationFrameworkCore.LHE3WeightMetadata import *

#====================================================================
# CONTENTS   
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
PHYSSlimmingHelper = SlimmingHelper("PHYSSlimmingHelper")

PHYSSlimmingHelper.SmartCollections = ["Electrons",
                                       "Photons",
                                       "Muons",
                                       "PrimaryVertices",
                                       "InDetTrackParticles",
                                       "AntiKt4EMTopoJets",
                                       "AntiKt4EMPFlowJets",
                                       "BTagging_AntiKt4EMTopo_201810",
                                       "BTagging_AntiKt4EMPFlow_201810",
                                       "BTagging_AntiKt4EMPFlow_201903",
                                       "TauJets",
                                       "DiTauJets",
                                       "DiTauJetsLowPt",
                                       "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"
                                      ]

excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
StaticContent = []
StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]

PHYSSlimmingHelper.StaticContent = StaticContent

# Trigger content
PHYSSlimmingHelper.IncludeTriggerNavigation = False
PHYSSlimmingHelper.IncludeJetTriggerContent = False
PHYSSlimmingHelper.IncludeMuonTriggerContent = False
PHYSSlimmingHelper.IncludeEGammaTriggerContent = False
PHYSSlimmingHelper.IncludeJetTauEtMissTriggerContent = False
PHYSSlimmingHelper.IncludeTauTriggerContent = False
PHYSSlimmingHelper.IncludeEtMissTriggerContent = False
PHYSSlimmingHelper.IncludeBJetTriggerContent = False
PHYSSlimmingHelper.IncludeBPhysTriggerContent = False
PHYSSlimmingHelper.IncludeMinBiasTriggerContent = False

# Add the jet containers to the stream (defined in JetCommon if import needed)
#addJetOutputs(PHYSSlimmingHelper,["PHYS"])

# Truth containers
if DerivationFrameworkIsMonteCarlo:
   PHYSSlimmingHelper.AppendToDictionary = {'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
                                            'MET_Truth':'xAOD::MissingETContainer','MET_TruthAux':'xAOD::MissingETAuxContainer',
                                            'TruthElectrons':'xAOD::TruthParticleContainer','TruthElectronsAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthMuons':'xAOD::TruthParticleContainer','TruthMuonsAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthPhotons':'xAOD::TruthParticleContainer','TruthPhotonsAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthTaus':'xAOD::TruthParticleContainer','TruthTausAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthNeutrinos':'xAOD::TruthParticleContainer','TruthNeutrinosAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBSM':'xAOD::TruthParticleContainer','TruthBSMAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBoson':'xAOD::TruthParticleContainer','TruthBosonAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthTop':'xAOD::TruthParticleContainer','TruthTopAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthForwardProtons':'xAOD::TruthParticleContainer','TruthForwardProtonsAux':'xAOD::TruthParticleAuxContainer',
                                            'BornLeptons':'xAOD::TruthParticleContainer','BornLeptonsAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBosonsWithDecayParticles':'xAOD::TruthParticleContainer','TruthBosonsWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBosonsWithDecayVertices':'xAOD::TruthVertexContainer','TruthBosonsWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthBSMWithDecayParticles':'xAOD::TruthParticleContainer','TruthBSMWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthBSMWithDecayVertices':'xAOD::TruthVertexContainer','TruthBSMWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'HardScatterParticles':'xAOD::TruthParticleContainer','HardScatterParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'HardScatterVertices':'xAOD::TruthVertexContainer','HardScatterVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthHFWithDecayParticles':'xAOD::TruthParticleContainer','TruthHFWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                            'TruthHFWithDecayVertices':'xAOD::TruthVertexContainer','TruthHFWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'TruthPrimaryVertices':'xAOD::TruthVertexContainer','TruthPrimaryVerticesAux':'xAOD::TruthVertexAuxContainer',
                                            'AntiKt10TruthTrimmedPtFrac5SmallR20Jets':'xAOD::JetContainer', 'AntiKt10TruthTrimmedPtFrac5SmallR20JetsAux':'xAOD::JetAuxContainer',
                                           }

   from DerivationFrameworkMCTruth.MCTruthCommon import addTruth3ContentToSlimmerTool
   addTruth3ContentToSlimmerTool(PHYSSlimmingHelper)
   PHYSSlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices']

PHYSSlimmingHelper.ExtraVariables += ["AntiKt10TruthTrimmedPtFrac5SmallR20Jets.pt.Tau1_wta.Tau2_wta.Tau3_wta.D2",
                                      "Electrons.TruthLink",
                                      "Muons.TruthLink",
                                      "Photons.TruthLink",
                                      "AntiKt2PV0TrackJets.pt.eta.phi.m",
                                      "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.NumTrkPt500PV.PartonTruthLabelID",
                                      "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.NumTrkPt500PV.PartonTruthLabelID.DFCommonJets_fJvt",
                                      "TruthPrimaryVertices.t.x.y.z"]

# Add trigger matching
trigmatching_helper.add_to_slimming(PHYSSlimmingHelper)

# Final construction of output stream
PHYSSlimmingHelper.AppendContentToStream(PHYSStream)


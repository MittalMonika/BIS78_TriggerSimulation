#********************************************************************
# EGAM4.py
# Z->mumugamma reduction for photon studies
# reductionConf flag EGAM4 in Reco_tf.py
# author: giovanni.marchiori@cern.ch
#********************************************************************

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *
from DerivationFrameworkEGamma.EGammaCommon import *
from DerivationFrameworkEGamma.EGAM4ExtraContent import *

# read common DFEGamma settings from egammaDFFlags
from DerivationFrameworkEGamma.egammaDFFlags import jobproperties
jobproperties.egammaDFFlags.print_JobProperties("full")

DoCellReweighting = jobproperties.egammaDFFlags.doEGammaCellReweighting
#override if needed (do at your own risk..)
#DoCellReweighting = False
#DoCellReweighting = True

# check if we run on data or MC (DataSource = geant4)
from AthenaCommon.GlobalFlags import globalflags
print "EGAM4 globalflags.DataSource(): ", globalflags.DataSource()
if globalflags.DataSource()!='geant4':
    DoCellReweighting = False


#====================================================================
# SET UP STREAM (to be done early in the game to set up thinning Svc
#====================================================================
streamName = derivationFlags.WriteDAOD_EGAM4Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_EGAM4Stream )
EGAM4Stream = MSMgr.NewPoolRootStream( streamName, fileName )


#====================================================================
# SET UP SKIMMING
#====================================================================


#====================================================================
# mumugamma and mumue selection for photon studies, single & di-muon triggers
# two opposite-sign muons, pT>15 GeV, |eta|<2.5, mmumu>40 GeV
# gamma: reco, ET>10 GeV, |eta|<2.5
# mumueegamma: one reco photon, ET>10 GeV< |eta|<2.5
# mumue: one reco electron, pT>10 GeV
#====================================================================

#requirement = 'Muons.pt>9.5*GeV && abs(Muons.eta)<2.7 && Muons.DFCommonGoodMuon'
requirementMuons = 'Muons.pt>9.5*GeV && abs(Muons.eta)<2.7 && Muons.DFCommonMuonsPreselection'

from DerivationFrameworkEGamma.DerivationFrameworkEGammaConf import DerivationFramework__EGInvariantMassTool
EGAM4_MuMuMassTool = DerivationFramework__EGInvariantMassTool( name = "EGAM4_MuMuMassTool",
                                                               Object1Requirements = requirementMuons,
                                                               Object2Requirements = requirementMuons,
                                                               StoreGateEntryName = "EGAM4_DiMuonMass",
                                                               Mass1Hypothesis = 105*MeV,
                                                               Mass2Hypothesis = 105*MeV,
                                                               Container1Name = "Muons",
                                                               Container2Name = "Muons",
                                                               CheckCharge = True,
                                                               DoTransverseMass = False,
                                                               MinDeltaR = 0.0)
ToolSvc += EGAM4_MuMuMassTool
print EGAM4_MuMuMassTool

skimmingExpression1a = 'count(DFCommonPhotons_et>9.5*GeV)>=1 && count(EGAM4_DiMuonMass > 40.0*GeV)>=1'
skimmingExpression1b = 'count(Electrons.pt>9.5*GeV)>=1 && count(EGAM4_DiMuonMass > 40.0*GeV)>=1'


#====================================================================
# SKIMMING TOOL
#====================================================================

skimmingExpression = skimmingExpression1a + ' || ' + skimmingExpression1b
print "EGAM4 skimming expression: ", skimmingExpression
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
EGAM4_SkimmingTool = DerivationFramework__xAODStringSkimmingTool( name = "EGAM4_SkimmingTool",
                                                                 expression = skimmingExpression)
ToolSvc += EGAM4_SkimmingTool
print "EGAM4 skimming tool: ", EGAM4_SkimmingTool


#====================================================================
# DECORATION TOOLS
#====================================================================


#====================================================================
# Gain and cluster energies per layer decoration tool
#====================================================================
from DerivationFrameworkCalo.DerivationFrameworkCaloFactories import GainDecorator, getGainDecorations, getClusterEnergyPerLayerDecorator, getClusterEnergyPerLayerDecorations
EGAM4_GainDecoratorTool = GainDecorator()
ToolSvc += EGAM4_GainDecoratorTool

cluster_sizes = (3,5), (5,7), (7,7), (7,11)
EGAM4_ClusterEnergyPerLayerDecorators = [getClusterEnergyPerLayerDecorator(neta, nphi)() for neta, nphi in cluster_sizes]


#====================================================================
# Cell reweighter
#====================================================================
if DoCellReweighting:
    from DerivationFrameworkCalo.DerivationFrameworkCaloFactories import NewCellTool, ClusterDecoratorWithNewCells, EGammaReweightTool

    # first, create the container with the new cells (after reweighting)
    EGAM4_NewCellTool = NewCellTool("EGAM4_NewCellTool")
    #                                OutputLevel = DEBUG)
    #                                ReweightCellContainerName="NewCellContainer",
    #                                SGKey_electrons = "Electrons", 
    #                                SGKey_photons = "Photons")
    print EGAM4_NewCellTool
    ToolSvc += EGAM4_NewCellTool

    # second, run a tool that creates the clusters and objects from these new cells
    EGAM4_ClusterDecoratorTool = ClusterDecoratorWithNewCells("EGAM4_ClusterDecoratorTool")
    #                                                          OutputClusterSGKey="EGammaSwClusterWithNewCells",
    #                                                          OutputClusterLink="NewSwClusterLink",
    #                                                          SGKey_caloCells = "NewCellContainer",
    #                                                          SGKey_electrons = "Electrons", 
    #                                                          SGKey_photons = "Photons")
    print EGAM4_ClusterDecoratorTool
    ToolSvc += EGAM4_ClusterDecoratorTool

    # third, run a tool that creates the shower shapes with the new cells
    from egammaTools.egammaToolsFactories import EMShowerBuilder
    EGAM4_EMShowerBuilderTool = EMShowerBuilder("EGAM4_EMShowerBuilderTool", 
                                                CellsName="NewCellContainer")
    print EGAM4_EMShowerBuilderTool
    ToolSvc += EGAM4_EMShowerBuilderTool

    # fourth, decorate the new objects with their shower shapes computed from the new clusters
    EGAM4_EGammaReweightTool = EGammaReweightTool("EGAM4_EGammaReweightTool",
                                                  SGKey_electrons = "Electrons",
                                                  SGKey_photons="Photons",
                                                  NewCellContainerName="NewCellContainer",
                                                  NewElectronContainer = "NewSwElectrons",
                                                  NewPhotonContainer = "NewSwPhotons",
                                                  EMShowerBuilderTool = EGAM4_EMShowerBuilderTool,
                                                  ClusterCorrectionToolName = "DFEgammaSWToolWithNewCells",
                                                  CaloClusterLinkName="NewSwClusterLink")
    #                                             OutputLevel=DEBUG)
    print EGAM4_EGammaReweightTool
    ToolSvc += EGAM4_EGammaReweightTool


#====================================================================
# SET UP THINNING
#====================================================================
from DerivationFrameworkCore.ThinningHelper import ThinningHelper
EGAM4ThinningHelper = ThinningHelper( "EGAM4ThinningHelper" )
EGAM4ThinningHelper.TriggerChains = '(^(?!.*_[0-9]*(j|xe|tau|ht|xs|te))(?!HLT_[eg].*_[0-9]*[eg][0-9].*)(?!HLT_eb.*)(?!.*larpeb.*)(?!HLT_.*_AFP_.*)(HLT_g[1-9].*|HLT_2g[1-9].*|HLT_mu.*|HLT_2mu.*))'
if globalflags.DataSource()!='geant4':
    ExtraContainersTrigger += ExtraContainersTriggerDataOnly
EGAM4ThinningHelper.AppendToStream( EGAM4Stream, ExtraContainersTrigger )

thinningTools=[]

# Track thinning
if jobproperties.egammaDFFlags.doEGammaDAODTrackThinning:

    TrackThinningKeepElectronTracks = True
    TrackThinningKeepPhotonTracks = True
    TrackThinningKeepAllPhotonTracks = True
    TrackThinningKeepJetTracks = False
    TrackThinningKeepMuonTracks = True
    TrackThinningKeepTauTracks = False
    TrackThinningKeepPVTracks = False

    # Tracks associated with Jets
    if (TrackThinningKeepJetTracks) : 
        from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__JetTrackParticleThinning
        EGAM4JetTPThinningTool = DerivationFramework__JetTrackParticleThinning( name                    = "EGAM4JetTPThinningTool",
                                                                                ThinningService         = EGAM4ThinningHelper.ThinningSvc(),
                                                                                JetKey                  = "AntiKt4EMTopoJets",
                                                                                InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                                ApplyAnd                = True)
        ToolSvc += EGAM4JetLCTPThinningTool
        print EGAM4JetLCTPThinningTool
        thinningTools.append(EGAM4JetLCTPThinningTool)
    
    # Tracks associated with Muons
    if (TrackThinningKeepMuonTracks) :
        from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
        EGAM4MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning( name                    = "EGAM4MuonTPThinningTool",
                                                                                  ThinningService         = EGAM4ThinningHelper.ThinningSvc(),
                                                                                  MuonKey                 = "Muons",
                                                                                  InDetTrackParticlesKey  = "InDetTrackParticles")
        ToolSvc += EGAM4MuonTPThinningTool
        print EGAM4MuonTPThinningTool
        thinningTools.append(EGAM4MuonTPThinningTool)
    
    # Tracks associated with Electrons
    if (TrackThinningKeepElectronTracks) : 
        from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
        EGAM4ElectronTPThinningTool = DerivationFramework__EgammaTrackParticleThinning( name                    = "EGAM4ElectronTPThinningTool",
                                                                                        ThinningService         = EGAM4ThinningHelper.ThinningSvc(),
                                                                                        SGKey                   = "Electrons",
                                                                                        GSFTrackParticlesKey    = "GSFTrackParticles",        
                                                                                        InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                                        SelectionString         = "Electrons.pt > 0*GeV",
                                                                                        BestMatchOnly = True,
                                                                                        ConeSize = 0.3,
                                                                                        ApplyAnd = False)
        ToolSvc += EGAM4ElectronTPThinningTool
        print EGAM4ElectronTPThinningTool
        thinningTools.append(EGAM4ElectronTPThinningTool)

    # Tracks associated with Photons
    if (TrackThinningKeepPhotonTracks) : 
        from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__EgammaTrackParticleThinning
        EGAM4PhotonTPThinningTool = DerivationFramework__EgammaTrackParticleThinning( name                    = "EGAM4PhotonTPThinningTool",
                                                                                      ThinningService         = EGAM4ThinningHelper.ThinningSvc(),
                                                                                      SGKey                   = "Photons",
                                                                                      GSFTrackParticlesKey    = "GSFTrackParticles",        
                                                                                      InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                                      SelectionString         = "Photons.pt > 0*GeV",
                                                                                      BestMatchOnly = True,
                                                                                      ConeSize = 0.3,
                                                                                      ApplyAnd = False)
        
        ToolSvc += EGAM4PhotonTPThinningTool
        print EGAM4PhotonTPThinningTool
        thinningTools.append(EGAM4PhotonTPThinningTool)

    # Tracks associated with Photons (all tracks, large cone, for track isolation studies of the selected photon)
    if (TrackThinningKeepAllPhotonTracks) : 
        EGAM4PhotonTPThinningTool2 = DerivationFramework__EgammaTrackParticleThinning( name                    = "EGAM4PhotonTPThinningTool2",
                                                                                       ThinningService         = EGAM4ThinningHelper.ThinningSvc(),
                                                                                       SGKey                   = "Photons",
                                                                                       GSFTrackParticlesKey    = "GSFTrackParticles",        
                                                                                       InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                                       SelectionString         = "Photons.pt > 9.5*GeV",
                                                                                       BestMatchOnly = False,
                                                                                       ConeSize = 0.6,
                                                                                       ApplyAnd = False)
        
        ToolSvc += EGAM4PhotonTPThinningTool2
        print EGAM4PhotonTPThinningTool2
        thinningTools.append(EGAM4PhotonTPThinningTool2)
        
    # Tracks associated with Taus
    if (TrackThinningKeepTauTracks) : 
        from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TauTrackParticleThinning
        EGAM4TauTPThinningTool = DerivationFramework__TauTrackParticleThinning( name                    = "EGAM4TauTPThinningTool",
                                                                                ThinningService         = EGAM4ThinningHelper.ThinningSvc(),
                                                                                TauKey                  = "TauJets",
                                                                                ConeSize                = 0.6,
                                                                                InDetTrackParticlesKey  = "InDetTrackParticles")
        ToolSvc += EGAM4TauTPThinningTool
        print EGAM4TauTPThinningTool
        thinningTools.append(EGAM4TauTPThinningTool)

    # Tracks from primary vertex
    if (TrackThinningKeepPVTracks) :
        from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
        EGAM4TPThinningTool = DerivationFramework__TrackParticleThinning( name                    = "EGAM4TPThinningTool",
                                                                          ThinningService         = EGAM4ThinningHelper.ThinningSvc(),
                                                                          SelectionString         = "InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm",
                                                                          InDetTrackParticlesKey  = "InDetTrackParticles")
        ToolSvc += EGAM4TPThinningTool
        print EGAM4TPThinningTool
        thinningTools.append(EGAM4TPThinningTool)

print "EGAM4 thinningTools: ", thinningTools


#=======================================
# CREATE PRIVATE SEQUENCE
#=======================================
egam4Seq = CfgMgr.AthSequencer("EGAM4Sequence")
DerivationFrameworkJob += egam4Seq


#=======================================
# CREATE THE DERIVATION KERNEL ALGORITHM   
#=======================================
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
augmentationTools = [EGAM4_MuMuMassTool,EGAM4_GainDecoratorTool]
if DoCellReweighting:
    augmentationTools += [EGAM4_NewCellTool, EGAM4_ClusterDecoratorTool, EGAM4_EGammaReweightTool]
augmentationTools += EGAM4_ClusterEnergyPerLayerDecorators
egam4Seq += CfgMgr.DerivationFramework__DerivationKernel("EGAM4Kernel",
                                                         AugmentationTools = augmentationTools,
                                                         SkimmingTools = [EGAM4_SkimmingTool],
                                                         ThinningTools = thinningTools
                                                         )


#====================================================================
# RESTORE JET COLLECTIONS REMOVED BETWEEN r20 AND r21
#====================================================================
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import replaceAODReducedJets
reducedJetList = ["AntiKt4TruthJets"]
replaceAODReducedJets(reducedJetList,egam4Seq,"EGAM4")


#====================================================================
# SET UP STREAM SELECTION   
#====================================================================
# Only events that pass the filters listed below are written out.
# Name must match that of the kernel above
# AcceptAlgs  = logical OR of filters
# RequireAlgs = logical AND of filters
EGAM4Stream.AcceptAlgs(["EGAM4Kernel"])


#====================================================================
# SET UP SLIMMING
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

EGAM4SlimmingHelper = SlimmingHelper("EGAM4SlimmingHelper")
EGAM4SlimmingHelper.SmartCollections = ["Electrons",
                                        "Photons",
                                        "Muons",
                                        "TauJets",
                                        "MET_Reference_AntiKt4EMTopo",
                                        "AntiKt4EMTopoJets",
                                        "BTagging_AntiKt4EMTopo",
                                        "InDetTrackParticles",
                                        "PrimaryVertices" ]

# Add egamma trigger objects
EGAM4SlimmingHelper.IncludeEGammaTriggerContent = True
EGAM4SlimmingHelper.IncludeMuonTriggerContent = True

# Append cell-reweighted collections to dictionary
if DoCellReweighting:
    EGAM4SlimmingHelper.AppendToDictionary = {"NewSwPhotons": "xAOD::PhotonContainer", "NewSwPhotonsAux": "xAOD::PhotonAuxContainer" }

# Extra variables
EGAM4SlimmingHelper.ExtraVariables = ExtraContentAll
EGAM4SlimmingHelper.AllVariables = ExtraContainersPhotons
EGAM4SlimmingHelper.AllVariables += ExtraContainersTrigger

if globalflags.DataSource()=='geant4':
    EGAM4SlimmingHelper.ExtraVariables += ExtraContentAllTruth
    EGAM4SlimmingHelper.AllVariables += ExtraContainersTruth
else:
    EGAM4SlimmingHelper.ExtraVariables += ExtraContainersTriggerDataOnly

for tool in EGAM4_ClusterEnergyPerLayerDecorators:
    EGAM4SlimmingHelper.ExtraVariables.extend( getClusterEnergyPerLayerDecorations( tool ) )

# Add detailed shower shape variables
from DerivationFrameworkEGamma.ElectronsCPDetailedContent import *
EGAM4SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent
EGAM4SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent
from DerivationFrameworkEGamma.PhotonsCPDetailedContent import *
EGAM4SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

# This line must come after we have finished configuring EGAM4SlimmingHelper
EGAM4SlimmingHelper.AppendContentToStream(EGAM4Stream)

#Add full CellContainer
EGAM4Stream.AddItem("CaloCellContainer#AODCellContainer")
EGAM4Stream.AddItem("CaloClusterCellLinkContainer#egammaClusters_links")

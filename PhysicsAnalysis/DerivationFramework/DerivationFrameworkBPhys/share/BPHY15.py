#====================================================================
# BPHY15.py
# Bc+>J/psiD_s+, Bc+>J/psiD+, Bc+>J/psiD*+, Bc+>J/psiD_s1+
# It requires the reductionConf flag BPHY15 in Reco_tf.py   
#====================================================================

# Set up common services and job object. 
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *

isSimulation = False
if globalflags.DataSource()=='geant4':
    isSimulation = True

print isSimulation


#====================================================================
# AUGMENTATION TOOLS 
#====================================================================
## 1/ setup vertexing tools and services
include("DerivationFrameworkBPhys/configureVertexing.py")
BPHY15_VertexTools = BPHYVertexTools("BPHY15")


from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__AugOriginalCounts
BPHY15_AugOriginalCounts = DerivationFramework__AugOriginalCounts(
   name = "BPHY15_AugOriginalCounts",
   VertexContainer = "PrimaryVertices",
   TrackContainer = "InDetTrackParticles" )
ToolSvc += BPHY15_AugOriginalCounts


#--------------------------------------------------------------------
# 2/ Select J/psi>mu+mu-
#--------------------------------------------------------------------
## a/ setup JpsiFinder tool
##    These are general tools independent of DerivationFramework that do the 
##    actual vertex fitting and some pre-selection.
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
BPHY15JpsiFinder = Analysis__JpsiFinder(
    name                       = "BPHY15JpsiFinder",
    OutputLevel                = INFO,
    muAndMu                    = True,
    muAndTrack                 = False,
    TrackAndTrack              = False,
    assumeDiMuons              = True, 
    muonThresholdPt            = 2700,
    invMassUpper               = 3600.0,
    invMassLower               = 2600.0,
    Chi2Cut                    = 30.,
    oppChargesOnly	       = True,
    combOnly		       = True,
    atLeastOneComb             = False,
    useCombinedMeasurement     = False, # Only takes effect if combOnly=True	
    muonCollectionKey          = "Muons",
    TrackParticleCollection    = "InDetTrackParticles",
    V0VertexFitterTool         = BPHY15_VertexTools.TrkV0Fitter,             # V0 vertex fitter
    useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool        = BPHY15_VertexTools.TrkVKalVrtFitter,        # VKalVrt vertex fitter
    TrackSelectorTool          = BPHY15_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool = BPHY15_VertexTools.InDetConversionHelper,
    VertexPointEstimator       = BPHY15_VertexTools.VtxPointEstimator,
    useMCPCuts                 = False)

ToolSvc += BPHY15JpsiFinder
print      BPHY15JpsiFinder

#--------------------------------------------------------------------
## b/ setup the vertex reconstruction "call" tool(s). They are part of the derivation framework.
##    These Augmentation tools add output vertex collection(s) into the StoreGate and add basic 
##    decorations which do not depend on the vertex mass hypothesis (e.g. lxy, ptError, etc).
##    There should be one tool per topology, i.e. Jpsi and Psi(2S) do not need two instance of the
##    Reco tool is the JpsiFinder mass window is wide enough.
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_mumu
BPHY15JpsiSelectAndWrite = DerivationFramework__Reco_mumu(
    name                   = "BPHY15JpsiSelectAndWrite",
    JpsiFinder             = BPHY15JpsiFinder,
    OutputVtxContainerName = "BPHY15JpsiCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    DoVertexType           = 1)

ToolSvc += BPHY15JpsiSelectAndWrite
print BPHY15JpsiSelectAndWrite

#--------------------------------------------------------------------
## c/ augment and select Jpsi->mumu candidates
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu
BPHY15_Select_Jpsi2mumu = DerivationFramework__Select_onia2mumu(
    name                  = "BPHY15_Select_Jpsi2mumu",
    HypothesisName        = "Jpsi",
    InputVtxContainerName = "BPHY15JpsiCandidates",
    VtxMassHypo           = 3096.900,
    MassMin               = 2600.0,
    MassMax               = 3600.0,
    Chi2Max               = 20,
    DoVertexType          = 1)
  
ToolSvc += BPHY15_Select_Jpsi2mumu
print      BPHY15_Select_Jpsi2mumu

#--------------------------------------------------------------------
# 3/ select B_c+->J/psi pi+
#--------------------------------------------------------------------
## a/ setup a new vertexing tool (necessary due to use of mass constraint) 
from TrkVKalVrtFitter.TrkVKalVrtFitterConf import Trk__TrkVKalVrtFitter
BcJpsipiVertexFit = Trk__TrkVKalVrtFitter(
    name               = "BcJpsipiVertexFit",
    Extrapolator       = BPHY15_VertexTools.InDetExtrapolator,
    FirstMeasuredPoint = True,
    MakeExtendedVertex = True)

ToolSvc += BcJpsipiVertexFit
print      BcJpsipiVertexFit

#--------------------------------------------------------------------
## b/ setup the Jpsi+1 track finder
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiPlus1Track
BPHY15BcJpsipi = Analysis__JpsiPlus1Track(
    name                    = "BPHY15BcJpsipi",
    OutputLevel             = INFO, #DEBUG,
    pionHypothesis          = True, #False,
    kaonHypothesis	    = False,#True,
    trkThresholdPt	    = 2700,
    trkMaxEta               = 2.7,
    BThresholdPt            = 100.0,
    BMassUpper	            = 6900.0,
    BMassLower	            = 5600.0,
    JpsiContainerKey        = "BPHY15JpsiCandidates",
    TrackParticleCollection = "InDetTrackParticles",
    MuonsUsedInJpsi         = "Muons",
    TrkVertexFitterTool     = BcJpsipiVertexFit,
    TrackSelectorTool       = BPHY15_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint       = True, 
    Chi2Cut 	            = 5,
    TrkTrippletMassUpper    = 6900,
    TrkTrippletMassLower    = 5600)
        
ToolSvc += BPHY15BcJpsipi
print      BPHY15BcJpsipi    

#--------------------------------------------------------------------
## c/ setup the combined augmentation/skimming tool for the Bc+>J/psi pi+
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_dimuTrk
BPHY15BcJpsipiSelectAndWrite = DerivationFramework__Reco_dimuTrk(
    name                   = "BPHY15BcJpsipiSelectAndWrite",
    Jpsi1PlusTrackName     = BPHY15BcJpsipi,
    OutputVtxContainerName = "BPHY15BcJpsipiCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "BPHY15RefittedPrimaryVertices",
    RefitPV                = True,
    MaxPVrefit		   = 1000)

ToolSvc += BPHY15BcJpsipiSelectAndWrite 
print      BPHY15BcJpsipiSelectAndWrite

#--------------------------------------------------------------------
## c/ augment and select B_c+>Jpsi pi+ candidates
BPHY15_Select_Bc2Jpsipi = DerivationFramework__Select_onia2mumu(
    name                  = "BPHY15_Select_Bc2Jpsipi",
    HypothesisName        = "Bc",
    InputVtxContainerName = "BPHY15BcJpsipiCandidates",
    TrkMasses             = [105.658, 105.658, 139.571],
    VtxMassHypo           = 6274.9,
    MassMin               = 5600.0,
    MassMax               = 6900.0,
    Chi2Max               = 200)

ToolSvc += BPHY15_Select_Bc2Jpsipi
print      BPHY15_Select_Bc2Jpsipi

#--------------------------------------------------------------------
# 4/ select J/psi pi+
#--------------------------------------------------------------------
## a/ setup the Jpsi+1 track finder
BPHY15JpsipiFinder = Analysis__JpsiPlus1Track(
    name                    = "BPHY15JpsipiFinder",
    OutputLevel             = INFO, #DEBUG,
    pionHypothesis          = True, #False,
    kaonHypothesis	    = False,#True,
    trkThresholdPt	    = 350.0,
    trkMaxEta               = 2.7,
    BThresholdPt            = 100.0,
    BMassUpper	            = 3600.0,
    BMassLower	            = 3200.0,
    JpsiContainerKey        = "BPHY15JpsiCandidates",
    TrackParticleCollection = "InDetTrackParticles",
    MuonsUsedInJpsi         = "Muons",
    TrkVertexFitterTool     = BcJpsipiVertexFit,
    TrackSelectorTool       = BPHY15_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint       = True, 
    Chi2Cut 	            = 5,
    TrkTrippletMassUpper    = 3600,
    TrkTrippletMassLower    = 3200)
        
ToolSvc += BPHY15JpsipiFinder
print      BPHY15JpsipiFinder    

#--------------------------------------------------------------------
## b/ setup the combined augmentation/skimming tool for J/psi pi+
BPHY15JpsipiSelectAndWrite = DerivationFramework__Reco_dimuTrk(
    name                   = "BPHY15JpsipiSelectAndWrite",
    Jpsi1PlusTrackName     = BPHY15JpsipiFinder,
    OutputVtxContainerName = "BPHY15JpsipiCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
   #RefitPV                = True,
    MaxPVrefit		   = 1000)

ToolSvc += BPHY15JpsipiSelectAndWrite 
print      BPHY15JpsipiSelectAndWrite


#--------------------------------------------------------------------
# 5/ select D0>pi+K- 
#--------------------------------------------------------------------
BPHY15D0Finder = Analysis__JpsiFinder(
    name                       = "BPHY15D0Finder",
    OutputLevel                = INFO,
    muAndMu                    = False,
    muAndTrack                 = False,
    TrackAndTrack              = True,
    assumeDiMuons              = False,    # If true, will assume dimu hypothesis and use PDG value for mu mass
    trackThresholdPt           = 700,
    invMassUpper               = 2200.0,
    invMassLower               = 1500.0,
    Chi2Cut                    = 20.,
    oppChargesOnly	       = True,
    atLeastOneComb             = False,
    useCombinedMeasurement     = False, # Only takes effect if combOnly=True	
    muonCollectionKey          = "Muons",
    TrackParticleCollection    = "InDetTrackParticles",
    V0VertexFitterTool         = BPHY15_VertexTools.TrkV0Fitter,             # V0 vertex fitter
    useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool        = BPHY15_VertexTools.TrkVKalVrtFitter,        # VKalVrt vertex fitter
    TrackSelectorTool          = BPHY15_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool = BPHY15_VertexTools.InDetConversionHelper,
    VertexPointEstimator       = BPHY15_VertexTools.VtxPointEstimator,
    useMCPCuts                 = False,
    track1Mass                 = 139.571, # Not very important, only used to calculate inv. mass cut, leave it loose here
    track2Mass                 = 493.677)
  
ToolSvc += BPHY15D0Finder
print      BPHY15D0Finder

#--------------------------------------------------------------------
BPHY15D0SelectAndWrite = DerivationFramework__Reco_mumu(
    name                   = "BPHY15D0SelectAndWrite",
    JpsiFinder             = BPHY15D0Finder,
    OutputVtxContainerName = "BPHY15D0Candidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    CheckCollections       = True,
    CheckVertexContainers  = ['BPHY15JpsipiCandidates'],
    DoVertexType           = 1)
  
ToolSvc += BPHY15D0SelectAndWrite
print      BPHY15D0SelectAndWrite

#--------------------------------------------------------------------
# 6/ select K_S0>pi+pi- 
#--------------------------------------------------------------------
doSimpleV0Finder = False
if doSimpleV0Finder:
  include("DerivationFrameworkBPhys/configureSimpleV0Finder.py")
else:
  include("DerivationFrameworkBPhys/configureV0Finder.py")

BPHY15_V0FinderTools = BPHYV0FinderTools("BPHY15")
print BPHY15_V0FinderTools

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_V0Finder
BPHY15_Reco_V0Finder   = DerivationFramework__Reco_V0Finder(
    name                   = "BPHY15_Reco_V0Finder",
    V0FinderTool           = BPHY15_V0FinderTools.V0FinderTool,
    #OutputLevel            = DEBUG,
    V0ContainerName        = "BPHY15RecoV0Candidates",
    KshortContainerName    = "BPHY15RecoKshortCandidates",
    LambdaContainerName    = "BPHY15RecoLambdaCandidates",
    LambdabarContainerName = "BPHY15RecoLambdabarCandidates",
    CheckVertexContainers  = ['BPHY15D0Candidates'])

ToolSvc += BPHY15_Reco_V0Finder
print BPHY15_Reco_V0Finder


#--------------------------------------------------------------------
# 7/ Select K+K- and K+pi-
#--------------------------------------------------------------------
## a/ Setup the vertex fitter tools
BPHY15phiFinder = Analysis__JpsiFinder(
    name                       = "BPHY15phiFinder",
    OutputLevel                = INFO,
    muAndMu                    = False,
    muAndTrack                 = False,
    TrackAndTrack              = True,
    assumeDiMuons              = False,    # If true, will assume dimu hypothesis and use PDG value for mu mass
    trackThresholdPt           = 700,
    invMassUpper               = 1060.0,
    invMassLower               = 980.0,
   #invMassUpper               = 1900.0,
   #invMassLower               = 280.0,
    Chi2Cut                    = 20.,
    oppChargesOnly	       = True,
    atLeastOneComb             = False,
    useCombinedMeasurement     = False, # Only takes effect if combOnly=True	
    muonCollectionKey          = "Muons",
    TrackParticleCollection    = "InDetTrackParticles",
    V0VertexFitterTool         = BPHY15_VertexTools.TrkV0Fitter,             # V0 vertex fitter
    useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool        = BPHY15_VertexTools.TrkVKalVrtFitter,        # VKalVrt vertex fitter
    TrackSelectorTool          = BPHY15_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool = BPHY15_VertexTools.InDetConversionHelper,
    VertexPointEstimator       = BPHY15_VertexTools.VtxPointEstimator,
    useMCPCuts                 = False,
    track1Mass                 = 493.677, # Not very important, only used to calculate inv. mass cut, leave it loose here
    track2Mass                 = 493.677)
   #track1Mass                 = 139.571, # Not very important, only used to calculate inv. mass cut, leave it loose here
   #track2Mass                 = 139.571)
  
ToolSvc += BPHY15phiFinder
print      BPHY15phiFinder


#--------------------------------------------------------------------
## b/ setup the vertex reconstruction "call" tool(s).
BPHY15phiSelectAndWrite = DerivationFramework__Reco_mumu(
    name                   = "BPHY15phiSelectAndWrite",
    JpsiFinder             = BPHY15phiFinder,
    OutputVtxContainerName = "BPHY15phiCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    CheckCollections       = True,
    CheckVertexContainers  = ['BPHY15JpsiCandidates'],
    DoVertexType           = 1)
  
ToolSvc += BPHY15phiSelectAndWrite
print      BPHY15phiSelectAndWrite



#--------------------------------------------------------------------
# 8/ select D_s+>K+K-pi+ and D+>K+pi-pi- candidates
#--------------------------------------------------------------------
## a/ setup a new vertexing tool (necessary due to use of mass constraint) 
Dh3VertexFit = Trk__TrkVKalVrtFitter(
    name                = "Dh3VertexFit",
    Extrapolator        = BPHY15_VertexTools.InDetExtrapolator,
    FirstMeasuredPoint  = True,
    MakeExtendedVertex  = True)

ToolSvc += Dh3VertexFit
print      Dh3VertexFit

#--------------------------------------------------------------------
## b/ setup the Jpsi+1 track finder
BPHY15DsFinder = Analysis__JpsiPlus1Track(
    name                    = "BPHY15DsFinder",
    OutputLevel             = INFO,
    pionHypothesis          = True,
    kaonHypothesis          = False,
    trkThresholdPt	    = 700.0,
    trkMaxEta	            = 2.7, # is this value fine?? default would be 102.5
    BThresholdPt            = 3000.0,
   #BThresholdPt            = 100.0,
    BMassUpper              = 1800.0, # What is this??
    BMassLower       	    = 1000.0,
    TrkTrippletMassUpper    = 1800,
    TrkTrippletMassLower    = 1000,
    TrkQuadrupletPt         = 3000,
    JpsiContainerKey        = "BPHY15phiCandidates",
    TrackParticleCollection = "InDetTrackParticles",
    MuonsUsedInJpsi         = "NONE", # ?
    ExcludeCrossJpsiTracks  = False,
    TrkVertexFitterTool     = Dh3VertexFit,
    TrackSelectorTool       = BPHY15_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint       = False, 
    Chi2Cut                 = 5) #Cut on chi2/Ndeg_of_freedom, so is very loose
 
ToolSvc += BPHY15DsFinder
print      BPHY15DsFinder

#--------------------------------------------------------------------
## c/ setup the combined augmentation/skimming tool for the D(s)+
BPHY15DsSelectAndWrite = DerivationFramework__Reco_dimuTrk(
    name                   = "BPHY15DsSelectAndWrite",
    OutputLevel            = INFO,
    Jpsi1PlusTrackName     = BPHY15DsFinder,
    OutputVtxContainerName = "BPHY15DsCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    MaxPVrefit             = 1000)

ToolSvc += BPHY15DsSelectAndWrite 
print      BPHY15DsSelectAndWrite

#--------------------------------------------------------------------
BPHY15pipiFinder = Analysis__JpsiFinder(
    name                       = "BPHY15pipiFinder",
    OutputLevel                = INFO,
    muAndMu                    = False,
    muAndTrack                 = False,
    TrackAndTrack              = True,
    assumeDiMuons              = False,    # If true, will assume dimu hypothesis and use PDG value for mu mass
    trackThresholdPt           = 700,
   #invMassUpper               = 1080.0,
   #invMassLower               = 980.0,
    invMassUpper               = 1550.0,
    invMassLower               = 300.0,
   #invMassLower               = 600.0,
    Chi2Cut                    = 20.,
   #oppChargesOnly	       = True,
    oppChargesOnly	       = False,
    sameChargesOnly            = True,
    atLeastOneComb             = False,
    useCombinedMeasurement     = False, # Only takes effect if combOnly=True	
    muonCollectionKey          = "Muons",
    TrackParticleCollection    = "InDetTrackParticles",
    V0VertexFitterTool         = BPHY15_VertexTools.TrkV0Fitter,             # V0 vertex fitter
    useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
    TrkVertexFitterTool        = BPHY15_VertexTools.TrkVKalVrtFitter,        # VKalVrt vertex fitter
    TrackSelectorTool          = BPHY15_VertexTools.InDetTrackSelectorTool,
    ConversionFinderHelperTool = BPHY15_VertexTools.InDetConversionHelper,
    VertexPointEstimator       = BPHY15_VertexTools.VtxPointEstimator,
    useMCPCuts                 = False,
   #track1Mass                 = 493.677, # Not very important, only used to calculate inv. mass cut, leave it loose here
   #track2Mass                 = 493.677)
    track1Mass                 = 139.571, # Not very important, only used to calculate inv. mass cut, leave it loose here
    track2Mass                 = 139.571)
  
ToolSvc += BPHY15pipiFinder
print      BPHY15pipiFinder

#--------------------------------------------------------------------
## b/ setup the vertex reconstruction "call" tool(s).
BPHY15pipiSelectAndWrite = DerivationFramework__Reco_mumu(
    name                   = "BPHY15pipiSelectAndWrite",
    JpsiFinder             = BPHY15pipiFinder,
    OutputVtxContainerName = "BPHY15pipiCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    CheckCollections       = True,
    CheckVertexContainers  = ['BPHY15JpsiCandidates'],
    DoVertexType           = 1)
  
ToolSvc += BPHY15pipiSelectAndWrite
print      BPHY15pipiSelectAndWrite



#--------------------------------------------------------------------

#--------------------------------------------------------------------
## b/ setup the Jpsi+1 track finder
BPHY15DpFinder = Analysis__JpsiPlus1Track(
    name                    = "BPHY15DpFinder",
    OutputLevel             = INFO,
   #pionHypothesis          = True,
   #kaonHypothesis          = False,
    pionHypothesis          = False,
    kaonHypothesis          = True,
    trkThresholdPt	    = 700.0,
    trkMaxEta	            = 2.7, # is this value fine?? default would be 102.5
   #BThresholdPt            = 100.0,
    BThresholdPt            = 3000.0,
   #BMassUpper              = 1800.0, # What is this??
   #BMassLower       	    = 1100.0,
    BMassUpper              = 2050.0, # What is this??
    BMassLower       	    = 1700.0,
   #TrkTrippletMassUpper    = 1800,
   #TrkTrippletMassLower    = 1100,
    TrkTrippletMassUpper    = 2050,
    TrkTrippletMassLower    = 1700,
    TrkQuadrupletPt         = 3000,
    JpsiContainerKey        = "BPHY15pipiCandidates",
    TrackParticleCollection = "InDetTrackParticles",
    MuonsUsedInJpsi         = "NONE", # ?
    ExcludeCrossJpsiTracks  = False,
    TrkVertexFitterTool     = Dh3VertexFit,
    TrackSelectorTool       = BPHY15_VertexTools.InDetTrackSelectorTool,
    UseMassConstraint       = False, 
    Chi2Cut                 = 5) #Cut on chi2/Ndeg_of_freedom, so is very loose
 
ToolSvc += BPHY15DpFinder
print      BPHY15DpFinder

#--------------------------------------------------------------------
## c/ setup the combined augmentation/skimming tool for the D(s)+
BPHY15DpSelectAndWrite = DerivationFramework__Reco_dimuTrk(
    name                   = "BPHY15DpSelectAndWrite",
    OutputLevel            = INFO,
    Jpsi1PlusTrackName     = BPHY15DpFinder,
    OutputVtxContainerName = "BPHY15DpCandidates",
    PVContainerName        = "PrimaryVertices",
    RefPVContainerName     = "SHOULDNOTBEUSED",
    MaxPVrefit             = 1000)

ToolSvc += BPHY15DpSelectAndWrite 
print      BPHY15DpSelectAndWrite


#--------------------------------------------------------------------
# 9/ select Bc+>J/psi D_(s)+/-
#--------------------------------------------------------------------
## a/ setup the cascade vertexing tool
BcJpsiDxVertexFit = Trk__TrkVKalVrtFitter(
    name                 = "BcJpsiDxVertexFit",
    Extrapolator         = BPHY15_VertexTools.InDetExtrapolator,
   #FirstMeasuredPoint   = True,
    FirstMeasuredPoint   = False,
    CascadeCnstPrecision = 1e-6,
    MakeExtendedVertex   = True)

ToolSvc += BcJpsiDxVertexFit
print      BcJpsiDxVertexFit

#--------------------------------------------------------------------
## b/ setup the Jpsi Ds finder
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__JpsiPlusDsCascade
BPHY15JpsiDs = DerivationFramework__JpsiPlusDsCascade(
    name                     = "BPHY15JpsiDs",
    HypothesisName           = "Bc",
    TrkVertexFitterTool      = BcJpsiDxVertexFit,
   #OutputLevel              = DEBUG,
    DxHypothesis             = 431,
    ApplyDxMassConstraint    = True,
    ApplyJpsiMassConstraint  = True,
    JpsiMassLowerCut         = 2600.,
    JpsiMassUpperCut         = 3600.,
    DxMassLowerCut           = 1968.28 - 200.,
    DxMassUpperCut           = 1968.28 + 200.,
    MassLowerCut             = 6274.90 - 600.,
    MassUpperCut             = 6274.90 + 600.,
    Chi2Cut 	             = 10,
    RefitPV                  = True,
    RefPVContainerName       = "BPHY15RefittedPrimaryVertices",
    JpsiVertices             = "BPHY15JpsiCandidates",
    CascadeVertexCollections = ["BcJpsiDsCascadeSV2", "BcJpsiDsCascadeSV1"],
    DxVertices               = "BPHY15DsCandidates")

ToolSvc += BPHY15JpsiDs
print      BPHY15JpsiDs

#--------------------------------------------------------------------
## c/ setup the Jpsi D+ finder
BPHY15JpsiDp = DerivationFramework__JpsiPlusDsCascade(
    name                     = "BPHY15JpsiDp",
    HypothesisName           = "Bc",
    TrkVertexFitterTool      = BcJpsiDxVertexFit,
    DxHypothesis             = 411,
    ApplyDxMassConstraint    = True,
    ApplyJpsiMassConstraint  = True,
    JpsiMassLowerCut         = 2600.,
    JpsiMassUpperCut         = 3600.,
   #DxMassLowerCut           = 1869.59 - 200.,
   #DxMassUpperCut           = 1869.59 + 200.,
    DxMassLowerCut           = 1869.59 - 180.,
    DxMassUpperCut           = 1869.59 + 180.,
    MassLowerCut             = 6274.90 - 600.,
    MassUpperCut             = 6274.90 + 600.,
    Chi2Cut 	             = 10,
    RefitPV                  = True,
    RefPVContainerName       = "BPHY15RefittedPrimaryVertices",
    JpsiVertices             = "BPHY15JpsiCandidates",
    CascadeVertexCollections = ["BcJpsiDpCascadeSV2", "BcJpsiDpCascadeSV1"],
   #DxVertices               = "BPHY15DsCandidates")
    DxVertices               = "BPHY15DpCandidates")

ToolSvc += BPHY15JpsiDp
print      BPHY15JpsiDp

#--------------------------------------------------------------------
## d/ setup the Jpsi D- finder
BPHY15JpsiDm = DerivationFramework__JpsiPlusDsCascade(
    name                     = "BPHY15JpsiDm",
    HypothesisName           = "Bc",
    TrkVertexFitterTool      = BcJpsiDxVertexFit,
    DxHypothesis             = -411,
    ApplyDxMassConstraint    = True,
    ApplyJpsiMassConstraint  = True,
    JpsiMassLowerCut         = 2600.,
    JpsiMassUpperCut         = 3600.,
   #DxMassLowerCut           = 1869.59 - 200.,
   #DxMassUpperCut           = 1869.59 + 200.,
    DxMassLowerCut           = 1869.59 - 180.,
    DxMassUpperCut           = 1869.59 + 180.,
    MassLowerCut             = 6274.90 - 600.,
    MassUpperCut             = 6274.90 + 600.,
    Chi2Cut 	             = 10,
    RefitPV                  = True,
    RefPVContainerName       = "BPHY15RefittedPrimaryVertices",
    JpsiVertices             = "BPHY15JpsiCandidates",
    CascadeVertexCollections = ["BcJpsiDmCascadeSV2", "BcJpsiDmCascadeSV1"],
   #DxVertices               = "BPHY15DsCandidates")
    DxVertices               = "BPHY15DpCandidates")

ToolSvc += BPHY15JpsiDm
print      BPHY15JpsiDm

#--------------------------------------------------------------------
# 10/ select Bc+>J/psi D*+/-
#--------------------------------------------------------------------
## a/ setup the cascade vertexing tool
BcJpsiDstVertexFit = Trk__TrkVKalVrtFitter(
    name                 = "BcJpsiDstVertexFit",
    Extrapolator         = BPHY15_VertexTools.InDetExtrapolator,
   #FirstMeasuredPoint   = True,
    FirstMeasuredPoint   = False,
    CascadeCnstPrecision = 1e-6,
    MakeExtendedVertex   = True)

ToolSvc += BcJpsiDstVertexFit
print      BcJpsiDstVertexFit

#--------------------------------------------------------------------
## b/ setup Jpsi D*+ finder
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__JpsiPlusDpstCascade
BPHY15JpsiDpst = DerivationFramework__JpsiPlusDpstCascade(
    name                     = "BPHY15JpsiDpst",
    HypothesisName           = "Bc",
    TrkVertexFitterTool      = BcJpsiDstVertexFit,
   #OutputLevel              = DEBUG,
    DxHypothesis             = 421,
    ApplyD0MassConstraint    = True,
    ApplyJpsiMassConstraint  = True,
    JpsiMassLowerCut         = 2600.,
    JpsiMassUpperCut         = 3600.,
    JpsipiMassLowerCut       = 2600.,
    JpsipiMassUpperCut       = 6800.,
    D0MassLowerCut           = 1864.83 - 200.,
    D0MassUpperCut           = 1864.83 + 200.,
    DstMassLowerCut          = 2010.26 - 300.,
    DstMassUpperCut          = 2010.26 + 300.,
    MassLowerCut             = 6274.90 - 600,
    MassUpperCut             = 6274.90 + 600.,
    Chi2Cut 	             = 10,
    RefitPV                  = True,
    RefPVContainerName       = "BPHY15RefittedPrimaryVertices",
    JpsipiVertices           = "BPHY15JpsipiCandidates",
    CascadeVertexCollections = ["BcJpsiDpstCascadeSV2", "BcJpsiDpstCascadeSV1"],
    D0Vertices               = "BPHY15D0Candidates")

ToolSvc += BPHY15JpsiDpst
print      BPHY15JpsiDpst

#--------------------------------------------------------------------
## c/ setup Jpsi D*- finder
BPHY15JpsiDmst = DerivationFramework__JpsiPlusDpstCascade(
    name                     = "BPHY15JpsiDmst",
    HypothesisName           = "Bc",
    TrkVertexFitterTool      = BcJpsiDstVertexFit,
    DxHypothesis             = -421,
    ApplyD0MassConstraint    = True,
    ApplyJpsiMassConstraint  = True,
    JpsiMassLowerCut         = 2600.,
    JpsiMassUpperCut         = 3600.,
    JpsipiMassLowerCut       = 2600.,
    JpsipiMassUpperCut       = 6800.,
    D0MassLowerCut           = 1864.83 - 200.,
    D0MassUpperCut           = 1864.83 + 200.,
    DstMassLowerCut          = 2010.26 - 300.,
    DstMassUpperCut          = 2010.26 + 300.,
    MassLowerCut             = 6274.90 - 600,
    MassUpperCut             = 6274.90 + 600.,
    Chi2Cut 	             = 10,
    RefitPV                  = True,
    RefPVContainerName       = "BPHY15RefittedPrimaryVertices",
    JpsipiVertices           = "BPHY15JpsipiCandidates",
    CascadeVertexCollections = ["BcJpsiDmstCascadeSV2", "BcJpsiDmstCascadeSV1"],
    D0Vertices               = "BPHY15D0Candidates")

ToolSvc += BPHY15JpsiDmst
print      BPHY15JpsiDmst


#--------------------------------------------------------------------
# 11/ select Bc+>J/psi D_s1+/-
#--------------------------------------------------------------------
## a/ setup the cascade vertexing tool
BcJpsiDs1VertexFit = Trk__TrkVKalVrtFitter(
    name                 = "BcJpsiDs1VertexFit",
   #OutputLevel          = DEBUG,
    Extrapolator         = BPHY15_VertexTools.InDetExtrapolator,
   #FirstMeasuredPoint   = True,
    FirstMeasuredPoint   = False,
    CascadeCnstPrecision = 1e-6,
    MakeExtendedVertex   = True)

ToolSvc += BcJpsiDs1VertexFit
print      BcJpsiDs1VertexFit

#--------------------------------------------------------------------
## b/ setup Jpsi D_s1+ finder
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__JpsiPlusDs1Cascade
BPHY15JpsiDps1 = DerivationFramework__JpsiPlusDs1Cascade(
    name                     = "BPHY15JpsiDps1",
    HypothesisName           = "Bc",
    TrkVertexFitterTool      = BcJpsiDs1VertexFit,
    DxHypothesis             = 421,
    ApplyD0MassConstraint    = True,
    ApplyK0MassConstraint    = True,
    ApplyJpsiMassConstraint  = True,
    JpsiMassLowerCut         = 2600.,
    JpsiMassUpperCut         = 3600.,
    JpsipiMassLowerCut       = 2600.,
    JpsipiMassUpperCut       = 6800.,
    D0MassLowerCut           = 1864.83 - 200.,
    D0MassUpperCut           = 1864.83 + 200.,
    K0MassLowerCut           = 300.,
    K0MassUpperCut           = 700.,
    DstMassLowerCut          = 2010.26 - 300.,
    DstMassUpperCut          = 2010.26 + 300.,
    MassLowerCut             = 6274.90 - 600,
    MassUpperCut             = 6274.90 + 600.,
    Chi2Cut 	             = 10,
    RefitPV                  = True,
    RefPVContainerName       = "BPHY15RefittedPrimaryVertices",
    JpsipiVertices           = "BPHY15JpsipiCandidates",
    CascadeVertexCollections = ["BcJpsiDps1CascadeSV3", "BcJpsiDps1CascadeSV2", "BcJpsiDps1CascadeSV1"],
    K0Vertices               = "BPHY15RecoV0Candidates",
    D0Vertices               = "BPHY15D0Candidates")

ToolSvc += BPHY15JpsiDps1
print      BPHY15JpsiDps1

#--------------------------------------------------------------------
## c/ setup Jpsi D_s1- finder
BPHY15JpsiDms1 = DerivationFramework__JpsiPlusDs1Cascade(
    name                     = "BPHY15JpsiDms1",
    HypothesisName           = "Bc",
    TrkVertexFitterTool      = BcJpsiDs1VertexFit,
    DxHypothesis             = -421,
    ApplyD0MassConstraint    = True,
    ApplyK0MassConstraint    = True,
    ApplyJpsiMassConstraint  = True,
    JpsiMassLowerCut         = 2600.,
    JpsiMassUpperCut         = 3600.,
    JpsipiMassLowerCut       = 2600.,
    JpsipiMassUpperCut       = 6800.,
    D0MassLowerCut           = 1864.83 - 200.,
    D0MassUpperCut           = 1864.83 + 200.,
    K0MassLowerCut           = 300.,
    K0MassUpperCut           = 700.,
    DstMassLowerCut          = 2010.26 - 300.,
    DstMassUpperCut          = 2010.26 + 300.,
    MassLowerCut             = 6274.90 - 600,
    MassUpperCut             = 6274.90 + 600.,
    Chi2Cut 	             = 10,
    RefitPV                  = True,
    RefPVContainerName       = "BPHY15RefittedPrimaryVertices",
    JpsipiVertices           = "BPHY15JpsipiCandidates",
    CascadeVertexCollections = ["BcJpsiDms1CascadeSV3", "BcJpsiDms1CascadeSV2", "BcJpsiDms1CascadeSV1"],
    K0Vertices               = "BPHY15RecoV0Candidates",
    D0Vertices               = "BPHY15D0Candidates")

ToolSvc += BPHY15JpsiDms1
print BPHY15JpsiDms1
#--------------------------------------------------------------------

CascadeCollections = []

CascadeCollections += BPHY15JpsiDs.CascadeVertexCollections
CascadeCollections += BPHY15JpsiDp.CascadeVertexCollections
#CascadeCollections += BPHY15JpsiDm.CascadeVertexCollections

CascadeCollections += BPHY15JpsiDpst.CascadeVertexCollections
#CascadeCollections += BPHY15JpsiDmst.CascadeVertexCollections
CascadeCollections += BPHY15JpsiDps1.CascadeVertexCollections
#CascadeCollections += BPHY15JpsiDms1.CascadeVertexCollections

#--------------------------------------------------------------------


if not isSimulation: #Only Skim Data
   from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
   BPHY15_SelectBcJpsipiEvent = DerivationFramework__xAODStringSkimmingTool(
     name = "BPHY15_SelectBcJpsipiEvent",
    #expression = "( count(BcJpsiDsCascadeSV1.x > -999) ) > 0")
    #expression = "( count(BcJpsiDpCascadeSV1.x > -999) ) > 0")
    #expression = "( count(BcJpsiDps1CascadeSV1.x > -999) ) > 0")
    #expression = "( count(BcJpsiDpstCascadeSV1.x > -999) ) > 0")
    #expression = "( count(BcJpsiDpCascadeSV1.x > -999) + count(BcJpsiDmCascadeSV1.x > -999) ) > 0")
    #expression = "( count(BcJpsiDsCascadeSV1.x > -999) + count(BcJpsiDpCascadeSV1.x > -999) + count(BcJpsiDmCascadeSV1.x > -999) ) > 0")
     expression = "(count(BPHY15BcJpsipiCandidates.passed_Bc > 0) + count(BcJpsiDsCascadeSV1.x > -999) + count(BcJpsiDpCascadeSV1.x > -999) + count(BcJpsiDpstCascadeSV1.x > -999) + count(BcJpsiDps1CascadeSV1.x > -999) ) > 0")
    #expression = "(count(BPHY15BcJpsipiCandidates.passed_Bc > 0) + count(BcJpsiDsCascadeSV1.x > -999) + count(BcJpsiDpCascadeSV1.x > -999) + count(BcJpsiDmCascadeSV1.x > -999) + count(BcJpsiDpstCascadeSV1.x > -999) + count(BcJpsiDmstCascadeSV1.x > -999) + count(BcJpsiDps1CascadeSV1.x > -999) + count(BcJpsiDms1CascadeSV1.x > -999) ) > 0")
   
   ToolSvc += BPHY15_SelectBcJpsipiEvent
   print      BPHY15_SelectBcJpsipiEvent

   #====================================================================
   # Make event selection based on an OR of the input skimming tools
   #====================================================================
      
   from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationOR
   BPHY15SkimmingOR = CfgMgr.DerivationFramework__FilterCombinationOR(
       "BPHY15SkimmingOR",
       FilterList = [BPHY15_SelectBcJpsipiEvent] )
   ToolSvc += BPHY15SkimmingOR
   print      BPHY15SkimmingOR

#--------------------------------------------------------------------
##10/ track and vertex thinning. We want to remove all reconstructed secondary vertices
##    which hasn't passed any of the selections defined by (Select_*) tools.
##    We also want to keep only tracks which are associates with either muons or any of the
##    vertices that passed the selection. Multiple thinning tools can perform the 
##    selection. The final thinning decision is based OR of all the decisions (by default,
##    although it can be changed by the JO).

## a) thining out vertices that didn't pass any selection and idetifying tracks associated with 
##    selected vertices. The "VertexContainerNames" is a list of the vertex containers, and "PassFlags"
##    contains all pass flags for Select_* tools that must be satisfied. The vertex is kept is it 
##    satisfy any of the listed selections.
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Thin_vtxTrk
BPHY15_thinningTool_Tracks = DerivationFramework__Thin_vtxTrk(
    name                       = "BPHY15_thinningTool_Tracks",
    ThinningService            = "BPHY15ThinningSvc",
    TrackParticleContainerName = "InDetTrackParticles",
   #VertexContainerNames       = ["BPHY15BcJpsipiCandidates", "BcJpsiDsCascadeSV1", "BcJpsiDsCascadeSV2", "BcJpsiDpCascadeSV1", "BcJpsiDpCascadeSV2", "BcJpsiDmCascadeSV1","BcJpsiDmCascadeSV2", "BcJpsiDpstCascadeSV1", "BcJpsiDpstCascadeSV2", "BcJpsiDmstCascadeSV1", "BcJpsiDmstCascadeSV2", "BcJpsiDps1CascadeSV1", "BcJpsiDps1CascadeSV2", "BcJpsiDps1CascadeSV3", "BcJpsiDms1CascadeSV1", "BcJpsiDms1CascadeSV2", "BcJpsiDms1CascadeSV3"],
    VertexContainerNames       = ["BPHY15BcJpsipiCandidates", "BcJpsiDsCascadeSV1", "BcJpsiDsCascadeSV2", "BcJpsiDpCascadeSV1", "BcJpsiDpCascadeSV2", "BcJpsiDpstCascadeSV1", "BcJpsiDpstCascadeSV2", "BcJpsiDps1CascadeSV1", "BcJpsiDps1CascadeSV2", "BcJpsiDps1CascadeSV3"],
   #VertexContainerNames       = ["BcJpsiDsCascadeSV1", "BcJpsiDsCascadeSV2"],
   #VertexContainerNames       = ["BcJpsiDpCascadeSV1", "BcJpsiDpCascadeSV2"],
   #VertexContainerNames       = ["BcJpsiDps1CascadeSV1", "BcJpsiDps1CascadeSV2"],
   #VertexContainerNames       = ["BcJpsiDpstCascadeSV1", "BcJpsiDpstCascadeSV2"],
   #VertexContainerNames       = ["BcJpsiDpCascadeSV1", "BcJpsiDpCascadeSV2", "BcJpsiDmCascadeSV1","BcJpsiDmCascadeSV2"],
   #VertexContainerNames       = ["BcJpsiDsCascadeSV1", "BcJpsiDsCascadeSV2", "BcJpsiDpCascadeSV1", "BcJpsiDpCascadeSV2", "BcJpsiDmCascadeSV1","BcJpsiDmCascadeSV2"],
    PassFlags                  = ["passed_Bc"])

ToolSvc += BPHY15_thinningTool_Tracks
print      BPHY15_thinningTool_Tracks

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__BPhysPVThinningTool
BPHY15_thinningTool_PV = DerivationFramework__BPhysPVThinningTool(
    name                 = "BPHY15_thinningTool_PV",
    ThinningService      = "BPHY15ThinningSvc",
   #CandidateCollections = ["BcJpsiDsCascadeSV1", "BcJpsiDsCascadeSV2"],
   #CandidateCollections = ["BcJpsiDps1CascadeSV1", "BcJpsiDps1CascadeSV2"],
   #CandidateCollections = ["BcJpsiDpstCascadeSV1", "BcJpsiDpstCascadeSV2"],
   #CandidateCollections = ["BcJpsiDpCascadeSV1", "BcJpsiDpCascadeSV2"],
   #CandidateCollections = ["BcJpsiDpCascadeSV1", "BcJpsiDpCascadeSV2", "BcJpsiDmCascadeSV1","BcJpsiDmCascadeSV2"],
   #CandidateCollections = ["BcJpsiDsCascadeSV1", "BcJpsiDsCascadeSV2", "BcJpsiDpCascadeSV1", "BcJpsiDpCascadeSV2", "BcJpsiDmCascadeSV1","BcJpsiDmCascadeSV2"],
    CandidateCollections = ["BPHY15BcJpsipiCandidates", "BcJpsiDsCascadeSV1", "BcJpsiDsCascadeSV2", "BcJpsiDpCascadeSV1", "BcJpsiDpCascadeSV2", "BcJpsiDpstCascadeSV1", "BcJpsiDpstCascadeSV2", "BcJpsiDps1CascadeSV1", "BcJpsiDps1CascadeSV2", "BcJpsiDps1CascadeSV3"],
   #CandidateCollections = ["BPHY15BcJpsipiCandidates", "BcJpsiDsCascadeSV1", "BcJpsiDsCascadeSV2", "BcJpsiDpCascadeSV1", "BcJpsiDpCascadeSV2", "BcJpsiDmCascadeSV1","BcJpsiDmCascadeSV2", "BcJpsiDpstCascadeSV1", "BcJpsiDpstCascadeSV2", "BcJpsiDmstCascadeSV1", "BcJpsiDmstCascadeSV2", "BcJpsiDps1CascadeSV1", "BcJpsiDps1CascadeSV2", "BcJpsiDps1CascadeSV3", "BcJpsiDms1CascadeSV1", "BcJpsiDms1CascadeSV2", "BcJpsiDms1CascadeSV3"],
    KeepPVTracks         = True)

ToolSvc += BPHY15_thinningTool_PV
print      BPHY15_thinningTool_PV

## b) thinning out tracks that are not attached to muons. The final thinning decision is based on the OR operation
##    between decision from this and the previous tools.
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
BPHY15MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(
    name                   = "BPHY15MuonTPThinningTool",
    ThinningService        = "BPHY15ThinningSvc",
    MuonKey                = "Muons",
    InDetTrackParticlesKey = "InDetTrackParticles")

ToolSvc += BPHY15MuonTPThinningTool
print      BPHY15MuonTPThinningTool

#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS  
#====================================================================

thiningCollection = [] 

print thiningCollection

# The name of the kernel (BPHY15Kernel in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel(
    "BPHY15Kernel",
    AugmentationTools = [BPHY15JpsiSelectAndWrite, BPHY15_Select_Jpsi2mumu,
                         BPHY15BcJpsipiSelectAndWrite, BPHY15_Select_Bc2Jpsipi,
                         BPHY15JpsipiSelectAndWrite,
                         BPHY15D0SelectAndWrite,
                         BPHY15_Reco_V0Finder, 
                         BPHY15phiSelectAndWrite,
                         BPHY15DsSelectAndWrite,
                         BPHY15JpsiDs,
                         BPHY15pipiSelectAndWrite,
                         BPHY15DpSelectAndWrite,
                         BPHY15JpsiDp,#BPHY15JpsiDm, 
                         BPHY15JpsiDpst,#BPHY15JpsiDmst,
                         BPHY15JpsiDps1,#BPHY15JpsiDms1,
                         BPHY15_AugOriginalCounts],
    #Only skim if not MC
    SkimmingTools     = [BPHY15SkimmingOR] if not isSimulation else [],
    ThinningTools     = thiningCollection
    )

#====================================================================
# SET UP STREAM   
#====================================================================
streamName   = derivationFlags.WriteDAOD_BPHY15Stream.StreamName
fileName     = buildFileName( derivationFlags.WriteDAOD_BPHY15Stream )
BPHY15Stream  = MSMgr.NewPoolRootStream( streamName, fileName )
BPHY15Stream.AcceptAlgs(["BPHY15Kernel"])

# Special lines for thinning
# Thinning service name must match the one passed to the thinning tools
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()

BPHY15ThinningSvc = createThinningSvc( svcName="BPHY15ThinningSvc", outStreams=[evtStream] )
svcMgr += BPHY15ThinningSvc

#====================================================================
# Slimming 
#====================================================================

# Added by ASC
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
BPHY15SlimmingHelper = SlimmingHelper("BPHY15SlimmingHelper")
AllVariables  = []
StaticContent = []

# Needed for trigger objects
BPHY15SlimmingHelper.IncludeMuonTriggerContent  = TRUE
BPHY15SlimmingHelper.IncludeBPhysTriggerContent = TRUE

## primary vertices
AllVariables  += ["PrimaryVertices"]
StaticContent += ["xAOD::VertexContainer#BPHY15RefittedPrimaryVertices"]
StaticContent += ["xAOD::VertexAuxContainer#BPHY15RefittedPrimaryVerticesAux."]

## ID track particles
AllVariables += ["InDetTrackParticles"]

## combined / extrapolated muon track particles 
## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
##        are store in InDetTrackParticles collection)
AllVariables += ["CombinedMuonTrackParticles"]
AllVariables += ["ExtrapolatedMuonTrackParticles"]

## muon container
AllVariables += ["Muons"] 


## Jpsi candidates 
StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY15JpsiSelectAndWrite.OutputVtxContainerName]
## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY15JpsiSelectAndWrite.OutputVtxContainerName]

## Bc+>J/psi pi+ candidates
StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY15BcJpsipiSelectAndWrite.OutputVtxContainerName]
StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY15BcJpsipiSelectAndWrite.OutputVtxContainerName]

## Bc+>J/psi D_(s)+/-, J/psi D*+/- and J/psi D_s1+/- candidates
for cascades in CascadeCollections:
   StaticContent += ["xAOD::VertexContainer#%s"   %     cascades]
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascades]

# Tagging information (in addition to that already requested by usual algorithms)
AllVariables += ["GSFTrackParticles", "MuonSpectrometerTrackParticles" ] 

# Added by ASC
# Truth information for MC only
if isSimulation:
    AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

AllVariables = list(set(AllVariables)) # remove duplicates

BPHY15SlimmingHelper.AllVariables = AllVariables
BPHY15SlimmingHelper.StaticContent = StaticContent
BPHY15SlimmingHelper.SmartCollections = []

BPHY15SlimmingHelper.AppendContentToStream(BPHY15Stream)

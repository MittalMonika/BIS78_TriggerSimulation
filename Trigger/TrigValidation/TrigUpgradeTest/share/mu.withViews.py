#
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration 
# 
#  OutputLevel: INFO < DEBUG < VERBOSE 
# 

include("TrigUpgradeTest/testHLT_MT.py") 

from AthenaCommon.DetFlags import DetFlags

### workaround to prevent online trigger folders to be enabled ###
from InDetTrigRecExample.InDetTrigFlags import InDetTrigFlags
InDetTrigFlags.useConditionsClasses.set_Value_and_Lock(False)

from InDetRecExample.InDetJobProperties import InDetFlags
InDetFlags.doCaloSeededBrem = False

from InDetRecExample.InDetJobProperties import InDetFlags
InDetFlags.InDet25nsec = True 
InDetFlags.doPrimaryVertex3DFinding = False 
InDetFlags.doPrintConfigurables = False
InDetFlags.doResolveBackTracks = True 
InDetFlags.doSiSPSeededTrackFinder = True
InDetFlags.doTRTPhaseCalculation = True
InDetFlags.doTRTSeededTrackFinder = True
InDetFlags.doTruth = False
InDetFlags.init()

### PixelLorentzAngleSvc and SCTLorentzAngleSvc ###
include("InDetRecExample/InDetRecConditionsAccess.py")

from InDetRecExample.InDetKeys import InDetKeys

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

### If inputFile is BS(ByteStream), the bool is true. ###
isData = False 
if globalflags.InputFormat.is_bytestream():
  isData = True

### provide a minimal menu information ###
if isData:
  topSequence.L1DecoderTest.ctpUnpacker.OutputLevel=DEBUG
  topSequence.L1DecoderTest.roiUnpackers[0].OutputLevel=DEBUG
  topSequence.L1DecoderTest.roiUnpackers[1].OutputLevel=DEBUG
else:
  pass
 
### for Desplaying StoreGate Dump ###
from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr.StoreGateSvc=Service("StoreGateSvc") 
ServiceMgr.StoreGateSvc.Dump=True 
 
### for Control Flow ###
from AthenaCommon.CFElements import parOR, seqAND, seqOR, stepSeq
from DecisionHandling.DecisionHandlingConf import RoRSeqFilter, DumpDecisions

 
from AthenaCommon.AlgScheduler import AlgScheduler
AlgScheduler.CheckDependencies( True )
AlgScheduler.OutputLevel( DEBUG )
AlgScheduler.ShowDataDependencies( True )
AlgScheduler.setDataLoaderAlg( 'SGInputLoader' )


from AthenaCommon.CfgGetter import getPublicTool, getPublicToolClone
from AthenaCommon import CfgMgr

TriggerFlags.doID   = False;
TriggerFlags.doMuon = True;

### Set muon sequence ###
if not 'doL2SA' in dir():
  doL2SA = True
if not 'doL2CB' in dir():
  doL2CB = True
  TriggerFlags.doID = True
if not 'doL2ISO' in dir():
  doL2ISO = True 
if not 'doEFSA' in dir():
  doEFSA = True

### muon thresholds ###
testChains = ["HLT_mu6", "HLT_2mu6"]


# ===============================================================================================
#               Setup the standard muon chain 
# ===============================================================================================

### ************* PRD data provider algorithm ************* ###

### Used the algorithms as Step2 "muComb step" ###

if doL2CB==True:
  from TrigUpgradeTest.InDetSetup import makeInDetAlgs

  (viewAlgs, eventAlgs) = makeInDetAlgs()
  
  from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinder_Muon
  theFTF = TrigFastTrackFinder_Muon()
  theFTF.OutputLevel = DEBUG
  theFTF.TracksName = "TrigFastTrackFinder_MuTracks"
  theFTF.isRoI_Seeded = True
  viewAlgs.append(theFTF)

  ### A simple algorithm to confirm that data has been inherited from parent view ###
  ### Required to satisfy data dependencies                                       ###
  ViewVerify = CfgMgr.AthViews__ViewDataVerifier("muFastViewDataVerifier")
  ViewVerify.DataObjects = [('xAOD::L2StandAloneMuonContainer','StoreGateSvc+MuonL2SAInfo')]
  viewAlgs.append(ViewVerify)


### Used the algorithms as Step1 "muFast step" ###

### Load data from Muon detectors ###
import MuonRecExample.MuonRecStandaloneOnlySetup
from MuonCombinedRecExample.MuonCombinedRecFlags import muonCombinedRecFlags
muonRecFlags.doTrackPerformance    = True
muonRecFlags.TrackPerfSummaryLevel = 2
muonRecFlags.TrackPerfDebugLevel   = 5
muonRecFlags.doNSWNewThirdChain    = False
muonCombinedRecFlags.doCaloTrkMuId = False
muonCombinedRecFlags.printSummary = False
from RecExConfig.RecFlags import rec
from AthenaCommon.AlgSequence import AthSequencer
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm

### Provide Muon_PrepDataAlgorithms ###
from TrigUpgradeTest.MuonSetup import makeMuonPrepDataAlgs
( eventAlgs_Muon, viewAlgs_MuL2SA, viewAlgs_MuEFSA ) = makeMuonPrepDataAlgs( doL2SA, doEFSA )


### ************* Step1  ************* ###
if TriggerFlags.doMuon:

  svcMgr.ToolSvc.TrigDataAccess.ApplyOffsetCorrection = False

  ### set up L1RoIsFilter ###
  filterL1RoIsAlg = RoRSeqFilter("filterL1RoIsAlg")
  filterL1RoIsAlg.Input = ["MURoIDecisions"]
  filterL1RoIsAlg.Output = ["FilteredMURoIDecisions"]
  filterL1RoIsAlg.Chains = testChains
  filterL1RoIsAlg.OutputLevel = DEBUG

  if doL2SA:
    ### set the EVCreator ###
    l2MuViewNode = parOR("l2MuViewNode")

    l2MuViewsMaker = EventViewCreatorAlgorithm("l2MuViewsMaker", OutputLevel=DEBUG)
    l2MuViewsMaker.ViewFallThrough = True
    l2MuViewsMaker.InputMakerInputDecisions = filterL1RoIsAlg.Output 
    l2MuViewsMaker.InputMakerOutputDecisions = ["MURoIDecisionsOutput"]
    l2MuViewsMaker.RoIsLink = "initialRoI" # -||-
    l2MuViewsMaker.InViewRoIs = "MURoIs" # contract with the consumer
    l2MuViewsMaker.Views = "MUViewRoIs"
    l2MuViewsMaker.ViewNodeName = l2MuViewNode.name()

    ### Define input data of L2MuonSA PRD provider algorithms  ###
    ### and Define EventViewNodes to run the algprithms        ###
    for eventAlg_Muon in eventAlgs_Muon:
      if eventAlg_Muon.properties().has_key("RoIs"):
        eventAlg_Muon.RoIs = l2MuViewsMaker.InViewRoIs
        
    #for viewAlg_MuL2SA in viewAlgs_MuL2SA:
    #  l2MuViewNode += viewAlg_MuL2SA

    ### set up MuFastSteering ###
    from TrigL2MuonSA.TrigL2MuonSAConfig import TrigL2MuonSAMTConfig
    muFastAlg = TrigL2MuonSAMTConfig("Muon")
    muFastAlg.OutputLevel = DEBUG
    muFastAlg.MuRoIs = l2MuViewsMaker.InViewRoIs
    muFastAlg.RecMuonRoI = "RecMURoIs"
    muFastAlg.MuonL2SAInfo = "MuonL2SAInfo"
    muFastAlg.MuonCalibrationStream = "MuonCalibrationStream"
    muFastAlg.forID = "forID"
    muFastAlg.forMS = "forMS"

    ### To create BS->RDO data                                                                     ###
    ### viewAlgs_MuL2SA should has                                                                 ###
    ###      [CSCRdoTool, MDTRdoTool, MDTRawTool, RPCRdoTool, RPCRawTool, TGCRdoTool, TGCRawTool ] ###
    ### CSCRDO ###
    if muonRecFlags.doCSCs():
      from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__CscDataPreparator
      L2CscDataPreparator = TrigL2MuonSA__CscDataPreparator(OutputLevel = DEBUG,
                                                            CscPrepDataProvider = viewAlgs_MuL2SA[0])
      ToolSvc += L2CscDataPreparator
       
      muFastAlg.DataPreparator.CSCDataPreparator = L2CscDataPreparator

    ### MDTRDO ###
    if muonRecFlags.doMDTs():
      from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__MdtDataPreparator
      L2MdtDataPreparator = TrigL2MuonSA__MdtDataPreparator(OutputLevel = DEBUG,
                                                            MdtPrepDataProvider = viewAlgs_MuL2SA[1],
                                                            MDT_RawDataProvider = viewAlgs_MuL2SA[2])
      ToolSvc += L2MdtDataPreparator
      
      muFastAlg.DataPreparator.MDTDataPreparator = L2MdtDataPreparator

    l2MuViewNode += muFastAlg
 
    ### RPCRDO ###
    if muonRecFlags.doRPCs():
      from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__RpcDataPreparator
      L2RpcDataPreparator = TrigL2MuonSA__RpcDataPreparator(OutputLevel = DEBUG,
                                                            RpcPrepDataProvider = viewAlgs_MuL2SA[3],
                                                            RpcRawDataProvider = viewAlgs_MuL2SA[4],
                                                            DecodeBS = DetFlags.readRDOBS.RPC_on())
      ToolSvc += L2RpcDataPreparator
       
      muFastAlg.DataPreparator.RPCDataPreparator = L2RpcDataPreparator

    ### TGCRDO ###
    if muonRecFlags.doTGCs():
      from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__TgcDataPreparator
      L2TgcDataPreparator = TrigL2MuonSA__TgcDataPreparator(OutputLevel         = DEBUG,
                                                            TgcPrepDataProvider = viewAlgs_MuL2SA[5],
                                                            TGC_RawDataProvider = viewAlgs_MuL2SA[6])
      ToolSvc += L2TgcDataPreparator
       
      muFastAlg.DataPreparator.TGCDataPreparator = L2TgcDataPreparator

   
    ### set up MuFastHypo ###
    from TrigMuonHypo.TrigMuonHypoConfigMT import TrigMufastHypoConfig
    trigMufastHypo = TrigMufastHypoConfig("TrigL2MufastHypoAlg")
    trigMufastHypo.OutputLevel = DEBUG
    trigMufastHypo.MuonL2SAInfoFromMuFastAlg = muFastAlg.MuonL2SAInfo
    trigMufastHypo.HypoOutputDecisions = "L2MuonFastDecisions"
    trigMufastHypo.HypoInputDecisions = l2MuViewsMaker.InputMakerOutputDecisions[0]
    trigMufastHypo.HypoTools = [ trigMufastHypo.TrigMufastHypoToolFromName( "TrigL2MufastHypoTool", c ) for c in testChains ] 

    ### set the dumper ###
    muFastDecisionsDumper = DumpDecisions("muFastDecisionsDumper", OutputLevel=DEBUG, Decisions = trigMufastHypo.HypoOutputDecisions )

    ### make sequences ###
    l2muFastSequence = seqAND("l2muFastSequence", [ l2MuViewsMaker, l2MuViewNode, trigMufastHypo ])

    muFastStep = stepSeq("muFastStep", filterL1RoIsAlg, [ l2muFastSequence,  muFastDecisionsDumper ] )


### ************* Step2  ************* ###

  if doL2CB:
    ### RoRSeqFilter step2 ###
    filterL2SAAlg = RoRSeqFilter("filterL2SAAlg")
    filterL2SAAlg.Input = [trigMufastHypo.HypoOutputDecisions]
    filterL2SAAlg.Output = ["Filtered"+trigMufastHypo.HypoOutputDecisions]
    filterL2SAAlg.Chains = testChains
    filterL2SAAlg.OutputLevel = DEBUG

    ### set the EVCreator ###
    l2muCombViewNode = parOR("l2muCombViewNode")

    l2muCombViewsMaker = EventViewCreatorAlgorithm("l2muCombViewsMaker", OutputLevel=DEBUG)
    l2muCombViewsMaker.ViewFallThrough = True
 
    l2muCombViewsMaker.InputMakerInputDecisions = [ filterL2SAAlg.Output[0] ] # Output of TrigMufastHypo
    l2muCombViewsMaker.InputMakerOutputDecisions = [ "MUL2SADecisionsOutput" ] 
    l2muCombViewsMaker.RoIsLink = "roi" # -||-
    l2muCombViewsMaker.InViewRoIs = "MUTrkRoIs" # contract with the consumer
    l2muCombViewsMaker.Views = "MUTrkViewRoIs"
    l2muCombViewsMaker.ViewNodeName = l2muCombViewNode.name()

    ### Define input data of Inner Detector algorithms  ###
    ### and Define EventViewNodes to run the algprithms ###
    TrackParticlesName = ""
    for viewAlg in viewAlgs:
      l2muCombViewNode += viewAlg
      if viewAlg.properties().has_key("RoIs"):
        viewAlg.RoIs = l2muCombViewsMaker.InViewRoIs
      if viewAlg.properties().has_key("roiCollectionName"):
        viewAlg.roiCollectionName = l2muCombViewsMaker.InViewRoIs
      if viewAlg.name() == "InDetTrigTrackParticleCreatorAlg":
        TrackParticlesName = viewAlg.TrackParticlesName
        viewAlg.TrackName = theFTF.TracksName

    ### please read out TrigmuCombMTConfig file ###
    ### and set up to run muCombMT algorithm    ###
    from TrigmuComb.TrigmuCombMTConfig import TrigmuCombMTConfig
    muCombAlg = TrigmuCombMTConfig("Muon", theFTF.getName())
    muCombAlg.OutputLevel = DEBUG
    muCombAlg.L2StandAloneMuonContainerName = muFastAlg.MuonL2SAInfo
    muCombAlg.TrackParticlesContainerName = TrackParticlesName
    muCombAlg.L2CombinedMuonContainerName = "MuonL2CBInfo"

    l2muCombViewNode += muCombAlg

    ### set up muCombHypo algorithm ###
    from TrigMuonHypo.TrigMuonHypoConfigMT import TrigmuCombHypoConfig
    trigmuCombHypo = TrigmuCombHypoConfig("TrigL2muCombHypoAlg")
    trigmuCombHypo.OutputLevel = DEBUG 
    trigmuCombHypo.HypoOutputDecisions = "MuonL2CBDecisions"
    trigmuCombHypo.HypoInputDecisions = l2muCombViewsMaker.InputMakerOutputDecisions[0]
    trigmuCombHypo.MuonL2CBInfoFromMuCombAlg = muCombAlg.L2CombinedMuonContainerName 
    trigmuCombHypo.HypoTools = [ trigmuCombHypo.TrigmuCombHypoToolFromName( "TrigL2muCombHypoTool", c ) for c in testChains ] 
  
    ### set the dumper ###
    muCombDecisionsDumper = DumpDecisions("muCombDecisionsDumper", OutputLevel=DEBUG, Decisions = trigmuCombHypo.HypoOutputDecisions )
 
    ### Define a Sequence to run for muComb ### 
    l2muCombSequence = seqAND("l2muCombSequence", eventAlgs + [l2muCombViewsMaker, l2muCombViewNode, trigmuCombHypo ] )

    muCombStep = stepSeq("muCombStep", filterL2SAAlg, [ l2muCombSequence,  muCombDecisionsDumper ] )


### ************* Step3  ************* ###

  if doEFSA:

    ### RoRSeqFilter step2 ###
    filterEFSAAlg = RoRSeqFilter("filterEFSAAlg")
    if doL2CB and doL2SA:
      filterEFSAAlg.Input = [trigmuCombHypo.HypoOutputDecisions]
      filterEFSAAlg.Output = ["Filtered"+trigmuCombHypo.HypoOutputDecisions]
    else :
      # for now just use the L1 input when L2 is not running
      filterEFSAAlg.Input = ["MURoIDecisions"]
      filterEFSAAlg.Output = ["FilteredMURoIDecisionsForEF"]
    filterEFSAAlg.Chains = testChains
    filterEFSAAlg.OutputLevel = DEBUG

    ### set the EVCreator ###
    efMuViewNode = parOR("efMuViewNode")

    efMuViewsMaker = EventViewCreatorAlgorithm("efMuViewsMaker", OutputLevel=DEBUG)
    efMuViewsMaker.ViewFallThrough = True
    # probably wrong input to the EVMaker
    efMuViewsMaker.InputMakerInputDecisions = filterL1RoIsAlg.Output
    efMuViewsMaker.InputMakerOutputDecisions = ["MURoIDecisionsOutputEF"]
    efMuViewsMaker.RoIsLink = "initialRoI" # -||-
    efMuViewsMaker.InViewRoIs = "MURoIs" # contract with the consumer
    efMuViewsMaker.Views = "EFMUViewRoIs"
    efMuViewsMaker.ViewNodeName = efMuViewNode.name()

    ### Define input data of L2MuonSA PRD provider algorithms  ###
    ### and Define EventViewNodes to run the algprithms        ### 
    for eventAlg_Muon in eventAlgs_Muon:
      if eventAlg_Muon.properties().has_key("RoIs"):
        eventAlg_Muon.RoIs = efMuViewsMaker.InViewRoIs
        
    for viewAlg_MuEFSA in viewAlgs_MuEFSA:
      efMuViewNode += viewAlg_MuEFSA

   
    from TrkDetDescrSvc.TrkDetDescrSvcConf import Trk__TrackingVolumesSvc
    ServiceMgr += Trk__TrackingVolumesSvc("TrackingVolumesSvc",BuildVolumesFromTagInfo = False)

    theSegmentFinder = CfgGetter.getPublicToolClone("MuonSegmentFinder","MooSegmentFinder")
    theSegmentFinder.DoSummary=True
    CfgGetter.getPublicTool("MuonLayerHoughTool").DoTruth=False
    theSegmentFinderAlg=CfgMgr.MooSegmentFinderAlg( "MuonSegmentMaker",
                                                    SegmentFinder=theSegmentFinder,
                                                    MuonSegmentOutputLocation = "MooreSegments",
                                                    UseCSC = muonRecFlags.doCSCs(),
                                                    UseMDT = muonRecFlags.doMDTs(),
                                                    UseRPC = muonRecFlags.doRPCs(),
                                                    UseTGC = muonRecFlags.doTGCs(),
                                                    doClusterTruth=False,
                                                    UseTGCPriorBC = False,
                                                    UseTGCNextBC  = False,
                                                    doTGCClust = muonRecFlags.doTGCClusterSegmentFinding(),
                                                    doRPCClust = muonRecFlags.doRPCClusterSegmentFinding(), OutputLevel=DEBUG )
    

 
    theNCBSegmentFinderAlg=CfgMgr.MooSegmentFinderAlg( "MuonSegmentMaker_NCB",
                                                       SegmentFinder = getPublicToolClone("MooSegmentFinder_NCB","MuonSegmentFinder",
                                                                                          DoSummary=False,
                                                                                          Csc2dSegmentMaker = getPublicToolClone("Csc2dSegmentMaker_NCB","Csc2dSegmentMaker",
                                                                                                                                 segmentTool = getPublicToolClone("CscSegmentUtilTool_NCB",
                                                                                                                                                                  "CscSegmentUtilTool",
                                                                                                                                                                  TightenChi2 = False, 
                                                                                                                                                                  IPconstraint=False)),
                                                                                          Csc4dSegmentMaker = getPublicToolClone("Csc4dSegmentMaker_NCB","Csc4dSegmentMaker",
                                                                                                                                 segmentTool = getPublicTool("CscSegmentUtilTool_NCB")),
                                                                                          DoMdtSegments=False,DoSegmentCombinations=False,DoSegmentCombinationCleaning=False),
                                                       MuonPatternCombinationLocation = "NCB_MuonHoughPatternCombinations", 
                                                       MuonSegmentOutputLocation = "NCB_MuonSegments", 
                                                       MuonSegmentCombinationOutputLocation = "NCB_MooreSegmentCombinations",
                                                       UseCSC = muonRecFlags.doCSCs(),
                                                       UseMDT = False,
                                                       UseRPC = False,
                                                       UseTGC = False,
                                                       UseTGCPriorBC = False,
                                                       UseTGCNextBC  = False,
                                                       doTGCClust = False,
                                                       doRPCClust = False)

    from MuonRecExample.MuonStandalone import MuonTrackSteering
    MuonTrackSteering.DoSummary=True
    MuonTrackSteering.DoSummary=DEBUG
    TrackBuilder = CfgMgr.MuPatTrackBuilder("MuPatTrackBuilder" )
    TrackBuilder.TrackSteering=CfgGetter.getPublicToolClone("MuonTrackSteering", "MuonTrackSteering")

    from AthenaCommon.Include import include
    include("InDetBeamSpotService/BeamCondSvc.py" )        
    from xAODTrackingCnv.xAODTrackingCnvConf import xAODMaker__TrackParticleCnvAlg, xAODMaker__TrackCollectionCnvTool, xAODMaker__RecTrackParticleContainerCnvTool
  
    muonParticleCreatorTool = getPublicTool("MuonParticleCreatorTool")
  
    muonTrackCollectionCnvTool = xAODMaker__TrackCollectionCnvTool( name = "MuonTrackCollectionCnvTool", TrackParticleCreator = muonParticleCreatorTool )
  
    muonRecTrackParticleContainerCnvTool = xAODMaker__RecTrackParticleContainerCnvTool(name = "MuonRecTrackParticleContainerCnvTool", TrackParticleCreator = muonParticleCreatorTool )
 
    xAODTrackParticleCnvAlg = xAODMaker__TrackParticleCnvAlg( name = "MuonStandaloneTrackParticleCnvAlg", 
                                                              TrackParticleCreator = muonParticleCreatorTool,
                                                              TrackCollectionCnvTool=muonTrackCollectionCnvTool,
                                                              RecTrackParticleContainerCnvTool = muonRecTrackParticleContainerCnvTool,
                                                              TrackContainerName = "MuonSpectrometerTracks",
                                                              xAODTrackParticlesFromTracksContainerName = "MuonSpectrometerTrackParticles",
                                                              ConvertTrackParticles = False,
                                                              ConvertTracks = True)


    thetrkbuilder = getPublicToolClone("CombinedMuonTrackBuilder_SA", "CombinedMuonTrackBuilder", MuonHoleRecovery="", CaloMaterialProvider='TMEF_TrkMaterialProviderTool')

    theCandidateTool = getPublicToolClone("MuonCandidateTool_SA", "MuonCandidateTool", TrackBuilder=thetrkbuilder)
    theMuonCandidateAlg=CfgMgr.MuonCombinedMuonCandidateAlg("MuonCandidateAlg",MuonCandidateTool=theCandidateTool)


    thecreatortool= getPublicToolClone("MuonCreatorTool_SA", "MuonCreatorTool", ScatteringAngleTool="", CaloMaterialProvider='TMEF_TrkMaterialProviderTool', MuonSelectionTool="", FillTimingInformation=False, OutputLevel=DEBUG)

    themuoncreatoralg = CfgMgr.MuonCreatorAlg("MuonCreatorAlg")
    themuoncreatoralg.MuonCreatorTool=thecreatortool
    themuoncreatoralg.CreateSAmuons=True
    themuoncreatoralg.MakeClusters=False
    themuoncreatoralg.MuonContainerLocation = "Muons"

    #Algorithms to views
    efMuViewNode += theSegmentFinderAlg
#    efMuViewNode += theNCBSegmentFinderAlg #The configuration still needs some sorting out for this so disabled for now.
    efMuViewNode += TrackBuilder
    efMuViewNode += xAODTrackParticleCnvAlg
    efMuViewNode += theMuonCandidateAlg
    efMuViewNode += themuoncreatoralg

    #Setup MS-only hypo
    from TrigMuonHypo.TrigMuonHypoConfigMT import TrigMuonEFMSonlyHypoConfig
    trigMuonEFSAHypo = TrigMuonEFMSonlyHypoConfig("TrigMuonEFSAHypoAlg")
    trigMuonEFSAHypo.OutputLevel = DEBUG
    trigMuonEFSAHypo.MuonDecisions = "Muons"
    trigMuonEFSAHypo.HypoOutputDecisions = "EFMuonSADecisions"
    trigMuonEFSAHypo.HypoInputDecisions = efMuViewsMaker.InputMakerOutputDecisions[0]

    trigMuonEFSAHypo.HypoTools = [ trigMuonEFSAHypo.TrigMuonEFMSonlyHypoToolFromName( "TrigMuonEFMSonlyHypoTool", c ) for c in testChains ] 

    muonEFSADecisionsDumper = DumpDecisions("muonEFSADecisionsDumper", OutputLevel=DEBUG, Decisions = trigMuonEFSAHypo.HypoOutputDecisions )

    muonEFSAStep = seqAND("muonEFSAStep", [filterEFSAAlg, efMuViewsMaker, efMuViewNode, trigMuonEFSAHypo, muonEFSADecisionsDumper])


  if doL2CB and doL2ISO:
    ### RoRSeqFilter step2 for muIso ###
    filterL2MuisoAlg = RoRSeqFilter("filterL2MuisoAlg")
    filterL2MuisoAlg.Input = [trigmuCombHypo.HypoOutputDecisions]
    filterL2MuisoAlg.Output = ["Filtered"+trigmuCombHypo.HypoOutputDecisions+"_Isolation"]
    filterL2MuisoAlg.Chains = testChains
    filterL2MuisoAlg.OutputLevel = DEBUG
    
    ### set the EVCreator ###    
    l2muIsoViewNode = parOR("l2muIsoViewNode")

    l2muIsoViewsMaker = EventViewCreatorAlgorithm("l2muIsoViewsMaker", OutputLevel=DEBUG)
    l2muIsoViewsMaker.ViewFallThrough = True 
    l2muIsoViewsMaker.InputMakerInputDecisions = [ filterL2MuisoAlg.Output[0] ] # Output of TrigMufastHypo
    l2muIsoViewsMaker.InputMakerOutputDecisions = [ "MUL2MuisoDecisionsOutput" ] 
    l2muIsoViewsMaker.RoIsLink = "roi" # -||-
    l2muIsoViewsMaker.InViewRoIs = "MUIsoRoIs" # contract with the consumer
    l2muIsoViewsMaker.Views = "MUIsoViewRoIs"
    l2muIsoViewsMaker.ViewNodeName = l2muIsoViewNode.name()

    from TrigmuIso.TrigmuIsoConfig import TrigmuIsoMTConfig
    trigL2muIso = TrigmuIsoMTConfig("L2muIso")
    trigL2muIso.OutputLevel = DEBUG
    trigL2muIso.MuonL2CBInfoName = muCombAlg.L2CombinedMuonContainerName
    trigL2muIso.TrackParticlesName = TrackParticlesName
    trigL2muIso.MuonL2ISInfoName = "MuonL2ISInfo"

    l2muIsoViewNode += trigL2muIso

    from TrigMuonHypo.TrigMuonHypoConfigMT import TrigMuisoHypoConfig
    trigmuIsoHypo = TrigMuisoHypoConfig("TrigL2MuisoHypoAlg")
    trigmuIsoHypo.OutputLevel = DEBUG
    trigmuIsoHypo.MuonL2ISInfoName = trigL2muIso.MuonL2ISInfoName
    trigmuIsoHypo.HypoOutputDecisions = "MuonL2IsoDecisions"
    trigmuIsoHypo.HypoInputDecisions = l2muIsoViewsMaker.InputMakerOutputDecisions[0]
    trigmuIsoHypo.HypoTools = [ trigmuIsoHypo.TrigMuisoHypoToolFromName( "TrigL2MuisoHypoTool", c ) for c in testChains ] 

    ### set the dumper ###
    muIsoDecisionsDumper = DumpDecisions("muIsoDecisionsDumper", OutputLevel=DEBUG, Decisions = trigmuIsoHypo.HypoOutputDecisions )
    
    ### Define a Sequence to run for muIso ### 
    muIsoStep = seqAND("muIsoStep", [ filterL2MuisoAlg, l2muIsoViewsMaker, l2muIsoViewNode, trigmuIsoHypo, muIsoDecisionsDumper ] )


# ===============================================================================================
#               Setup CF(Control Flow)
# ===============================================================================================

### CF construction ###
def summarySteps ( name, decisions ):
  from DecisionHandling.DecisionHandlingConf import TriggerSummaryAlg
  summarySteps = TriggerSummaryAlg( "TriggerSummary"+name )
  summarySteps.InputDecision = "HLTChains"
  summarySteps.HLTSummary = "MonitoringSummary"+name
  summarySteps.OutputLevel = DEBUG
  summarySteps.FinalDecisions = decisions
  return summarySteps


def muonViewsMergers( name ):
  from TrigOutputHandling.TrigOutputHandlingConf import HLTEDMCreator
  muonViewsMerger = HLTEDMCreator("muonViewsMerger_" + name )
  muonViewsMerger.TrigCompositeContainer = [ "MURoIDecisions", "HLTChainsResult", "MonitoringSummaryStep1", "MonitoringSummaryStep2", "MonitoringSummaryStep3"]
  muonViewsMerger.OutputLevel = VERBOSE

  if doL2SA==True:
    muonViewsMerger.TrigCompositeContainer += [ filterL1RoIsAlg.Output[0], trigMufastHypo.HypoOutputDecisions ]
    muonViewsMerger.L2StandAloneMuonContainerViews = [ l2MuViewsMaker.Views ]
    muonViewsMerger.L2StandAloneMuonContainerInViews = [ muFastAlg.MuonL2SAInfo ]
    muonViewsMerger.L2StandAloneMuonContainer = [ muFastAlg.MuonL2SAInfo ]

  if doL2CB==True:
    muonViewsMerger.TrigCompositeContainer += [ filterL2SAAlg.Output[0], trigmuCombHypo.HypoOutputDecisions ]
    muonViewsMerger.TrackParticleContainerViews = [ l2muCombViewsMaker.Views ]
    muonViewsMerger.TrackParticleContainerInViews = [ TrackParticlesName ]
    muonViewsMerger.TrackParticleContainer = [ TrackParticlesName ]

    muonViewsMerger.L2CombinedMuonContainerViews = [ l2muCombViewsMaker.Views ]
    muonViewsMerger.L2CombinedMuonContainerInViews = [ muCombAlg.L2CombinedMuonContainerName ]
    muonViewsMerger.L2CombinedMuonContainer = [ muCombAlg.L2CombinedMuonContainerName ]

  if doEFSA==True:
    muonViewsMerger.TrigCompositeContainer += [ filterEFSAAlg.Output[0], trigMuonEFSAHypo.HypoOutputDecisions ]
    muonViewsMerger.MuonContainerViews = [ efMuViewsMaker.Views ]
    muonViewsMerger.MuonContainerInViews = [ themuoncreatoralg.MuonContainerLocation ]
    muonViewsMerger.MuonContainer = [ themuoncreatoralg.MuonContainerLocation ]

  if doL2CB==True and doL2ISO==True: # L2CB should be also executed with L2ISO
    muonViewsMerger.TrigCompositeContainer += [ filterL2MuisoAlg.Output[0], trigmuIsoHypo.HypoOutputDecisions ]
    muonViewsMerger.L2IsoMuonContainerViews = [ l2muIsoViewsMaker.Views ]
    muonViewsMerger.L2IsoMuonContainerInViews = [ trigL2muIso.MuonL2ISInfoName ]
    muonViewsMerger.L2IsoMuonContainer = [ trigL2muIso.MuonL2ISInfoName ]


  return muonViewsMerger


def muonStreamESD( muonViewsMerger ):
  import AthenaPoolCnvSvc.WriteAthenaPool
  from OutputStreamAthenaPool.OutputStreamAthenaPool import  createOutputStream
  StreamESD=createOutputStream("StreamESD","myMuonESD.pool.root",True)
  topSequence.remove( StreamESD )

  def addTC(name):
     StreamESD.ItemList += [ "xAOD::TrigCompositeContainer#"+name, "xAOD::TrigCompositeAuxContainer#"+name+"Aux." ]

  for tc in muonViewsMerger.TrigCompositeContainer:
     addTC( tc + "_remap" )

  addTC("HLTSummary")

  StreamESD.ItemList += [ "EventInfo#ByteStreamEventInfo" ]

  StreamESD.ItemList += [ "TrigRoiDescriptorCollection#EMRoIs" ]
  StreamESD.ItemList += [ "TrigRoiDescriptorCollection#JRoIs" ]
  StreamESD.ItemList += [ "TrigRoiDescriptorCollection#METRoI" ]
  StreamESD.ItemList += [ "TrigRoiDescriptorCollection#MURoIs" ]
  StreamESD.ItemList += [ "TrigRoiDescriptorCollection#TAURoIs" ]

  StreamESD.ItemList += [ "ROIB::RoIBResult#*" ]

  if doL2SA==True:
    StreamESD.ItemList += [ "xAOD::L2StandAloneMuonContainer#"+muFastAlg.MuonL2SAInfo ]
    StreamESD.ItemList += [ "xAOD::L2StandAloneMuonAuxContainer#"+muFastAlg.MuonL2SAInfo+"Aux." ]

  if doL2CB==True:
    StreamESD.ItemList += [ "xAOD::TrackParticleContainer#"+TrackParticlesName,
                            "xAOD::L2CombinedMuonContainer#"+muCombAlg.L2CombinedMuonContainerName ]
    StreamESD.ItemList += [ "xAOD::TrackParticleAuxContainer#"+TrackParticlesName+"Aux.",
                            "xAOD::L2CombinedMuonAuxContainer#"+muCombAlg.L2CombinedMuonContainerName+"Aux." ]

  if doEFSA==True:
    StreamESD.ItemList += [ "xAOD::MuonContainer#"+themuoncreatoralg.MuonContainerLocation ]
    StreamESD.ItemList += [ "xAOD::MuonAuxContainer#"+themuoncreatoralg.MuonContainerLocation+"Aux." ]

  if doL2CB==True and doL2ISO==True:
    StreamESD.ItemList += [ "xAOD::L2IsoMuonContainer#"+trigL2muIso.MuonL2ISInfoName ]
    StreamESD.ItemList += [ "xAOD::L2IsoMuonAuxContainer#"+trigL2muIso.MuonL2ISInfoName+"Aux." ]

  print "ESD file content "
  num = 0;
  for Item in StreamESD.ItemList:
    print " Contents["+str(num)+"] : "+Item
    num+=1

  return StreamESD




### NO Trackinhg ###
if TriggerFlags.doMuon==True and TriggerFlags.doID==False:    
  from DecisionHandling.DecisionHandlingConf import TriggerSummaryAlg 
  if doL2SA==True and doL2CB==False and doEFSA==False and doL2ISO==False:
    summary0 = summarySteps("Step1", ["L2MuonFastDecisions"] )
    step0 = parOR("step0", [ muFastStep, summary0 ] )
    step0filter = parOR("step0filter", [ filterL1RoIsAlg ] )
    HLTsteps = seqAND("HLTsteps", [ step0filter, step0 ]  )

    muonViewsMerger = muonViewsMergers("onlyL2SA")

    ### final summary
    summary = summarySteps("FinalAlg", ["L2MuonFastDecisions"] )
    summary.OutputTools = [ muonViewsMerger ]

    ### final monitor algorithm
    from TrigSteerMonitor.TrigSteerMonitorConf import TrigSignatureMoniMT, DecisionCollectorTool
    mon = TrigSignatureMoniMT()
    mon.FinalDecisions = [ "L2MuonFastDecisions", "WhateverElse" ]
    from TrigUpgradeTest.TestUtils import MenuTest
    mon.ChainsList = list( set( MenuTest.CTPToChainMapping.keys() ) )
    mon.OutputLevel = DEBUG

    step1Collector = DecisionCollectorTool("Step1Collector")
    step1Collector.Decisions = [ "L2MuonFastDecisions" ]
    mon.CollectorTools = [ step1Collector ]

    StreamESD = muonStreamESD(muonViewsMerger)

    hltTop = seqOR( "hltTop", [ HLTsteps, mon, summary, StreamESD ] )
    topSequence += hltTop   


  if doL2SA==False and doL2CB==False and doEFSA==True and doL2ISO==False:
    summary0 = summarySteps("Step1", ["EFMuonSADecisions"] )
    step0 = parOR("step0", [ muonEFSAStep, summary0 ] )
    step0filter = parOR("step0filter", [ filterEFSAAlg ] )
    HLTsteps = seqAND("HLTsteps", [ step0filter, step0 ]  )

    muonViewsMerger = muonViewsMergers("onlyEFSA")

    ### final summary
    summary = summarySteps("FinalAlg", ["EFMuonSADecisions"] )
    summary.OutputTools = [ muonViewsMerger ]

    ### final monitor algorithm
    from TrigSteerMonitor.TrigSteerMonitorConf import TrigSignatureMoniMT, DecisionCollectorTool
    mon = TrigSignatureMoniMT()
    mon.FinalDecisions = [ "EFMuonSADecisions", "WhateverElse" ]
    from TrigUpgradeTest.TestUtils import MenuTest
    mon.ChainsList = list( set( MenuTest.CTPToChainMapping.keys() ) )
    mon.OutputLevel = DEBUG

    step1Collector = DecisionCollectorTool("Step1Collector")
    step1Collector.Decisions = [ "EFMuonSADecisions"]
    mon.CollectorTools = [ step1Collector ]

    StreamESD = muonStreamESD(muonViewsMerger)

    hltTop = seqOR( "hltTop", [ HLTsteps, mon, summary, StreamESD ] )
    topSequence += hltTop   


  if doL2SA==True and doEFSA==True and doL2CB==False and doL2ISO==False:
    summary0 = summarySteps("Step1", ["L2MuonFastDecisions"] )
    step0 = parOR("step0", [ muFastStep, summary0 ] )
    summary1 = summarySteps("Step2", ["EFMuonSADecisions"] )
    step1 = parOR("step1", [ muonEFSAStep, summary1 ] )
    step0filter = parOR("step0filter", [ filterL1RoIsAlg ] )
    step1filter = parOR("step1filter", [ filterEFSAAlg ] )
    HLTsteps = seqAND("HLTsteps", [ step0filter, step0, step1filter, step1 ]  )

    muonViewsMerger = muonViewsMergers("MSonly")

    ### final summary
    summary = summarySteps("FinalAlg", ["EFMuonSADecisions"] )
    summary.OutputTools = [ muonViewsMerger ]

    ### final monitor algorithm
    from TrigSteerMonitor.TrigSteerMonitorConf import TrigSignatureMoniMT, DecisionCollectorTool
    mon = TrigSignatureMoniMT()
    mon.FinalDecisions = [ "EFMuonSADecisions", "WhateverElse" ]
    from TrigUpgradeTest.TestUtils import MenuTest
    mon.ChainsList = list( set( MenuTest.CTPToChainMapping.keys() ) )
    mon.OutputLevel = DEBUG

    step1Collector = DecisionCollectorTool("Step1Collector")
    step1Collector.Decisions = [ "L2MuonFastDecisions" ]
    step2Collector = DecisionCollectorTool("Step2Collector")
    step2Collector.Decisions = [ "EFMuonSADecisions"]
    mon.CollectorTools = [ step1Collector, step2Collector ]

    StreamESD = muonStreamESD(muonViewsMerger)

    hltTop = seqOR( "hltTop", [ HLTsteps, mon, summary, StreamESD ] )
    topSequence += hltTop   


### Use Tracking ###
if TriggerFlags.doMuon==True and TriggerFlags.doID==True:    
  if doL2SA==True and doL2CB==True and doEFSA==False and doL2ISO==False:
    summary0 = summarySteps("Step1", ["L2MuonFastDecisions"] )
    step0 = parOR("step0", [ muFastStep, summary0 ] )
    summary1 = summarySteps("Step2", ["MuonL2CBDecisions"] )
    step1 = parOR("step1", [ muCombStep, summary1 ] )
    step0filter = parOR("step0filter", [ filterL1RoIsAlg ] )
    step1filter = parOR("step1filter", [ filterL2SAAlg ] )
    HLTsteps = seqAND("HLTsteps", [ step0filter, step0, step1filter, step1 ]  )

    muonViewsMerger = muonViewsMergers("L2SAandL2CB")

    ### final summary
    summary = summarySteps("FinalAlg", ["MuonL2CBDecisions"] )
    summary.OutputTools = [ muonViewsMerger ]

    ### final monitor algorithm
    from TrigSteerMonitor.TrigSteerMonitorConf import TrigSignatureMoniMT, DecisionCollectorTool
    mon = TrigSignatureMoniMT()
    mon.FinalDecisions = [ "MuonL2CBDecisions", "WhateverElse" ]
    from TrigUpgradeTest.TestUtils import MenuTest
    mon.ChainsList = list( set( MenuTest.CTPToChainMapping.keys() ) )
    mon.OutputLevel = DEBUG

    step1Collector = DecisionCollectorTool("Step1Collector")
    step1Collector.Decisions = [ "L2MuonFastDecisions" ]
    step2Collector = DecisionCollectorTool("Step2Collector")
    step2Collector.Decisions = [ "MuonL2CBDecisions" ]
    mon.CollectorTools = [ step1Collector, step2Collector ]

    StreamESD = muonStreamESD(muonViewsMerger)

    hltTop = seqOR( "hltTop", [ HLTsteps, mon, summary, StreamESD ] )
    topSequence += hltTop   


  if doL2SA==True and doL2CB==True and doEFSA==True and doL2ISO==False:
    summary0 = summarySteps("Step1", ["L2MuonFastDecisions"] )
    step0 = parOR("step0", [ muFastStep, summary0 ] )
    summary1 = summarySteps("Step2", ["MuonL2CBDecisions"] )
    step1 = parOR("step1", [ muCombStep, summary1 ] )
    summary2 = summarySteps("Step3", ["EFMuonSADecisions"] )
    step2 = parOR("step2", [ muonEFSAStep, summary2 ] )
    step0filter = parOR("step0filter", [ filterL1RoIsAlg ] )
    step1filter = parOR("step1filter", [ filterL2SAAlg ] )
    step2filter = parOR("step2filter", [ filterEFSAAlg] )
    HLTsteps = seqAND("HLTsteps", [ step0filter, step0, step1filter, step1, step2filter, step2 ]  )

    muonViewsMerger = muonViewsMergers("L2SAandL2CBandEFSA")

    ### final summary
    summary = summarySteps("FinalAlg", ["EFMuonSADecisions"] )
    summary.OutputTools = [ muonViewsMerger ]

    ### final monitor algorithm
    from TrigSteerMonitor.TrigSteerMonitorConf import TrigSignatureMoniMT, DecisionCollectorTool
    mon = TrigSignatureMoniMT()
    mon.FinalDecisions = [ "EFMuonSADecisions", "WhateverElse" ]
    from TrigUpgradeTest.TestUtils import MenuTest
    mon.ChainsList = list( set( MenuTest.CTPToChainMapping.keys() ) )
    mon.OutputLevel = DEBUG

    step1Collector = DecisionCollectorTool("Step1Collector")
    step1Collector.Decisions = [ "L2MuonFastDecisions" ]
    step2Collector = DecisionCollectorTool("Step2Collector")
    step2Collector.Decisions = [ "MuonL2CBDecisions" ]
    step3Collector = DecisionCollectorTool("Step3Collector")
    step3Collector.Decisions = [ "EFMuonSADecisions"]
    mon.CollectorTools = [ step1Collector, step2Collector, step3Collector ]

    StreamESD = muonStreamESD(muonViewsMerger)

    hltTop = seqOR( "hltTop", [ HLTsteps, mon, summary, StreamESD ] )
    topSequence += hltTop   


  if doL2SA==True and doL2CB==True and doEFSA==False and doL2ISO==True:
    summary0 = summarySteps("Step1", ["L2MuonFastDecisions"] )
    step0 = parOR("step0", [ muFastStep, summary0 ] )
    summary1 = summarySteps("Step2", ["MuonL2CBDecisions"] )
    step1 = parOR("step1", [ muCombStep, summary1 ] )
    summary2 = summarySteps("Step3", ["MuonL2IsoDecisions"] )
    step2 = parOR("step2", [ muIsoStep, summary2 ] )
    step0filter = parOR("step0filter", [ filterL1RoIsAlg ] )
    step1filter = parOR("step1filter", [ filterL2SAAlg ] )
    step2filter = parOR("step2filter", [ filterL2MuisoAlg] )
    HLTsteps = seqAND("HLTsteps", [ step0filter, step0, step1filter, step1, step2filter, step2 ]  )

    muonViewsMerger = muonViewsMergers("L2SAandL2CBandL2Iso")

    ### final summary
    summary = summarySteps("FinalAlg", ["MuonL2IsoDecisions"] )
    summary.OutputTools = [ muonViewsMerger ]

    ### final monitor algorithm
    from TrigSteerMonitor.TrigSteerMonitorConf import TrigSignatureMoniMT, DecisionCollectorTool
    mon = TrigSignatureMoniMT()
    mon.FinalDecisions = [ "MuonL2IsoDecisions", "WhateverElse" ]
    from TrigUpgradeTest.TestUtils import MenuTest
    mon.ChainsList = list( set( MenuTest.CTPToChainMapping.keys() ) )
    mon.OutputLevel = DEBUG

    step1Collector = DecisionCollectorTool("Step1Collector")
    step1Collector.Decisions = [ "L2MuonFastDecisions" ]
    step2Collector = DecisionCollectorTool("Step2Collector")
    step2Collector.Decisions = [ "MuonL2CBDecisions" ]
    step3Collector = DecisionCollectorTool("Step3Collector")
    step3Collector.Decisions = [ "MuonL2IsoDecisions"]
    mon.CollectorTools = [ step1Collector, step2Collector, step3Collector ]

    StreamESD = muonStreamESD(muonViewsMerger)

    hltTop = seqOR( "hltTop", [ HLTsteps, mon, summary, StreamESD ] )
    topSequence += hltTop   

 
  if doL2SA==True and doL2CB==True and doEFSA==True and doL2ISO==True:
    summary0 = summarySteps("Step1", ["L2MuonFastDecisions"] )
    step0 = parOR("step0", [ muFastStep, summary0 ] )
    summary1 = summarySteps("Step2", ["MuonL2CBDecisions"] )
    step1 = parOR("step1", [ muCombStep, summary1 ] )
    summary2 = summarySteps("Step3", ["EFMuonSADecisions", "MuonL2IsoDecisions"] )
    step2 = parOR("step2", [ muonEFSAStep, muIsoStep, summary2 ] )
    step0filter = parOR("step0filter", [ filterL1RoIsAlg ] )
    step1filter = parOR("step1filter", [ filterL2SAAlg ] )
    step2filter = parOR("step2filter", [ filterEFSAAlg, filterL2MuisoAlg] )
    HLTsteps = seqAND("HLTsteps", [ step0filter, step0, step1filter, step1, step2filter, step2 ]  )

    muonViewsMerger = muonViewsMergers("all")

    ### final summary
    summary = summarySteps("FinalAlg", ["EFMuonSADecisions", "MuonL2IsoDecisions"] )
    summary.OutputTools = [ muonViewsMerger ]

    ### final monitor algorithm
    from TrigSteerMonitor.TrigSteerMonitorConf import TrigSignatureMoniMT, DecisionCollectorTool
    mon = TrigSignatureMoniMT()
    mon.FinalDecisions = [ "EFMuonSADecisions", "WhateverElse" ]
    from TrigUpgradeTest.TestUtils import MenuTest
    mon.ChainsList = list( set( MenuTest.CTPToChainMapping.keys() ) )
    mon.OutputLevel = DEBUG

    step1Collector = DecisionCollectorTool("Step1Collector")
    step1Collector.Decisions = [ "L2MuonFastDecisions" ]
    step2Collector = DecisionCollectorTool("Step2Collector")
    step2Collector.Decisions = [ "MuonL2CBDecisions" ]
    step3Collector = DecisionCollectorTool("Step3Collector")
    step3Collector.Decisions = [ "EFMuonSADecisions", "MuonL2IsoDecisions"]
    mon.CollectorTools = [ step1Collector, step2Collector, step3Collector ]

    StreamESD = muonStreamESD(muonViewsMerger)

    hltTop = seqOR( "hltTop", [ HLTsteps, mon, summary, StreamESD ] )
    topSequence += hltTop   

   
def TMEF_TrkMaterialProviderTool(name='TMEF_TrkMaterialProviderTool',**kwargs):
    from TrkMaterialProvider.TrkMaterialProviderConf import Trk__TrkMaterialProviderTool
    kwargs.setdefault("UseCaloEnergyMeasurement", False)
    return Trk__TrkMaterialProviderTool(name,**kwargs)

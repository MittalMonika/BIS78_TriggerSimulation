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

#doL2SA=True
doL2SA=False
#doL2CB=True
doL2CB=False
doEFSA=True

doEFIso=True

TriggerFlags.doID=True

 ### muon thresholds ###
testChains = ["HLT_mu6", "HLT_2mu6"]



# ===============================================================================================
#               Setup PrepData                                                                    
# ===============================================================================================
 
### Used the algorithms as Step2 "muComb step" ###
if TriggerFlags.doID:

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
if TriggerFlags.doMuon:
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

  if doL2SA:
    l2MuViewNode = seqAND("l2MuViewNode")
   
  if doEFSA:

    efMuViewNode = seqAND("efMuViewNode")

  if doEFSA or doL2SA:
    ### ==================== Data prepartion needed for the EF and L2 SA #######################333
    ### CSC RDO data ###
    if muonRecFlags.doCSCs():
      from MuonCSC_CnvTools.MuonCSC_CnvToolsConf import Muon__CscROD_Decoder
      CSCRodDecoder = Muon__CscROD_Decoder(name		= "CSCRodDecoder",
                                           IsCosmics		= False,
                                           IsOldCosmics 	= False,
                                           OutputLevel 	= INFO )
      ToolSvc += CSCRodDecoder
  
      from MuonCSC_CnvTools.MuonCSC_CnvToolsConf import Muon__CSC_RawDataProviderTool
      MuonCscRawDataProviderTool = Muon__CSC_RawDataProviderTool(name        = "MuonCscRawDataProviderTool",
                                                                 Decoder     = ToolSvc.CSCRodDecoder,
                                                                 OutputLevel = INFO )
      ToolSvc += MuonCscRawDataProviderTool
  
      from MuonCSC_CnvTools.MuonCSC_CnvToolsConf import Muon__CscRdoToCscPrepDataTool
      CscRdoToCscPrepDataTool = Muon__CscRdoToCscPrepDataTool(name                = "CscRdoToPrepDataTool",
                                                              OutputLevel         = INFO,
                                                              RawDataProviderTool = MuonCscRawDataProviderTool,
                                                              useBStoRdoTool	    = True)
      ToolSvc += CscRdoToCscPrepDataTool
  
      from MuonRdoToPrepData.MuonRdoToPrepDataConf import CscRdoToCscPrepData
      CscRdoToCscPrepData = CscRdoToCscPrepData(name                    = "CscRdoToCscPrepDataProvider",
                                                CscRdoToCscPrepDataTool = CscRdoToCscPrepDataTool,
                                                PrintPrepData	    	  = False,
                                                OutputLevel             = INFO)
      CscRdoToCscPrepData.DoSeededDecoding = True
      CscRdoToCscPrepData.RoIs = "MURoIs"
   
      from MuonByteStream.MuonByteStreamConf import Muon__CscRawDataProvider
      CscRawDataProvider = Muon__CscRawDataProvider(name         = "CscRawDataProvider",
                                                    ProviderTool = MuonCscRawDataProviderTool,
                                                    OutputLevel  = INFO)

      from CscClusterization.CscClusterizationConf import CscThresholdClusterBuilderTool
      CscClusterBuilderTool = CscThresholdClusterBuilderTool(name        = "CscThesholdClusterBuilderTool",
                                                             OutputLevel = INFO)
      ToolSvc += CscClusterBuilderTool

      #CSC cluster building
      from CscClusterization.CscClusterizationConf import CscThresholdClusterBuilder
      CscClusterBuilder = CscThresholdClusterBuilder(name            = "CscThesholdClusterBuilder",
                                                     OutputLevel     = INFO,
                                                     cluster_builder = CscClusterBuilderTool)    
     
      if doEFSA: 
        efMuViewNode += CscRdoToCscPrepData  
        efMuViewNode += CscClusterBuilder 
      #if doL2SA:
      #  l2MuViewNode += CscRawDataProvider
   
    ### MDT RDO data ###
    if muonRecFlags.doMDTs():
      from MuonMDT_CnvTools.MuonMDT_CnvToolsConf import MdtROD_Decoder
      MDTRodDecoder = MdtROD_Decoder(name	   = "MDTRodDecoder",
                                     OutputLevel = INFO )
      ToolSvc += MDTRodDecoder
  
      from MuonMDT_CnvTools.MuonMDT_CnvToolsConf import Muon__MDT_RawDataProviderTool
      MuonMdtRawDataProviderTool = Muon__MDT_RawDataProviderTool(name        = "MuonMdtRawDataProviderTool",
                                                                 Decoder     = ToolSvc.MDTRodDecoder,
                                                                 OutputLevel = INFO )
      ToolSvc += MuonMdtRawDataProviderTool
  
      from MuonMDT_CnvTools.MuonMDT_CnvToolsConf import Muon__MdtRdoToPrepDataTool
      MdtRdoToMdtPrepDataTool = Muon__MdtRdoToPrepDataTool(name                = "MdtRdoToPrepDataTool",
                                                           OutputLevel         = INFO,
                                                           RawDataProviderTool = MuonMdtRawDataProviderTool,
                                                           useBStoRdoTool      = True)
      ToolSvc += MdtRdoToMdtPrepDataTool
  
      from MuonRdoToPrepData.MuonRdoToPrepDataConf import MdtRdoToMdtPrepData
      MdtRdoToMdtPrepData = MdtRdoToMdtPrepData(name          = "MdtRdoToMdtPrepDataProvider",
                                                DecodingTool  = MdtRdoToMdtPrepDataTool,
                                                PrintPrepData = False,
                                                OutputLevel   = INFO)  
      MdtRdoToMdtPrepData.DoSeededDecoding = True
      MdtRdoToMdtPrepData.RoIs = "MURoIs"
      
      from MuonByteStream.MuonByteStreamConf import Muon__MdtRawDataProvider
      MdtRawDataProvider = Muon__MdtRawDataProvider(name         = "MdtRawDataProvider",
                                                    ProviderTool = MuonMdtRawDataProviderTool,
                                                    OutputLevel  = INFO)
      if doEFSA:
        efMuViewNode += MdtRdoToMdtPrepData
      #if doL2SA:
      #  l2MuViewNode += MdtRawDataProvider

    ### RPC RDO data ###
    if muonRecFlags.doRPCs():
      from MuonRPC_CnvTools.MuonRPC_CnvToolsConf import Muon__RpcROD_Decoder
      RPCRodDecoder = Muon__RpcROD_Decoder(name	 = "RPCRodDecoder",
                                           OutputLevel = INFO )
      ToolSvc += RPCRodDecoder
  
      from MuonRPC_CnvTools.MuonRPC_CnvToolsConf import Muon__RPC_RawDataProviderTool
      MuonRpcRawDataProviderTool = Muon__RPC_RawDataProviderTool(name    = "MuonRpcRawDataProviderTool",
                                                                 Decoder = RPCRodDecoder )

      ToolSvc += MuonRpcRawDataProviderTool
  
      from MuonRPC_CnvTools.MuonRPC_CnvToolsConf import Muon__RpcRdoToPrepDataTool
      RpcRdoToRpcPrepDataTool = Muon__RpcRdoToPrepDataTool(name                = "RpcRdoToPrepDataTool",
                                                           OutputLevel         = INFO)


      ToolSvc += RpcRdoToRpcPrepDataTool
  
      from MuonRdoToPrepData.MuonRdoToPrepDataConf import RpcRdoToRpcPrepData
      RpcRdoToRpcPrepData = RpcRdoToRpcPrepData(name          = "RpcRdoToRpcPrepDataProvider",
                                                DecodingTool  = RpcRdoToRpcPrepDataTool,
                                                PrintPrepData = False,
                                                OutputLevel   = INFO)    
      RpcRdoToRpcPrepData.DoSeededDecoding = True
      RpcRdoToRpcPrepData.RoIs = "MURoIs"
  
      from MuonByteStream.MuonByteStreamConf import Muon__RpcRawDataProvider
      RpcRawDataProvider = Muon__RpcRawDataProvider(name         = "RpcRawDataProvider",
                                                    ProviderTool = MuonRpcRawDataProviderTool,
                                                    OutputLevel  = INFO)
      if doEFSA:
        efMuViewNode += RpcRawDataProvider
        efMuViewNode += RpcRdoToRpcPrepData
      #if doL2SA:
      #  l2MuViewNode += RpcRawDataProvider

    ### TGC RDO data ###
    if muonRecFlags.doTGCs():
      from MuonTGC_CnvTools.MuonTGC_CnvToolsConf import Muon__TGC_RodDecoderReadout
      TGCRodDecoder = Muon__TGC_RodDecoderReadout(name		= "TGCRodDecoder",
                                                  OutputLevel 	= INFO )
      ToolSvc += TGCRodDecoder
  
      from MuonTGC_CnvTools.MuonTGC_CnvToolsConf import Muon__TGC_RawDataProviderTool
      MuonTgcRawDataProviderTool = Muon__TGC_RawDataProviderTool(name    = "MuonTgcRawDataProviderTool",
                                                                 Decoder = TGCRodDecoder )
      ToolSvc += MuonTgcRawDataProviderTool
        
      from MuonTGC_CnvTools.MuonTGC_CnvToolsConf import Muon__TgcRdoToPrepDataTool
      TgcRdoToTgcPrepDataTool = Muon__TgcRdoToPrepDataTool(name                = "TgcRdoToPrepDataTool",
                                                           OutputLevel         = INFO,
                                                           RawDataProviderTool = MuonTgcRawDataProviderTool,
                                                           useBStoRdoTool      = True)
  
      ToolSvc += TgcRdoToTgcPrepDataTool

      from MuonRdoToPrepData.MuonRdoToPrepDataConf import TgcRdoToTgcPrepData
      TgcRdoToTgcPrepData = TgcRdoToTgcPrepData(name          = "TgcRdoToTgcPrepDataProvider",
                                                DecodingTool  = TgcRdoToTgcPrepDataTool,
                                                PrintPrepData = False,
                                                OutputLevel   = INFO)
      TgcRdoToTgcPrepData.DoSeededDecoding = True
      TgcRdoToTgcPrepData.RoIs = "MURoIs"


      from MuonByteStream.MuonByteStreamConf import Muon__TgcRawDataProvider
      TgcRawDataProvider = Muon__TgcRawDataProvider(name         = "TgcRawDataProvider",
                                                    ProviderTool = MuonTgcRawDataProviderTool,
                                                    OutputLevel  = INFO)
      if doEFSA:
        efMuViewNode += TgcRdoToTgcPrepData
      #if doL2SA:
      #  l2MuViewNode += TgcRawDataProvider

    
    #Run clustering
    if muonRecFlags.doCreateClusters():
      from MuonClusterization.MuonClusterizationConf import MuonClusterizationTool
      MuonClusterTool = MuonClusterizationTool(name        = "MuonClusterizationTool",
                                               OutputLevel = INFO)
      ToolSvc += MuonClusterTool
      
      from MuonClusterization.MuonClusterizationConf import MuonClusterizationAlg
      MuonClusterAlg = MuonClusterizationAlg(name                 = "MuonClusterizationAlg",
                                             OutputLevel          = INFO,
                                             ClusterTool          = MuonClusterTool,
                                             TgcPrepDataContainer = "TGC_MeasurementsAllBCs")
    
      if doEFSA:
        efMuViewNode += MuonClusterAlg 


 


# ===============================================================================================
#               Setup L2MuonSA
# ===============================================================================================
 


  svcMgr.ToolSvc.TrigDataAccess.ApplyOffsetCorrection = False

  ### set up L1RoIsFilter ###
  filterL1RoIsAlg = RoRSeqFilter("filterL1RoIsAlg")
  filterL1RoIsAlg.Input = ["MURoIDecisions"]
  filterL1RoIsAlg.Output = ["FilteredMURoIDecisions"]
  filterL1RoIsAlg.Chains = testChains
  filterL1RoIsAlg.OutputLevel = DEBUG

  if doL2SA:
    # set the EVCreator
    l2MuViewsMaker = EventViewCreatorAlgorithm("l2MuViewsMaker", OutputLevel=DEBUG)
    l2MuViewsMaker.ViewFallThrough = True
    l2MuViewsMaker.InputMakerInputDecisions = filterL1RoIsAlg.Output 
    l2MuViewsMaker.InputMakerOutputDecisions = ["MURoIDecisionsOutput"]
    l2MuViewsMaker.RoIsLink = "initialRoI" # -||-
    l2MuViewsMaker.InViewRoIs = "MURoIs" # contract with the consumer
    l2MuViewsMaker.Views = "MUViewRoIs"
    l2MuViewsMaker.ViewNodeName = l2MuViewNode.name()

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

    ### To create BS->RDO data ###
    ### RPCRDO ###
    if muonRecFlags.doRPCs():
      from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__RpcDataPreparator
      L2RpcDataPreparator = TrigL2MuonSA__RpcDataPreparator(OutputLevel = DEBUG,
                                                            RpcPrepDataProvider = RpcRdoToRpcPrepDataTool,
                                                            RpcRawDataProvider = MuonRpcRawDataProviderTool,
                                                            DecodeBS = DetFlags.readRDOBS.RPC_on())
      ToolSvc += L2RpcDataPreparator
       
      muFastAlg.DataPreparator.RPCDataPreparator = L2RpcDataPreparator

    ### TGCRDO ###
    if muonRecFlags.doTGCs():
      from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__TgcDataPreparator
      L2TgcDataPreparator = TrigL2MuonSA__TgcDataPreparator(OutputLevel         = DEBUG,
                                                            TgcPrepDataProvider = TgcRdoToTgcPrepDataTool,
                                                            TGC_RawDataProvider = MuonTgcRawDataProviderTool)
      ToolSvc += L2TgcDataPreparator
       
      muFastAlg.DataPreparator.TGCDataPreparator = L2TgcDataPreparator

    ### MDTRDO ###
    if muonRecFlags.doMDTs():
      from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__MdtDataPreparator
      L2MdtDataPreparator = TrigL2MuonSA__MdtDataPreparator(OutputLevel = DEBUG,
                                                            MDT_RawDataProvider = MuonMdtRawDataProviderTool,
                                                            MdtPrepDataProvider = MdtRdoToMdtPrepDataTool)
      ToolSvc += L2MdtDataPreparator
      
      muFastAlg.DataPreparator.MDTDataPreparator = L2MdtDataPreparator

    ### CSCRDO ###
    if muonRecFlags.doCSCs():
      from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__CscDataPreparator
      L2CscDataPreparator = TrigL2MuonSA__CscDataPreparator(OutputLevel = DEBUG,
                                                            CscPrepDataProvider = CscRdoToCscPrepDataTool)
      ToolSvc += L2CscDataPreparator
       
      muFastAlg.DataPreparator.CSCDataPreparator = L2CscDataPreparator

    l2MuViewNode += muFastAlg
    
    # set up MuFastHypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMufastHypoConfig
    trigMufastHypo = TrigMufastHypoConfig("L2MufastHypoAlg")
    trigMufastHypo.OutputLevel = DEBUG
    trigMufastHypo.MuonL2SAInfoFromMuFastAlg = muFastAlg.MuonL2SAInfo
    trigMufastHypo.HypoOutputDecisions = "L2MuonFastDecisions"
    trigMufastHypo.HypoInputDecisions = l2MuViewsMaker.InputMakerOutputDecisions[0]
    trigMufastHypo.HypoTools = [ trigMufastHypo.TrigMufastHypoToolFromName( "L2MufastHypoTool", c ) for c in testChains ] 

    # set the dumper
    muFastDecisionsDumper = DumpDecisions("muFastDecisionsDumper", OutputLevel=DEBUG, Decisions = trigMufastHypo.HypoOutputDecisions )

    # make sequences
    l2muFastSequence = seqAND("l2muFastSequence", [ l2MuViewsMaker, l2MuViewNode, trigMufastHypo ])
    muFastStep = stepSeq("muFastStep", filterL1RoIsAlg, [ l2muFastSequence,  muFastDecisionsDumper ] )



# ===============================================================================================
#               Setup muComb                                                                    
# ===============================================================================================
 
  if doL2CB:
    ### RoRSeqFilter step2 ###
    filterL2SAAlg = RoRSeqFilter("filterL2SAAlg")
    filterL2SAAlg.Input = [trigMufastHypo.HypoOutputDecisions]
    filterL2SAAlg.Output = ["Filtered"+trigMufastHypo.HypoOutputDecisions]
    filterL2SAAlg.Chains = testChains
    filterL2SAAlg.OutputLevel = DEBUG

    l2muCombViewNode = AthSequencer("l2muCombViewNode", Sequential=False, ModeOR=False, StopOverride=False)
    l2muCombViewsMaker = EventViewCreatorAlgorithm("l2muCombViewsMaker", OutputLevel=DEBUG)
    l2muCombViewsMaker.ViewFallThrough = True
 
    l2muCombViewsMaker.InputMakerInputDecisions = [ filterL2SAAlg.Output[0] ] # Output of TrigMufastHypo
    l2muCombViewsMaker.InputMakerOutputDecisions = [ filterL2SAAlg.Output[0]+"Comb" ] # Output of TrigMufastHypo
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
    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoConfig
    trigmuCombHypo = TrigmuCombHypoConfig("L2muCombHypoAlg")
    trigmuCombHypo.OutputLevel = DEBUG 
    trigmuCombHypo.HypoOutputDecisions = "MuonL2CBDecisions"
    trigmuCombHypo.HypoInputDecisions = l2muCombViewsMaker.InputMakerOutputDecisions[0]
    trigmuCombHypo.MuonL2CBInfoFromMuCombAlg = muCombAlg.L2CombinedMuonContainerName 
    trigmuCombHypo.HypoTools = [ trigmuCombHypo.TrigmuCombHypoToolFromName( "L2muCombHypoTool", c ) for c in testChains ] 
  
    # set the dumper
    muCombDecisionsDumper = DumpDecisions("muCombDecisionsDumper", OutputLevel=DEBUG, Decisions = trigmuCombHypo.HypoOutputDecisions )
 
    ### Define a Sequence to run for muComb ### 
    l2muCombSequence = seqAND("l2muCombSequence", eventAlgs + [l2muCombViewsMaker, l2muCombViewNode, trigmuCombHypo ] )
    muCombStep = stepSeq("muCombStep", filterL2SAAlg, [ l2muCombSequence,  muCombDecisionsDumper ] )


# ===============================================================================================
#               Setup EFMuonSA                                                                    
# ===============================================================================================

  if doEFSA:

    efMuViewsMaker = EventViewCreatorAlgorithm("efMuViewsMaker", OutputLevel=DEBUG)
    efMuViewsMaker.ViewFallThrough = True
    # probably wrong input to the EVMaker
    efMuViewsMaker.InputMakerInputDecisions = filterL1RoIsAlg.Output
    efMuViewsMaker.InputMakerOutputDecisions = ["MURoIDecisionsOutputEF"]
    efMuViewsMaker.RoIsLink = "initialRoI" # -||-
    efMuViewsMaker.InViewRoIs = "MURoIs" # contract with the consumer
    efMuViewsMaker.Views = "EFMUViewRoIs"
    efMuViewsMaker.ViewNodeName = efMuViewNode.name()

    
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
    themuoncreatoralg.ClusterContainerName=""

    #Algorithms to views
    efMuViewNode += theSegmentFinderAlg
#    efMuViewNode += theNCBSegmentFinderAlg #The configuration still needs some sorting out for this so disabled for now.
    efMuViewNode += TrackBuilder
    efMuViewNode += xAODTrackParticleCnvAlg
    efMuViewNode += theMuonCandidateAlg
    efMuViewNode += themuoncreatoralg

    #Setup MS-only hypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFMSonlyHypoConfig
    trigMuonEFSAHypo = TrigMuonEFMSonlyHypoConfig("MuonEFSAHypoAlg")
    trigMuonEFSAHypo.OutputLevel = DEBUG

    trigMuonEFSAHypo.MuonDecisions = "Muons"
    trigMuonEFSAHypo.HypoOutputDecisions = "EFMuonSADecisions"
    trigMuonEFSAHypo.HypoInputDecisions = efMuViewsMaker.InputMakerOutputDecisions[0]

    trigMuonEFSAHypo.HypoTools = [ trigMuonEFSAHypo.TrigMuonEFMSonlyHypoToolFromName( "TrigMuonEFMSonlyHypoTool", c ) for c in testChains ] 

    muonEFSADecisionsDumper = DumpDecisions("muonEFSADecisionsDumper", OutputLevel=DEBUG, Decisions = trigMuonEFSAHypo.HypoOutputDecisions )
    muonEFSAStep = seqAND("muonEFSAStep", [filterEFSAAlg, efMuViewsMaker, efMuViewNode, trigMuonEFSAHypo, muonEFSADecisionsDumper])


# ===============================================================================================
#               Setup CF(Control Flow)
# ===============================================================================================

  ### CF construction ###
if TriggerFlags.doMuon==True:    
  from DecisionHandling.DecisionHandlingConf import TriggerSummaryAlg 
  if doL2SA==True and doL2CB==False and doEFSA==False:
    summary = TriggerSummaryAlg( "TriggerSummaryAlg" ) 
    summary.InputDecision = "HLTChains" 
    summary.FinalDecisions = [ trigMufastHypo.HypoOutputDecisions ]
    summary.OutputLevel = DEBUG

#    step = seqAND("firstStep", [parOR("FilterStep1",[filterL1RoIsAlg]), muFastStep])

    stepfilter = parOR("stepfilter", [ filterL1RoIsAlg ] )
    HLTsteps = seqAND("HLTsteps", [ stepfilter, step0, summary ]  ) 

    mon = TriggerSummaryAlg( "TriggerMonitoringAlg" ) 
    mon.InputDecision = "HLTChains" 
    mon.FinalDecisions = [ trigMufastHypo.HypoOutputDecisions, "WhateverElse" ] 
    mon.HLTSummary = "MonitoringSummary" 
    mon.OutputLevel = DEBUG 
    hltTop = seqOR( "hltTop", [ HLTsteps, mon] )

    topSequence += hltTop  

  if doEFSA==True and doL2SA==False and doL2CB==False and doEFIso==False:
    summary = TriggerSummaryAlg( "TriggerSummaryAlg" ) 
    summary.InputDecision = "HLTChains" 
    summary.FinalDecisions = [ trigMuonEFSAHypo.HypoOutputDecisions ]
    summary.OutputLevel = DEBUG 
    step0 = parOR("step0", [ muonEFSAStep ] )
    stepfilter = parOR("stepfilter", [ filterL1RoIsAlg ] )
    HLTsteps = seqAND("HLTsteps", [ stepfilter, step0, summary ]  ) 

    mon = TriggerSummaryAlg( "TriggerMonitoringAlg" ) 
    mon.InputDecision = "HLTChains" 
    mon.FinalDecisions = [ trigMuonEFSAHypo.HypoOutputDecisions, "WhateverElse" ] 
    mon.HLTSummary = "MonitoringSummary" 
    mon.OutputLevel = DEBUG 
    hltTop = seqOR( "hltTop", [ HLTsteps, mon] )
    topSequence += hltTop 

if TriggerFlags.doMuon==True and TriggerFlags.doID==True:    
  if doL2SA==True and doL2CB==True and doEFSA==False:
    from DecisionHandling.DecisionHandlingConf import TriggerSummaryAlg 
    summary = TriggerSummaryAlg( "TriggerSummaryAlg" ) 
    summary.InputDecision = "HLTChains" 
    summary.FinalDecisions = [ trigmuCombHypo.HypoOutputDecisions ]
    summary.OutputLevel = DEBUG 
    step0 = parOR("step0", [ muFastStep ] )
    step1 = parOR("step1", [ muCombStep ] )
    step0filter = parOR("step0filter", [ filterL1RoIsAlg ] )
    step1filter = parOR("step1filter", [ filterL2SAAlg ] )
    HLTsteps = seqAND("HLTsteps", [ step0filter, step0, step1filter, step1, summary ]  ) 

    mon = TriggerSummaryAlg( "TriggerMonitoringAlg" ) 
    mon.InputDecision = "HLTChains" 
    mon.FinalDecisions = [ trigmuCombHypo.HypoOutputDecisions, "WhateverElse" ] 
    mon.HLTSummary = "MonitoringSummary" 
    mon.OutputLevel = DEBUG 
    hltTop = seqOR( "hltTop", [ HLTsteps, mon] )

    topSequence += hltTop   

if TriggerFlags.doMuon==True and TriggerFlags.doID==True:    
  if doL2SA==True and doEFSA==True and doL2CB==True:
    from DecisionHandling.DecisionHandlingConf import TriggerSummaryAlg 
    summary = TriggerSummaryAlg( "TriggerSummaryAlg" ) 
    summary.InputDecision = "HLTChains" 
    summary.FinalDecisions = [ trigMuonEFSAHypo.HypoOutputDecisions ]
    summary.OutputLevel = DEBUG 
    step0 = parOR("step0", [ muFastStep ] )
    step1 = parOR("step1", [ muCombStep ] )
    step2 = parOR("step2", [ muonEFSAStep ] )

    step0filter = parOR("step0filter", [ filterL1RoIsAlg ] )
    step1filter = parOR("step1filter", [ filterL2SAAlg ] )
    step2filter = parOR("step2filter", [ filterEFSAAlg ] )

    HLTsteps = seqAND("HLTsteps", [ step0filter, step0, step1filter, step1, step2filter, step2, summary ]  ) 

    mon = TriggerSummaryAlg( "TriggerMonitoringAlg" ) 
    mon.InputDecision = "HLTChains" 
    mon.FinalDecisions = [ trigMuonEFSAHypo.HypoOutputDecisions, "WhateverElse" ] 
    mon.HLTSummary = "MonitoringSummary" 
    mon.OutputLevel = DEBUG 
    hltTop = seqOR( "hltTop", [ HLTsteps, mon] )

    topSequence += hltTop   

if TriggerFlags.doMuon==True and TriggerFlags.doID==True:
  if doL2SA==False and doL2CB==False and doEFSA==True and doEFIso==True:
    from DecisionHandling.DecisionHandlingConf import TriggerSummaryAlg 
    summary = TriggerSummaryAlg( "TriggerSummaryAlg" ) 
    summary.InputDecision = "HLTChains" 
    summary.FinalDecisions = [ trigMuonEFSAHypo.HypoOutputDecisions ]
    summary.OutputLevel = DEBUG 
    step0 = parOR("step0", [ muonEFSAStep ] )
    stepfilter = parOR("stepfilter", [ filterL1RoIsAlg ] )
    HLTsteps = seqAND("HLTsteps", [ stepfilter, step0, summary ]  ) 

    mon = TriggerSummaryAlg( "TriggerMonitoringAlg" ) 
    mon.InputDecision = "HLTChains" 
    mon.FinalDecisions = [ trigMuonEFSAHypo.HypoOutputDecisions, "WhateverElse" ] 
    mon.HLTSummary = "MonitoringSummary" 
    mon.OutputLevel = DEBUG 
    hltTop = seqOR( "hltTop", [ HLTsteps, mon] )
    topSequence += hltTop 

   
def TMEF_TrkMaterialProviderTool(name='TMEF_TrkMaterialProviderTool',**kwargs):
    from TrkMaterialProvider.TrkMaterialProviderConf import Trk__TrkMaterialProviderTool
    kwargs.setdefault("UseCaloEnergyMeasurement", False)
    return Trk__TrkMaterialProviderTool(name,**kwargs)

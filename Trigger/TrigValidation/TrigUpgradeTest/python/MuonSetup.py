#
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Constants import VERBOSE,DEBUG, INFO

### ==================== Data prepartion needed for the EF and L2 SA ==================== ###
def makeMuonPrepDataAlgs():

  eventAlgs_MuonPRD = [] # These algs should be prepared for configuring RoIs same as muon RoIs used in viewAlg.
  viewAlgs_MuonPRD = []  # These algs should be executed to prepare muon PRDs for muFast and muEF steps.

  from MuonRecExample.MuonRecFlags import muonRecFlags
  from AthenaCommon.AppMgr import ToolSvc

  ### CSC RDO data ###
  from MuonCSC_CnvTools.MuonCSC_CnvToolsConf import Muon__CscROD_Decoder
  CSCRodDecoder = Muon__CscROD_Decoder(name		= "CscROD_Decoder",
                                       IsCosmics	= False,
                                       IsOldCosmics 	= False )
  ToolSvc += CSCRodDecoder

  from MuonCSC_CnvTools.MuonCSC_CnvToolsConf import Muon__CSC_RawDataProviderTool
  MuonCscRawDataProviderTool = Muon__CSC_RawDataProviderTool(name        = "CSC_RawDataProviderTool",
                                                             Decoder     = CSCRodDecoder )
  ToolSvc += MuonCscRawDataProviderTool

  from MuonCSC_CnvTools.MuonCSC_CnvToolsConf import Muon__CscRdoToCscPrepDataTool
  CscRdoToCscPrepDataTool = Muon__CscRdoToCscPrepDataTool(name                = "CscRdoToCscPrepDataTool",
                                                          RawDataProviderTool = MuonCscRawDataProviderTool,
                                                          useBStoRdoTool      = True)
  ToolSvc += CscRdoToCscPrepDataTool

  from MuonRdoToPrepData.MuonRdoToPrepDataConf import CscRdoToCscPrepData
  CscRdoToCscPrepData = CscRdoToCscPrepData(name                    = "CscRdoToCscPrepData",
                                            CscRdoToCscPrepDataTool = CscRdoToCscPrepDataTool,
                                            PrintPrepData           = False, 
                                            DoSeededDecoding        = True,
                                            RoIs                    = "MURoIs" )

 
  from MuonByteStream.MuonByteStreamConf import Muon__CscRawDataProvider
  CscRawDataProvider = Muon__CscRawDataProvider(name         = "CscRawDataProvider",
                                                ProviderTool = MuonCscRawDataProviderTool )

  from CscClusterization.CscClusterizationConf import CscThresholdClusterBuilderTool
  CscClusterBuilderTool = CscThresholdClusterBuilderTool(name        = "CscThesholdClusterBuilderTool" )
  ToolSvc += CscClusterBuilderTool

  #CSC cluster building
  from CscClusterization.CscClusterizationConf import CscThresholdClusterBuilder
  CscClusterBuilder = CscThresholdClusterBuilder(name            = "CscThesholdClusterBuilder",
                                                 cluster_builder = CscClusterBuilderTool)    
  
  eventAlgs_MuonPRD.append( CscRdoToCscPrepData )  
  viewAlgs_MuonPRD.append( CscRdoToCscPrepData )  
  viewAlgs_MuonPRD.append( CscClusterBuilder ) 

 
  ### MDT RDO data ###
  from MuonMDT_CnvTools.MuonMDT_CnvToolsConf import MdtROD_Decoder
  MDTRodDecoder = MdtROD_Decoder(name	   = "MdtROD_Decoder" )
  ToolSvc += MDTRodDecoder

  from MuonMDT_CnvTools.MuonMDT_CnvToolsConf import Muon__MDT_RawDataProviderTool
  MuonMdtRawDataProviderTool = Muon__MDT_RawDataProviderTool(name        = "MDT_RawDataProviderTool",
                                                             Decoder     = MDTRodDecoder )
  ToolSvc += MuonMdtRawDataProviderTool

  from MuonMDT_CnvTools.MuonMDT_CnvToolsConf import Muon__MdtRdoToPrepDataTool
  MdtRdoToMdtPrepDataTool = Muon__MdtRdoToPrepDataTool(name                = "MdtRdoToPrepDataTool",
                                                       RawDataProviderTool = MuonMdtRawDataProviderTool,
                                                       useBStoRdoTool      = True)
  ToolSvc += MdtRdoToMdtPrepDataTool

  from MuonRdoToPrepData.MuonRdoToPrepDataConf import MdtRdoToMdtPrepData
  MdtRdoToMdtPrepData = MdtRdoToMdtPrepData(name             = "MdtRdoToMdtPrepData",
                                            DecodingTool     = MdtRdoToMdtPrepDataTool,
                                            PrintPrepData    = False,
                                            DoSeededDecoding = True,
                                            RoIs             = "MURoIs")
  
  
  from MuonByteStream.MuonByteStreamConf import Muon__MdtRawDataProvider
  MdtRawDataProvider = Muon__MdtRawDataProvider(name         = "MdtRawDataProvider",
                                                ProviderTool = MuonMdtRawDataProviderTool )

  eventAlgs_MuonPRD.append( MdtRdoToMdtPrepData )
  viewAlgs_MuonPRD.append( MdtRdoToMdtPrepData )


  ### RPC RDO data ###
  from MuonRPC_CnvTools.MuonRPC_CnvToolsConf import Muon__RpcROD_Decoder
  RPCRodDecoder = Muon__RpcROD_Decoder(name	     = "RpcROD_Decoder" )
  ToolSvc += RPCRodDecoder

  from MuonRPC_CnvTools.MuonRPC_CnvToolsConf import Muon__RPC_RawDataProviderTool
  MuonRpcRawDataProviderTool = Muon__RPC_RawDataProviderTool(name    = "RPC_RawDataProviderTool",
                                                             Decoder = RPCRodDecoder )
  ToolSvc += MuonRpcRawDataProviderTool

  from MuonRPC_CnvTools.MuonRPC_CnvToolsConf import Muon__RpcRdoToPrepDataTool
  RpcRdoToRpcPrepDataTool = Muon__RpcRdoToPrepDataTool(name                = "RpcRdoToPrepDataTool")
                                                       #RawDataProviderTool = MuonRpcRawDataProviderTool,
                                                       #useBStoRdoTool      = True)
  ToolSvc += RpcRdoToRpcPrepDataTool

  from MuonRdoToPrepData.MuonRdoToPrepDataConf import RpcRdoToRpcPrepData
  RpcRdoToRpcPrepData = RpcRdoToRpcPrepData(name             = "RpcRdoToRpcPrepData",
                                            DecodingTool     = RpcRdoToRpcPrepDataTool,
                                            PrintPrepData    = False,
                                            DoSeededDecoding = True,
                                            RoIs             = "MURoIs")

  from MuonByteStream.MuonByteStreamConf import Muon__RpcRawDataProvider
  RpcRawDataProvider = Muon__RpcRawDataProvider(name         = "RpcRawDataProvider",
                                                ProviderTool = MuonRpcRawDataProviderTool,
                                                DoSeededDecoding = True,
                                                RoIs = "MURoIs")

  eventAlgs_MuonPRD.append( RpcRawDataProvider )
  eventAlgs_MuonPRD.append( RpcRdoToRpcPrepData )
  viewAlgs_MuonPRD.append( RpcRawDataProvider )
  viewAlgs_MuonPRD.append( RpcRdoToRpcPrepData )


  ### TGC RDO data ###
  from MuonTGC_CnvTools.MuonTGC_CnvToolsConf import Muon__TGC_RodDecoderReadout
  TGCRodDecoder = Muon__TGC_RodDecoderReadout(name	    = "TGC_RodDecoderReadout" )
  ToolSvc += TGCRodDecoder

  from MuonTGC_CnvTools.MuonTGC_CnvToolsConf import Muon__TGC_RawDataProviderTool
  MuonTgcRawDataProviderTool = Muon__TGC_RawDataProviderTool(name    = "TGC_RawDataProviderTool",
                                                             Decoder = TGCRodDecoder )
  ToolSvc += MuonTgcRawDataProviderTool
    
  from MuonTGC_CnvTools.MuonTGC_CnvToolsConf import Muon__TgcRdoToPrepDataTool
  TgcRdoToTgcPrepDataTool = Muon__TgcRdoToPrepDataTool(name                = "TgcRdoToPrepDataTool",
                                                       RawDataProviderTool = MuonTgcRawDataProviderTool,
                                                       useBStoRdoTool      = True)
  ToolSvc += TgcRdoToTgcPrepDataTool

  from MuonRdoToPrepData.MuonRdoToPrepDataConf import TgcRdoToTgcPrepData
  TgcRdoToTgcPrepData = TgcRdoToTgcPrepData(name             = "TgcRdoToTgcPrepData",
                                            DecodingTool     = TgcRdoToTgcPrepDataTool,
                                            PrintPrepData    = False,
                                            DoSeededDecoding = True,
                                            RoIs             = "MURoIs")

  from MuonByteStream.MuonByteStreamConf import Muon__TgcRawDataProvider
  TgcRawDataProvider = Muon__TgcRawDataProvider(name         = "TgcRawDataProvider",
                                                ProviderTool = MuonTgcRawDataProviderTool )

  eventAlgs_MuonPRD.append( TgcRdoToTgcPrepData )
  viewAlgs_MuonPRD.append( TgcRdoToTgcPrepData )


  if muonRecFlags.doCreateClusters():  
    #Run clustering
    from MuonClusterization.MuonClusterizationConf import MuonClusterizationTool
    MuonClusterTool = MuonClusterizationTool(name        = "MuonClusterizationTool" )
    ToolSvc += MuonClusterTool
    
    from MuonClusterization.MuonClusterizationConf import MuonClusterizationAlg
    MuonClusterAlg = MuonClusterizationAlg(name                 = "MuonClusterizationAlg",
                                           ClusterTool          = MuonClusterTool,
                                           TgcPrepDataContainer = "TGC_MeasurementsAllBCs")
    
    viewAlgs_MuonPRD.append( MuonClusterAlg )


  return ( eventAlgs_MuonPRD, viewAlgs_MuonPRD )


# This def should be removed in the future. 
# We have not yet integrated muon decoding tool, so this def is used now.
def makeMuFastAlgs():
   

  from MuonRecExample.MuonRecFlags import muonRecFlags
  from AthenaCommon.AppMgr import ToolSvc
  from AthenaCommon.DetFlags import DetFlags

  ### CSC RDO data ###
  from MuonCSC_CnvTools.MuonCSC_CnvToolsConf import Muon__CscROD_Decoder
  CSCRodDecoder = Muon__CscROD_Decoder(name		= "CscROD_Decoder_L2SA",
                                       IsCosmics	= False,
                                       IsOldCosmics 	= False )
  ToolSvc += CSCRodDecoder


  from MuonCSC_CnvTools.MuonCSC_CnvToolsConf import Muon__CSC_RawDataProviderTool
  MuonCscRawDataProviderTool = Muon__CSC_RawDataProviderTool(name        = "CSC_RawDataProviderTool_L2SA",
                                                             RdoLocation = "CSCRDO_L2SA",
                                                             Decoder     = CSCRodDecoder )
  ToolSvc += MuonCscRawDataProviderTool

  from MuonCSC_CnvTools.MuonCSC_CnvToolsConf import Muon__CscRdoToCscPrepDataTool
  CscRdoToCscPrepDataTool = Muon__CscRdoToCscPrepDataTool(name                = "CscRdoToCscPrepDataTool_L2SA",
                                                          RawDataProviderTool = MuonCscRawDataProviderTool,
                                                          RDOContainer        = MuonCscRawDataProviderTool.RdoLocation,
                                                          OutputCollection    = "CSC_Measurements_L2SA",
                                                          useBStoRdoTool      = True)
  ToolSvc += CscRdoToCscPrepDataTool


  from CscClusterization.CscClusterizationConf import CscThresholdClusterBuilderTool
  CscClusterBuilderTool = CscThresholdClusterBuilderTool(name        = "CscThesholdClusterBuilderTool_L2SA",
                                                         digit_key   = CscRdoToCscPrepDataTool.OutputCollection,
                                                         cluster_key = "CSC_Clusters_L2SA" )

  ToolSvc += CscClusterBuilderTool

  from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__CscDataPreparator
  L2CscDataPreparator = TrigL2MuonSA__CscDataPreparator(#OutputLevel = DEBUG,
                                                        CscPrepDataProvider  = CscRdoToCscPrepDataTool,
                                                        CscClusterProvider   = CscClusterBuilderTool,
                                                        CSCPrepDataContainer = CscClusterBuilderTool.cluster_key)
  ToolSvc += L2CscDataPreparator

 
  ### MDT RDO data ###
  from MuonMDT_CnvTools.MuonMDT_CnvToolsConf import MdtROD_Decoder
  MDTRodDecoder = MdtROD_Decoder(name	   = "MdtROD_Decoder_L2SA" )

  ToolSvc += MDTRodDecoder

  from MuonMDT_CnvTools.MuonMDT_CnvToolsConf import Muon__MDT_RawDataProviderTool
  MuonMdtRawDataProviderTool = Muon__MDT_RawDataProviderTool(name        = "MDT_RawDataProviderTool_L2SA",
                                                             RdoLocation = "MDTCSM_L2SA",
                                                             Decoder     = MDTRodDecoder )
  ToolSvc += MuonMdtRawDataProviderTool

  from MuonMDT_CnvTools.MuonMDT_CnvToolsConf import Muon__MdtRdoToPrepDataTool
  MdtRdoToMdtPrepDataTool = Muon__MdtRdoToPrepDataTool(name                = "MdtRdoToPrepDataTool_L2SA",
                                                       RawDataProviderTool = MuonMdtRawDataProviderTool,
                                                       RDOContainer        = MuonMdtRawDataProviderTool.RdoLocation,
                                                       OutputCollection    = "MDT_DriftCircles_L2SA",
                                                       useBStoRdoTool      = True)
  ToolSvc += MdtRdoToMdtPrepDataTool

 
  from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__MdtDataPreparator
  L2MdtDataPreparator = TrigL2MuonSA__MdtDataPreparator(#OutputLevel = DEBUG,
                                                        MdtPrepDataProvider  = MdtRdoToMdtPrepDataTool,
                                                        MDTPrepDataContainer = MdtRdoToMdtPrepDataTool.OutputCollection,
                                                        MDT_RawDataProvider  = MuonMdtRawDataProviderTool,
                                                        MDTCSMContainer      = MuonMdtRawDataProviderTool.RdoLocation )
  ToolSvc += L2MdtDataPreparator
  

  ### RPC RDO data ###
  from MuonRPC_CnvTools.MuonRPC_CnvToolsConf import Muon__RpcROD_Decoder
  RPCRodDecoder = Muon__RpcROD_Decoder(name  = "RpcROD_Decoder_L2SA" )

  ToolSvc += RPCRodDecoder

  from MuonRPC_CnvTools.MuonRPC_CnvToolsConf import Muon__RPC_RawDataProviderTool
  MuonRpcRawDataProviderTool = Muon__RPC_RawDataProviderTool(name        = "RPC_RawDataProviderTool_L2SA",
                                                             RdoLocation = "RPCPAD_L2SA",
                                                             RPCSec      = "RPC_SECTORLOGIC_L2SA",
                                                             Decoder     = RPCRodDecoder )
  ToolSvc += MuonRpcRawDataProviderTool

  from MuonRPC_CnvTools.MuonRPC_CnvToolsConf import Muon__RpcRdoToPrepDataTool
  RpcRdoToRpcPrepDataTool = Muon__RpcRdoToPrepDataTool(name                    = "RpcRdoToPrepDataTool_L2SA",
                                                       OutputCollection        = MuonRpcRawDataProviderTool.RdoLocation,
                                                       TriggerOutputCollection = "RPC_Measurements_L2SA",
                                                       InputCollection         = "RPC_triggerHits_L2SA")
  ToolSvc += RpcRdoToRpcPrepDataTool

  from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__RpcDataPreparator
  L2RpcDataPreparator = TrigL2MuonSA__RpcDataPreparator(#OutputLevel = DEBUG,
                                                        RpcPrepDataProvider  = RpcRdoToRpcPrepDataTool,
                                                        RpcPrepDataContainer = RpcRdoToRpcPrepDataTool.TriggerOutputCollection,
                                                        RpcRawDataProvider   = MuonRpcRawDataProviderTool,
                                                        DecodeBS = DetFlags.readRDOBS.RPC_on() )
  ToolSvc += L2RpcDataPreparator


  ### TGC RDO data ###
  from MuonTGC_CnvTools.MuonTGC_CnvToolsConf import Muon__TGC_RodDecoderReadout
  TGCRodDecoder = Muon__TGC_RodDecoderReadout(name	    = "TGC_RodDecoderReadout_L2SA" )

  ToolSvc += TGCRodDecoder

  from MuonTGC_CnvTools.MuonTGC_CnvToolsConf import Muon__TGC_RawDataProviderTool
  MuonTgcRawDataProviderTool = Muon__TGC_RawDataProviderTool(name        = "TGC_RawDataProviderTool_L2SA",
                                                             RdoLocation = "TGCRDO_L2SA",
                                                             Decoder     = TGCRodDecoder )
  ToolSvc += MuonTgcRawDataProviderTool
    
  from MuonTGC_CnvTools.MuonTGC_CnvToolsConf import Muon__TgcRdoToPrepDataTool
  TgcRdoToTgcPrepDataTool = Muon__TgcRdoToPrepDataTool(name                 = "TgcRdoToPrepDataTool_L2SA",
                                                       RawDataProviderTool  = MuonTgcRawDataProviderTool,
                                                       RDOContainer         = MuonTgcRawDataProviderTool.RdoLocation,
                                                       OutputCollection     = "TGC_Measurements_L2SA",
                                                       OutputCoinCollection = "TrigT1CoinDataCollection_L2SA",
                                                       useBStoRdoTool       = True)

  ToolSvc += TgcRdoToTgcPrepDataTool

  from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__TgcDataPreparator
  L2TgcDataPreparator = TrigL2MuonSA__TgcDataPreparator(#OutputLevel          = DEBUG,
                                                        TgcPrepDataProvider  = TgcRdoToTgcPrepDataTool,
                                                        TGCPrepDataContainer = TgcRdoToTgcPrepDataTool.OutputCollection )
  ToolSvc += L2TgcDataPreparator


  ### set up MuFastSteering ###
  from TrigL2MuonSA.TrigL2MuonSAConfig import TrigL2MuonSAMTConfig
  muFastAlg = TrigL2MuonSAMTConfig("Muon")
  
  from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__MuFastDataPreparator
  MuFastDataPreparator = TrigL2MuonSA__MuFastDataPreparator()
  MuFastDataPreparator.CSCDataPreparator = L2CscDataPreparator  
  MuFastDataPreparator.MDTDataPreparator = L2MdtDataPreparator
  MuFastDataPreparator.RPCDataPreparator = L2RpcDataPreparator
  MuFastDataPreparator.TGCDataPreparator = L2TgcDataPreparator

  muFastAlg.DataPreparator = MuFastDataPreparator

  return muFastAlg


def makeMuEFSAAlgs():

  from MuonRecExample.MuonRecFlags import muonRecFlags
  from AthenaCommon.DetFlags import DetFlags
  from AthenaCommon.AppMgr import ToolSvc
  from AthenaCommon.AppMgr import ServiceMgr
  import AthenaCommon.CfgMgr as CfgMgr
  import AthenaCommon.CfgGetter as CfgGetter

  from AthenaCommon.CfgGetter import getPublicTool, getPublicToolClone
  from AthenaCommon import CfgMgr
  
  efAlgs = [] 
  
  ### Provide Muon_PrepDataAlgorithms ###
  from TrigUpgradeTest.MuonSetup import makeMuonPrepDataAlgs
  ( eventAlgs_MuonPRD, viewAlgs_MuonPRD ) = makeMuonPrepDataAlgs()

  # setup RDO preparator algorithms 
  for viewAlg_MuonPRD in viewAlgs_MuonPRD:
    efAlgs.append( viewAlg_MuonPRD )
   
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
  
  
  theCandidateTool = getPublicToolClone("MuonCandidateTool_SA", "MuonCandidateTool", TrackBuilder="",ExtrapolationStrategy=1)

  theMuonCandidateAlg=CfgMgr.MuonCombinedMuonCandidateAlg("MuonCandidateAlg",MuonCandidateTool=theCandidateTool)
  
  
  muonparticlecreator = getPublicToolClone("MuonParticleCreator", "TrackParticleCreatorTool", UseTrackSummaryTool=False, UseMuonSummaryTool=True, KeepAllPerigee=True)
  thecreatortool= getPublicToolClone("MuonCreatorTool_SA", "MuonCreatorTool", ScatteringAngleTool="", MuonSelectionTool="", FillTimingInformation=False, UseCaloCells=False, MakeSAMuons=True, MomentumBalanceTool="",  TrackParticleCreator=muonparticlecreator, OutputLevel=DEBUG)
  
  themuoncreatoralg = CfgMgr.MuonCreatorAlg("MuonCreatorAlg", MuonCreatorTool=thecreatortool, CreateSAmuons=True, MakeClusters=False, TagMaps=[])
  
  #Algorithms to views
  efAlgs.append( theSegmentFinderAlg )
  #efAlgs.append( theNCBSegmentFinderAlg ) #The configuration still needs some sorting out for this so disabled for now.
  efAlgs.append( TrackBuilder )
  efAlgs.append( xAODTrackParticleCnvAlg )
  efAlgs.append( theMuonCandidateAlg )
  efAlgs.append( themuoncreatoralg )
  
  return efAlgs


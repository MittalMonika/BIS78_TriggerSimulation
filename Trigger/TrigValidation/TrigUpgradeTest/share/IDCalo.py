#
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#

include("TrigUpgradeTest/testHLT_MT.py")

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

isData = False
if globalflags.InputFormat.is_bytestream():
  isData = True

# ----------------------------------------------------------------
# Setup Views
# ----------------------------------------------------------------
from AthenaCommon.AlgSequence import AthSequencer
viewSeq = AthSequencer("AthViewSeq", Sequential=True, ModeOR=False, StopOverride=False)
topSequence += viewSeq

#Create IdentifiableCaches
from InDetPrepRawDataFormation.InDetPrepRawDataFormationConf import InDet__CacheCreator
InDetCacheCreatorTrigViews = InDet__CacheCreator(name = "InDetCacheCreatorTrigViews",
                                     Pixel_ClusterKey = "PixelTrigClustersCache",
                                     SCT_ClusterKey   = "SCT_ClustersCache",
                                     SpacePointCachePix = "PixelSpacePointCache",
                                     SpacePointCacheSCT   = "SctSpacePointCache",
                                     SCTRDOCacheKey       = "SctRDOCache",
                                     PixRDOCacheKey = "PixRDOCache",
                                     OutputLevel=DEBUG)
viewSeq += InDetCacheCreatorTrigViews

# View maker alg
viewNodeName = "allViewAlgorithms"
viewMaker = CfgMgr.AthViews__RoiCollectionToViews("viewMaker")
viewMaker.ViewBaseName = "testView"
viewMaker.InputRoICollection = "EMRoIs"
viewMaker.ViewNodeName = viewNodeName
viewMaker.OutputRoICollection = "EMViewRoIs"
viewMaker.ViewFallThrough = True
viewSeq += viewMaker

# Set of view algs
allViewAlgorithms = AthSequencer(viewNodeName, Sequential=False, ModeOR=False, StopOverride=False)
viewSeq += allViewAlgorithms


from InDetRecExample.InDetKeys import InDetKeys



if TriggerFlags.doID:
  #workaround to prevent online trigger folders to be enabled
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
  
  # PixelLorentzAngleSvc and SCTLorentzAngleSvc
  include("InDetRecExample/InDetRecConditionsAccess.py")
  
  from InDetRecExample.InDetKeys import InDetKeys
  
  #Only add raw data decoders to AlgSeq if we're running over raw data
  if isData:
    #Pixel
    
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRodDecoder
    InDetPixelRodDecoder = PixelRodDecoder(name = "InDetPixelRodDecoder")
    ToolSvc += InDetPixelRodDecoder
    
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRawDataProviderTool
    InDetPixelRawDataProviderTool = PixelRawDataProviderTool(name    = "InDetPixelRawDataProviderTool",
                                                             Decoder = InDetPixelRodDecoder)
    ToolSvc += InDetPixelRawDataProviderTool
    if (InDetFlags.doPrintConfigurables()):
      print      InDetPixelRawDataProviderTool
    
    # load the PixelRawDataProvider
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRawDataProvider
    InDetPixelRawDataProvider = PixelRawDataProvider(name         = "InDetPixelRawDataProvider",
                                                     RDOKey       = InDetKeys.PixelRDOs(),
                                                     ProviderTool = InDetPixelRawDataProviderTool,
                                                     OutputLevel = INFO)
    
    allViewAlgorithms += InDetPixelRawDataProvider
    allViewAlgorithms.InDetPixelRawDataProvider.isRoI_Seeded = True
    allViewAlgorithms.InDetPixelRawDataProvider.RoIs = "EMViewRoIs"
    allViewAlgorithms.InDetPixelRawDataProvider.RDOCacheKey = InDetCacheCreatorTrigViews.PixRDOCacheKey
    
    
    if (InDetFlags.doPrintConfigurables()):
      print          InDetPixelRawDataProvider
    
    
    #SCT
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCT_RodDecoder
    InDetSCTRodDecoder = SCT_RodDecoder(name        = "InDetSCTRodDecoder",
                                        TriggerMode = False, OutputLevel = INFO)
    ToolSvc += InDetSCTRodDecoder
    
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTRawDataProviderTool
    InDetSCTRawDataProviderTool = SCTRawDataProviderTool(name    = "InDetSCTRawDataProviderTool",
                                                        Decoder = InDetSCTRodDecoder)
    ToolSvc += InDetSCTRawDataProviderTool
    if (InDetFlags.doPrintConfigurables()):
      print      InDetSCTRawDataProviderTool
    
    # load the SCTRawDataProvider
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTRawDataProvider
    InDetSCTRawDataProvider = SCTRawDataProvider(name         = "InDetSCTRawDataProvider",
                                                RDOKey       = InDetKeys.SCT_RDOs(),
                                                ProviderTool = InDetSCTRawDataProviderTool,
                                                OutputLevel = INFO)
    
    allViewAlgorithms += InDetSCTRawDataProvider
    allViewAlgorithms.InDetSCTRawDataProvider.isRoI_Seeded = True
    allViewAlgorithms.InDetSCTRawDataProvider.RoIs = "EMViewRoIs"
    allViewAlgorithms.InDetSCTRawDataProvider.RDOCacheKey = InDetCacheCreatorTrigViews.SCTRDOCacheKey
    
    
    #TRT
    from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbSvc
    InDetTRTCalDbSvc = TRT_CalDbSvc()
    ServiceMgr += InDetTRTCalDbSvc
    
    from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_StrawStatusSummarySvc
    InDetTRTStrawStatusSummarySvc = TRT_StrawStatusSummarySvc(name = "InDetTRTStrawStatusSummarySvc")
    ServiceMgr += InDetTRTStrawStatusSummarySvc
    
    from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConf import TRT_RodDecoder
    InDetTRTRodDecoder = TRT_RodDecoder(name = "InDetTRTRodDecoder",
                                        LoadCompressTableDB = True)#(globalflags.DataSource() != 'geant4'))  
    ToolSvc += InDetTRTRodDecoder
      
    from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConf import TRTRawDataProviderTool
    InDetTRTRawDataProviderTool = TRTRawDataProviderTool(name    = "InDetTRTRawDataProviderTool",
                                                          Decoder = InDetTRTRodDecoder)
    ToolSvc += InDetTRTRawDataProviderTool
    
      
    # load the TRTRawDataProvider
    from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConf import TRTRawDataProvider
    InDetTRTRawDataProvider = TRTRawDataProvider(name         = "InDetTRTRawDataProvider",
                                                 RDOKey       = "TRT_RDOs",
                                                  ProviderTool = InDetTRTRawDataProviderTool)
    
    allViewAlgorithms += InDetTRTRawDataProvider
    allViewAlgorithms.InDetTRTRawDataProvider.isRoI_Seeded = True
    allViewAlgorithms.InDetTRTRawDataProvider.RoIs = "EMViewRoIs"
  
  
  #Pixel clusterisation
  
  from SiClusterizationTool.SiClusterizationToolConf import InDet__ClusterMakerTool
  InDetClusterMakerTool = InDet__ClusterMakerTool(name                 = "InDetClusterMakerTool",
      PixelCalibSvc        = None,
      PixelOfflineCalibSvc = None,
      UsePixelCalibCondDB  = False)
  
  ToolSvc += InDetClusterMakerTool
  
  
  from SiClusterizationTool.SiClusterizationToolConf import InDet__MergedPixelsTool
  InDetMergedPixelsTool = InDet__MergedPixelsTool(name                    = "InDetMergedPixelsTool",
                                                  globalPosAlg            = InDetClusterMakerTool,
                                                  MinimalSplitSize        = 0,
                                                  MaximalSplitSize        = 49,
                                                  MinimalSplitProbability = 0,
                                                  DoIBLSplitting = True,
                                                  SplitClusterAmbiguityMap= InDetKeys.SplitClusterAmbiguityMap())
  ToolSvc += InDetMergedPixelsTool
  
  from SiClusterizationTool.SiClusterizationToolConf import InDet__PixelGangedAmbiguitiesFinder
  InDetPixelGangedAmbiguitiesFinder = InDet__PixelGangedAmbiguitiesFinder(name = "InDetPixelGangedAmbiguitiesFinder")
  ToolSvc += InDetPixelGangedAmbiguitiesFinder
  
  from InDetPrepRawDataFormation.InDetPrepRawDataFormationConf import InDet__PixelClusterization
  InDetPixelClusterization = InDet__PixelClusterization(name                    = "InDetPixelClusterization",
                                                        clusteringTool          = InDetMergedPixelsTool,
                                                        gangedAmbiguitiesFinder = InDetPixelGangedAmbiguitiesFinder,
                                                        DetectorManagerName     = InDetKeys.PixelManager(),
                                                        DataObjectName          = InDetKeys.PixelRDOs(),
                                                        ClustersName            = "PixelTrigClusters", OutputLevel = INFO)
  allViewAlgorithms += InDetPixelClusterization
  allViewAlgorithms.InDetPixelClusterization.isRoI_Seeded = True
  allViewAlgorithms.InDetPixelClusterization.RoIs = "EMViewRoIs"
  allViewAlgorithms.InDetPixelClusterization.ClusterContainerCacheKey = InDetCacheCreatorTrigViews.Pixel_ClusterKey 
  
  
  
  #
  # --- SCT_ClusteringTool (public)
  #
  from SiClusterizationTool.SiClusterizationToolConf import InDet__SCT_ClusteringTool
  InDetSCT_ClusteringTool = InDet__SCT_ClusteringTool(name              = "InDetSCT_ClusteringTool",
                                                      globalPosAlg      = InDetClusterMakerTool,
                                                      conditionsService = InDetSCT_ConditionsSummarySvcWithoutFlagged)
  #
  # --- SCT_Clusterization algorithm
  #
  from InDetPrepRawDataFormation.InDetPrepRawDataFormationConf import InDet__SCT_Clusterization
  InDetSCT_Clusterization = InDet__SCT_Clusterization(name                    = "InDetSCT_Clusterization",
                                                      clusteringTool          = InDetSCT_ClusteringTool,
                                                      # ChannelStatus         = InDetSCT_ChannelStatusAlg,
                                                      DetectorManagerName     = InDetKeys.SCT_Manager(),
                                                      DataObjectName          = InDetKeys.SCT_RDOs(),
                                                      ClustersName            = "SCT_TrigClusters",
                                                      conditionsService       = InDetSCT_ConditionsSummarySvcWithoutFlagged,
                                                      OutputLevel = INFO)
  
  allViewAlgorithms += InDetSCT_Clusterization
  allViewAlgorithms.InDetSCT_Clusterization.isRoI_Seeded = True
  allViewAlgorithms.InDetSCT_Clusterization.RoIs = "EMViewRoIs"
  allViewAlgorithms.InDetSCT_Clusterization.ClusterContainerCacheKey = InDetCacheCreatorTrigViews.SCT_ClusterKey 
  
  
  #Space points and FTF
  
  from SiSpacePointTool.SiSpacePointToolConf import InDet__SiSpacePointMakerTool
  InDetSiSpacePointMakerTool = InDet__SiSpacePointMakerTool(name = "InDetSiSpacePointMakerTool")
  ToolSvc += InDetSiSpacePointMakerTool
  
  from SiSpacePointFormation.SiSpacePointFormationConf import InDet__SiTrackerSpacePointFinder
  InDetSiTrackerSpacePointFinder = InDet__SiTrackerSpacePointFinder(name                   = "InDetSiTrackerSpacePointFinder",
                                                                    SiSpacePointMakerTool  = InDetSiSpacePointMakerTool,
                                                                    PixelsClustersName     = "PixelTrigClusters",
                                                                    SCT_ClustersName       = "SCT_TrigClusters",
                                                                    SpacePointsPixelName   = "PixelTrigSpacePoints",
                                                                    SpacePointsSCTName     = "SCT_TrigSpacePoints",
                                                                    SpacePointsOverlapName = InDetKeys.OverlapSpacePoints(),
                                                                    ProcessPixels          = DetFlags.haveRIO.pixel_on(),
                                                                    ProcessSCTs            = DetFlags.haveRIO.SCT_on(),
                                                                    ProcessOverlaps        = DetFlags.haveRIO.SCT_on(),
                                                                    OutputLevel=INFO)
  
  
  from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinder_eGamma
  theFTF = TrigFastTrackFinder_eGamma()
  
  allViewAlgorithms += InDetSiTrackerSpacePointFinder
  allViewAlgorithms += theFTF
  allViewAlgorithms.TrigFastTrackFinder_eGamma.isRoI_Seeded = True
  allViewAlgorithms.TrigFastTrackFinder_eGamma.RoIs = "EMViewRoIs"
  InDetSiTrackerSpacePointFinder.SpacePointCacheSCT = InDetCacheCreatorTrigViews.SpacePointCacheSCT
  InDetSiTrackerSpacePointFinder.SpacePointCachePix = InDetCacheCreatorTrigViews.SpacePointCachePix

if TriggerFlags.doCalo:
  svcMgr.ToolSvc.TrigDataAccess.ApplyOffsetCorrection=False
  
  from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import T2CaloEgamma_FastAlgo
  algo=T2CaloEgamma_FastAlgo("testFastAlgo")
  algo.OutputLevel=VERBOSE
  #TopHLTSeq += algo

  algo.RoIs="EMViewRoIs"
  allViewAlgorithms += algo


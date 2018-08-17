#
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#

def makeInDetAlgs():
  eventAlgs = []
  viewAlgs = []
  from InDetRecExample.InDetKeys import InDetKeys
  #Create IdentifiableCaches
  from InDetPrepRawDataFormation.InDetPrepRawDataFormationConf import InDet__CacheCreator
  InDetCacheCreatorTrigViews = InDet__CacheCreator(name = "InDetCacheCreatorTrigViews",
                                       Pixel_ClusterKey = "PixelTrigClustersCache",
                                       SCT_ClusterKey   = "SCT_ClustersCache",
                                       SpacePointCachePix = "PixelSpacePointCache",
                                       SpacePointCacheSCT   = "SctSpacePointCache",
                                       SCTRDOCacheKey       = "SctRDOCache",
                                       PixRDOCacheKey = "PixRDOCache",)
                                       #OutputLevel=DEBUG)
  eventAlgs.append(InDetCacheCreatorTrigViews)

  from AthenaCommon.AppMgr import ToolSvc
  from AthenaCommon.AppMgr import ServiceMgr

  from AthenaCommon.GlobalFlags import globalflags
  #Only add raw data decoders if we're running over raw data
  if globalflags.InputFormat.is_bytestream():
    #Pixel
    
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRodDecoder
    InDetPixelRodDecoder = PixelRodDecoder(name = "InDetPixelRodDecoder")
    ToolSvc += InDetPixelRodDecoder
    
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRawDataProviderTool
    InDetPixelRawDataProviderTool = PixelRawDataProviderTool(name    = "InDetPixelRawDataProviderTool",
                                                             Decoder = InDetPixelRodDecoder)
    ToolSvc += InDetPixelRawDataProviderTool

    from InDetRecExample.InDetJobProperties import InDetFlags
    if (InDetFlags.doPrintConfigurables()):
      print      InDetPixelRawDataProviderTool
    
    # load the PixelRawDataProvider
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRawDataProvider
    InDetPixelRawDataProvider = PixelRawDataProvider(name         = "InDetPixelRawDataProvider",
                                                     RDOKey       = InDetKeys.PixelRDOs(),
                                                     ProviderTool = InDetPixelRawDataProviderTool,)
                                                     #OutputLevel = INFO)
    InDetPixelRawDataProvider.isRoI_Seeded = True
    InDetPixelRawDataProvider.RoIs = "EMViewRoIs"
    InDetPixelRawDataProvider.RDOCacheKey = InDetCacheCreatorTrigViews.PixRDOCacheKey
    
    viewAlgs.append(InDetPixelRawDataProvider)
    
    
    if (InDetFlags.doPrintConfigurables()):
      print          InDetPixelRawDataProvider
    
    
    #SCT
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCT_RodDecoder
    InDetSCTRodDecoder = SCT_RodDecoder(name        = "InDetSCTRodDecoder",
                                        TriggerMode = False,)# OutputLevel = INFO)
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
                                                ProviderTool = InDetSCTRawDataProviderTool, )
                                                #OutputLevel = INFO)
    InDetSCTRawDataProvider.isRoI_Seeded = True
    InDetSCTRawDataProvider.RoIs = "EMViewRoIs"
    InDetSCTRawDataProvider.RDOCacheKey = InDetCacheCreatorTrigViews.SCTRDOCacheKey
    
    viewAlgs.append(InDetSCTRawDataProvider)

    # load the SCTEventFlagWriter
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTEventFlagWriter
    InDetSCTEventFlagWriter = SCTEventFlagWriter(name = "InDetSCTEventFlagWriter")

    viewAlgs.append(InDetSCTEventFlagWriter)

    
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
    InDetTRTRawDataProvider.isRoI_Seeded = True
    InDetTRTRawDataProvider.RoIs = "EMViewRoIs"

    viewAlgs.append(InDetTRTRawDataProvider)
  

  # SCTLorentzAngleTool for ClusterMakerTool
  if not hasattr(ToolSvc, "SCTLorentzAngleTool"):
      from SiLorentzAngleSvc.SCTLorentzAngleToolSetup import SCTLorentzAngleToolSetup
      sctLorentzAngleToolSetup = SCTLorentzAngleToolSetup()
  
  #Pixel clusterisation
  
  from SiClusterizationTool.SiClusterizationToolConf import InDet__ClusterMakerTool
  InDetClusterMakerTool = InDet__ClusterMakerTool(name                 = "InDetClusterMakerTool",
      PixelCalibSvc        = None,
      PixelOfflineCalibSvc = None,
      UsePixelCalibCondDB  = False,
                                                  SCTLorentzAngleTool = ToolSvc.SCTLorentzAngleTool)
  
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
                                                        AmbiguitiesMap          = 'TrigPixelClusterAmbiguitiesMap',
                                                        ClustersName            = "PixelTrigClusters",)# OutputLevel = INFO)
  InDetPixelClusterization.isRoI_Seeded = True
  InDetPixelClusterization.RoIs = "EMViewRoIs"
  InDetPixelClusterization.ClusterContainerCacheKey = InDetCacheCreatorTrigViews.Pixel_ClusterKey 

  viewAlgs.append(InDetPixelClusterization)

  from SCT_ConditionsTools.SCT_FlaggedConditionToolSetup import SCT_FlaggedConditionToolSetup
  sct_FlaggedConditionToolSetup = SCT_FlaggedConditionToolSetup()
  sct_FlaggedConditionToolSetup.setup()
  InDetSCT_FlaggedConditionTool = sct_FlaggedConditionToolSetup.getTool()
  
  from SCT_ConditionsTools.SCT_ConditionsSummaryToolSetup import SCT_ConditionsSummaryToolSetup
  sct_ConditionsSummaryToolSetup = SCT_ConditionsSummaryToolSetup()
  sct_ConditionsSummaryToolSetup.setToolName("InDetSCT_ConditionsSummaryTool")
  sct_ConditionsSummaryToolSetup.setup()
  InDetSCT_ConditionsSummaryTool = sct_ConditionsSummaryToolSetup.getTool()
  condTools = []
  for condToolHandle in InDetSCT_ConditionsSummaryTool.ConditionsTools:
    condTool = condToolHandle.typeAndName
    if condTool not in condTools:
      if condTool != InDetSCT_FlaggedConditionTool.getFullName():
        condTools.append(condTool)
  sct_ConditionsSummaryToolSetupWithoutFlagged = SCT_ConditionsSummaryToolSetup()
  sct_ConditionsSummaryToolSetupWithoutFlagged.setToolName("InDetSCT_ConditionsSummaryToolWithoutFlagged")
  sct_ConditionsSummaryToolSetupWithoutFlagged.setup()
  InDetSCT_ConditionsSummaryToolWithoutFlagged = sct_ConditionsSummaryToolSetupWithoutFlagged.getTool()
  InDetSCT_ConditionsSummaryToolWithoutFlagged.ConditionsTools = condTools
  
  #
  # --- SCT_ClusteringTool (public)
  #
  from SiClusterizationTool.SiClusterizationToolConf import InDet__SCT_ClusteringTool
  InDetSCT_ClusteringTool = InDet__SCT_ClusteringTool(name              = "InDetSCT_ClusteringTool",
                                                      globalPosAlg      = InDetClusterMakerTool,
                                                      conditionsTool    = InDetSCT_ConditionsSummaryToolWithoutFlagged)
  #
  # --- SCT_Clusterization algorithm
  #

  from InDetPrepRawDataFormation.InDetPrepRawDataFormationConf import InDet__SCT_Clusterization
  InDetSCT_Clusterization = InDet__SCT_Clusterization(name                    = "InDetSCT_Clusterization",
                                                      clusteringTool          = InDetSCT_ClusteringTool,
                                                      # ChannelStatus         = InDetSCT_ChannelStatusAlg,
                                                      DataObjectName          = InDetKeys.SCT_RDOs(),
                                                      ClustersName            = "SCT_TrigClusters",
                                                      conditionsTool          = InDetSCT_ConditionsSummaryToolWithoutFlagged)
  InDetSCT_Clusterization.isRoI_Seeded = True
  InDetSCT_Clusterization.RoIs = "EMViewRoIs"
  InDetSCT_Clusterization.ClusterContainerCacheKey = InDetCacheCreatorTrigViews.SCT_ClusterKey 
  
  viewAlgs.append(InDetSCT_Clusterization)
  
  
  #Space points and FTF
  
  from SiSpacePointTool.SiSpacePointToolConf import InDet__SiSpacePointMakerTool
  InDetSiSpacePointMakerTool = InDet__SiSpacePointMakerTool(name = "InDetSiSpacePointMakerTool")
  ToolSvc += InDetSiSpacePointMakerTool

  from AthenaCommon.DetFlags import DetFlags 
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
                                                                    SpacePointCacheSCT = InDetCacheCreatorTrigViews.SpacePointCacheSCT,
                                                                    SpacePointCachePix = InDetCacheCreatorTrigViews.SpacePointCachePix,)
                                                                    #OutputLevel=INFO)

  viewAlgs.append(InDetSiTrackerSpacePointFinder)

  # Condition algorithm for SiTrackerSpacePointFinder
  if InDetSiTrackerSpacePointFinder.ProcessSCTs:
    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")
    if not hasattr(condSeq, "InDetSiElementPropertiesTableCondAlg"):
      # Setup alignment folders and conditions algorithms
      from InDetCondFolders import InDetAlignFolders
      from SiSpacePointFormation.SiSpacePointFormationConf import InDet__SiElementPropertiesTableCondAlg
      condSeq += InDet__SiElementPropertiesTableCondAlg(name = "InDetSiElementPropertiesTableCondAlg")

  from TrigInDetConf.TrigInDetRecCommonTools import InDetTrigFastTrackSummaryTool
  from TrigInDetConf.TrigInDetPostTools import  InDetTrigParticleCreatorToolFTF

  from InDetTrigParticleCreation.InDetTrigParticleCreationConf import InDet__TrigTrackingxAODCnvMT
  theTrackParticleCreatorAlg = InDet__TrigTrackingxAODCnvMT(name = "InDetTrigTrackParticleCreatorAlg",
                                                           doIBLresidual = False,
                                                           TrackName = "TrigFastTrackFinder_Tracks",
                                                           TrackParticlesName = "xAODTracks",
                                                           ParticleCreatorTool = InDetTrigParticleCreatorToolFTF)
  theTrackParticleCreatorAlg.roiCollectionName = "EMViewRoIs"
  viewAlgs.append(theTrackParticleCreatorAlg)

  
  return (viewAlgs, eventAlgs)


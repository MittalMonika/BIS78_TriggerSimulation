#
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

class InDetCacheNames:
  Pixel_ClusterKey   = "PixelTrigClustersCache"
  SCT_ClusterKey     = "SCT_ClustersCache"
  SpacePointCachePix = "PixelSpacePointCache"
  SpacePointCacheSCT = "SctSpacePointCache"
  SCTRDOCacheKey     = "SctRDOCache"
  PixRDOCacheKey     = "PixRDOCache"

def InDetIDCCacheCreatorCfg():
  #Create IdentifiableCaches
  acc = ComponentAccumulator()
  from InDetPrepRawDataFormation.InDetPrepRawDataFormationConf import InDet__CacheCreator
  InDetCacheCreatorTrig = InDet__CacheCreator(name = "InDetCacheCreatorTrig",
                                              Pixel_ClusterKey   = InDetCacheNames.Pixel_ClusterKey,
                                              SCT_ClusterKey     = InDetCacheNames.SCT_ClusterKey,
                                              SpacePointCachePix = InDetCacheNames.SpacePointCachePix,
                                              SpacePointCacheSCT = InDetCacheNames.SpacePointCacheSCT,
                                              SCTRDOCacheKey     = InDetCacheNames.SCTRDOCacheKey,
                                              PixRDOCacheKey     = InDetCacheNames.PixRDOCacheKey)

  acc.addEventAlgo( InDetCacheCreatorTrig )
  return acc

#Set up conditions algorithms
def TrigInDetCondConfig( flags ):

  acc = ComponentAccumulator()
  from AtlasGeoModel.InDetGMConfig import InDetGeometryCfg
  acc.merge(InDetGeometryCfg(flags))
  #acc.merge(InDetGMConfig(flags))

  from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline, addFolders
  acc.merge(addFoldersSplitOnline(flags, "INDET","/Indet/Onl/AlignL1/ID","/Indet/AlignL1/ID",className="CondAttrListCollection"))
  acc.merge(addFoldersSplitOnline(flags, "INDET","/Indet/Onl/AlignL2/PIX","/Indet/AlignL2/PIX",className="CondAttrListCollection"))
  acc.merge(addFoldersSplitOnline(flags, "INDET","/Indet/Onl/AlignL2/SCT","/Indet/AlignL2/SCT",className="CondAttrListCollection"))
  acc.merge(addFoldersSplitOnline(flags, "INDET","/Indet/Onl/AlignL3","/Indet/AlignL3",className="AlignableTransformContainer"))

  from SCT_ConditionsTools.SCT_ConditionsToolsConf import SCT_DCSConditionsTool
  dcsTool = SCT_DCSConditionsTool(ReadAllDBFolders = True, ReturnHVTemp = True)

  from SCT_ConditionsTools.SCT_SiliconConditionsToolSetup import SCT_SiliconConditionsToolSetup
  sct_SiliconConditionsToolSetup = SCT_SiliconConditionsToolSetup()
  sct_SiliconConditionsToolSetup.setDcsTool(dcsTool)
  sct_SiliconConditionsToolSetup.setToolName("InDetSCT_SiliconConditionsTool")
  sct_SiliconConditionsToolSetup.setup()

  sctSiliconConditionsTool = sct_SiliconConditionsToolSetup.getTool()
  sctSiliconConditionsTool.CheckGeoModel = False
  sctSiliconConditionsTool.ForceUseGeoModel = False


  from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConf import SCT_AlignCondAlg
  acc.addCondAlgo(SCT_AlignCondAlg(UseDynamicAlignFolders =  True))

  from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConf import SCT_DetectorElementCondAlg
  acc.addCondAlgo(SCT_DetectorElementCondAlg(name = "SCT_DetectorElementCondAlg"))
  from SCT_Cabling.SCT_CablingConfig import SCT_CablingCondAlgCfg
  acc.merge(SCT_CablingCondAlgCfg(flags))
  from SCT_ConditionsTools.SCT_ConditionsToolsConf import SCT_ConfigurationConditionsTool
  acc.addPublicTool(SCT_ConfigurationConditionsTool())
  channelFolder = "/SCT/DAQ/Config/Chip"
  moduleFolder = "/SCT/DAQ/Config/Module"
  murFolder = "/SCT/DAQ/Config/MUR"
  from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConf import SCT_ConfigurationCondAlg
  acc.addCondAlgo(SCT_ConfigurationCondAlg(ReadKeyChannel = channelFolder,
                                           ReadKeyModule = moduleFolder,
                                           ReadKeyMur = murFolder))
  acc.merge(addFolders(flags, [channelFolder, moduleFolder, murFolder], "SCT", className="CondAttrListVec"))
  # Set up SCTSiLorentzAngleCondAlg
  from SCT_ConditionsTools.SCT_ConditionsToolsConf import SCT_ConfigurationConditionsTool
  stateFolder = "/SCT/DCS/CHANSTAT"
  hvFolder = "/SCT/DCS/HV"
  tempFolder = "/SCT/DCS/MODTEMP"
  dbInstance = "DCS_OFL"
  acc.merge(addFolders(flags, [stateFolder, hvFolder, tempFolder], dbInstance, className="CondAttrListCollection"))

  from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConf import SCT_DCSConditionsTempCondAlg
  acc.addCondAlgo(SCT_DCSConditionsTempCondAlg( ReadKey = tempFolder ))
  from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConf import SCT_DCSConditionsStatCondAlg
  acc.addCondAlgo(SCT_DCSConditionsStatCondAlg(ReturnHVTemp = True,
                                               ReadKeyHV = hvFolder,
                                               ReadKeyState = stateFolder))
  from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConf import SCT_DCSConditionsHVCondAlg
  acc.addCondAlgo(SCT_DCSConditionsHVCondAlg(ReadKey = hvFolder))

  from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConf import SCT_SiliconHVCondAlg
  acc.addCondAlgo(SCT_SiliconHVCondAlg(UseState = dcsTool.ReadAllDBFolders,
                                       DCSConditionsTool = dcsTool))
  from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConf import SCT_SiliconTempCondAlg
  acc.addCondAlgo(SCT_SiliconTempCondAlg(UseState = dcsTool.ReadAllDBFolders, DCSConditionsTool = dcsTool))


  from SiLorentzAngleTool.SiLorentzAngleToolConf import SCTSiLorentzAngleCondAlg
  acc.addCondAlgo(SCTSiLorentzAngleCondAlg(name = "SCTSiLorentzAngleCondAlg",
                                           SiConditionsTool = sctSiliconConditionsTool,
                                           UseMagFieldSvc = True,
                                           UseMagFieldDcs = False))
  from SiLorentzAngleTool.SiLorentzAngleToolConf import SiLorentzAngleTool
  SCTLorentzAngleTool = SiLorentzAngleTool(name = "SCTLorentzAngleTool", DetectorName="SCT", SiLorentzAngleCondData="SCTSiLorentzAngleCondData")
  SCTLorentzAngleTool.UseMagFieldSvc = True #may need also MagFieldSvc instance
  acc.addPublicTool(SCTLorentzAngleTool)


  acc.merge(addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Beampos", "/Indet/Beampos", className="AthenaAttributeList"))
  acc.merge(addFolders(flags, "/TRT/Onl/ROD/Compress","TRT_ONL", className='CondAttrListCollection'))
  acc.merge(addFoldersSplitOnline(flags, "TRT","/TRT/Onl/Calib/RT","/TRT/Calib/RT",className="TRTCond::RtRelationMultChanContainer"))
  acc.merge(addFoldersSplitOnline(flags, "TRT","/TRT/Onl/Calib/T0","/TRT/Calib/T0",className="TRTCond::StrawT0MultChanContainer"))
  acc.merge(addFoldersSplitOnline (flags, "TRT","/TRT/Onl/Calib/errors","/TRT/Calib/errors",className="TRTCond::RtRelationMultChanContainer"))
  acc.merge(addFoldersSplitOnline(flags, "TRT","/TRT/Onl/Calib/ToTCalib","/TRT/Calib/ToTCalib",className="CondAttrListCollection"))
  acc.merge(addFoldersSplitOnline(flags, "TRT","/TRT/Onl/Calib/HTCalib","/TRT/Calib/HTCalib",className="CondAttrListCollection"))


  from AthenaCommon.CfgGetter import getService
  PixelCablingSvc = getService("PixelCablingSvc")
  acc.addService(PixelCablingSvc)

  PixelTDAQFolder   = "/TDAQ/Resources/ATLAS/PIXEL/Modules"
  PixelTDAQInstance = "TDAQ_ONL"
  acc.merge(addFolders(flags, PixelTDAQFolder, PixelTDAQInstance, className="CondAttrListCollection"))

  PixelHVFolder = "/PIXEL/DCS/HV"
  PixelTempFolder = "/PIXEL/DCS/TEMPERATURE"
  PixelDBInstance = "DCS_OFL"

  acc.merge(addFolders(flags, PixelHVFolder, PixelDBInstance, className="CondAttrListCollection"))
  acc.merge(addFolders(flags, PixelTempFolder, PixelDBInstance, className="CondAttrListCollection"))

  from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelDCSCondHVAlg
  acc.addCondAlgo(PixelDCSCondHVAlg(name="PixelDCSCondHVAlg", ReadKey=PixelHVFolder))

  from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelDCSCondTempAlg
  acc.addCondAlgo(PixelDCSCondTempAlg(name="PixelDCSCondTempAlg", ReadKey=PixelTempFolder))

  from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelTDAQCondAlg
  acc.addCondAlgo(PixelTDAQCondAlg(name="PixelTDAQCondAlg", ReadKey=PixelTDAQFolder))

  PixelDeadMapFolder = "/PIXEL/PixMapOverlay"
  PixelDeadMapInstance = "PIXEL_OFL"

  acc.merge(addFolders(flags, PixelTempFolder, PixelDBInstance, className="CondAttrListCollection"))
  acc.merge(addFolders(flags, PixelDeadMapFolder, PixelDeadMapInstance, className="CondAttrListCollection"))

  from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelConfigCondAlg
  acc.addCondAlgo(PixelConfigCondAlg(name="PixelConfigCondAlg", UseDeadMap=False, ReadDeadMapKey=PixelDeadMapFolder))

  from SiPropertiesTool.SiPropertiesToolConf import PixelSiPropertiesCondAlg
  acc.addCondAlgo(PixelSiPropertiesCondAlg(name="PixelSiPropertiesCondAlg"))

  from SiPropertiesTool.SiPropertiesToolConf import SiPropertiesTool
  TrigSiPropertiesTool = SiPropertiesTool(name="PixelSiPropertiesTool", DetectorName="Pixel", ReadKey="PixelSiliconPropertiesVector")

  acc.addPublicTool(TrigSiPropertiesTool)

  from SiLorentzAngleTool.SiLorentzAngleToolConf import PixelSiLorentzAngleCondAlg
  acc.addCondAlgo(PixelSiLorentzAngleCondAlg(name="PixelSiLorentzAngleCondAlg",
                                             SiPropertiesTool=TrigSiPropertiesTool,
                                             UseMagFieldSvc = True,
                                             UseMagFieldDcs = False))

  from SiLorentzAngleTool.SiLorentzAngleToolConf import SiLorentzAngleTool
  TrigPixelLorentzAngleTool = SiLorentzAngleTool(name = "PixelLorentzAngleTool", DetectorName="Pixel", SiLorentzAngleCondData="PixelSiLorentzAngleCondData")

  acc.addPublicTool(TrigPixelLorentzAngleTool)

  from BeamSpotConditions.BeamSpotConditionsConf import BeamSpotCondAlg
  acc.addCondAlgo(BeamSpotCondAlg( "BeamSpotCondAlg" ))


  from MagFieldServices.MagFieldServicesConfig import MagneticFieldSvcCfg
  mfsc = MagneticFieldSvcCfg(flags)
  acc.merge( mfsc )
  return acc

def TrigInDetConfig( flags, roisKey="EMRoIs" ):
  acc = ComponentAccumulator()
  acc.merge(TrigInDetCondConfig(flags))

  from InDetRecExample.InDetKeys import InDetKeys

  #Only add raw data decoders if we're running over raw data
  isMC = flags.Input.isMC
  if not isMC:
    #Pixel

    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRodDecoder
    InDetPixelRodDecoder = PixelRodDecoder(name = "InDetPixelRodDecoder")
    acc.addPublicTool(InDetPixelRodDecoder)

    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRawDataProviderTool
    InDetPixelRawDataProviderTool = PixelRawDataProviderTool(name    = "InDetPixelRawDataProviderTool",
                                                             Decoder = InDetPixelRodDecoder)
    acc.addPublicTool(InDetPixelRawDataProviderTool)


    # load the PixelRawDataProvider
    from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConf import PixelRawDataProvider
    InDetPixelRawDataProvider = PixelRawDataProvider(name         = "InDetPixelRawDataProvider",
                                                     RDOKey       = InDetKeys.PixelRDOs(),
                                                     ProviderTool = InDetPixelRawDataProviderTool,)

    InDetPixelRawDataProvider.isRoI_Seeded = True
    InDetPixelRawDataProvider.RoIs = roisKey
    InDetPixelRawDataProvider.RDOCacheKey = InDetCacheNames.PixRDOCacheKey
    acc.addEventAlgo(InDetPixelRawDataProvider)




    #SCT
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCT_RodDecoder
    InDetSCTRodDecoder = SCT_RodDecoder(name        = "InDetSCTRodDecoder")
    acc.addPublicTool(InDetSCTRodDecoder)

    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTRawDataProviderTool
    InDetSCTRawDataProviderTool = SCTRawDataProviderTool(name    = "InDetSCTRawDataProviderTool",
                                                         Decoder = InDetSCTRodDecoder)
    acc.addPublicTool(InDetSCTRawDataProviderTool)

    # load the SCTRawDataProvider
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTRawDataProvider
    InDetSCTRawDataProvider = SCTRawDataProvider(name         = "InDetSCTRawDataProvider",
                                                 RDOKey       = InDetKeys.SCT_RDOs(),
                                                 ProviderTool = InDetSCTRawDataProviderTool, )

    InDetSCTRawDataProvider.isRoI_Seeded = True
    InDetSCTRawDataProvider.RoIs = roisKey
    InDetSCTRawDataProvider.RDOCacheKey = InDetCacheNames.SCTRDOCacheKey

    acc.addEventAlgo(InDetSCTRawDataProvider)

    # load the SCTEventFlagWriter
    from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConf import SCTEventFlagWriter
    InDetSCTEventFlagWriter = SCTEventFlagWriter(name = "InDetSCTEventFlagWriter")

    acc.addEventAlgo(InDetSCTEventFlagWriter)


    #TRT
    from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbSvc
    InDetTRTCalDbSvc = TRT_CalDbSvc()
    acc.addService(InDetTRTCalDbSvc)

    from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_StrawStatusSummarySvc
    InDetTRTStrawStatusSummarySvc = TRT_StrawStatusSummarySvc(name = "InDetTRTStrawStatusSummarySvc")
    acc.addService(InDetTRTStrawStatusSummarySvc)

    from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConf import TRT_RodDecoder
    InDetTRTRodDecoder = TRT_RodDecoder(name = "InDetTRTRodDecoder",
                                        LoadCompressTableDB = True)#(globalflags.DataSource() != 'geant4'))
    acc.addPublicTool(InDetTRTRodDecoder)

    from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConf import TRTRawDataProviderTool
    InDetTRTRawDataProviderTool = TRTRawDataProviderTool(name    = "InDetTRTRawDataProviderTool",
                                                         Decoder = InDetTRTRodDecoder)
    acc.addPublicTool(InDetTRTRawDataProviderTool)


    # load the TRTRawDataProvider
    from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConf import TRTRawDataProvider
    InDetTRTRawDataProvider = TRTRawDataProvider(name         = "InDetTRTRawDataProvider",
                                                 RDOKey       = "TRT_RDOs",
                                                 ProviderTool = InDetTRTRawDataProviderTool)
    InDetTRTRawDataProvider.isRoI_Seeded = True
    InDetTRTRawDataProvider.RoIs = roisKey

    acc.addEventAlgo(InDetTRTRawDataProvider)


  #Pixel clusterisation

  from SiClusterizationTool.SiClusterizationToolConf import InDet__ClusterMakerTool
  InDetClusterMakerTool = InDet__ClusterMakerTool(name                 = "InDetClusterMakerTool",
                                                  UsePixelCalibCondDB  = False)

  acc.addPublicTool(InDetClusterMakerTool)


  from SiClusterizationTool.SiClusterizationToolConf import InDet__MergedPixelsTool
  InDetMergedPixelsTool = InDet__MergedPixelsTool(name                    = "InDetMergedPixelsTool",
                                                  globalPosAlg            = InDetClusterMakerTool,
                                                  MinimalSplitSize        = 0,
                                                  MaximalSplitSize        = 49,
                                                  MinimalSplitProbability = 0,
                                                  DoIBLSplitting = True,
                                                  SplitClusterAmbiguityMap= InDetKeys.SplitClusterAmbiguityMap())
  acc.addPublicTool(InDetMergedPixelsTool)

  from SiClusterizationTool.SiClusterizationToolConf import InDet__PixelGangedAmbiguitiesFinder
  InDetPixelGangedAmbiguitiesFinder = InDet__PixelGangedAmbiguitiesFinder(name = "InDetPixelGangedAmbiguitiesFinder")
  acc.addPublicTool(InDetPixelGangedAmbiguitiesFinder)

  from InDetPrepRawDataFormation.InDetPrepRawDataFormationConf import InDet__PixelClusterization
  InDetPixelClusterization = InDet__PixelClusterization(name                    = "InDetPixelClusterization",
                                                        clusteringTool          = InDetMergedPixelsTool,
                                                        gangedAmbiguitiesFinder = InDetPixelGangedAmbiguitiesFinder,
                                                        DetectorManagerName     = InDetKeys.PixelManager(),
                                                        DataObjectName          = InDetKeys.PixelRDOs(),
                                                        AmbiguitiesMap          = 'TrigPixelClusterAmbiguitiesMap',
                                                        ClustersName            = "PixelTrigClusters",)
  InDetPixelClusterization.isRoI_Seeded = True
  InDetPixelClusterization.RoIs = roisKey
  InDetPixelClusterization.ClusterContainerCacheKey = InDetCacheNames.Pixel_ClusterKey

  acc.addEventAlgo(InDetPixelClusterization)

  from SCT_ConditionsTools.SCT_FlaggedConditionToolSetup import SCT_FlaggedConditionToolSetup
  sct_FlaggedConditionToolSetup = SCT_FlaggedConditionToolSetup()
  sct_FlaggedConditionToolSetup.setup()
  InDetSCT_FlaggedConditionTool = sct_FlaggedConditionToolSetup.getTool()

  from SCT_ConditionsTools.SCT_ConditionsSummaryToolSetup import SCT_ConditionsSummaryToolSetup
  sct_ConditionsSummaryToolSetup = SCT_ConditionsSummaryToolSetup()
  sct_ConditionsSummaryToolSetup = SCT_ConditionsSummaryToolSetup("InDetSCT_ConditionsSummaryTool")
  sct_ConditionsSummaryToolSetup.setup()
  InDetSCT_ConditionsSummaryTool = sct_ConditionsSummaryToolSetup.getTool()
  condTools = []
  for condToolHandle in InDetSCT_ConditionsSummaryTool.ConditionsTools:
    condTool = condToolHandle.typeAndName
    if condTool not in condTools:
      if condTool != InDetSCT_FlaggedConditionTool.getFullName():
        condTools.append(condTool)
  sct_ConditionsSummaryToolSetupWithoutFlagged = SCT_ConditionsSummaryToolSetup("InDetSCT_ConditionsSummaryToolWithoutFlagged")
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
  InDetSCT_Clusterization.RoIs = roisKey
  InDetSCT_Clusterization.ClusterContainerCacheKey = InDetCacheNames.SCT_ClusterKey

  acc.addEventAlgo(InDetSCT_Clusterization)


  #Space points and FTF

  from SiSpacePointTool.SiSpacePointToolConf import InDet__SiSpacePointMakerTool
  InDetSiSpacePointMakerTool = InDet__SiSpacePointMakerTool(name = "InDetSiSpacePointMakerTool")
  acc.addPublicTool(InDetSiSpacePointMakerTool)

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
                                                                    SpacePointCacheSCT = InDetCacheNames.SpacePointCacheSCT,
                                                                    SpacePointCachePix = InDetCacheNames.SpacePointCachePix,)

  acc.addEventAlgo(InDetSiTrackerSpacePointFinder)


  #from IOVSvc.IOVSvcConf import CondSvc
  #acc.addService(CondSvc())


  #from TrigInDetConf.TrigInDetRecCommonTools import InDetTrigFastTrackSummaryTool
  #from TrigInDetConf.TrigInDetPostTools import  InDetTrigParticleCreatorToolFTF

  #from InDetTrigParticleCreation.InDetTrigParticleCreationConf import InDet__TrigTrackingxAODCnvMT
  #theTrackParticleCreatorAlg = InDet__TrigTrackingxAODCnvMT(name = "InDetTrigTrackParticleCreatorAlg",
  #                                                         doIBLresidual = False,
  #                                                         TrackName = "TrigFastTrackFinder_Tracks",
  #                                                         TrackParticlesName = "xAODTracks",
  #                                                         ParticleCreatorTool = InDetTrigParticleCreatorToolFTF)
  #theTrackParticleCreatorAlg.roiCollectionName = roisKey
  #acc.addEventAlgo(theTrackParticleCreatorAlg)



  return acc

def indetInViewRecoCfg( flags, viewMakerName ):
  """ TBD if this function should be defined here or moved to the menu are, for sake of symmetry it is kept here now 
  There would certainly be additional algorithms
  """  
  from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import InViewReco
  reco = InViewReco( viewMakerName )
  algAcc = TrigInDetConfig( flags, roisKey=reco.inputMaker().InViewRoIs )

  reco.mergeReco( algAcc )  
  return reco

  
if __name__ == "__main__":
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior=1

    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    ConfigFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data17_13TeV.00327265.physics_EnhancedBias.merge.RAW._lb0100._SFO-1._0001.1"]
    ConfigFlags.Trigger.LVL1ConfigFile = "LVL1config_Physics_pp_v7.xml"
    #ConfigFlags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2018-13"
    ConfigFlags.lock()

    acc = ComponentAccumulator()

    nThreads=1

    from StoreGate.StoreGateConf import SG__HiveMgrSvc
    eventDataSvc = SG__HiveMgrSvc("EventDataSvc")
    eventDataSvc.NSlots = nThreads
    acc.addService( eventDataSvc )
    #from AthenaConfiguration.MainServicesConfig import MainServicesThreadedCfg
    #acc.merge( MainServicesThreadedCfg( ConfigFlags ) )
    from L1Decoder.L1DecoderConfig import L1DecoderCfg
    l1DecoderAcc, l1DecoderAlg = L1DecoderCfg( ConfigFlags )
    acc.addEventAlgo(l1DecoderAlg)
    acc.merge(l1DecoderAcc)
    from ByteStreamCnvSvc.ByteStreamConfig import TrigBSReadCfg
    acc.merge(TrigBSReadCfg(ConfigFlags))

    acc.merge( TrigInDetConfig( ConfigFlags ) )
    from RegionSelector.RegSelConfig import regSelCfg
    rsc = regSelCfg( ConfigFlags )
    acc.merge( rsc )

    acc.printConfig()
    acc.store( open("test.pkl", "w") )
    print 'All ok'
    

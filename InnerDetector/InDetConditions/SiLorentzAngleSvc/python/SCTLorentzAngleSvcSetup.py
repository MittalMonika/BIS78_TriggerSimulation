# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration


# Usage:
#
#  from SiLorentzAngleSvc.LorentzAngleSvcSetup import SCTLorentzAngleSvcSetup
#
# By default will determine whether to use DB or not from GeoModel.
# To force usage of DB (that is, use SiliconConditionsServices which should
# normally read from DB):
#
#  SCTLorentzAngleSvcSetup.forceUseDB()
#
# For more configuration one can get access to the service configurable via
# members pixel and sct: Eg:
#  lorentzAngleSvc.pixel.OutputLevel = VERBOSE
#  lorentzAngleSvc.sct.OutputLevel = VERBOSE
#  print lorentzAngleSvc.pixel
#  print lorentzAngleSvc.sct
#
# You can also access the pixel and sct SiliconConditionsSvc:
#
#  lorentzAngleSvc.PixelSiliconConditionsSvc.CheckGeoModel = False # same as forceUseDB
#  lorentzAngleSvc.SCT_SiliconConditionsSvc.CheckGeoModel = False # same as forceUseDB
#  print lorentzAngleSvc.PixelSiliconConditionsSvc
#  print lorentzAngleSvc.SCT_SiliconConditionsSvc
#
# 
# Alternatively, you can also access the services by:
#  from SiLorentzAngleSvc.LorentzAngleSvcSetup import PixelLorentzAngleSvc,SCTLorentzAngleSvc
# 
#  PixelLorentzAngleSvc.ForceUseDB = True
#  SCTLorentzAngleSvc.ForceUseDB = True
#  PixelLorentzAngleSvc.OutputLevel = VERBOSE
#  SCTLorentzAngleSvc.OutputLevel = VERBOSE
#

class SCTLorentzAngleSvcSetup:
    "Class to simplify setup of LorentzAngleSvc"
    def __init__(self, forceUseDB=False, forceUseGeoModel=False):
        if forceUseDB and forceUseGeoModel:
            from AthenaCommon import Logging
            msg = Logging.logging.getLogger("SCTLorentzAngleSvcSetup")
            msg.error("Setting is wrong: both forceUseDB and forceUseGeoModel cannot be True at the same time")
            return

        # Set up SCT_DCSConditiosnTool if necessary
        if not forceUseGeoModel:
            from SCT_ConditionsTools.SCT_DCSConditionsToolSetup import SCT_DCSConditionsToolSetup
            sct_DCSConditionsToolSetup = SCT_DCSConditionsToolSetup()
            sct_DCSConditionsToolSetup.setup()

        # Set up SCT_SiliconConditionsTool
        from SCT_ConditionsTools.SCT_SiliconConditionsToolSetup import SCT_SiliconConditionsToolSetup
        sct_SiliconConditionsToolSetup = SCT_SiliconConditionsToolSetup()
        if forceUseGeoModel:
            sct_SiliconConditionsToolSetup.setUseDB(False)
            sct_SiliconConditionsToolSetup.setForceUseGeoModel(True)
        else:
            sct_SiliconConditionsToolSetup.setDcsTool(sct_DCSConditionsToolSetup.getTool())
        sct_SiliconConditionsToolSetup.setup()
        sctSiliconConditionsTool = sct_SiliconConditionsToolSetup.getTool()
        self.sctSiliconConditionsTool = sctSiliconConditionsTool
        self.SCT_SiliconConditionsTool = sctSiliconConditionsTool

        # Set up SCTSiLorentzAngleCondAlg
        from AthenaCommon.AlgSequence import AthSequencer
        condSeq = AthSequencer("AthCondSeq")
        if not hasattr(condSeq, "SCTSiLorentzAngleCondAlg"):
            from SiLorentzAngleSvc.SiLorentzAngleSvcConf import SCTSiLorentzAngleCondAlg
            from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
            condSeq += SCTSiLorentzAngleCondAlg(name = "SCTSiLorentzAngleCondAlg",
                                                SiConditionsTool = sctSiliconConditionsTool,
                                                UseMagFieldSvc = True,
                                                UseMagFieldDcs = (not athenaCommonFlags.isOnline()),
                                                UseGeoModel = forceUseGeoModel)
        sctSiLorentzAngleCondAlg = condSeq.SCTSiLorentzAngleCondAlg
        if forceUseDB:
            sctSiLorentzAngleCondAlg.useSctDefaults = False

        # Set up SCTLorentzAngleSvc
        from AthenaCommon.AppMgr import ServiceMgr
        if not hasattr(ServiceMgr, "SCTLorentzAngleSvc"):
            from SiLorentzAngleSvc.SiLorentzAngleSvcConf import SiLorentzAngleCHSvc
            ServiceMgr += SiLorentzAngleCHSvc(name="SCTLorentzAngleSvc", DetectorName="SCT")
        sctLorentzAngleSvc = ServiceMgr.SCTLorentzAngleSvc
        # Pass the silicon conditions tool to the Lorentz angle service
        # Also make sure UseMagFieldTool is True as AtlasGeoModel sets this to False
        # if loaded first.
        sctLorentzAngleSvc.UseMagFieldSvc = True
        self.SCTLorentzAngleSvc = sctLorentzAngleSvc

    # Force the Lorentz angle sercive to use SiliconConditions service (which are assumed to use the DB)
    # Default is to decide based on GeoModel.
    def forceUseDB(self) :
        "Force usage of conditions DB"
        self.SCT_SiliconConditionsTool.CheckGeoModel = False
        from AthenaCommon import Logging
        msg = Logging.logging.getLogger("SCTLorentzAngleSvcSetup")
        msg.warning("Please set forceUseDB in constructor. Unnecessary service, algorithms, folders are configured")

    # Force to use the defaults from GeoModel. In case it is not possible to use DCS
    def forceUseGeoModel(self) :
        "Force usage of GeoModel defaults"
        self.SCT_SiliconConditionsTool.ForceUseGeoModel = True
        msg.warning("Please set forceUseGeoModel in constructor. Unnecessary service, algorithms, folders are configured")

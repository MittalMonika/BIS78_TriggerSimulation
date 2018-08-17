# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

# Usage:
#
#  from SiLorentzAngleSvc.LorentzAngleSvcSetup import lorentzAngleSvc
#
# By default will determine whether to use DB or not from GeoModel.
# To force usage of DB (that is, use SiliconConditionsServices which should
# normally read from DB):
#
#  lorentzAngleSvc.forceUseDB()
#
# for both pixel and SCT or to set pixel and SCT separately do:
#
#  lorentzAngleSvc.pixelForceUseDB()
#  lorentzAngleSvc.sctForceUseDB()
#
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
#  PixelLorentzAngleSvc.OutputLevel = VERBOSE
#  SCTLorentzAngleSvc.OutputLevel = VERBOSE
#

class LorentzAngleSvcSetup:
    "Class to simplify setup of LorentzAngleSvc"
    def __init__(self):

        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        
        from SiLorentzAngleSvc.SiLorentzAngleSvcConf import SiLorentzAngleSvc

        from AthenaCommon.DetFlags      import DetFlags

        from SiPropertiesSvc.SiPropertiesSvcConf import SiPropertiesSvc

        ##
        ## Pixel
        ##
        if ( DetFlags.detdescr.pixel_on() ):

            from PixelLorentzAngleSvcSetup import pixelLorentzAngleSvcSetup
            self.pixel = pixelLorentzAngleSvcSetup.PixelLorentzAngleSvc
            self.pixelSiliconConditionsSvc = pixelLorentzAngleSvcSetup.PixelSiliconConditionsSvc
            self.PixelSiliconConditionsSvc = pixelLorentzAngleSvcSetup.PixelSiliconConditionsSvc
            self.pixelSiPropertiesSvc      = pixelLorentzAngleSvcSetup.PixelSiPropertiesSvc
            self.PixelSiPropertiesSvc      = pixelLorentzAngleSvcSetup.PixelSiPropertiesSvc

        ##
        ## SCT 
        ##
        if ( DetFlags.detdescr.SCT_on() ):
            # Init SCTLorentzAngleTool

            from SCTLorentzAngleToolSetup import SCTLorentzAngleToolSetup
            sctLorentzAngleToolSetup = SCTLorentzAngleToolSetup()
            self.sct = sctLorentzAngleToolSetup.SCTLorentzAngleTool
            self.sctSiliconConditionsTool  = sctLorentzAngleToolSetup.SCT_SiliconConditionsTool
            self.SCT_SiliconConditionsTool = sctLorentzAngleToolSetup.SCT_SiliconConditionsTool

    # Force the Lorentz angle sercive to use SiliconConditions service (which are assumed to use the DB)
    # Default is to decide based on GeoModel.
    def forceUseDB(self) :
        "Force usage of conditions DB"
        from AthenaCommon.DetFlags      import DetFlags
        if ( DetFlags.detdescr.pixel_on() ):
            self.pixelForceUseDB()

        if ( DetFlags.detdescr.SCT_on() ):
            self.sctForceUseDB() 
    
    def sctForceUseDB(self) :
        "Force usage of conditions DB for SCT"
        self.SCT_SiliconConditionsTool.CheckGeoModel = False


    # Force to use the defaults from GeoModel. In case it is not possible to use DCS
    def forceUseGeoModel(self) :
        "Force usage of GeoModel defaults"
        from AthenaCommon.DetFlags      import DetFlags
        if ( DetFlags.detdescr.SCT_on() ):
            self.sctForceUseGeoModel() 

    def sctForceUseGeoModel(self) :
        "Force usage of GeoModel defaults for SCT"
        self.SCT_SiliconConditionsTool.ForceUseGeoModel = True


# configuration instance
lorentzAngleSvc = LorentzAngleSvcSetup()

from AthenaCommon.DetFlags      import DetFlags
if ( DetFlags.detdescr.pixel_on() ):
   from PixelLorentzAngleSvcSetup import PixelLorentzAngleSvc
if ( DetFlags.detdescr.SCT_on() ):
   from SCTLorentzAngleToolSetup import SCTLorentzAngleToolSetup
   sctLorentzAngleToolSetup = SCTLorentzAngleToolSetup()
   SCTLorentzAngleTool = sctLorentzAngleToolSetup.SCTLorentzAngleTool

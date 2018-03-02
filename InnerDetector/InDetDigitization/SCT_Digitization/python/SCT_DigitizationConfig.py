# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr
# The earliest bunch crossing time for which interactions will be sent
# to the SCT Digitization code.
def SCT_FirstXing():
    return -50
# The latest bunch crossing time for which interactions will be sent
# to the SCT Digitization code.
def SCT_LastXing():
    return 25



######################################################################################
def getSCT_RandomDisabledCellGenerator(name="SCT_RandomDisabledCellGenerator", **kwargs):
    kwargs.setdefault("TotalBadChannels", 0.01)
    from SCT_Digitization.SCT_DigitizationConf import SCT_RandomDisabledCellGenerator
    return SCT_RandomDisabledCellGenerator(name, **kwargs)


######################################################################################
def getSCT_Amp(name="SCT_Amp", **kwargs):
    kwargs.setdefault("CrossFactor2sides", 0.1)
    kwargs.setdefault("CrossFactorBack", 0.07)
    kwargs.setdefault("PeakTime", 21)
    kwargs.setdefault("deltaT", 1.0)
    kwargs.setdefault("Tmin", -25.0)
    kwargs.setdefault("Tmax", 150.0)
    from SCT_Digitization.SCT_DigitizationConf import SCT_Amp
    return SCT_Amp(name, **kwargs)


######################################################################################
def getSCT_SurfaceChargesGenerator(name="SCT_SurfaceChargesGenerator", **kwargs):
    ## Set up services used by SCT_SurfaceChargesGenerator
    # Set up SCT_DCSConditiosnSvc
    from SCT_ConditionsServices.SCT_DCSConditionsSvcSetup import SCT_DCSConditionsSvcSetup
    sct_DCSConditionsSvcSetup = SCT_DCSConditionsSvcSetup()
    sct_DCSConditionsSvcSetup.setup()
    # Set up SCT_SiliconConditionsSvc
    from SCT_ConditionsServices.SCT_SiliconConditionsSvcSetup import SCT_SiliconConditionsSvcSetup
    sct_SiliconConditionsSvcSetup = SCT_SiliconConditionsSvcSetup()
    sct_SiliconConditionsSvcSetup.setDcsSvc(sct_DCSConditionsSvcSetup.getSvc())
    sct_SiliconConditionsSvcSetup.setup()
    # Set up SCT_SiPropertiesTool
    from SiPropertiesSvc.SCT_SiPropertiesToolSetup import SCT_SiPropertiesToolSetup
    sct_SiPropertiesToolSetup = SCT_SiPropertiesToolSetup()
    sct_SiPropertiesToolSetup.setSiliconSvc(sct_SiliconConditionsSvcSetup.getSvc())
    sct_SiPropertiesToolSetup.setup()
    ## Charge trapping tool - used by SCT_SurfaceChargesGenerator
    from AthenaCommon.AppMgr import ToolSvc
    if not hasattr(ToolSvc, "InDetSCT_RadDamageSummaryTool"):
        from SCT_ConditionsTools.SCT_ConditionsToolsConf import SCT_RadDamageSummaryTool
        ToolSvc += SCT_RadDamageSummaryTool(name = "InDetSCT_RadDamageSummaryTool")

    kwargs.setdefault("FixedTime", -999)
    kwargs.setdefault("SubtractTime", -999)
    kwargs.setdefault("SurfaceDriftTime", 10)
    kwargs.setdefault("NumberOfCharges", 1)
    kwargs.setdefault("SmallStepLength", 5)
    kwargs.setdefault("DepletionVoltage", 70)
    kwargs.setdefault("BiasVoltage", 150)
    kwargs.setdefault("SiPropertiesTool", sct_SiPropertiesToolSetup.getTool())
    from AthenaCommon.GlobalFlags import globalflags
    kwargs.setdefault("isOverlay", globalflags.isOverlay())

    # kwargs.setdefault("doTrapping", True) # ATL-INDET-INT-2016-019

    from Digitization.DigitizationFlags import digitizationFlags
    if 'doDetailedSurfChargesGen' in digitizationFlags.experimentalDigi():
        kwargs.setdefault("ChargeDriftModel", 1)
        kwargs.setdefault("EFieldModel", 2)
        kwargs.setdefault("MagneticField", -2.0)
        kwargs.setdefault("SensorTemperature", 273.15)
        kwargs.setdefault("TransportTimeStep", 0.25)
        kwargs.setdefault("TransportTimeMax", 25.0)
        from SCT_Digitization.SCT_DigitizationConf import SCT_DetailedSurfaceChargesGenerator
        return SCT_DetailedSurfaceChargesGenerator(name, **kwargs)
    else:
        from SCT_Digitization.SCT_DigitizationConf import SCT_SurfaceChargesGenerator
        kwargs.setdefault("RadDamageSummaryTool", getattr(ToolSvc, "InDetSCT_RadDamageSummaryTool"))
        return SCT_SurfaceChargesGenerator(name, **kwargs)

######################################################################################
def getSCT_FrontEnd(name="SCT_FrontEnd", **kwargs):
    from Digitization.DigitizationFlags import digitizationFlags
    #Setup noise treament in SCT_FrontEnd
    # If noise is turned off:
    if not digitizationFlags.doInDetNoise.get_Value():
        ###kwargs.setdefault("OnlyHitElements", True)
        print 'SCT_Digitization:::: Turned off Noise in SCT_FrontEnd'
        kwargs.setdefault("NoiseBarrel", 0.0)
        kwargs.setdefault("NoiseBarrel3", 0.0)
        kwargs.setdefault("NoiseInners", 0.0)
        kwargs.setdefault("NoiseMiddles", 0.0)
        kwargs.setdefault("NoiseShortMiddles", 0.0)
        kwargs.setdefault("NoiseOuters", 0.0)
        kwargs.setdefault("NOBarrel", 0.0)
        kwargs.setdefault("NOBarrel3", 0.0)
        kwargs.setdefault("NOInners", 0.0)
        kwargs.setdefault("NOMiddles", 0.0)
        kwargs.setdefault("NOShortMiddles", 0.0)
        kwargs.setdefault("NOOuters", 0.0)
        kwargs.setdefault("NoiseOn", False)
        # To set the mean noise values for the different module types
        # Default values set at 0 degrees, plus/minus ~5 enc per plus/minus degree
    else:
        kwargs.setdefault("NoiseBarrel", 1500.0)
        kwargs.setdefault("NoiseBarrel3", 1541.0)
        kwargs.setdefault("NoiseInners", 1090.0)
        kwargs.setdefault("NoiseMiddles", 1557.0)
        kwargs.setdefault("NoiseShortMiddles", 940.0)
        kwargs.setdefault("NoiseOuters", 1618.0)
        kwargs.setdefault("NOBarrel", 1.5e-5)
        kwargs.setdefault("NOBarrel3", 2.1e-5)
        kwargs.setdefault("NOInners", 5.0e-9)
        kwargs.setdefault("NOMiddles", 2.7e-5)
        kwargs.setdefault("NOShortMiddles", 2.0e-9)
        kwargs.setdefault("NOOuters", 3.5e-5)
        kwargs.setdefault("NoiseOn", True)
    # Use Calibration data from Conditions DB, still for testing purposes only
    kwargs.setdefault("UseCalibData", True)

    # Setup the ReadCalibChip folders and Svc
    from SCT_ConditionsTools.SCT_ReadCalibChipDataToolSetup import SCT_ReadCalibChipDataToolSetup
    sct_ReadCalibChipDataToolSetup = SCT_ReadCalibChipDataToolSetup()
    sct_ReadCalibChipDataToolSetup.setup()
    kwargs.setdefault("SCT_ReadCalibChipDataTool", sct_ReadCalibChipDataToolSetup.getTool())
    # DataCompressionMode: 1 is level mode x1x (default), 2 is edge mode 01x, 3 is expanded any hit xxx
    from AthenaCommon.BeamFlags import jobproperties
    from AthenaCommon.GlobalFlags import globalflags
    if digitizationFlags.PileUpPremixing:
        kwargs.setdefault("DataCompressionMode", 3)
    elif globalflags.isOverlay() and globalflags.DataSource == 'geant4':
        kwargs.setdefault("DataCompressionMode", 2)
    elif (jobproperties.Beam.bunchSpacing() <= 50):
        kwargs.setdefault("DataCompressionMode", 1) 
    else: 
        kwargs.setdefault("DataCompressionMode", 3) 
        kwargs.setdefault("NoiseExpandedMode", True)
    # DataReadOutMode: 0 is condensed mode and 1 is expanded mode
    if globalflags.isOverlay() and globalflags.DataSource == 'geant4':
        kwargs.setdefault("DataReadOutMode", 0)
    else:
        kwargs.setdefault("DataReadOutMode", 1)
    from SCT_Digitization.SCT_DigitizationConf import SCT_FrontEnd
    return SCT_FrontEnd(name, **kwargs)

######################################################################################
def getPileupSCT_FrontEnd(name="PileupSCT_FrontEnd", **kwargs):

    kwargs.setdefault("NoiseBarrel", 0.0)
    kwargs.setdefault("NoiseBarrel3", 0.0)
    kwargs.setdefault("NoiseInners", 0.0)
    kwargs.setdefault("NoiseMiddles", 0.0)
    kwargs.setdefault("NoiseShortMiddles", 0.0)
    kwargs.setdefault("NoiseOuters", 0.0)
    kwargs.setdefault("NOBarrel", 0.0)
    kwargs.setdefault("NOBarrel3", 0.0)
    kwargs.setdefault("NOInners", 0.0)
    kwargs.setdefault("NOMiddles", 0.0)
    kwargs.setdefault("NOShortMiddles", 0.0)
    kwargs.setdefault("NOOuters", 0.0)
    kwargs.setdefault("NoiseOn", False)

    return getSCT_FrontEnd(name, **kwargs)

######################################################################################


def commonSCT_DigitizationConfig(name,**kwargs):

    from Digitization.DigitizationFlags import digitizationFlags
    # If noise is turned off:
    if not digitizationFlags.doInDetNoise.get_Value():
        kwargs.setdefault("OnlyHitElements", True)

    kwargs.setdefault("InputObjectName", "SCT_Hits")
    kwargs.setdefault("EnableHits", True)
    kwargs.setdefault("BarrelOnly", False)

    # Use of random disabled cells
    #kwargs.setdefault("RandomDisabledCells", True)

    # Set FixedTime for cosmics for use in SurfaceChargesGenerator
    from AthenaCommon.BeamFlags import jobproperties
    if jobproperties.Beam.beamType == "cosmics" :
        kwargs.setdefault("CosmicsRun", True)
        kwargs.setdefault("FixedTime", 10)

    # write out SCT1_RawData
    #kwargs.setdefault("WriteSCT1_RawData", False)

    kwargs.setdefault("RndmSvc", digitizationFlags.rndmSvc() )
    streamName = "SCT_Digitization" # FIXME ideally random stream name would be configurable
    if not digitizationFlags.rndmSeedList.checkForExistingSeed(streamName):
        digitizationFlags.rndmSeedList.addSeed(streamName, 49261510, 105132394 )

    if digitizationFlags.doXingByXingPileUp():
        kwargs.setdefault("FirstXing", SCT_FirstXing())
        kwargs.setdefault("LastXing",  SCT_LastXing() )

    from AthenaCommon import CfgMgr
    return CfgMgr.SCT_DigitizationTool(name,**kwargs)
#    else:
#        from AthenaCommon import CfgMgr
#        return CfgMgr.SCT_Digitization(name, **kwargs)

######################################################################################

def SCT_DigitizationTool(name="SCT_DigitizationTool", **kwargs):
    kwargs.setdefault("OutputObjectName", "SCT_RDOs")
    kwargs.setdefault("OutputSDOName", "SCT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    return commonSCT_DigitizationConfig(name,**kwargs)

######################################################################################

def SCT_GeantinoTruthDigitizationTool(name="SCT_GeantinoTruthDigitizationTool", **kwargs):
    kwargs.setdefault("ParticleBarcodeVeto", 0)
    return SCT_DigitizationTool(name,**kwargs)

######################################################################################

def SCT_DigitizationToolHS(name="SCT_DigitizationToolHS",**kwargs):
    kwargs.setdefault("OutputObjectName", "SCT_RDOs")
    kwargs.setdefault("OutputSDOName", "SCT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 1)
    return commonSCT_DigitizationConfig(name,**kwargs)

######################################################################################

def SCT_DigitizationToolPU(name="SCT_DigitizationToolPU",**kwargs):
    kwargs.setdefault("OutputObjectName", "SCT_PU_RDOs")
    kwargs.setdefault("OutputSDOName", "SCT_PU_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 2)
    return commonSCT_DigitizationConfig(name,**kwargs)

######################################################################################

def SCT_DigitizationToolSplitNoMergePU(name="SCT_DigitizationToolSplitNoMergePU",**kwargs):

    kwargs.setdefault("InputObjectName", "PileupSCT_Hits")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("OutputObjectName", "SCT_PU_RDOs")
    kwargs.setdefault("OutputSDOName", "SCT_PU_SDO_Map")
    kwargs.setdefault("OnlyHitElements", True)
    kwargs.setdefault("FrontEnd", "PileupSCT_FrontEnd")

    return commonSCT_DigitizationConfig(name,**kwargs)

######################################################################################

def SCT_OverlayDigitizationTool(name="SCT_OverlayDigitizationTool",**kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags
    kwargs.setdefault("EvtStore", overlayFlags.evtStore())
    kwargs.setdefault("OutputObjectName", overlayFlags.evtStore() + "+SCT_RDOs")
    kwargs.setdefault("OutputSDOName", overlayFlags.evtStore() + "+SCT_SDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    return commonSCT_DigitizationConfig(name,**kwargs)

######################################################################################

def getSiliconRange(name="SiliconRange" , **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', SCT_FirstXing() )
    kwargs.setdefault('LastXing',  SCT_LastXing()  )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["SiHitCollection#SCT_Hits"] )
    return CfgMgr.PileUpXingFolder(name, **kwargs)

######################################################################################

def SCT_DigitizationHS(name="SCT_DigitizationHS",**kwargs):
    kwargs.setdefault("DigitizationTool", "SCT_DigitizationToolHS")
    from SCT_Digitization.SCT_DigitizationConf import SCT_Digitization
    return SCT_Digitization(name,**kwargs)

######################################################################################

def SCT_DigitizationPU(name="SCT_DigitizationPU",**kwargs):
    kwargs.setdefault("DigitizationTool", "SCT_DigitizationToolPU")
    return CfgMgr.SCT_Digitization(name,**kwargs)

######################################################################################

def SCT_OverlayDigitization(name="SCT_OverlayDigitization",**kwargs):
    kwargs.setdefault("DigitizationTool", "SCT_OverlayDigitizationTool")
    return CfgMgr.SCT_Digitization(name,**kwargs)

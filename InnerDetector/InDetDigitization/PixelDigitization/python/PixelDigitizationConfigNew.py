"""Define methods to construct configured Pixel Digitization tools and algorithms

Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline, addFolders
from PileUpComps.PileUpCompsConf import PileUpXingFolder
from PixelCabling.PixelCablingConfigNew import PixelCablingSvcCfg
from PixelDigitization.PixelDigitizationConf import (
    PixelDigitizationTool, PixelDigitization, ChargeCollProbSvc,
    EnergyDepositionTool, SensorSimPlanarTool, SensorSim3DTool,
    RD53SimTool, FEI4SimTool, FEI3SimTool,
)
from SiPropertiesTool.PixelSiPropertiesConfig import PixelSiPropertiesCfg
from SiLorentzAngleTool.PixelLorentzAngleConfig import PixelLorentzAngleCfg
from PixelConditionsTools.PixelConditionsSummaryConfig import PixelConditionsSummaryCfg
from PixelConditionsAlgorithms.PixelConditionsConfig import PixelChargeCalibCondAlgCfg, PixelOfflineCalibCondAlgCfg
from PixelGeoModel.PixelGeoModelConfig import PixelGeometryCfg
from StoreGate.StoreGateConf import StoreGateSvc
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

# The earliest and last bunch crossing times for which interactions will be sent
# to the Pixel Digitization code
def Pixel_FirstXing(flags):
    if flags.Beam.estimatedLuminosity > 0.5e33:
        return -25
    else:
        return -50

def Pixel_LastXing(flags):
    if flags.Beam.estimatedLuminosity > 0.5e33:
        if flags.Beam.BunchSpacing > 50:
            return 75
        else:
            return 25
    else:
        return 100

def PixelItemList():
    """Return list of item names needed for Pixel output"""
    return ["InDet::SiClusterContainer#*", "InDet::PixelGangedClusterAmbiguities#*", "PixelRDO_Container#*"]

def ChargeCollProbSvcCfg(name="ChargeCollProbSvc", **kwargs):
    """Return a Charge Collection Prob service"""
    return ChargeCollProbSvc(name, **kwargs)

def EnergyDepositionToolCfg(flags, name="EnergyDepositionTool", **kwargs):
    """Return a configured EnergyDepositionTool"""
    kwargs.setdefault("DeltaRayCut", 117.)
    kwargs.setdefault("nCols", 5)
    kwargs.setdefault("LoopLimit", 100000)
    kwargs.setdefault("doBichsel", True)
    kwargs.setdefault("doBichselBetaGammaCut", 0.7) # dEdx not quite consistent below this
    kwargs.setdefault("doDeltaRay", False)          # needs validation
    kwargs.setdefault("doPU", True)
    return EnergyDepositionTool(name, **kwargs)

def SensorSimPlanarToolCfg(flags, name="SensorSimPlanarTool", **kwargs):
    """Return a ComponentAccumulator with configured SensorSimPlanarTool"""
    acc = PixelSiPropertiesCfg(flags)
    SiTool = acc.popPrivateTools()
    LorentzTool = acc.popToolsAndMerge(PixelLorentzAngleCfg(flags))
    kwargs.setdefault("SiPropertiesTool", SiTool)
    kwargs.setdefault("LorentzAngleTool", LorentzTool)
    acc.setPrivateTools(SensorSimPlanarTool(name, **kwargs))
    return acc

def SensorSim3DToolCfg(flags, name="SensorSim3DTool", **kwargs):
    """Return a ComponentAccumulator with configured SensorSim3DTool"""
    acc = PixelSiPropertiesCfg(flags)
    SiTool = acc.popPrivateTools()
    LorentzTool = acc.popToolsAndMerge(PixelLorentzAngleCfg(flags))
    kwargs.setdefault("SiPropertiesTool", SiTool)
    acc.setPrivateTools(SensorSim3DTool(name, **kwargs))
    return acc

def BarrelRD53SimToolCfg(flags, name="BarrelRD53SimTool", **kwargs):
    """Return a RD53SimTool configured for Barrel"""
    kwargs.setdefault("BarrelEC", 0)
    return RD53SimTool(name, **kwargs)

def EndcapRD53SimToolCfg(flags, name="EndcapRD53SimTool", **kwargs):
    """Return a RD53SimTool configured for Endcap"""
    kwargs.setdefault("BarrelEC", 2)
    return RD53SimTool(name, **kwargs)

def BarrelFEI4SimToolCfg(flags, name="BarrelFEI4SimTool", **kwargs):
    """Return a FEI4SimTool configured for Barrel"""    
    kwargs.setdefault("BarrelEC", 0)
    return FEI4SimTool(name, **kwargs)

def DBMFEI4SimToolCfg(flags, name="DBMFEI4SimTool", **kwargs):
    """Return a FEI4SimTool configured for Endcap"""
    kwargs.setdefault("BarrelEC", 4)
    return FEI4SimTool(name, **kwargs)

def BarrelFEI3SimToolCfg(flags, name="BarrelFEI3SimTool", **kwargs):
    """Return a FEI3SimTool configured for Barrel"""
    kwargs.setdefault("BarrelEC", 0)
    return FEI3SimTool(name, **kwargs)

def EndcapFEI3SimToolCfg(flags, name="EndcapFEI3SimTool", **kwargs):
    """Return a FEI3SimTool configured for Endcap"""
    kwargs.setdefault("BarrelEC", 2)
    return FEI3SimTool(name, **kwargs)

def PixelDigitizationBasicToolCfg(flags, name="PixelDigitizationTool", **kwargs):
    """Return a ComponentAccumulator with configured PixelDigitizationTool"""
    acc = PixelGeometryCfg(flags)
    acc.popToolsAndMerge(PixelConditionsSummaryCfg(flags))
    acc.merge(PixelChargeCalibCondAlgCfg(flags))
    acc.popToolsAndMerge(PixelSiPropertiesCfg(flags))
    acc.popToolsAndMerge(PixelLorentzAngleCfg(flags))
    acc.merge(PixelCablingSvcCfg(flags))
    acc.merge(PixelOfflineCalibCondAlgCfg(flags))
    # set up tool handle lists
    chargeTools = []
    feSimTools = []
    chargeTools.append(acc.popToolsAndMerge(SensorSimPlanarToolCfg(flags)))
    if flags.GeoModel.Run == "RUN4":
        feSimTools.append(BarrelRD53SimToolCfg(flags))
        feSimTools.append(EndcapRD53SimToolCfg(flags))
    else:
        chargeTools.append(acc.popToolsAndMerge(SensorSim3DToolCfg(flags)))
        feSimTools.append(BarrelFEI4SimToolCfg(flags))
        feSimTools.append(DBMFEI4SimToolCfg(flags))
        feSimTools.append(BarrelFEI3SimToolCfg(flags))
        feSimTools.append(EndcapFEI3SimToolCfg(flags))
    kwargs.setdefault("InputObjectName", "PixelHits")
    kwargs.setdefault("ChargeTools", chargeTools)
    kwargs.setdefault("FrontEndSimTools", feSimTools)
    kwargs.setdefault("EnergyDepositionTool", EnergyDepositionToolCfg(flags)) 
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", Pixel_FirstXing(flags))
        kwargs.setdefault("LastXing", Pixel_LastXing(flags))
    acc.setPrivateTools(PixelDigitizationTool(name, **kwargs))
    return acc

def PixelDigitizationToolCfg(flags, name="PixelDigitizationTool", **kwargs):
    """Return a ComponentAccumulator with configured BasicPixelDigitizationTool"""
    kwargs.setdefault("HardScatterSplittingMode", 0)
    return PixelDigitizationBasicToolCfg(flags, name, **kwargs)

def PixelGeantinoTruthDigitizationToolCfg(flags, name="PixelGeantinoTruthDigitizationTool", **kwargs):
    """Return a configured PixelDigitizationTool"""
    kwargs.setdefault("ParticleBarcodeVeto", 0)
    return PixelDigitizationTool(name, **kwargs)

def PixelDigitizationHSToolCfg(flags, name="PixelDigitizationToolHS", **kwargs):
    """Return a ComponentAccumulator with PixelDigitizationTool configured for Hard Scatter"""
    kwargs.setdefault("HardScatterSplittingMode", 1)
    return PixelDigitizationBasicToolCfg(flags, name, **kwargs)

def PixelDigitizationPUToolCfg(flags, name="PixelDigitizationToolPU", **kwargs):
    """Return a ComponentAccumulator with PixelDigitizationTool configured for PileUp"""
    kwargs.setdefault("HardScatterSplittingMode", 2)
    kwargs.setdefault("RDOCollName", "Pixel_PU_RDOs")
    kwargs.setdefault("SDOCollName", "Pixel_PU_SDO_Map")
    return PixelDigitizationBasicToolCfg(flags, name, **kwargs)

def PixelDigitizationSplitNoMergePUToolCfg(flags, name="PixelDigitizationToolSplitNoMergePU", **kwargs):
    """Return a ComponentAccumulator with PixelDigitizationTool configured for PileUpPixelHits"""
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("InputObjectName", "PileupPixelHits")
    kwargs.setdefault("RDOCollName", "Pixel_PU_RDOs")
    kwargs.setdefault("SDOCollName", "Pixel_PU_SDO_Map")
    return PixelDigitizationBasicToolCfg(flags, name, **kwargs)

def PixelDigitizationOverlayToolCfg(flags, name="PixelDigitizationOverlayTool", **kwargs):
    """Return a ComponentAccumulator with PixelDigitizationTool configured for overlay"""
    acc = ComponentAccumulator()
    acc.addService(StoreGateSvc(flags.Overlay.Legacy.EventStore))
    kwargs.setdefault("EvtStore", flags.Overlay.Legacy.EventStore)
    kwargs.setdefault("RDOCollName", flags.Overlay.Legacy.EventStore + "+PixelRDOs")
    kwargs.setdefault("SDOCollName", flags.Overlay.Legacy.EventStore + "+PixelSDO_Map")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    return PixelDigitizationBasicToolCfg(flags, name, **kwargs)

def PixelRangeCfg(flags, name="PixelRange", **kwargs):
    """Return a configured PileUpXingFolder tool"""
    kwargs.setdefault("FirstXing", Pixel_FirstXing(flags))
    kwargs.setdefault("LastXing", Pixel_LastXing(flags))
    kwargs.setdefault("CacheRefreshFrequency", 1.0) # default 0 no dataproxy reset
    kwargs.setdefault("ItemList", ["SiHitCollection#PixelHits"])
    return PileUpXingFolder(name, **kwargs)

def PixelDigitizationCfg(toolCfg, flags, name="PixelDigitization", **kwargs):
    """Return a ComponentAccumulator with toolCfg type Pixel digitization"""
    acc = ComponentAccumulator()
    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(toolCfg(flags))
        kwargs["DigitizationTool"] = tool
    acc.addEventAlgo(PixelDigitization(name, **kwargs))
    # FIXME once OutputStreamCfg merges correctly
    #acc.merge(OutputStreamCfg(flags, "RDO", PixelItemList()))
    return acc

def PixelDigitizationHSCfg(flags, name="PixelDigitizationHS", **kwargs):
    """Return a ComponentAccumulator with Hard Scatter Pixel Digitization"""
    return PixelDigitizationCfg(PixelDigitizationHSToolCfg, flags, name, **kwargs)

def PixelDigitizationPUCfg(flags, name="PixelDigitizationPU", **kwargs):
    """Return a ComponentAccumulator with Hard Scatter Pixel Digitization"""
    return PixelDigitizationCfg(PixelDigitizationPUToolCfg, flags, name, **kwargs)

def PixelDigitizationOverlayCfg(flags, name="PixelDigitizationOverlay", **kwargs):
    """Return a ComponentAccumulator with Hard Scatter Pixel Digitization"""
    return PixelDigitizationCfg(PixelDigitizationOverlayToolCfg, flags, name, **kwargs)


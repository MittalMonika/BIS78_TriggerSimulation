"""Define methods to construct configured BCM Digitization tools and algs

Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
"""
from RngComps.RandomServices import RNG
from PileUpComps.PileUpCompsConf import PileUpXingFolder
from PixelGeoModel.PixelGeoModelConfig import PixelGeometryCfg
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from BCM_Digitization.BCM_DigitizationConf import BCM_DigitizationTool, BCM_Digitization

# The earliest and last bunch crossing times for which interactions will be sent
# to the BCM Digitization code.
def BCM_FirstXing():
    return -25

def BCM_LastXing():
    return 0

def BCM_RangeCfg(flags, name="BCM_Range", **kwargs):
    """Return a BCM configured PileUpXingFolder tool"""
    kwargs.setdefault("FirstXing", BCM_FirstXing())
    kwargs.setdefault("LastXing",  BCM_LastXing())
    # Default 0 no dataproxy reset
    kwargs.setdefault("CacheRefreshFrequency", 1.0)
    kwargs.setdefault("ItemList", ["SiHitCollection#BCMHits"])
    return PileUpXingFolder(name, **kwargs)

def BCM_DigitizationToolCfg(flags, name="BCM_DigitizationTool", **kwargs):
    """Return a ComponentAccumulator with configured BCM_DigitizationTool"""
    # take initial ComponentAccumulator from RNG
    acc = RNG(flags.Random.Engine)
    kwargs.setdefault("RndmSvc", "AthRNGSvc")
    kwargs.setdefault("HitCollName", "BCMHits")
    if flags.Digitization.PileUpPremixing:
        kwargs.setdefault("OutputRDOKey", flags.Overlay.BkgPrefix + "BCM_RDOs")
        kwargs.setdefault("OutputSDOKey", flags.Overlay.BkgPrefix + "BCM_SDO_Map")
    else:
        kwargs.setdefault("OutputRDOKey", "BCM_RDOs")
        kwargs.setdefault("OutputSDOKey", "BCM_SDO_Map")
    if flags.Digitization.DoInnerDetectorNoise:
        kwargs.setdefault("ModNoise", [90.82] * 8)
    else:
        kwargs.setdefault("ModNoise", [0.0] * 8)
    kwargs.setdefault("ModSignal", [450.0] * 8)
    kwargs.setdefault("NinoThr", [330.0] * 8)
    # BCM with diamond
    kwargs.setdefault("MIPDeposit", 0.33)
    # Alternative BCM with graphite
    # kwargs.setdefault("MIPDeposit", 0.25)
    kwargs.setdefault("EffDistanceParam", 4.0)
    kwargs.setdefault("EffSharpnessParam", 0.11)
    kwargs.setdefault("TimeDelay", 9.0)
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", BCM_FirstXing())
        kwargs.setdefault("LastXing",  BCM_LastXing())
    acc.setPrivateTools(BCM_DigitizationTool(name, **kwargs))
    return acc

def BCM_OverlayDigitizationToolCfg(flags, name, **kwargs):
    """Return ComponentAccumulator with BCM_DigitizationTool configured for Overlay"""
    kwargs.setdefault("EvtStore", flags.Overlay.Legacy.EventStore)
    return BCM_DigitizationToolCfg(flags, name, **kwargs)


def BCM_DigitizationBasicCfg(toolCfg, flags, name, **kwargs):
    """Return ComponentAccumulator with toolCfg configured BCM_Digitization algorithm"""
    acc = PixelGeometryCfg(flags)
    if "DigitizationTool" not in kwargs:
        tool = acc.popToolsAndMerge(toolCfg(flags, **kwargs))
        kwargs["DigitizationTool"] = tool
    acc.addEventAlgo(BCM_Digitization(name, **kwargs))
    return acc

def BCM_DigitizationOutputCfg(toolCfg, flags, name="BCM_Digitization", **kwargs):
    """Return ComponentAccumulator with toolCfg configured BCM_Digitization algorithm and OutputStream"""
    acc = BCM_DigitizationBasicCfg(toolCfg, flags, name, **kwargs)
    acc.merge(OutputStreamCfg(flags, "RDO", ["InDetSimDataCollection#*", "BCM_RDO_Container#*"]))
    return acc


def BCM_DigitizationCfg(flags, name="BCM_Digitization", **kwargs):
    """Return ComponentAccumulator with standard BCM_Digitization and Output"""
    return BCM_DigitizationOutputCfg(BCM_DigitizationToolCfg, flags, **kwargs)

def BCM_OverlayDigitizationCfg(flags, name="BCM_OverlayDigitization", **kwargs):
    """Return ComponentAccumulator with Overlay confgured BCM_Digitization and Output"""
    return BCM_DigitizationOutputCfg(BCM_OverlayDigitizationToolCfg, flags, **kwargs)

"""Define methods to construct configured CSC Digitization tools and algorithms

Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from StoreGate.StoreGateConf import StoreGateSvc
from CSC_Digitization.CSC_DigitizationConf import (
    CscDigitizationTool, CscDigitBuilder,
)
from PileUpComps.PileUpCompsConf import PileUpXingFolder

# The earliest and last bunch crossing times for which interactions will be sent
# to the CscDigitizationTool.
def CSC_FirstXing():
    return -375

def CSC_LastXing():
    return 175

def CSC_RangeToolCfg(flags, name="CSC_Range", **kwargs):
    """Return a PileUpXingFolder tool configured for CSC"""
    kwargs.setdefault("FirstXing", CSC_FirstXing())
    kwargs.setdefault("LastXing",  CSC_LastXing())
    kwargs.setdefault("CacheRefreshFrequency", 1.0)
    kwargs.setdefault("ItemList", ["CSCSimHitCollection#CSC_Hits"])
    return PileUpXingFolder(name, **kwargs)

def CSC_DigitizationToolCfg(flags, name="CSC_DigitizationTool", **kwargs):
    """Return a ComponentAccumulator with configured CscDigitizationTool"""
    acc = ComponentAccumulator()
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", CSC_FirstXing())
        kwargs.setdefault("LastXing",  CSC_LastXing())
    kwargs.setdefault("InputObjectName", "CSC_Hits")
    kwargs.setdefault("OutputObjectName", "CSC_DIGITS")
    kwargs.setdefault("pedestal", 0.0)
    kwargs.setdefault("WindowLowerOffset", -25.0)
    kwargs.setdefault("WindowUpperOffset",  25.0)
    kwargs.setdefault("isPileUp", False)
    kwargs.setdefault("amplification", 0.43e5)
    kwargs.setdefault("NewDigitEDM", True)
    kwargs.setdefault("DriftVelocity", 39)
    kwargs.setdefault("ElectronEnergy", 66) # eV
    acc.setPrivateTools(CscDigitizationTool(name, **kwargs))
    return acc

def CSC_DigitBuilderCfg(flags, name="CSC_DigitBuilder", **kwargs):
    """Return a ComponentAccumulator with configured CscDigitBuilder algorithm"""
    acc = CSC_DigitizationToolCfg(flags)
    kwargs.setdefault("DigitizationTool", acc.popPrivateTools())
    acc.addEventAlgo(CscDigitBuilder(name, **kwargs))
    return acc
    
def CSC_OverlayDigitizationToolCfg(flags, name="CSC_OverlayDigitizationTool",**kwargs):
    """Return a ComponentAccumulator with CscDigitizationTool configured for Overlay"""
    acc = ComponentAccumulator()
    acc.addService(StoreGateSvc(flags.Overlay.Legacy.EventStore))
    kwargs.setdefault("EvtStore", flags.Overlay.Legacy.EventStore)
    kwargs.setdefault("OutputObjectName", flags.Overlay.Legacy.EventStore + "+CSC_DIGITS")
    if not flags.Detector.Overlay:
        kwargs.setdefault("CSCSimDataCollectionOutputName", flags.Overlay.Legacy.EventStore + "+CSC_SDO")
    acc.setPrivateTools(CscDigitizationTool(name, **kwargs))
    return acc

def CSC_OverlayDigitBuilderCfg(flags, name="CSC_OverlayDigitBuilder", **kwargs):
    """Return a ComponentAccumulator with CscDigitBuilder algorithm configured for Overlay"""
    acc = CSC_OverlayDigitizationToolCfg(flags)
    kwargs.setdefault("DigitizationTool", acc.popPrivateTools())
    acc.addEventAlgo(CscDigitBuilder(name, **kwargs))
    return acc


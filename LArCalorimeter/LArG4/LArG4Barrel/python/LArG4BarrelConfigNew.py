# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from LArG4Barrel.LArG4BarrelConf import LArG4__BarrelCryostat__CalibrationCalculator
from LArG4Barrel.LArG4BarrelConf import LArG4__BarrelCryostat__CalibrationLArCalculator
from LArG4Barrel.LArG4BarrelConf import LArG4__BarrelCryostat__CalibrationMixedCalculator
from LArG4Barrel.LArG4BarrelConf import LArG4__DM__CalibrationCalculator
from LArG4Barrel.LArG4BarrelConf import LArG4__Barrel__CalibrationCalculator
from LArG4Barrel.LArG4BarrelConf import LArG4__BarrelPresampler__CalibrationCalculator
from LArG4Barrel.LArG4BarrelConf import LArBarrelCalculator
from LArG4Barrel.LArG4BarrelConf import LArBarrelPresamplerCalculator
from LArG4Barrel.LArG4BarrelConf import LArG4__Barrel__Geometry
from LArG4Barrel.LArG4BarrelConf import LArG4__BarrelPresampler__Geometry

def BarrelCryostatCalibrationCalculatorCfg(ConfigFlags, name="BarrelCryostatCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( LArG4__BarrelCryostat__CalibrationCalculator(name, **kwargs) )
    return result


def BarrelCryostatCalibrationLArCalculatorCfg(ConfigFlags, name="BarrelCryostatCalibrationLArCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( LArG4__BarrelCryostat__CalibrationLArCalculator(name, **kwargs) )
    return result


def BarrelCryostatCalibrationMixedCalculatorCfg(ConfigFlags, name="BarrelCryostatCalibrationMixedCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( LArG4__BarrelCryostat__CalibrationMixedCalculator(name, **kwargs) )
    return result


def DMCalibrationCalculatorCfg(ConfigFlags, name="DMCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( LArG4__DM__CalibrationCalculator(name, **kwargs) )
    return result


def BarrelCalibrationCalculatorCfg(ConfigFlags, name="BarrelCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( LArG4__Barrel__CalibrationCalculator(name, **kwargs) )
    return result


def BarrelPresamplerCalibrationCalculatorCfg(ConfigFlags, name="BarrelPresamplerCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( LArG4__BarrelPresampler__CalibrationCalculator(name, **kwargs) )
    return result


def EMBCalculatorCfg(ConfigFlags, name="EMBCalculator", **kwargs):
    result = LArBarrelGeometryCfg(ConfigFlags)
    kwargs.setdefault("GeometryCalculator", result.getService("LArBarrelGeometry"))
    result.addService(LArBarrelCalculator(name, **kwargs))
    return result


def EMBPresamplerCalculatorCfg(ConfigFlags, name="EMBPresamplerCalculator", **kwargs):
    result = LArBarrelPresamplerGeometryCfg(ConfigFlags)
    kwargs.setdefault("GeometryCalculator", result.getService("LArBarrelPresamplerGeometry"))

    result.addService(LArBarrelPresamplerCalculator(name, **kwargs))
    return result


def LArBarrelGeometryCfg(ConfigFlags, name="LArBarrelGeometry", **kwargs):
    result = ComponentAccumulator()
    result.addService(LArG4__Barrel__Geometry(name, **kwargs))
    return result


def LArBarrelPresamplerGeometryCfg(ConfigFlags, name="LArBarrelPresamplerGeometry", **kwargs):
    result = ComponentAccumulator()
    result.addService(LArG4__BarrelPresampler__Geometry(name, **kwargs))
    return result

# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from LArG4HEC.LArG4HECConf import LArHECLocalCalculator
from LArG4HEC.LArG4HECConf import LArG4__HEC__LocalCalibrationCalculator
from LArG4HEC.LArG4HECConf import LArG4__HEC__LArHECCalibrationWheelCalculator
from LArG4HEC.LArG4HECConf import LArG4__HEC__LocalGeometry
from LArG4HEC.LArG4HECConf import LArHECWheelCalculator
from LArG4HEC.LArG4HECConf import LArG4__HEC__HECGeometry
from LArG4HEC import HECGeometryType

def LArHECLocalCalculatorCfg(ConfigFlags, name="LArHECLocalCalculator", **kwargs):
    return LArHECLocalCalculator(name, **kwargs)


def LocalCalibrationCalculatorCfg(ConfigFlags, name="LocalCalibrationCalculator", **kwargs):
    return LArG4__HEC__LocalCalibrationCalculator(name, **kwargs)


def LocalHECGeometry(name="LocalHECGeometry", **kwargs):
    return LArG4__HEC__LocalGeometry(name, **kwargs)


def HECWheelCalculatorCfg(ConfigFlags, name="HECWheelCalculator", **kwargs):
    result = HECGeometryCfg(ConfigFlags)
    kwargs.setdefault("GeometryCalculator", result.getService("HECGeometry"))
    result.addService(LArHECWheelCalculator(name, **kwargs))
    return result


def LArHECCalibrationWheelCalculatorCfg(ConfigFlags, name="LArHECCalibrationWheelCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( LArG4__HEC__LArHECCalibrationWheelCalculator(name, **kwargs) )
    return result


def HECCalibrationWheelActiveCalculatorCfg(ConfigFlags, name="HECCalibrationWheelActiveCalculator", **kwargs):
    kwargs.setdefault("GeometryType", HECGeometryType.kWheelActive)
    return LArHECCalibrationWheelCalculatorCfg(ConfigFlags, name, **kwargs)


def HECCalibrationWheelInactiveCalculatorCfg(ConfigFlags, name="HECCalibrationWheelInactiveCalculator", **kwargs):
    kwargs.setdefault("GeometryType", HECGeometryType.kWheelInactive)
    return LArHECCalibrationWheelCalculatorCfg(ConfigFlags, name, **kwargs)


def HECCalibrationWheelDeadCalculatorCfg(ConfigFlags, name="HECCalibrationWheelDeadCalculator", **kwargs):
    kwargs.setdefault("GeometryType", HECGeometryType.kWheelDead)
    return LArHECCalibrationWheelCalculatorCfg(ConfigFlags, name, **kwargs)


def HECGeometryCfg(ConfigFlags, name="HECGeometry", **kwargs):
    result = ComponentAccumulator()
    result.addService(LArG4__HEC__HECGeometry(name, **kwargs))
    return result

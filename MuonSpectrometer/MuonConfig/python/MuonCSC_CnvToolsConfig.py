# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory


def MuonCscRDODecoderCfg(flags, **kwargs):
    from MuonConfig.MuonCalibConfig import CscCalibToolCfg
    acc = CscCalibToolCfg(flags)
    kwargs.setdefault("cscCalibTool", acc.popPrivateTools())

    CscRDO_Decoder = CompFactory.Muon__CscRDO_Decoder
    acc.setPrivateTools(CscRDO_Decoder(**kwargs))
    return acc

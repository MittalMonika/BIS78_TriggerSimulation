# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr

def getMdtOverlay(name="MdtOverlay", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags
    kwargs.setdefault("MainInputDigitKey",overlayFlags.dataStore()+"+MDT_DIGITS")
    kwargs.setdefault("OverlayInputDigitKey",overlayFlags.evtStore()+"+MDT_DIGITS")
    kwargs.setdefault("OutputDigitKey",overlayFlags.outputStore()+"+MDT_DIGITS")
    kwargs.setdefault("IntegrationWindow", 20) # in ns
    kwargs.setdefault("MCStore",overlayFlags.evtStore())
    kwargs.setdefault("DataStore", overlayFlags.dataStore())
    kwargs.setdefault("CopySDO",not overlayFlags.isDataOverlay())
    if overlayFlags.doSignal():
        kwargs.setdefault("CopyObject", True)
    return CfgMgr.MdtOverlay(name, **kwargs)

# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr
from AthenaCommon.ConfiguredFactory import ConfigurationError


def getCopyCaloCalibrationHitContainer(name, collectionName="", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags

    if not collectionName:
        raise ConfigurationError("'collectionName' should be set! Use `CfgGetter.getAlgorithmClone`")

    kwargs.setdefault("collectionName", collectionName)
    if overlayFlags.isOverlayMT():
        kwargs.setdefault("SignalInputKey", "StoreGateSvc+" + overlayFlags.sigPrefix() + collectionName)
        kwargs.setdefault("OutputKey", "StoreGateSvc+" + collectionName)
    else:
        kwargs.setdefault("SignalInputKey", overlayFlags.evtStore() + "+" + collectionName)
        kwargs.setdefault("OutputKey", overlayFlags.outputStore() + "+" + collectionName)

    return CfgMgr.CopyCaloCalibrationHitContainer(name, **kwargs)


def getCopyInTimeJetTruthInfo(name="CopyInTimeJetTruthInfo", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags

    if overlayFlags.isOverlayMT():
        kwargs.setdefault("BkgInputKey", "StoreGateSvc+" + overlayFlags.bkgPrefix() + "InTimeAntiKt4TruthJets")
        kwargs.setdefault("OutputKey", "StoreGateSvc+" + "InTimeAntiKt4TruthJets")
    else:
        kwargs.setdefault("BkgInputKey", overlayFlags.dataStore() + "+InTimeAntiKt4TruthJets")
        kwargs.setdefault("OutputKey", overlayFlags.outputStore() + "+InTimeAntiKt4TruthJets")

    return CfgMgr.CopyJetTruthInfo(name, **kwargs)


def getCopyOutOfTimeJetTruthInfo(name="CopyOutOfTimeJetTruthInfo", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags

    if overlayFlags.isOverlayMT():
        kwargs.setdefault("BkgInputKey", "StoreGateSvc+" + overlayFlags.bkgPrefix() + "OutOfTimeAntiKt4TruthJets")
        kwargs.setdefault("OutputKey", "StoreGateSvc+" + "OutOfTimeAntiKt4TruthJets")
    else:
        kwargs.setdefault("BkgInputKey", overlayFlags.dataStore() + "+OutOfTimeAntiKt4TruthJets")
        kwargs.setdefault("OutputKey", overlayFlags.outputStore() + "+OutOfTimeAntiKt4TruthJets")        

    return CfgMgr.CopyJetTruthInfo(name, **kwargs)


def getCopyMcEventCollection(name="CopyMcEventCollection", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags

    kwargs.setdefault("RemoveBkgHardScatterTruth", True)

    if overlayFlags.isOverlayMT():
        if overlayFlags.isDataOverlay():
            # Disable background for data overlay
            kwargs.setdefault("BkgInputKey", "")
        else:
            kwargs.setdefault("BkgInputKey", "StoreGateSvc+" + overlayFlags.bkgPrefix() + "TruthEvent")
        kwargs.setdefault("SignalInputKey", "StoreGateSvc+" + overlayFlags.sigPrefix() + "TruthEvent")
        kwargs.setdefault("OutputKey", "StoreGateSvc+TruthEvent")
    else:
        if overlayFlags.isDataOverlay():
            # Disable background for data overlay
            kwargs.setdefault("BkgInputKey", "")
        else:
            kwargs.setdefault("BkgInputKey", overlayFlags.dataStore() + "+TruthEvent")
        kwargs.setdefault("SignalInputKey", overlayFlags.evtStore() + "+TruthEvent")
        kwargs.setdefault("OutputKey", overlayFlags.outputStore() + "+TruthEvent")

    return CfgMgr.CopyMcEventCollection(name, **kwargs)


def getCopyTimings(name="CopyTimings", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags

    if overlayFlags.isOverlayMT():
        if overlayFlags.isDataOverlay():
            # Disable background for data overlay
            kwargs.setdefault("BkgInputKey", "")
        else:
            kwargs.setdefault("BkgInputKey", "StoreGateSvc+" + overlayFlags.bkgPrefix() + "EVNTtoHITS_timings")
        kwargs.setdefault("SignalInputKey", "StoreGateSvc+" + overlayFlags.sigPrefix() + "EVNTtoHITS_timings")
        kwargs.setdefault("OutputKey", "StoreGateSvc+EVNTtoHITS_timings")
    else:
        kwargs.setdefault("BkgInputKey", overlayFlags.dataStore() + "+EVNTtoHITS_timings")
    kwargs.setdefault("SignalInputKey", overlayFlags.evtStore() + "+EVNTtoHITS_timings")
    kwargs.setdefault("OutputKey", overlayFlags.outputStore() + "+EVNTtoHITS_timings")

    return CfgMgr.CopyTimings(name, **kwargs)


def getCopyTrackRecordCollection(name, collectionName="", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags

    if not collectionName:
        raise ConfigurationError("'collectionName' should be set! Use `CfgGetter.getAlgorithmClone`")

    kwargs.setdefault("collectionName", collectionName)
    if overlayFlags.isOverlayMT():
        kwargs.setdefault("SignalInputKey", "StoreGateSvc+" + overlayFlags.sigPrefix() + collectionName)
        kwargs.setdefault("OutputKey", "StoreGateSvc+" + collectionName)
    else:
        kwargs.setdefault("SignalInputKey", overlayFlags.evtStore() + "+" + collectionName)
        kwargs.setdefault("OutputKey", overlayFlags.outputStore() + "+" + collectionName)

    return CfgMgr.CopyTrackRecordCollection(name, **kwargs)

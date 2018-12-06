# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr


def getCopyInTimeJetTruthInfo(name="CopyInTimeJetTruthInfo", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags

    kwargs.setdefault("BkgInputKey", overlayFlags.dataStore() + "+InTimeAntiKt4TruthJets")
    kwargs.setdefault("OutputKey", overlayFlags.outputStore() + "+InTimeAntiKt4TruthJets")

    return CfgMgr.CopyJetTruthInfo(name, **kwargs)


def getCopyOutOfTimeJetTruthInfo(name="CopyOutOfTimeJetTruthInfo", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags

    kwargs.setdefault("BkgInputKey", overlayFlags.dataStore() + "+OutOfTimeAntiKt4TruthJets")
    kwargs.setdefault("OutputKey", overlayFlags.outputStore() + "+OutOfTimeAntiKt4TruthJets")

    return CfgMgr.CopyJetTruthInfo(name, **kwargs)


def getCopyMcEventCollection(name="CopyMcEventCollection", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags

    kwargs.setdefault("RemoveBkgHardScatterTruth", True)

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

    if overlayFlags.isDataOverlay():
        # Disable background for data overlay
        kwargs.setdefault("BkgInputKey", "")
    else:
        kwargs.setdefault("BkgInputKey", overlayFlags.dataStore() + "+EVNTtoHITS_timings")
    kwargs.setdefault("SignalInputKey", overlayFlags.evtStore() + "+EVNTtoHITS_timings")
    kwargs.setdefault("OutputKey", overlayFlags.outputStore() + "+EVNTtoHITS_timings")

    return CfgMgr.CopyTimings(name, **kwargs)

# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

__doc__ = "ToolFactory to instantiate EMTrackMatchBuilder with default configuration"

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from egammaTools.egammaToolsConf import EMTrackMatchBuilder
from egammaTrackTools.egammaTrackToolsConfig import EMExtrapolationToolsCacheCfg


def EMTrackMatchBuilderCfg(flags, name='EMTrackMatchBuilder', **kwargs):

    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    acc = ComponentAccumulator()

    if "ExtrapolationTool" not in kwargs:
        extrapcacheAcc = EMExtrapolationToolsCacheCfg(flags)
        kwargs["ExtrapolationTool"] = extrapcacheAcc.popPrivateTools()
        acc.merge(extrapcacheAcc)

    kwargs.setdefault("TrackParticlesName", flags.Egamma.Keys.Output.GSFTrackParticles)
    kwargs.setdefault("broadDeltaEta",      0.1)    # candidate match is done in 2 times this  so +- 0.2
    kwargs.setdefault("broadDeltaPhi",      0.15)   # candidate match is done in 2 times this  so +- 0.3
    kwargs.setdefault("useLastMeasurement", False)  # important for GSF!!!
    kwargs.setdefault("useCandidateMatch",  True)
    kwargs.setdefault("useScoring",         True)
    kwargs.setdefault("SecondPassRescale",  True)
    kwargs.setdefault("UseRescaleMetric",   True)
    kwargs.setdefault("isCosmics",          flags.Beam.Type == "cosmics")

    tool = EMTrackMatchBuilder(name, **kwargs)

    acc.setPrivateTools(tool)
    return acc

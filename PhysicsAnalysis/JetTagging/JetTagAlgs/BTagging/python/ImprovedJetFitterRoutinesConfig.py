# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from BTagging.ImprovedJetFitterInitializationHelperConfig import ImprovedJetFitterInitializationHelperCfg
from BTagging.TrkDistanceFinderNeutralNeutralConfig import TrkDistanceFinderNeutralNeutralCfg
from BTagging.TrkDistanceFinderNeutralChargedConfig import TrkDistanceFinderNeutralChargedCfg

from TrkJetVxFitter.TrkJetVxFitterConf import Trk__JetFitterRoutines

def ImprovedJetFitterRoutinesCfg(name, useBTagFlagsDefaults = True, **options):
    """Sets up a ImprovedJetFitterRoutines tool and returns it.

    The following options have BTaggingFlags defaults:

    BeFast                              default: False
    maxDRshift                          default: 0.0
    noPrimaryVertexRefit                default: False

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    acc = ComponentAccumulator()
    if useBTagFlagsDefaults:
        accImprovedJetFitterInitializationHelper = ImprovedJetFitterInitializationHelperCfg('ImprovedJFInitHelper')
        improvedJetFitterInitializationHelper = accImprovedJetFitterInitializationHelper.popPrivateTools()
        acc.merge(accImprovedJetFitterInitializationHelper)
        accTrkDistanceFinderNeutralNeutral = TrkDistanceFinderNeutralNeutralCfg('TrkDistFinderNeutralNeutral')
        trkDistanceFinderNeutralNeutral = accTrkDistanceFinderNeutralNeutral.popPrivateTools()
        acc.merge(accTrkDistanceFinderNeutralNeutral)
        accTrkDistanceFinderNeutralCharged = TrkDistanceFinderNeutralChargedCfg('TrkDistFinderNeutralCharged')
        trkDistanceFinderNeutralCharged = accTrkDistanceFinderNeutralCharged.popPrivateTools()
        acc.merge(accTrkDistanceFinderNeutralCharged)
        #JFKalmanVertexOnJetAxisSmoother = acc.popToolsAndMerge(KalmanVertexOnJetAxisSmootherCfg('JFKalmanVertexOnJetAxisSmoother')
        defaults = {
                     'BeFast'               : False,
                     'maxDRshift'           : 0.0,
                     'noPrimaryVertexRefit' : False,
                     'JetFitterInitializationHelper' : improvedJetFitterInitializationHelper,
                     'JetFitterMinimumDistanceFinder' : trkDistanceFinderNeutralCharged,
                     'JetFitterMinimumDistanceFinderNeutral' : trkDistanceFinderNeutralNeutral, 
                     #'KalmanVertexOnJetAxisSmoother' : JFKalmanVertexOnJetAxisSmoother,
                   }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    acc.setPrivateTools(Trk__JetFitterRoutines(**options))

    return acc
# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# Configuration functions for NewJetFitterTag
# Author: Wouter van den Wollenberg (2013-2014)
# Because there are some many different JetFitter tools floating around
# now, this file was created to contain them all.
from BTagging.BTaggingFlags import BTaggingFlags


metaInDetImprovedJetFitterTrackSelectorTool_Trig = { 'DependsOn'      : ['JetFitterExtrapolator',],
                                                'PassByPointer'  : {'Extrapolator' : 'JetFitterExtrapolator'},
                                                'ToolCollection' : 'JetFitterCollection_Trig' }

def toolInDetImprovedJetFitterTrackSelectorTool_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a InDetImprovedJetFitterTrackSelectorTool tool and returns it.

    The following options have BTaggingFlags defaults:

    pTMin                               default: 500.0
    IPd0Max                             default: 7.0
    IPz0Max                             default: 10.0
    sigIPd0Max                          default: 0.35
    sigIPz0Max                          default: 2.5
    etaMax                              default: 9999.0
    useTrackSummaryInfo                 default: True
    nHitBLayer                          default: 0
    nHitPix                             default: 1
    nHitSct                             default: 4
    nHitSi                              default: 7
    nHitTrt                             default: 0
    useSharedHitInfo                    default: False
    useTrackQualityInfo                 default: True
    fitChi2OnNdfMax                     default: 3.5
    TrackSummaryTool                    default: None

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'pTMin'                        : 500.0,
                     'IPd0Max'                      : 7.0,
                     'IPz0Max'                      : 10.0,
                     'sigIPd0Max'                   : 0.35,
                     'sigIPz0Max'                   : 2.5,
                     'etaMax'                       : 9999.0,
                     'useTrackSummaryInfo'          : True,
                     'nHitBLayer'                   : 0,
                     'nHitPix'                      : 1,
                     'nHitSct'                      : 4,
                     'nHitSi'                       : 7,
                     'nHitTrt'                      : 0,
                     'useSharedHitInfo'             : False,
                     'useTrackQualityInfo'          : True,
                     'fitChi2OnNdfMax'              : 3.5,
                     'TrackSummaryTool'             : None }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from InDetTrackSelectorTool.InDetTrackSelectorToolConf import InDet__InDetDetailedTrackSelectorTool
    return InDet__InDetDetailedTrackSelectorTool(**options)

#--------------------------------------------------------------------------

metaJetFitterFullLinearizedTrackFactory_Trig = {'DependsOn'     : ['JetFitterExtrapolator',],
                                           'PassByPointer' : {'Extrapolator' : 'JetFitterExtrapolator' },
                                           'ToolCollection': 'JetFitterCollection_Trig' }

def toolJetFitterFullLinearizedTrackFactory_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a JetFitterFullLinearizedTrackFactory tool and returns it.

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                             Note however that this tool has no BTaggingFlags defaults; the option is
                             here only for consistency.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    options['name'] = name
    from TrkVertexFitterUtils.TrkVertexFitterUtilsConf import Trk__FullLinearizedTrackFactory
    return Trk__FullLinearizedTrackFactory(**options)

#--------------------------------------------------------------------------

metaJetFitterMode3dTo1dFinder_Trig = { 'ToolCollection' : 'JetFitterCollection_Trig' }

def toolJetFitterMode3dTo1dFinder_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a JetFitterMode3dTo1dFinder tool and returns it.

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                             Note however that this tool has no BTaggingFlags defaults; the option is
                             here only for consistency.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    options['name'] = name
    from TrkVertexSeedFinderUtils.TrkVertexSeedFinderUtilsConf import Trk__Mode3dTo1dFinder
    return Trk__Mode3dTo1dFinder(**options)

#--------------------------------------------------------------------------

metaJetFitterSequentialVertexSmoother_Trig = { 'ToolCollection' : 'JetFitterCollection_Trig' }

def toolJetFitterSequentialVertexSmoother_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a JetFitterSequentialVertexSmoother tool and returns it.

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                             Note however that this tool has no BTaggingFlags defaults; the option is
                             here only for consistency.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    options['name'] = name
    from TrkVertexFitters.TrkVertexFittersConf import Trk__SequentialVertexSmoother
    return Trk__SequentialVertexSmoother(**options)

#--------------------------------------------------------------------------

metaVxInternalEdmFactory_Trig = {'DependsOn'     : ['JetFitterFullLinearizedTrackFactory_Trig',],
                            'PassByPointer' : {'LinearizedTrackFactory' : 'JetFitterFullLinearizedTrackFactory_Trig' },
                            'ToolCollection': 'JetFitterCollection_Trig' }

def toolVxInternalEdmFactory_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a VxInternalEdmFactory tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'OutputLevel'             : BTaggingFlags.OutputLevel, }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from TrkVxEdmCnv.TrkVxEdmCnvConf import Trk__VxCandidateXAODVertex
    return Trk__VxCandidateXAODVertex(**options)

#--------------------------------------------------------------------------

metaJetFitterSequentialVertexFitter_Trig = { 'DependsOn'      : ['JetFitterSequentialVertexSmoother_Trig',
                                                            'JetFitterFullLinearizedTrackFactory_Trig'] ,
                                        'PassByPointer'  : {'VertexSmoother'         : 'JetFitterSequentialVertexSmoother',
                                                            'LinearizedTrackFactory' : 'JetFitterFullLinearizedTrackFactory_Trig'},

                                        'ToolCollection' : 'JetFitterCollection_Trig' }

def toolJetFitterSequentialVertexFitter_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a JetFitterSequentialVertexFitter tool and returns it.

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                             Note however that this tool has no BTaggingFlags defaults; the option is
                             here only for consistency.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    options['name'] = name
    from TrkVertexFitters.TrkVertexFittersConf import Trk__SequentialVertexFitter
    return Trk__SequentialVertexFitter(**options)

#--------------------------------------------------------------------------

metaImprovedJetFitterInitializationHelper_Trig = { 'DependsOn'      : ['JetFitterFullLinearizedTrackFactory_Trig',],
                                              'PassByPointer'  : {'LinearizedTrackFactory' : 'JetFitterFullLinearizedTrackFactory_Trig' },
                                              'ToolCollection' : 'JetFitterCollection_Trig' }

def toolImprovedJetFitterInitializationHelper_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a ImprovedJetFitterInitializationHelper tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'OutputLevel' : BTaggingFlags.OutputLevel }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from TrkJetVxFitter.TrkJetVxFitterConf import Trk__JetFitterInitializationHelper
    return Trk__JetFitterInitializationHelper(**options)

#--------------------------------------------------------------------------

metaTrkDistanceFinderNeutralNeutral_Trig = { 'ToolCollection' : 'JetFitterCollection_Trig' }

def toolTrkDistanceFinderNeutralNeutral_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a TrkDistanceFinderNeutralNeutral tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'OutputLevel' : BTaggingFlags.OutputLevel }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from TrkJetVxFitter.TrkJetVxFitterConf import Trk__TrkDistanceFinderNeutralNeutral
    return Trk__TrkDistanceFinderNeutralNeutral(**options)

#--------------------------------------------------------------------------

metaTrkDistanceFinderNeutralCharged_Trig = { 'ToolCollection' : 'JetFitterCollection_Trig' }

def toolTrkDistanceFinderNeutralCharged_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a TrkDistanceFinderNeutralCharged tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'OutputLevel' : BTaggingFlags.OutputLevel }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from TrkJetVxFitter.TrkJetVxFitterConf import Trk__TrkDistanceFinderNeutralCharged
    return Trk__TrkDistanceFinderNeutralCharged(**options)

#--------------------------------------------------------------------------

metaImprovedJetFitterRoutines_Trig = { 'DependsOn'      : ['ImprovedJetFitterInitializationHelper_Trig',
                                                      'TrkDistanceFinderNeutralNeutral_Trig',
                                                      'TrkDistanceFinderNeutralCharged_Trig'],
                                  'PassByPointer'  : {'JetFitterInitializationHelper'         : 'ImprovedJetFitterInitializationHelper_Trig',
                                                      'JetFitterMinimumDistanceFinderNeutral' : 'TrkDistanceFinderNeutralNeutral_Trig',
                                                      'JetFitterMinimumDistanceFinder'        : 'TrkDistanceFinderNeutralCharged_Trig' },
                                  'ToolCollection' : 'JetFitterCollection_Trig' }

def toolImprovedJetFitterRoutines_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a ImprovedJetFitterRoutines tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel
    BeFast                              default: False
    maxDRshift                          default: 0.0
    noPrimaryVertexRefit                default: False

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'OutputLevel'          : BTaggingFlags.OutputLevel,
                     'BeFast'               : False,
                     'maxDRshift'           : 0.0,
                     'noPrimaryVertexRefit' : False }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from TrkJetVxFitter.TrkJetVxFitterConf import Trk__JetFitterRoutines
    return Trk__JetFitterRoutines(**options)

#--------------------------------------------------------------------------

metaInDetJetFitterUtils_Trig = { 'DependsOn'      : ['JetFitterFullLinearizedTrackFactory_Trig',
                                                'JetFitterExtrapolator'],
                            'PassByPointer'  : {'LinearizedTrackFactory' : 'JetFitterFullLinearizedTrackFactory_Trig',
                                               'Extrapolator'           : 'JetFitterExtrapolator' },
                            'ToolCollection' : 'JetFitterCollection_Trig' }

def toolInDetJetFitterUtils_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a InDetJetFitterUtils tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'OutputLevel'          : BTaggingFlags.OutputLevel }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from InDetSecVxFinderTool.InDetSecVxFinderToolConf import InDet__InDetJetFitterUtils
    return InDet__InDetJetFitterUtils(**options)

#--------------------------------------------------------------------------

metaNewJetFitterVxFinder_Trig = { 'IsAVertexFinder' : True,
                             'VertexFinderxAODBaseName' : 'JetFitter',
                             'VertexFinderPassByPointer': {'JetFitterVariableFactory' : 'JetFitterVariablesFactory_Trig' },
                             'DependsOn'       : ['BTagTrackToVertexTool',
                                                  'InDetJetFitterUtils_Trig',
                                                  'ImprovedJetFitterRoutines_Trig',
                                                  'TrkDistanceFinderNeutralCharged_Trig',
                                                  'TrkDistanceFinderNeutralNeutral_Trig',
                                                  'ImprovedJetFitterInitializationHelper_Trig',
                                                  'JetFitterSequentialVertexFitter_Trig',
                                                  'JetFitterSequentialVertexSmoother_Trig',
                                                  'JetFitterMode3dTo1dFinder_Trig',
                                                  'InDetImprovedJetFitterTrackSelectorTool_Trig',
                                                  'JetFitterExtrapolator',
                                                  'JetFitterFullLinearizedTrackFactory_Trig',
                                                  'JetFitterVariablesFactory_Trig',
                                                  'VxInternalEdmFactory_Trig'],
                             'PassByPointer'    : { 'Mode3dFinder'                  : 'JetFitterMode3dTo1dFinder_Trig',
                                                    'JetFitterInitializationHelper' : 'ImprovedJetFitterInitializationHelper_Trig',
                                                    'TrackSelector'                 : 'InDetImprovedJetFitterTrackSelectorTool_Trig',
                                                    'InDetJetFitterUtils'           : 'InDetJetFitterUtils_Trig',
                                                    'Extrapolator'                  : 'JetFitterExtrapolator',
                                                    'SequentialVertexFitter'        : 'JetFitterSequentialVertexFitter_Trig',
                                                    'JetFitterRoutines'             : 'ImprovedJetFitterRoutines_Trig',
                                                    'VertexEdmFactory'              : 'VxInternalEdmFactory_Trig' },
                             'ToolCollection'   : 'JetFitterCollection_Trig' }

def toolNewJetFitterVxFinder_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a NewJetFitterVxFinder tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel
    VxPrimaryContainer                  default: BTaggingFlags.PrimaryVertexCollectionName
    MaxNumDeleteIterations              default: 30
    VertexProbCut                       default: 0.001
    MaxClusteringIterations             default: 30
    VertexClusteringProbabilityCut      default: 0.005

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        if not 'InDetKeys' in dir():
            from InDetRecExample.InDetKeys import InDetKeys
        defaults = { 'OutputLevel'                         : BTaggingFlags.OutputLevel,
                     'VxPrimaryContainer'                  : BTaggingFlags.PrimaryVertexCollectionName,
                     'MaxNumDeleteIterations'              : 30,
                     'VertexProbCut'                       : 0.001,
                     'MaxClusteringIterations'             : 30,
                     'VertexClusteringProbabilityCut'      : 0.005 }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from InDetSecVxFinderTool.InDetSecVxFinderToolConf import InDet__InDetImprovedJetFitterVxFinder
    return InDet__InDetImprovedJetFitterVxFinder(**options)

#--------------------------------------------------------------------------

metaJetFitterVariablesFactory_Trig = { #'PassByName'     : {'JetFitterInstance' : 'JetFitterTag',
                                  #                    'secVxFinderName'   : 'JetFitterVxFinder' },
                                  'ToolCollection' : 'JetFitterCollection_Trig' }

def toolJetFitterVariablesFactory_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a NewJetFitterVariablesFactory tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel
    JetFitterInstance                   default: "JetFitterTag"
    secVxFinderName                     default: "JetFitterVxFinder"

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'OutputLevel'            : BTaggingFlags.OutputLevel,
                     'JetFitterInstance'      : 'JetFitterTag',
                     'secVxFinderName'        : 'JetFitterVxFinder' }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from JetTagTools.JetTagToolsConf import Analysis__JetFitterVariablesFactory
    return Analysis__JetFitterVariablesFactory(**options)

#-------------------------------------------------------------------------

metaJetFitterNNTool_Trig = { 'CalibrationTaggers' : ['JetFitter',],
                        'DependsOn'          : ['NeuralNetworkToHistoToolNN'],
                        'PassByPointer'      : {'NeuralNetworkToHistoTool' : 'NeuralNetworkToHistoToolNN'},
                        'ToolCollection'     : 'JetFitterCollection_Trig' }

def _slim_jf_options(options):
    new_opts = {}
    removed = set(
        ['CalibrationDirectory', 'CalibrationSubDirectory',
         'NeuralNetworkToHistoTool'])
    kept = set(options) - removed
    for optname in kept:
        new_opts[optname] = options[optname]
    return new_opts

def toolJetFitterNNTool_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a JetFitterNNTool tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel
    CalibrationDirectory                default: "JetFitter"
    CalibrationSubDirectory             default: "NeuralNetwork"
    useCombinedIPNN                     default: False
    usePtCorrectedMass                  default: True

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'OutputLevel'                      : BTaggingFlags.OutputLevel,
                     'useCombinedIPNN'                  : False,
                     'CalibrationDirectory'             : 'JetFitter',
                     'CalibrationSubDirectory'          : 'NeuralNetwork',
                     'usePtCorrectedMass'               : True, }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from JetTagTools.JetTagToolsConf import Analysis__JetFitterNNTool
    from JetTagTools.JetTagToolsConf import Analysis__JetFitterInputWriter
    if BTaggingFlags.RunJetFitterNNTool:
        return Analysis__JetFitterNNTool(**options)
    else:
        return Analysis__JetFitterInputWriter(**_slim_jf_options(options))

#-------------------------------------------------------------------------

metaJetFitterTagNN_Trig = { 'IsATagger'         : True,
                       'xAODBaseName'      : 'JetFitter',
                       'DependsOn'         : ['AtlasExtrapolator',
                                              'BTagTrackToVertexTool',
                                              'NewJetFitterVxFinder',
                                              'JetFitterNNTool',
                                              'NeuralNetworkToHistoToolNN',
                                              'JetFitterNtupleWriterNN'],
                       'PassByPointer'     : { 'jetfitterClassifier'       : 'JetFitterNNTool',
                                               'jetfitterNtupleWriter'     : 'JetFitterNtupleWriterNN' },
                       'JetCollectionList' : 'jetCollectionList',
                       'ToolCollection'    : 'JetFitterCollection_Trig' }

def toolJetFitterTagNN_Trig(name, useBTagFlagsDefaults = True, **options):
    """Sets up a JetFitterTagNN tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel
    Runmodus                            default: BTaggingFlags.Runmodus
    jetCollectionList                   default: BTaggingFlags.Jets
    SecVxFinderName                     default: "JetFitter"
    useForcedCalibration                default: False
    ipinfoTaggerName                    default: ""

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'OutputLevel'                      : BTaggingFlags.OutputLevel,
                     'Runmodus'                         : BTaggingFlags.Runmodus,
                     'jetCollectionList'                : BTaggingFlags.Jets,
                     'SecVxFinderName'                  : 'JetFitter',
                     'useForcedCalibration'             : False,
                     'ipinfoTaggerName'                 : "",
                     }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from JetTagTools.JetTagToolsConf import Analysis__JetFitterTag
    return Analysis__JetFitterTag(**options)

#-------------------------------------------------------------------------

# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# Configuration functions for MV1Tag
# Author: Wouter van den Wollenberg (2013-2014)
from BTagging.BTaggingFlags import BTaggingFlags

metaMV1Tag = { 'IsATagger'          : True,
               'xAODBaseName'       : "MV1",
               'DependsOn'          : ['AtlasExtrapolator',
                                       'BTagTrackToVertexTool',
                                       'BTagCalibrationBrokerTool',
                                       'IP3DTag',
                                       'JetFitterTagCOMBNN',
                                       'SV1Tag'],
               'CalibrationFolders' : ['MV1',],
               'PassByPointer'      : {'calibrationTool' : 'BTagCalibrationBrokerTool'},
               'ToolCollection'     : 'MV1Tag' }

def toolMV1Tag(name, useBTagFlagsDefaults = True, **options):
    """Sets up a MV1Tag tool and returns it.

    The following options have BTaggingFlags defaults:

    OutputLevel                         default: BTaggingFlags.OutputLevel
    Runmodus                            default: BTaggingFlags.Runmodus
    taggerName                          default: "MV1"
    taggerNameBase                      default: "MV1"
    inputIP3DWeightName                 default: "IP3D"
    inputSV1WeightName                  default: "SV1"
    inputJetFitterWeightName            default: "JetFitterCombNN"

    input:             name: The name of the tool (should be unique).
      useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool, which can then by added to ToolSvc via ToolSvc += output."""
    if useBTagFlagsDefaults:
        defaults = { 'OutputLevel'                      : BTaggingFlags.OutputLevel,
                     'Runmodus'                         : BTaggingFlags.Runmodus,
                     'taggerName'                       : 'MV1',
                     'taggerNameBase'                   : 'MV1',
                     'inputIP3DWeightName'              : 'IP3D',
                     'inputSV1WeightName'               : 'SV1',
                     'inputJetFitterWeightName'         : 'JetFitterCombNN',
                     }
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    from JetTagTools.JetTagToolsConf import Analysis__MV1Tag
    return Analysis__MV1Tag(**options)

# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from BTagging.BTaggingFlags import BTaggingFlags
from BTagging.MV2defaultValues import default_values
from BTagging.MV2defaultValues import MVTM_varNames

# import the MV2Tag configurable
from JetTagTools.JetTagToolsConf import Analysis__MV2Tag

def MV2TagCfg( name = 'MV2c10', scheme = '', useBTagFlagsDefaults = True, **options ):
    """Sets up a MV2c10Tag tool and returns it.

    The following options have BTaggingFlags defaults:

    Runmodus                            default: BTaggingFlags.Runmodus
    taggerName                          default: "MV2c10"
    taggerNameBase                      default: "MV2c10"
    forceMV2CalibrationAlias            default: BTaggingFlags.ForceMV2CalibrationAlias
    MV2CalibAlias                       default: BTaggingFlags.MV2CalibAlias

    input:             name: The name of the tool (should be unique).
          useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool."""
    acc = ComponentAccumulator()
    options['name'] = name + 'Tag'
    basename = name
    options['xAODBaseName'] = basename

    if useBTagFlagsDefaults:
        defaults = { 'Runmodus'                         : BTaggingFlags.Runmodus,
                     'taggerName'                       : basename,
                     'taggerNameBase'                   : basename,
                     'forceMV2CalibrationAlias'         : BTaggingFlags.ForceMV2CalibrationAlias,
                     'MV2CalibAlias'                    : BTaggingFlags.MV2CalibAlias,
                     'defaultvals'                      : default_values,
                     'MVTMvariableNames'                : MVTM_varNames,
                     }
        for option in defaults:
            options.setdefault(option, defaults[option])
    acc.setPrivateTools(Analysis__MV2Tag(**options))

    return acc

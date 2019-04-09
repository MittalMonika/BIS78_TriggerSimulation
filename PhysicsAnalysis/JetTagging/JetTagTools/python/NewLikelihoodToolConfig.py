# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from BTagging.BTaggingFlags import BTaggingFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# import the NewLikelihoodTool configurable
from JetTagTools.JetTagToolsConf import Analysis__NewLikelihoodTool

def NewLikelihoodToolCfg( name = 'NewLikelihoodTool', taggername = 'IP2D', useBTagFlagsDefaults = True, **options):
    """Sets up a NewLikelihoodTool tool and returns it.

    The following options have BTaggingFlags defaults:

    taggerName                          default: "IP2D"

    input:             name: The name of the tool (should be unique).
          useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool."""
    acc = ComponentAccumulator()
    if useBTagFlagsDefaults:
        if taggername != 'SMT':
            defaults = {'taggerName'                          : taggername }
        else:
            defaults = {'taggerName'                          : taggername,
                        'smoothNTimes'                        : 0,
                        'normalizedProb'                      : True,
                        'interpolate'                         : True }
            if(BTaggingFlags.Runmodus == 'reference'):
                defaults['smoothNTimes'] = 1
        for option in defaults:
            options.setdefault(option, defaults[option])
    options['name'] = name
    acc.setPrivateTools(Analysis__NewLikelihoodTool(**options))

    return acc

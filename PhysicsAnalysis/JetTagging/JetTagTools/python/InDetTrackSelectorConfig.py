# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# import the TrackToVertexIPEstimator configurable
from InDetTrackSelectionTool.InDetTrackSelectionToolConf import InDet__InDetTrackSelectionTool

def InDetTrackSelectorCfg( name = 'InDetTrackSelector', useBTagFlagsDefaults = True, **options ):
    """Sets up a InDetTrackSelector tool and returns it.

    The following options have BTaggingFlags defaults:

    input:             name: The name of the tool (should be unique).
          useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool."""
    acc = ComponentAccumulator()
    if useBTagFlagsDefaults:
        defaults = {'CutLevel'               : "Loose",
                     'maxZ0SinTheta'          : 3 }
    for option in defaults:
        options.setdefault(option, defaults[option])
    options['name'] = name
    acc.setPrivateTools(InDet__InDetTrackSelectionTool( **options))

    return acc
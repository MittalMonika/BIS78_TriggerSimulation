# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile Fit raw channel builder tool"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def TileRawChannelBuilderFitFilterCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile Fit raw channel builder tool

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    Keyword arguments:
        CreateContainer - flag to create output container. Defaults to True.
    """

    kwargs.setdefault('CreateContainer', True)
    createContainer = kwargs['CreateContainer']

    from TileRecUtils.TileRecUtilsConf import TileRawChannelBuilderFitFilter
    kwargs['TileRawChannelBuilder'] = TileRawChannelBuilderFitFilter

    kwargs.setdefault('Name', 'TileRawChannelBuilderFitFilter')

    from TileRawChannelBuilderConfig import TileRawChannelBuilderCfg
    acc = TileRawChannelBuilderCfg(flags, **kwargs)
    tileRawChannelBuilderFit = acc.getPrimary()

    from TileConditions.TileSampleNoiseConfig import TileCondToolNoiseSampleCfg
    sampleNoiseTool = acc.popToolsAndMerge( TileCondToolNoiseSampleCfg(flags) )
    tileRawChannelBuilderFit.TileCondToolNoiseSample = sampleNoiseTool

    tileRawChannelBuilderFit.correctTime = flags.Tile.correctTime
    tileRawChannelBuilderFit.FrameLength = 7

    outputContainer = 'TileRawChannelFit' if createContainer else ""
    tileRawChannelBuilderFit.TileRawChannelContainer = outputContainer

    if flags.Tile.correctTime:
        from TileConditions.TileTimingConfig import TileCondToolTimingCfg
        timingTool = acc.popToolsAndMerge( TileCondToolTimingCfg(flags) )
        tileRawChannelBuilderFit.TileCondToolTiming = timingTool

    acc.setPrivateTools( tileRawChannelBuilderFit )

    return acc


def TileRawChannelBuilderFitOverflowCfg(flags, **kwargs):
    return TileRawChannelBuilderFitFilterCfg(flags,
                                             Name = 'TileRawChannelBuilderFitOverflow',
                                             CreateContainer = False)


if __name__ == "__main__":

    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = 1
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    ConfigFlags.Input.Files = defaultTestFiles.RAW
    ConfigFlags.Tile.RunType = 'PHY'
    ConfigFlags.Tile.NoiseFilter = 1
    ConfigFlags.lock()

    ConfigFlags.dump()

    acc = ComponentAccumulator()

    print( acc.popToolsAndMerge( TileRawChannelBuilderFitFilterCfg(ConfigFlags) ) )

    print( acc.popToolsAndMerge( TileRawChannelBuilderFitOverflowCfg(ConfigFlags) ) )

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileRawChannelBuilderFit.pkl','w') )

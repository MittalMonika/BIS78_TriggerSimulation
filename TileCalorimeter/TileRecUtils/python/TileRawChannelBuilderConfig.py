# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured base Tile raw channel builder tool"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

_runTypes = {'PHY' : 1, 'LAS' : 2, 'BILAS' : 2, 'PED' : 4, 'CIS' : 8, 'MONOCIS' : 8}

def TileRawChannelBuilderCfg(flags, **kwargs):
    """Return component accumulator with configured private base Tile raw channel builder tool

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    Keyword arguments:
        TileRawChannelbuilder -- concrete Tile raw channel builder tool.
        Name -- name of Tile raw channel builder tool.
        CreateContainer - flag ot create output container. Defaults to True.
    """

    acc = ComponentAccumulator()

    if 'TileRawChannelBuilder' not in kwargs:
        raise(Exception("No cocrete Tile raw channel builder is given"))

    TileRawChannelBuilder = kwargs['TileRawChannelBuilder']

    if 'Name' not in kwargs:
        raise(Exception("No name of Tile raw channel builder is given"))

    name = kwargs['Name']

    runType = flags.Tile.RunType
    runType = runType.upper()

    if runType not in _runTypes.keys():
        raise(Exception("Invalid Tile run type: %s" % runType))

    createContainer = kwargs.get('CreateContainer', True)

    tileRawChannelBuilder = TileRawChannelBuilder(name)

    if createContainer:
        from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
        acc.merge( TileDQstatusAlgCfg(flags) )
    else:
        tileRawChannelBuilder.TileDQstatus = ""

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    tileRawChannelBuilder.RunType = _runTypes[runType]
    tileRawChannelBuilder.calibrateEnergy = False
    tileRawChannelBuilder.AmpMinForAmpCorrection = flags.Tile.AmpMinForAmpCorrection
    tileRawChannelBuilder.TimeMinForAmpCorrection = flags.Tile.TimeMinForAmpCorrection
    tileRawChannelBuilder.TimeMaxForAmpCorrection = flags.Tile.TimeMaxForAmpCorrection

    tileRawChannelContainerDSP = ""
    if flags.Tile.NoiseFilter == 1 and createContainer:
        from TileRecUtils.TileRawChannelCorrectionConfig import TileRawChannelCorrectionToolsCfg
        correctionTools = acc.popToolsAndMerge( TileRawChannelCorrectionToolsCfg(flags) )
        tileRawChannelBuilder.NoiseFilterTools = correctionTools

        if not (flags.Input.isMC or flags.Overlay.DataOverlay):
            tileRawChannelContainerDSP = 'TileRawChannelCntCorrected'
            from TileRecUtils.TileRawChannelCorrectionConfig import TileRawChannelCorrectionAlgCfg
            acc.merge( TileRawChannelCorrectionAlgCfg(flags) )

    tileRawChannelBuilder.DSPContainer = tileRawChannelContainerDSP

    acc.setPrivateTools( tileRawChannelBuilder )

    return acc


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

    from TileRecUtils.TileRecUtilsConf import TileRawChannelBuilderFitFilter
    rchBuilderFitAcc = TileRawChannelBuilderCfg(ConfigFlags,
                                                Name = 'TileRawChannelBuilderFit',
                                                TileRawChannelBuilder = TileRawChannelBuilderFitFilter)
    print( acc.popToolsAndMerge(rchBuilderFitAcc) )

    from TileRecUtils.TileRecUtilsConf import TileRawChannelBuilderOpt2Filter
    rchBuilderOpt2Acc = TileRawChannelBuilderCfg(ConfigFlags,
                                                 Name = 'TileRawChannelBuilderOpt2',
                                                 TileRawChannelBuilder = TileRawChannelBuilderOpt2Filter)
    print( acc.popToolsAndMerge(rchBuilderOpt2Acc) )


    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileRawChannelBuilder.pkl','w') )



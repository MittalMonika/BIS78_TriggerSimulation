# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

"""Define methods to construct configured Tile L2 builder tool and algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def TileL2BuilderCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile L2 builder tool

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    """

    kwargs.setdefault('name', 'TileL2Builder')
    kwargs.setdefault('TileRawChannelContainer', 'TileRawChannelCnt')

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge( TileGMCfg(flags) )

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    if 'TileBadChanTool' not in kwargs:
        from TileConditions.TileBadChannelsConfig import TileBadChanToolCfg
        badChannelsTool = acc.popToolsAndMerge( TileBadChanToolCfg(flags) )
        kwargs['TileBadChanTool'] = badChannelsTool

    if 'TileCondToolEmscale' not in kwargs:
        from TileConditions.TileEMScaleConfig import TileCondToolEmscaleCfg
        emScaleTool = acc.popToolsAndMerge( TileCondToolEmscaleCfg(flags) )
        kwargs['TileCondToolEmscale'] = emScaleTool

    from TileL2Algs.TileL2AlgsConf import TileL2Builder
    acc.setPrivateTools( TileL2Builder(**kwargs) )

    return acc



def TileRawChannelToL2Cfg(flags, **kwargs):
    """Return component accumulator with configured Tile raw channels to L2 algorithm

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    """

    kwargs.setdefault('name', 'TileRawChannelToL2')

    if flags.Digitization.PileUpPremixing:
        kwargs.setdefault('TileL2Container', flags.Overlay.BkgPrefix + 'TileL2Cnt')
    else:
        kwargs.setdefault('TileL2Container', 'TileL2Cnt')

    acc = ComponentAccumulator()

    if 'TileL2Builder' not in kwargs:
        l2Builder = acc.popToolsAndMerge( TileL2BuilderCfg(flags) )
        kwargs['TileL2Builder'] = l2Builder


    from TileL2Algs.TileL2AlgsConf import TileRawChannelToL2
    acc.addEventAlgo( TileRawChannelToL2(**kwargs) )

    return acc


if __name__ == "__main__":

    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = True
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    ConfigFlags.Input.Files = defaultTestFiles.RAW

    ConfigFlags.fillFromArgs()
    ConfigFlags.lock()

    # Construct our accumulator to run
    from AthenaConfiguration.MainServicesConfig import MainServicesThreadedCfg
    acc = MainServicesThreadedCfg(ConfigFlags)

    from ByteStreamCnvSvc.ByteStreamConfig import TrigBSReadCfg
    acc.merge( TrigBSReadCfg(ConfigFlags) )

    acc.merge( TileRawChannelToL2Cfg(ConfigFlags) )

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(OutputStreamCfg(ConfigFlags, 'ESD', ['TileL2Container#*']))
    acc.getEventAlgo('OutputStreamESD').ExtraInputs = [('TileL2Container', 'StoreGateSvc+TileL2Cnt')]

    acc.getService('StoreGateSvc').Dump = True

    ConfigFlags.dump()
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileL2.pkl','w') )


    sc = acc.run(maxEvents=3)

    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())
# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

"""Define methods to construct configured Tile conditions tools for tests"""


def TileCondToolsTestCfg(flags):
    """Test Tile conditions tools configuration and return component accumulator"""
    
    from AthenaCommon.Logging import logging
    msg = logging.getLogger('TileCondToolsTestCfg')

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator    
    acc = ComponentAccumulator()

    from TileEMScaleConfig import TileCondToolEmscaleCfg
    emScaleTool = acc.popToolsAndMerge( TileCondToolEmscaleCfg(flags) )
    msg.info(emScaleTool)

    from TileEMScaleConfig import TileExpertToolEmscaleCfg
    emScaleExpertTool = acc.popToolsAndMerge( TileExpertToolEmscaleCfg(flags) )
    msg.info(emScaleExpertTool)

    from TileBadChannelsConfig import TileBadChanToolCfg
    badChanTool = acc.popToolsAndMerge( TileBadChanToolCfg(flags) )
    msg.info(badChanTool)

    from TileMuIDConfig import TileCondToolMuIDCfg
    muIDTool = acc.popToolsAndMerge( TileCondToolMuIDCfg(flags) )
    msg.info(muIDTool)

    from TileIntegratorConfig import TileCondToolIntegratorCfg
    integratorTool = acc.popToolsAndMerge( TileCondToolIntegratorCfg(flags) )
    msg.info(integratorTool)

    from TileTimingConfig import TileCondToolTimingCfg
    timingTool = acc.popToolsAndMerge( TileCondToolTimingCfg(flags) )
    msg.info(timingTool)

    from TileTimingConfig import TileCondToolOnlineTimingCfg
    onlineTimingTool = acc.popToolsAndMerge( TileCondToolOnlineTimingCfg(flags) )
    msg.info(onlineTimingTool)

    if flags.IOVDb.DatabaseInstance  == 'CONDBR2':
        from TileTMDBConfig import TileCondToolTMDBCfg
        tmdbTool = acc.popToolsAndMerge( TileCondToolTMDBCfg(flags) )
        msg.info(tmdbTool)

        from TileDSPThresholdConfig import TileCondToolDspThresholdCfg
        dspThrTool = acc.popToolsAndMerge( TileCondToolDspThresholdCfg(flags) )
        msg.info(dspThrTool)

    if not (flags.IOVDb.DatabaseInstance  == 'CONDBR2' and flags.Common.isOnline):
        from TilePulseShapeConfig import TileCondToolPulseShapeCfg
        pulseShapeTool = acc.popToolsAndMerge( TileCondToolPulseShapeCfg(flags) )
        msg.info(pulseShapeTool)

    if not (flags.Input.isMC or flags.Common.isOnline):
        from TileConditions.TileDCSConfig import TileDCSToolCfg
        dcsTool = acc.popToolsAndMerge( TileDCSToolCfg(flags) )
        msg.info(dcsTool)
    
    return acc


if __name__ == "__main__":

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = 1
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    
    # Test setup
    log.setLevel(DEBUG)

    ConfigFlags.Input.Files = defaultTestFiles.RAW
    ConfigFlags.lock()

    acc = ComponentAccumulator()
    acc.merge( TileCondToolsTestCfg(ConfigFlags) )

    acc.printConfig(withDetails = True, summariseProps = True)
    print(acc.getService('IOVDbSvc'))
    acc.store( open('TileCondTools.pkl','w') )

    print('All OK')

# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

"""Define methods to construct configured Tile auto correlation tool and conditions algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def TileAutoCorrelationCondAlgCfg(flags, **kwargs):
    """Return component accumulator with configured Tile auto correlation conditions algorithm

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    Keyword arguments:
        Source -- source of Tile auto correlation conditions (COOL, FILE). Defaults to COOL.
        TileAutoCorrelation -- name of Tile auto correlation conditions object. Defaults to TileAutoCorrelaton.
    """

    acc = ComponentAccumulator()

    source = kwargs.get('Source', 'COOL')
    autoCorrelation = kwargs.get('TileAutoCorrelation', 'TileAutoCorrelation')

    name = autoCorrelation + 'CondAlg'

    if source == 'COOL':
        # Connect COOL Tile conditions proxies to the algorithm (default)

        from TileConditions.TileFolders import TileFolders
        folders = TileFolders(isMC = flags.Input.isMC, isOnline = flags.Common.isOnline)

        if flags.IOVDb.DatabaseInstance  == 'CONDBR2':
            if not flags.Common.isOnline:
                autoCorrelationFolder = folders.add('/TILE/OFL02/NOISE/AUTOCR', 'TILE_OFL')
            else:
                raise(Exception("No online auto correlation folder in CONDBR2 DB"))
        else:
            autoCorrelationFolder = folders.addSplitOnline('/TILE/OFL01/NOISE/AUTOCR', '/TILE/OFL02/NOISE/AUTOCR')

        from TileConditions.TileConditionsConf import TileCondProxyCool_TileCalibDrawerFlt_ as TileCondProxyCoolFlt
        autoCorrelationProxy = TileCondProxyCoolFlt('TileCondProxyCool_NoiseAutoCr', Source = autoCorrelationFolder)
        
        from IOVDbSvc.IOVDbSvcConfig import addFolderList
        acc.merge( addFolderList(flags, folders.get()) )

    elif source == 'FILE':
        # Connect FILE Tile conditions proxies to the algorithm
        from TileConditions.TileConditionsConf import TileCondProxyFile_TileCalibDrawerFlt_ as TileCondProxyFileFlt
        autoCorrelationProxy = TileCondProxyFileFlt('TileCondProxyFile_NoiseAutoCr', Source = 'TileDefault.acr')

    else:
        raise(Exception("Invalid source: %s" % source))

    from TileConditions.TileConditionsConf import TileCalibCondAlg_TileCalibDrawerFlt_ as TileCalibFltCondAlg
    autoCorrelationCondAlg = TileCalibFltCondAlg( name = name,
                                                  ConditionsProxy = autoCorrelationProxy,
                                                  TileCalibData = autoCorrelation)

    acc.addCondAlgo(autoCorrelationCondAlg)

    return acc


def TileCondToolAutoCrCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile auto correlation tool
    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    Keyword arguments:
        Source -- source of Tile auto correlation conditions (COOL, FILE). Defaults to COOL.
        TileAutoCorrelation -- name of Tile auto correlation conditions object. Defaults to TileAutoCorrelaton.
    """

    acc = ComponentAccumulator()

    kwargs.setdefault('Source', 'COOL')
    kwargs.setdefault('TileAutoCorrelation', 'TileAutoCorrelation')

    autoCorrelation = kwargs['TileAutoCorrelation']
    name = 'TileCondToolAutoCr'

    acc.merge( TileAutoCorrelationCondAlgCfg(flags, **kwargs) )

    from TileConditions.TileConditionsConf import TileCondToolAutoCr
    acc.setPrivateTools( TileCondToolAutoCr(name, TileAutoCorrelation = autoCorrelation) )

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
    ConfigFlags.lock()

    acc = ComponentAccumulator()

    autoCorrelationTool = acc.popToolsAndMerge( TileCondToolAutoCrCfg(ConfigFlags) )
    print(autoCorrelationTool)

    acc.printConfig(withDetails = True, summariseProps = True)
    print(acc.getService('IOVDbSvc'))
    acc.store( open('TileAutoCorrelation.pkl','w') )

    print('All OK')


# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured private Tile hit vector to container tool"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def getTileFirstXing():
    """Return the earliest bunch crossing time for which interactions will be sent to the TileHitVecToCntTool"""
    return -200

def getTileLastXing():
    """Return the latest bunch crossing time for which interactions will be sent to the TileHitVecToCntTool"""
    return 150

def getTileRange(name = 'TileRange', **kwargs):
    """Return a PileUpXingFolder tool for Tile"""
    kwargs.setdefault('FirstXing', getTileFirstXing() )
    kwargs.setdefault('LastXing',  getTileLastXing() )
    kwargs.setdefault('ItemList', ['TileHitVector#TileHitVec', 'TileHitVector#MBTSHits'] )

    from PileUpComps.PileUpCompsConf import PileUpXingFolder
    return PileUpXingFolder(name, **kwargs)


def TileHitVecToCntToolCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile hit vector to container tool

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    """

    kwargs.setdefault('name', 'TileHitVecToCntTool')
    kwargs.setdefault('RndmEvtOverlay', flags.Detector.OverlayTile)

    acc = ComponentAccumulator()

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    kwargs.setdefault('TileHitVectors', ['TileHitVec' , 'MBTSHits'])
    kwargs.setdefault('TileHitContainer', 'TileHitCnt')

    kwargs.setdefault('DoHSTruthReconstruction', flags.Digitization.DoDigiTruth)
    if kwargs['DoHSTruthReconstruction']:
        kwargs.setdefault('TileHitContainer_DigiHSTruth', 'TileHitCnt_DigiHSTruth')
    else:
        kwargs.setdefault('TileHitContainer_DigiHSTruth', '')

    if 'RndmSvc' not in kwargs:
        from RngComps.RandomServices import RNG
        acc.merge( RNG(flags.Random.Engine) )
        kwargs['RndmSvc'] = acc.getService('AthRNGSvc')

    if kwargs['RndmEvtOverlay']:
        kwargs.setdefault('PileUp', False)
    else:
        kwargs.setdefault('PileUp', flags.Digitization.Pileup)

    if kwargs['RndmEvtOverlay'] or kwargs['PileUp']:
        from PileUpTools.PileUpToolsConf import PileUpMergeSvc
        acc.addService( PileUpMergeSvc() )

    if flags.Beam.Type == 'cosmics':
        from CommissionUtils.CommissionUtilsConf import CosmicTriggerTimeTool
        kwargs.setdefault('TriggerTimeTool', CosmicTriggerTimeTool())
        kwargs.setdefault('HitTimeFlag', 2)
        kwargs.setdefault('UseTriggerTime', True)

    if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
        kwargs.setdefault("FirstXing", getTileFirstXing() )
        kwargs.setdefault("LastXing",  getTileLastXing() )

    from TileSimAlgs.TileSimAlgsConf import TileHitVecToCntTool
    acc.setPrivateTools(TileHitVecToCntTool(**kwargs))

    return acc



def TileHitVecToCntCfg(flags, **kwargs):
    """Return component accumulator with configured Tile hit vector to container algorithm

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    """

    kwargs.setdefault('name', 'TileHitVecToCnt')
    acc = ComponentAccumulator()

    if 'DigitizationTool' not in kwargs:
        tool = acc.popToolsAndMerge( TileHitVecToCntToolCfg(flags) )
        kwargs.setdefault('DigitizationTool', tool)

    from TileSimAlgs.TileSimAlgsConf import TileHitVecToCnt
    acc.addEventAlgo(TileHitVecToCnt(**kwargs))

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

    ConfigFlags.Input.Files = defaultTestFiles.HITS
    ConfigFlags.Output.RDOFileName = 'myRDO.pool.root'
    ConfigFlags.IOVDb.GlobalTag = 'OFLCOND-MC16-SDR-16'
    ConfigFlags.Digitization.Pileup = False
    ConfigFlags.Digitization.Pileup = False

    ConfigFlags.fillFromArgs()
    ConfigFlags.lock()

    # Construct our accumulator to run
    from AthenaConfiguration.MainServicesConfig import MainServicesThreadedCfg
    acc = MainServicesThreadedCfg(ConfigFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(ConfigFlags))

    print( acc.merge( TileHitVecToCntCfg(ConfigFlags) ) )

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(OutputStreamCfg(ConfigFlags, 'RDO', ['TileHitContainer#*']))
    acc.getEventAlgo('OutputStreamRDO').ExtraInputs = [('TileHitContainer', 'StoreGateSvc+TileHitCnt')]

    acc.getService('StoreGateSvc').Dump = True
    acc.printConfig(withDetails = True, summariseProps = True)
    ConfigFlags.dump()
    acc.store( open('TileHitVecToCnt.pkl','w') )

    sc = acc.run(maxEvents=3)
    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())

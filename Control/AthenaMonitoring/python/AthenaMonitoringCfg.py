# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def AthenaMonitoringCfg(flags):
    import logging
    local_logger = logging.getLogger('AthenaMonitoringCfg')
    info = local_logger.info
    result = ComponentAccumulator()

    if flags.DQ.Steering.doPixelMon:
        info('Set up Pixel monitoring')
        from PixelMonitoring.PixelMonitoringConfig import PixelMonitoringConfig
        result.merge(PixelMonitoringConfig(flags))
    
    if flags.DQ.Steering.doSCTMon:
        info('Set up SCT monitoring')
        from SCT_Monitoring.SCTMonitoringConfig import SCTMonitoringConfig
        result.merge(SCTMonitoringConfig(flags))

    if flags.DQ.Steering.doTRTMon:
        info('Set up TRT monitoring')
        from TRTMonitoringRun3.TRTMonitoringRun3_Tool import TRTMonitoringRun3_ToolConfig
        result.merge(TRTMonitoringRun3_ToolConfig(flags))

    if flags.DQ.Steering.doLArMon:
        info('Set up LAr monitoring')
        from LArConfiguration.LArMonitoringConfig import LArMonitoringConfig
        result.merge(LArMonitoringConfig(flags))

    if flags.DQ.Steering.doTileMon:
        info('Set up Tile monitoring')
        from TileMonitoring.TileJetMonitorAlgorithm import TileJetMonitoringConfig
        result.merge(TileJetMonitoringConfig(flags))

    if flags.DQ.Steering.doMuonMon:
        info('Set up Muon monitoring')
        from MuonDQAMonitoring.MuonDQAMonitoringConfig import MuonDQAMonitoringConfig
        result.merge(MuonDQAMonitoringConfig(flags))
        
    if flags.DQ.Steering.doHLTMon:
        info('Set up HLT monitoring')
        from TrigHLTMonitoring.TrigHLTMonitorAlgorithm import TrigHLTMonTopConfig
        result.merge(TrigHLTMonTopConfig(flags))

    if flags.DQ.Steering.doJetMon:
        info('Set up Jet monitoring')
        from JetMonitoring.JetMonitoringExample import jetMonitoringExampleConfig
        result.merge(jetMonitoringExampleConfig(flags))
        
    if flags.DQ.Steering.doGlobalMon:
        info('Set up Global monitoring')
        from DataQualityTools.DataQualityToolsConfig import DataQualityToolsConfig
        result.merge(DataQualityToolsConfig(flags))

    if flags.DQ.Steering.doTauMon:
        local_logger.info('Set up Tau monitoring')
        from tauMonitoring.TauMonitoringConfig import TauMonitoringConfig
        result.merge(TauMonitoringConfig(flags))

    return result

#!/usr/bin/env python
"""Test various ComponentAccumulator Digitization configuration modules

Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
"""
import sys
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from AthenaCommon.Configurable import Configurable
from AthenaConfiguration.AllConfigFlags import ConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesSerialCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from BCM_Digitization.BCM_DigitizationConfigNew import BCM_DigitizationCfg
from SCT_Digitization.SCT_DigitizationConfigNew import SCT_DigitizationCfg
from TRT_Digitization.TRT_DigitizationConfigNew import TRT_DigitizationCfg
from PixelDigitization.PixelDigitizationConfigNew import PixelDigitizationCfg
from MuonConfig.MDT_DigitizationConfig import MDT_DigitizerDigitToRDOCfg
from MuonConfig.TGC_DigitizationConfig import TGC_DigitizerDigitToRDOCfg
from MuonConfig.RPC_DigitizationConfig import RPC_DigitizerDigitToRDOCfg
from MuonConfig.CSC_DigitizationConfig import CSC_DigitBuilderDigitToRDOCfg
from LArDigitization.LArDigitizationConfigNew import LArTriggerDigitizationCfg
from TileSimAlgs.TileDigitizationConfig import TileDigitizationCfg, TileTriggerDigitizationCfg
from Digitization.DigitizationParametersConfig import writeDigitizationMetadata

# Set up logging and new style config
log.setLevel(DEBUG)
Configurable.configurableRun3Behavior = True
# Configure
ConfigFlags.Input.Files = defaultTestFiles.HITS_SPECIAL
ConfigFlags.Output.RDOFileName = "myRDO.pool.root"
ConfigFlags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"
ConfigFlags.GeoModel.Align.Dynamic = False
ConfigFlags.Concurrency.NumThreads = 1
ConfigFlags.Beam.NumberOfCollisions = 0.
ConfigFlags.lock()
# Construct our accumulator to run
acc = MainServicesSerialCfg()
acc.merge(PoolReadCfg(ConfigFlags))
acc.merge(BCM_DigitizationCfg(ConfigFlags))
acc.merge(SCT_DigitizationCfg(ConfigFlags))
acc.merge(TRT_DigitizationCfg(ConfigFlags))
acc.merge(PixelDigitizationCfg(ConfigFlags))
acc.merge(MDT_DigitizerDigitToRDOCfg(ConfigFlags))
acc.merge(TGC_DigitizerDigitToRDOCfg(ConfigFlags))
acc.merge(RPC_DigitizerDigitToRDOCfg(ConfigFlags))
acc.merge(CSC_DigitBuilderDigitToRDOCfg(ConfigFlags))
acc.merge(LArTriggerDigitizationCfg(ConfigFlags))
acc.merge(TileDigitizationCfg(ConfigFlags))
acc.merge(TileTriggerDigitizationCfg(ConfigFlags))
acc.merge(writeDigitizationMetadata(ConfigFlags))

# Dump config
acc.getService("StoreGateSvc").Dump = True
acc.getService("ConditionStore").Dump = True
acc.printConfig(withDetails=True)
ConfigFlags.dump()
# Execute and finish
sc = acc.run(maxEvents=3)
# Success should be 0
sys.exit(not sc.isSuccess())


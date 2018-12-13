#
#  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
from AthenaCommon.CFElements import parOR, seqOR, seqAND, stepSeq
from AthenaCommon.AlgSequence import dumpMasterSequence
from AthenaCommon.AppMgr import theApp

from TriggerMenuMT.HLTMenuConfig.Menu.LS2_v1_newJO import setupMenu

from AthenaCommon.Configurable import Configurable
Configurable.configurableRun3Behavior=1



setupMenu(flags)


flags.Input.isMC = False
flags.Input.Files= ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data17_13TeV.00327265.physics_EnhancedBias.merge.RAW._lb0100._SFO-1._0001.1"] 

flags.Trigger.LVL1ConfigFile = "LVL1config_Physics_pp_v7.xml" 
flags.Trigger.L1Decoder.forceEnableAllChains = True


flags.lock()

from AthenaCommon.Constants import INFO,DEBUG
acc = ComponentAccumulator()

from ByteStreamCnvSvc.ByteStreamConfig import TrigBSReadCfg
acc.merge(TrigBSReadCfg(flags ))


from TrigUpgradeTest.TriggerHistSvcConfig import TriggerHistSvcConfig
acc.merge(TriggerHistSvcConfig(flags ))


from TriggerMenuMT.HLTMenuConfig.Menu.GenerateMenuMT_newJO import generateMenu
from TriggerJobOpts.TriggerConfig import triggerRunCfg
acc.merge( triggerRunCfg( flags, generateMenu ) )

# TODO take care of merging RegSel, and remove it from here
from RegionSelector.RegSelConfig import RegSelConfig
rsc, regSel = RegSelConfig( flags )
regSel.enableCalo=True
regSel.enableID=False
regSel.enablePixel = False
regSel.enableSCT = False
regSel.enableTRT = False
acc.merge( rsc )
acc.addService(regSel)

acc.getEventAlgo( "TrigSignatureMoniMT" ).OutputLevel=DEBUG
print acc.getEventAlgo( "TrigSignatureMoniMT" )



# from TrigUpgradeTest.TestUtils import applyMenu
# applyMenu( acc.getEventAlgo( "L1Decoder" ) )
#acc.getEventAlgo( "L1Decoder" ).OutputLevel=DEBUG
#acc.getEventAlgo( "L2ElectronCaloHypo" ).OutputLevel=DEBUG
#acc.getEventAlgo( "FastEMCaloAlgo" ).OutputLevel=DEBUG
#acc.getEventAlgo( "Filter_for_L2PhotonCaloHypo" ).OutputLevel=DEBUG

acc.printConfig()


fname = "newJOtest.pkl"
print "Storing config in the config", fname
with file(fname, "w") as p:
    acc.store( p )
    p.close()




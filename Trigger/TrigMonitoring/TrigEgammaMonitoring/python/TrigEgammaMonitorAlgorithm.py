#
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

'''@file TrigEgammaMonitorAlgorithm.py
@author J. Pinto (jodafons)
@author D. Maximov
@author E. Bergeaas Kuutmann
@author T. Bold
@date 2019-06-28
@brief Egamma trigger python configuration for the Run III AthenaMonitoring package, based on the example by E. Bergeaas Kuutmann
'''
from AthenaCommon.Logging import logging
log_trigeg = logging.getLogger( 'TrigEgammaMonitorAlgorithm' )



def TrigEgammaMonConfig(inputFlags):
    '''Function to configures some algorithms in the monitoring system.'''

 
    # The following class will make a sequence, configure algorithms, and link
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'TrigEgammaAthMonitorCfg')
    # configure alg and ana tools
    from TrigEgammaMonitoring.TrigEgammaMonitoringMTConfig import TrigEgammaMonAlgBuilder
    monAlgCfg = TrigEgammaMonAlgBuilder( helper, '2018' ) # Using 2018 e/g tunings
    
    # Force monitor builder
    monAlgCfg.activate_zee=False
    monAlgCfg.activate_jpsiee=False
    monAlgCfg.activate_electron=True
    monAlgCfg.activate_photon=False
    # build monitor and book histograms
    monAlgCfg.configure()




    # Test matching tool 
    # The following class will make a sequence, configure algorithms, and link
    #from AthenaMonitoring import AthMonitorCfgHelper
    #helper = AthMonitorCfgHelper(inputFlags,'TrigEgammaAthMonitorCfg')
    #from TrigEgammaMatchingTool.TrigEgammaMatchingToolConf import TrigEgammaMatchingToolMTTest, Trig__TrigEgammaMatchingToolMT 
    #mon = helper.addAlgorithm( TrigEgammaMatchingToolMTTest, "TrigEgammaMatchingToolMTTest" )
    #triggers = [
    #    "HLT_e26_etcut_L1EM22VHI",
    #    "HLT_e26_lhtight_L1EM24VHI",
    #    "HLT_e300_etcut_L1EM24VHI",
    #    "HLT_e3_etcut_L1EM3",
    #    "HLT_e5_etcut_L1EM3",
    #    "HLT_e7_etcut_L1EM3",
    #    ]
    #tool = Trig__TrigEgammaMatchingToolMT("MatchingTool")
    #mon.TrigEgammaMatchingToolMT = tool
    #mon.TriggerList = triggers
    #helper.resobj.addPublicTool(tool)
   




    # Finalize. The return value should be a tuple of the ComponentAccumulator
    # and the sequence containing the created algorithms. If we haven't called
    # any configuration other than the AthMonitorCfgHelper here, then we can 
    # just return directly (and not create "result" above)
    # return the componenet accumulator to the main call
    return helper.result()
    

    




if __name__=='__main__':
    # Setup the Run III behavior
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = 1

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    path = '/afs/cern.ch/work/j/jodafons/public/valid_sampleA/AOD.20745922._000041.pool.root.1'
    ConfigFlags.Input.Files = [path]
    ConfigFlags.Input.isMC = False
    ConfigFlags.Output.HISTFileName = 'TrigEgammaMonitorOutput.root'
    
    ConfigFlags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesSerialCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesSerialCfg()
    cfg.merge(PoolReadCfg(ConfigFlags))

    trigEgammaMonitorAcc = TrigEgammaMonConfig(ConfigFlags)
    cfg.merge(trigEgammaMonitorAcc)
  


    # If you want to turn on more detailed messages ...
    #trigEgammaMonitorAcc.getEventAlgo('TrigEgammaMonAlg').OutputLevel = 2 # DEBUG
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    cfg.run(10) #use cfg.run(20) to only run on first 20 events

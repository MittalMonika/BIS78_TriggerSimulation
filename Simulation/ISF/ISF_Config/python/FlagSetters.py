# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration


## Base method

def configureFlagsBase():
    from AthenaCommon.DetFlags import DetFlags
    if not DetFlags.any_on():
        # If you configure some of the DetFlags then you're
        # responsible for configuring all of them.
        DetFlags.all_setOn()
        DetFlags.Truth_setOn() # FIXME redundant?
        DetFlags.LVL1_setOff() # LVL1 is not part of G4 sim
        DetFlags.Forward_setOff() # Forward dets are off by default

    ## Configure tasks
    DetFlags.digitize.all_setOff()
    DetFlags.makeRIO.all_setOff()
    #DetFlags.overlay.all_setOff() # will soon be needed
    DetFlags.pileup.all_setOff()
    DetFlags.readRDOBS.all_setOff()
    DetFlags.readRDOPool.all_setOff()
    DetFlags.readRIOBS.all_setOff()
    DetFlags.readRIOPool.all_setOff()
    DetFlags.simulateLVL1.all_setOff()
    DetFlags.writeBS.all_setOff()
    DetFlags.writeRDOPool.all_setOff()
    DetFlags.writeRIOPool.all_setOff()
    return

## methods for Geant4 only simulators

def configureFlagsFullG4():
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "FullG4"
    return

def configureFlagsPassBackG4():
    configureFlagsFullG4()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "PassBackG4"
    return

def configureFlagsMC12G4():
    configureFlagsFullG4()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "MC12G4"
    return

def configureFlagsMC12G4_longLived():
    configureFlagsMC12G4()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "MC12G4_longLived"
    return

def configureFlagsCosmicsG4():
    configureFlagsMC12G4()
    return

def configureFlagsMC12G4_IDOnly():
    configureFlagsFullG4()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "MC12G4_IDOnly"
    return

def configureFlagsMC12G4_IDCalo():
    configureFlagsFullG4()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "MC12G4_IDCalo"
    return

## methods for simulators which use G4 + FastCaloSim

def configureFlagsATLFASTII():
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "AtlfastII" # TODO: can we rename this to "ATLFASTII" ?
    return

## methods for simulators which use Fatras + FastCaloSim

def configureFlagsATLFASTIIF():
    from AthenaCommon.DetFlags import DetFlags
    # BCM simulation not supported in FATRAS. Currently it is the
    # geometry task which is used to determine whether a detector has
    # been simulated (see ISF_Example/python/ISF_Output.py)
    DetFlags.geometry.BCM_setOff()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "ATLFASTIIF"
    return

def configureFlagsFatras_newExtrapolation():
    configureFlagsATLFASTIIF()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "Fatras_newExtrapolation"
    return

def configureFlagsFatras_newExtrapolation_IDOnly():
    configureFlagsATLFASTIIF()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "Fatras_newExtrapolation_IDOnly"
    return

def configureFlagsATLFASTIIF_PileUp():
    configureFlagsATLFASTIIF()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "ATLFASTIIF_PileUp"
    return

def configureFlagsATLFASTIIF_IDOnly():
    configureFlagsATLFASTIIF()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "ATLFASTIIF_IDOnly"
    return

def configureFlagsATLFASTIIF_IDCalo():
    configureFlagsATLFASTIIF()
    from G4AtlasApps.SimFlags import simFlags
    simFlags.SimulationFlavour = "ATLFASTIIF_IDCalo"
    return

def configureFlagsFastOnly():
    configureFlagsATLFASTIIF()
    return

## methods for simulators which combine Geant4,  Fatras and FastCaloSim

def configureFlagsMultiSimTest():
    return

def configureFlagsG4GammaCones():
    return

def configureFlagsG4GammaCones_FastCalo():
    return

def configureFlagsFatrasGammaCones_FastCalo():
    return

def configureFlagsG4BHadronsOnly_FastCalo():
    return

def configureFlagsG4BHadrons_FatrasID_FastCalo():
    return

def configureFlagsG4TauProductsOnly():
    return

def configureFlagsG4HiggsLeptonsConeOnly():
    return

def configureFlagsG4HiggsLeptons():
    return

def configureFlagsG4WLeptonsCone_FatrasID_FastCalo ():
    return

def configureFlagsG4JPsiLeptonsCone_FatrasID_FastCalo ():
    return

def configureFlagsG4ZLeptonsCone_FatrasID_FastCalo ():
    return

def configureFlagsG4ZLeptonsConeSticky_FatrasID_FastCalo ():
    return

def configureFlagsG4ZDecayProducts_FatrasID_FastCalo ():
    return


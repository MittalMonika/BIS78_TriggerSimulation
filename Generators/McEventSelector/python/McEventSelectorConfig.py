# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

# https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/AthenaJobConfigRun3

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def McEventSelectorCfg(configFlags):
    cfg=ComponentAccumulator()

    from AthenaCommon import AtlasUnixStandardJob

    from McEventSelector.McEventSelectorConf import McCnvSvc
    mcCnvSvc = McCnvSvc()
    cfg.addService(mcCnvSvc)
    from GaudiSvc.GaudiSvcConf import EvtPersistencySvc
    cfg.addService(EvtPersistencySvc("EventPersistencySvc",CnvServices=[mcCnvSvc.getFullJobOptName(),]))

    from McEventSelector.McEventSelectorConf import McEventSelector
    evSel=McEventSelector("EventSelector")
    evSel.RunNumber = configFlags.Input.RunNumber
    evSel.InitialTimeStamp = configFlags.Input.InitialTimeStamp
    cfg.addService(evSel)
    cfg.setAppProperty("EvtSel",evSel.getFullJobOptName())

    return cfg

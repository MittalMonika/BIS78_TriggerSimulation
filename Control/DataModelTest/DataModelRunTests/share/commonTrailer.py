# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration.

#--------------------------------------------------------------
# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
#--------------------------------------------------------------
svcMgr.MessageSvc.OutputLevel = 3
svcMgr.MessageSvc.debugLimit  = 100000
svcMgr.ClassIDSvc.OutputLevel = 3

# No stats printout
from GaudiSvc import GaudiSvcConf
ChronoStatSvc = GaudiSvcConf.ChronoStatSvc()
ChronoStatSvc.ChronoPrintOutTable = FALSE
ChronoStatSvc.PrintUserTime       = FALSE
ChronoStatSvc.StatPrintOutTable   = FALSE

include ('DataModelRunTests/setCatalog.py')

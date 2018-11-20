#
# $Id$
#
# File: share/CondWriter_jo.py
# Author: snyder@bnl.gov
# Date: Jul 2017
# Purpose: Write some conditions objects for testing purposes.
#

## basic job configuration (for generator)
import AthenaCommon.AtlasUnixGeneratorJob

## get a handle to the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

## get a handle to the ApplicationManager
from AthenaCommon.AppMgr import theApp


#--------------------------------------------------------------
# Load POOL support
#--------------------------------------------------------------
import AthenaPoolCnvSvc.WriteAthenaPool

include ('DataModelRunTests/loadWriteDicts.py')


#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
theApp.EvtMax = 20

#--------------------------------------------------------------
# Output options
#--------------------------------------------------------------


from AthenaServices.AthenaServicesConf import AthenaOutputStreamTool
condstream = AthenaOutputStreamTool ('CondStream',
                                         OutputFile = 'condtest.pool.root')

from DataModelTestDataCommon.DataModelTestDataCommonConf import \
     DMTest__CondWriterAlg
topSequence += DMTest__CondWriterAlg (Streamer = condstream)


# Configure conditions DB output to local sqlite file.
try:
    os.remove('condtest.db')
except OSError:
    pass
try:
    os.remove('condtest.pool.root')
except OSError:
    pass
import IOVDbSvc.IOVDb
svcMgr.IOVDbSvc.dbConnection  = "sqlite://;schema=condtest.db;dbname=OFLP200"


#--------------------------------------------------------------
# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
#--------------------------------------------------------------
svcMgr.MessageSvc.OutputLevel = 3
svcMgr.MessageSvc.debugLimit  = 100000
svcMgr.ClassIDSvc.OutputLevel = 3

# No stats printout
ChronoStatSvc = Service( "ChronoStatSvc" )
ChronoStatSvc.ChronoPrintOutTable = FALSE
ChronoStatSvc.PrintUserTime       = FALSE
ChronoStatSvc.StatPrintOutTable   = FALSE


# Avoid races when running tests in parallel.
FILECATALOG = 'CondWriter_catalog.xml'
include ('DataModelRunTests/setCatalog.py')

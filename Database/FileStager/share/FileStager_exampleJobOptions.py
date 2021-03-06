DetDescrVersion="ATLAS-GEO-02-01-00" ## Set your geometry here
#ConditionsTag="COMCOND-REPC-003-00" ## Set the conditions tag here;not needed for AOD

##------------------------------------------------------------
## These 4 lines are needed for actual data
##------------------------------------------------------------
#from AthenaCommon.GlobalFlags import GlobalFlags
#GlobalFlags.DetGeo.set_atlas()
#GlobalFlags.DataSource.set_data()
#from IOVDbSvc.CondDB import conddb

doTrigger = False ## Need to check
EvtMax=-1 ## number of event to process

## the Input AOD File(s)
include ("FileStager/input_FileStagerRFCP.py")  ## for rfcp
#include ("FileStager/input_FileStager.py") ## for lcg-cp

## UserAlgs section
## include your algorithm job options here
UserAlgs=[ # 'package/yourJOwouldGoHere.py',
         ]

## Output log setting; this is for the framework in general
## You may over-ride this in your job options for your algorithm

OutputLevel = INFO

## Read settings; for performance DPD set ESD to true
readRDO = False
readESD = False
readAOD = True

## Write settings; keep all of these to false.
## Control the writing of your own n-tuple in the alg's job options
doCBNT = False
doWriteESD = False
doWriteAOD = False
doWriteTAG = False

## need these two flags to turn on lumiblockmetadatatool and output metadatastore
doDPD=True
doFileMetaData=True

## Turn off perfmon
from RecExConfig.RecFlags import rec
rec.doPerfMon=False

## Turn down verbosity of eventmanager
AthenaEventLoopMgr = Service( "AthenaEventLoopMgr" )
AthenaEventLoopMgr.OutputLevel = WARNING

## main jobOption - must always be included
include ("RecExCommon/RecExCommon_topOptions.py")


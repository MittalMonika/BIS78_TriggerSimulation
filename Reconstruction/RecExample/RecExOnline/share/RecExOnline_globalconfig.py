if (not 'usePickleConfig' in dir()):
    usePickleConfig = False

if (not 'pickleconfigfile' in dir()):
    usePickleConfig = False

if (not 'DataSource' in dir()):
    DataSource = 'data'

if (not 'InputFormat' in dir()):
    InputFormat = 'bytestream'

if (not 'fileName' in dir()):
    fileName = None

if (not 'doESD' in dir()):
    doESD = True

if (not 'writeESD' in dir()):
    writeESD = False

if (not 'doAOD' in dir()):
    doAOD = False

if (not 'writeAOD' in dir()):
    writeAOD = False

if (not 'isOnlineStateless' in dir()):
    isOnlineStateless = True

if (not 'beamType' in dir()):
    beamType = 'collisions'

if (not 'is_T0_project_tag' in dir()):
    is_T0_project_tag = 'data12_8TeV'

if (not 'ConditionsTag' in dir()):
    ConditionsTag = 'COMCOND-HLTP-004-01'

if (not 'DetDescrVersion' in dir()):
    DetDescrVersion = 'ATLAS-GEO-20-00-01'

if (not 'doPixelOnlyMon' in dir()):
    doPixelOnlyMon = False

if (not 'REO_doAutoConfiguration' in dir()):
    REO_doAutoConfiguration = False


## -------------------------------------------------------------------
## configuration from pickle file

import os,pickle

from AthenaCommon.Logging import logging
logRecExOnline_globalconfig = logging.getLogger( 'RecExOnline_globalconfig' )

if usePickleConfig and os.path.exists(pickleconfigfile):
    f = open(pickleconfigfile, 'r')
    onlinegoodies = pickle.load(f)
    f.close()

    if onlinegoodies.has_key('DataSource'):
        DataSource = onlinegoodies['DataSource']
    if onlinegoodies.has_key('ConditionsTag'):
        ConditionsTag = onlinegoodies['ConditionsTag']
    if onlinegoodies.has_key('beamType'):
        beamType = onlinegoodies['beamType']
    if onlinegoodies.has_key('DetDescrVersion'):
        DetDescrVersion = onlinegoodies['DetDescrVersion']

    if onlinegoodies.has_key('preExec'):
        preExec = onlinegoodies['preExec'].split(',,')
    if onlinegoodies.has_key('postExec'):
        postExec = onlinegoodies['postExec'].split(',,')
    if onlinegoodies.has_key('preInclude'):
        preInclude = onlinegoodies['preInclude'].split(',,')
    if onlinegoodies.has_key('postInclude'):
        postInclude = onlinegoodies['postInclude'].split(',,')

## -------------------------------------------------------------------
## Online Configuration from IS

import sys

# The dl module seems to be missing on 64-bit platforms.
# Since RTLD_NOW==0x002 and RTLD_GLOBAL==0x100 very commonly
# we will just guess that the proper flags are 0x102 when there
# is no dl module.
try:
    import dl
    newflags = dl.RTLD_NOW|dl.RTLD_GLOBAL
    logRecExOnline_globalconfig.info("import dl failed with newflags=%s" % newflags )
    logRecExOnline_globalconfig.info("proceeding with preconfigured newflags=0x102")
except:
    newflags = 0x102  # No dl module, so guess (see above).
try:
    sys.setdlopenflags(newflags)
except:
    logRecExOnline_globalconfig.info("sys.setdlopenflags(newflags) failed with newflags=%s" % newflags )

#import dl
#sys.setdlopenflags(dl.RTLD_GLOBAL | dl.RTLD_NOW)


### remember flags to set this correctly via default
from AthenaCommon.BFieldFlags import jobproperties

#if (useEmon and (partitionName == 'ATLAS' or partitionName == 'TDAQ')):
if (useEmon and (partitionName == 'ATLAS')):
    import ispy
    from ispy import *
    from ipc import IPCPartition
    from ispy import ISObject

    p2  = ispy.IPCPartition(partitionName)
    obj = ispy.ISObject(p2, 'RunParams.RunParams', 'RunParams')
    obj.checkout()
    is_T0_project_tag = obj.T0_project_tag
    is_run_number = obj.run_number
    is_beam_type = obj.beam_type
    logRecExOnline_globalconfig.info("is_run_number = %s" % is_run_number)
    logRecExOnline_globalconfig.info("is_beam_type = %s" % is_beam_type)
    logRecExOnline_globalconfig.info("is_T0_project_tag = %s" % is_T0_project_tag)
##     print 'is_run_number', is_run_number
##     print 'is_beam_type', is_beam_type
##     print 'is_T0_project_tag', is_T0_project_tag

    part_name = 'initial'
    p = IPCPartition(part_name)
    toroidCurrent = 0
    toroidInvalid = 1
    solenoidCurrent = 0
    solenoidInvalid = 1

    # AL playing around: start
    toroidCurrent   = ispy.ISInfoDynAny(p2, 'DdcFloatInfo')
    solenoidCurrent = ispy.ISInfoDynAny(p2, 'DdcFloatInfo')
    toroidInvalid   = ispy.ISInfoDynAny(p2, 'DdcIntInfo')
    solenoidInvalid = ispy.ISInfoDynAny(p2, 'DdcIntInfo')
    
    toroidInvalid.value=1
    solenoidInvalid.value=1
    # AL playing around: end
    

    logRecExOnline_globalconfig.info("toroidCurrent = %f", toroidCurrent.value)
    logRecExOnline_globalconfig.info("toroidInvalid = %f", toroidInvalid.value)
    logRecExOnline_globalconfig.info("solenoidCurrent = %f", solenoidCurrent.value)
    logRecExOnline_globalconfig.info("solenoidInvalid = %f", solenoidInvalid.value)
    
    
 
    from BFieldAth.BFieldAthConf import MagFieldAthenaSvc
    svcMgr += MagFieldAthenaSvc(SetupCOOL=True, NameOfTheSource='COOL_HLT', onlineSolCur=solenoidCurrent.value, onlineTorCur=toroidCurrent.value)

## # ---> AK
## # LArMon stuff
## # CaloMon problems of setting up FirstSample
## try:
##     import ispy
##     from ispy import *
##     from ipc import IPCPartition
##     from ispy import ISObject

##     p3 = IPCPartition("ATLAS")
##     x = ISObject(p3, 'LargParams.LArg.RunLogger.GlobalParams', 'GlobalParamsInfo')
## except:
##     logRecExOnline_globalconfig.info("Could not find IS Parameters for LargParams.LArg.RunLogger.GlobalParams - Set default flags (FirstSample=3, NSamples=5")
##     # print "Could not find IS Parameters for LargParams.LArg.RunLogger.GlobalParams - Set default flags (FirstSample=3, NSamples=5) "
##     FirstSample = 3
##     NSamples = 5
## else:
##     try:
##         x.checkout()
##     except:
##         logRecExOnline_globalconfig.info("Could not find IS Parameters for LargParams.LArg.RunLogger.GlobalParams - Set default flags: FirstSample=3, NSamples=5")
##         #print Couldn not find IS Parameters - Set default flag"
##         FirstSample = 3
##         NSamples = 5
##     else:
##         FirstSample = x.firstSample
##         NSamples = x.nbOfSamples
## # ---> AK
          

# ----------------------------------------------- Set Run configuration

## Second, fall back to manual configuration or default values
from AthenaCommon.GlobalFlags import globalflags
from AthenaCommon.AthenaCommonFlags import jobproperties,athenaCommonFlags

globalflags.DataSource.set_Value_and_Lock(DataSource)
globalflags.InputFormat.set_Value_and_Lock(InputFormat)

if not useEmon:
    athenaCommonFlags.BSRDOInput.set_Value_and_Lock([fileName])
else:
    athenaCommonFlags.BSRDOInput.set_Value_and_Lock([''])

## suppress needless warnings "no stream X associated with id Y" for online jobs
from GaudiSvc.GaudiSvcConf import THistSvc
THistSvc.OutputLevel = ERROR

### switch off NN Pixel cluster splitting, as suggested in bug #91340
from InDetRecExample.InDetJobProperties import InDetFlags
InDetFlags.doPixelClusterSplitting.set_Value(False) # does not work online

# ----------------------------------------------- Online flag

athenaCommonFlags.isOnline = isOnline
athenaCommonFlags.EvtMax.set_Value_and_Lock(evtMax)
athenaCommonFlags.isOnlineStateless = isOnlineStateless

# ----------------------------------------------- Remove DCS problems
from InDetRecExample.InDetJobProperties import InDetFlags
InDetFlags.useBeamConstraint.set_Value_and_Lock(False)

InDetFlags.doRobustReco = False

if athenaCommonFlags.isOnline:
    InDetFlags.useDCS.set_Value_and_Lock(False)                               # fails when true: pixel accessing Temperature info (getting fixed)

# ----------------------------------------------- Special settings for pixel monitoring
logRecExOnline_globalconfig.info("RecExOnline: flag doPixelOnlyMon = %s" %doPixelOnlyMon) 
if doPixelOnlyMon:
    ## force pixel on, turn off the rest of the ID:
    InDetFlags.doTrackSegmentsPixel.set_Value_and_Lock(True)
    InDetFlags.doCTBTrackSegmentsPixel.set_Value_and_Lock(False)
    InDetFlags.doCTBTracking.set_Value_and_Lock(False)
    InDetFlags.doCTBTrackSegmentsSCT.set_Value_and_Lock(False)
    InDetFlags.doCTBTrackSegmentsTRT.set_Value_and_Lock(False)
    InDetFlags.doTrackSegmentsSCT.set_Value_and_Lock(False)
    InDetFlags.doTrtSegments.set_Value_and_Lock(False)

    from AthenaCommon.DetFlags import DetFlags
    DetFlags.SCT_setOff()
    DetFlags.detdescr.SCT_setOn()
    DetFlags.TRT_setOff()
    DetFlags.detdescr.TRT_setOn()
    DetFlags.pixel_setOn()
    DetFlags.detdescr.pixel_setOn()
    DetFlags.detdescr.ID_on()

# ----------------------------------------------- Output flags
from RecExConfig.RecFlags import rec

if (isOnline and useEmon and (partitionName == 'ATLAS' or partitionName == 'TDAQ')):
    rec.projectName.set_Value_and_Lock(is_T0_project_tag)
    #rec.AutoConfiguration = ['FieldAndGeo', 'ConditionsTag', 'BeamType']
    rec.AutoConfiguration = ['FieldAndGeo', 'ConditionsTag', 'BeamType', 'BeamEnergy', 'LumiFlags']

if (isOnline and useEmon and (partitionName != 'ATLAS' and partitionName != 'TDAQ')):
    from AthenaCommon.BFieldFlags import jobproperties
    jobproperties.BField.barrelToroidOn.set_Value_and_Lock(True)
    jobproperties.BField.endcapToroidOn.set_Value_and_Lock(True)
    jobproperties.BField.solenoidOn.set_Value_and_Lock(True)
    from AthenaCommon.BeamFlags import jobproperties
    jobproperties.Beam.beamType.set_Value_and_Lock(beamType)
    globalflags.ConditionsTag.set_Value_and_Lock(ConditionsTag)
    globalflags.DetDescrVersion.set_Value_and_Lock(DetDescrVersion)
    rec.AutoConfiguration = ['FieldAndGeo', 'ConditionsTag', 'BeamType']

if (not isOnline and not useEmon) or (isOnline and REO_doAutoConfiguration):
    rec.AutoConfiguration = ['everything']
    logRecExOnline_globalconfig.info("RecExOnline: running with autoconfiguration = everything") 


if (isOnline and not useEmon):
    from AthenaCommon.BFieldFlags import jobproperties
    jobproperties.BField.barrelToroidOn.set_Value_and_Lock(True)
    jobproperties.BField.endcapToroidOn.set_Value_and_Lock(True)
    jobproperties.BField.solenoidOn.set_Value_and_Lock(True)
    from AthenaCommon.BeamFlags import jobproperties
    jobproperties.Beam.beamType.set_Value_and_Lock(beamType)
    globalflags.ConditionsTag.set_Value_and_Lock(ConditionsTag)
    globalflags.DetDescrVersion.set_Value_and_Lock(DetDescrVersion)
    rec.AutoConfiguration = ['FieldAndGeo', 'ConditionsTag', 'BeamType']
    logRecExOnline_globalconfig.info(" Running with isOnline=True with autoconfiguration = FieldAndGeo, ConditionsTag, BeamType (taken from P1 job)")




rec.doESD.set_Value_and_Lock(doESD)
rec.doAOD.set_Value_and_Lock(doAOD)
rec.doWriteESD.set_Value_and_Lock(writeESD)
rec.doWriteAOD.set_Value_and_Lock(writeAOD)
rec.doCBNT.set_Value_and_Lock(False)
rec.doWriteTAG.set_Value_and_Lock(False)

# Fixing the error message for deprecated doAOD call: "ERROR updating doAOD to value False . WILL SOON NOT WORK ANYMORE ! Please update to filling directly jp.Rec.blah or jp.AthenaCommonFlags.blah "
#from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
#athenaCommonFlags.doAOD.set_Value_and_Lock(doAOD)
#
#from RecExConfig.RecFlags import jobproperties
#jobproperties.Rec.doAOD.set_Value_and_Lock(doAOD)



# ---------------------------------------------- Debug flags
rec.doPerfMon.set_Value_and_Lock(False)                                     # optional for performance check
rec.doDetailedPerfMon.set_Value_and_Lock(False)
rec.doSemiDetailedPerfMon.set_Value_and_Lock(False)
rec.doNameAuditor.set_Value_and_Lock(False)                                 # optional for debugging
rec.doDetStatus.set_Value_and_Lock(False)                                   # fails when true (fixed in next release)

if not 'IOVDbSvcMessage' in dir():
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    import IOVDbSvc.IOVDb
    svcMgr.IOVDbSvc.OutputLevel = 2

#
# get_files LVL1config_Physics_pp_v5.xml
# ln -s /afs/cern.ch/atlas/project/trigger/pesa-sw/validation/atn-test/data15_13TeV.00266904.physics_EnhancedBias.merge.RAW._lb0452._SFO-1._0001.1 input.data
# 

from AthenaCommon.GlobalFlags import globalflags;
globalflags.DataSource.set_Value_and_Lock("data");
DetDescrVersion="ATLAS-R2-2015-03-01-00";
ConditionsTag="CONDBR2-BLKPA-2015-16";
globalflags.DetDescrVersion.set_Value_and_Lock(DetDescrVersion);
globalflags.InputFormat.set_Value_and_Lock("bytestream");
globalflags.ConditionsTag.set_Value_and_Lock(ConditionsTag)
globalflags.DatabaseInstance.set_Value_and_Lock('CONDBR2')
print "GLOBAL"
print globalflags

include ("RecExRecoTest/RecExRecoTest_RTT_common.py")

#jp.AthenaCommonFlags.PoolESDOutput="id_ESD.pool.root"
rec.doTrigger=False
rec.doWriteESD=False # uncomment if do not write ESD
rec.doAOD=False
rec.doWriteAOD=False # uncomment if do not write AOD
rec.doWriteTAG=False # uncomment if do not write TAG
rec.doESD=False

from RecExConfig.RecFlags import rec
rec.doForwardDet=False
rec.doInDet=False
rec.doMuon=False
rec.doCalo=True
rec.doEgamma=False
rec.doMuonCombined=False
rec.doJetMissingETTag=False
rec.doTau=False
rec.doCaloRinger=False
#rec.readRDO=False

print 'DENIS'
print rec
# main jobOption
include ("RecExCommon/RecExCommon_topOptions.py")

include ("RecExRecoTest/RecExRecoTest_RTT_common_postOptions.py")



## @file L1Topo_ReadBS_test.py
## @brief Example job options file to read BS file to test a converter
## $Id: decodeBS.parallel.py 717359 2016-01-12 14:40:21Z bwynne $
###############################################################
#
# This Job option:
# ----------------
# 1. Read ByteStream test data file and decode the L1Topo part
#
#==============================================================

## basic job configuration
import AthenaCommon.AtlasUnixStandardJob
#import AthenaCommon.AtlasThreadedJob

include( "ByteStreamCnvSvc/BSEventStorageEventSelector_jobOptions.py" )

## get a handle on the ServiceManager
from AthenaCommon.AppMgr import ServiceMgr as svcMgr

#Determine whether we're running in threaded mode (threads= >=1)
from AthenaCommon.ConcurrencyFlags import jobproperties as jp
nThreads = jp.ConcurrencyFlags.NumThreads()
print ' nThreads : ',nThreads

if nThreads >= 1:
  ## get a handle on the ForwardScheduler
  from GaudiHive.GaudiHiveConf import ForwardSchedulerSvc
  svcMgr += ForwardSchedulerSvc()
  svcMgr.ForwardSchedulerSvc.CheckDependencies = True

# Use McEventSelector so we can run with AthenaMP
#import AthenaCommon.AtlasUnixGeneratorJob

## get a handle on the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

# older file, has wrong ROD id: svcMgr.ByteStreamInputSvc.FullFileName = [ "/afs/cern.ch/user/m/mzinser/public/l1calo-event.sim"]
#svcMgr.ByteStreamInputSvc.FullFileName = [ "/afs/cern.ch/user/m/mzinser/public/InputSimon/Mode7/l1calo-event.sim" ]
svcMgr.ByteStreamInputSvc.FullFileName = [ "./input.data" ]
#svcMgr.ByteStreamInputSvc.FullFileName = [ "/afs/cern.ch/user/s/sgeorge/atlaspublic/L1TopoCnv/mergedsim._0001.data" ]

svcMgr.ByteStreamInputSvc.ValidateEvent = True
svcMgr.EventSelector.ProcessBadEvent = True



if not hasattr( svcMgr, "ByteStreamAddressProviderSvc" ):
    from ByteStreamCnvSvcBase.ByteStreamCnvSvcBaseConf import ByteStreamAddressProviderSvc 
    svcMgr += ByteStreamAddressProviderSvc()


from TrigConfigSvc.TrigConfigSvcConf import TrigConf__LVL1ConfigSvc
l1svc = TrigConf__LVL1ConfigSvc("LVL1ConfigSvc")
l1svc.XMLMenuFile = "LVL1config_Physics_pp_v5.xml"
svcMgr += l1svc


# This is the list of proxies to set up so that retrieval attempt will trigger the BS conversion
svcMgr.ByteStreamAddressProviderSvc.TypeNames += [
    "ROIB::RoIBResult/RoIBResult" ]

#--------------------------------------------------------------
# Private Application Configuration options
#--------------------------------------------------------------
# Load "user algorithm" top algorithms to be run, and the libraries that house them


if nThreads >= 1:
  #Retrieve input data
  from SGComps.SGCompsConf import SGInputLoader
  topSequence += SGInputLoader( OutputLevel=INFO, ShowEventDump=False )
  topSequence.SGInputLoader.Load = [ ('ROIB::RoIBResult','RoIBResult') ]


#Run calo decoder
from L1Decoder.L1DecoderConf import L1CaloDecoder
caloDecoder = L1CaloDecoder() # by default it is steered towards the RoIBResult of the name above
caloDecoder.OutputLevel=VERBOSE
topSequence += caloDecoder

#Dumper
#from ViewAlgs.ViewAlgsConf import DumpDecisions
#dumper = DumpDecisions("L1CaloDecisions")
#dumper.OutputLevel=VERBOSE
#topSequence += dumper

from RegionSelector.RegSelSvcDefault import RegSelSvcDefault
svcMgr+=RegSelSvcDefault()
svcMgr.RegSelSvc.enableCalo=True
print 'RegSelSvc : '
print svcMgr.RegSelSvc

from TrigT2CaloCommon.TrigT2CaloCommonConfig import TrigDataAccess
svcMgr.ToolSvc+=TrigDataAccess()
svcMgr.ToolSvc.TrigDataAccess.ApplyOffsetCorrection=False

from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import T2CaloEgamma_FastAlgo
topSequence+=T2CaloEgamma_FastAlgo("testFastAlgo")
topSequence.testFastAlgo.OutputLevel=VERBOSE

#--------------------------------------------------------------
# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)
#--------------------------------------------------------------
svcMgr.MessageSvc.OutputLevel = INFO
svcMgr.MessageSvc.Format = "% F%42W%S%7W%R%T %0W%M"
svcMgr.MessageSvc.verboseLimit = 0
svcMgr.StoreGateSvc.OutputLevel = INFO
svcMgr.StoreGateSvc.Dump=False #This is required to avoid a bug in bytestream decoding in AthenaMT mode
svcMgr.ByteStreamAddressProviderSvc.OutputLevel = INFO
svcMgr.ByteStreamCnvSvc.OutputLevel = INFO
svcMgr.ByteStreamInputSvc.OutputLevel = INFO
svcMgr.AthDictLoaderSvc.OutputLevel = INFO
svcMgr.EventPersistencySvc.OutputLevel = INFO
svcMgr.ROBDataProviderSvc.OutputLevel = INFO
print svcMgr



theApp.EvtMax = 100


print "topSequence dump:", topSequence
#
# End of job options file
#
###############################################################


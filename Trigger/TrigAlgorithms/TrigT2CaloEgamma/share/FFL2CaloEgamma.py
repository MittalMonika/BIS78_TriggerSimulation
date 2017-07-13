#
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#


from AthenaCommon.AppMgr import ServiceMgr as svcMgr
from AthenaCommon.AlgScheduler import AlgScheduler
AlgScheduler.CheckDependencies( True )
AlgScheduler.OutputLevel( VERBOSE )
#AlgScheduler.ShowDataDependencies( True )
AlgScheduler.setDataLoaderAlg( 'SGInputLoader' )


from AthenaCommon.GlobalFlags import globalflags;
globalflags.DataSource.set_Value_and_Lock("data");
DetDescrVersion="ATLAS-R2-2015-03-01-00";
ConditionsTag="CONDBR2-BLKPA-2015-16";
globalflags.DetDescrVersion.set_Value_and_Lock(DetDescrVersion);
globalflags.InputFormat.set_Value_and_Lock("bytestream");
globalflags.ConditionsTag.set_Value_and_Lock(ConditionsTag)
globalflags.DatabaseInstance.set_Value_and_Lock('CONDBR2')
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags;
athenaCommonFlags.BSRDOInput = [ "/afs/cern.ch/atlas/project/trigger/pesa-sw/validation/atn-test/data15_13TeV.00266904.physics_EnhancedBias.merge.RAW._lb0452._SFO-1._0001.1" ]
#FilesInput 

#include ("RecExRecoTest/RecExRecoTest_RTT_common.py")

#jp.AthenaCommonFlags.PoolESDOutput="id_ESD.pool.root"
rec.doTrigger=False
rec.doWriteESD=False # uncomment if do not write ESD
rec.doAOD=False
rec.doWriteAOD=False # uncomment if do not write AOD
rec.doWriteTAG=False # uncomment if do not write TAG
rec.doESD=False
rec.doWritexAOD=False

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
rec.doPerfMon=False
#rec.readRDO=False

from PerfMonComps.PerfMonFlags import jobproperties
jobproperties.PerfMonFlags.doDsoMonitoring.set_Value_and_Lock(False)
jobproperties.PerfMonFlags.doMonitoring.set_Value_and_Lock(False)
jobproperties.PerfMonFlags.doPersistencyMonitoring.set_Value_and_Lock(False)

from LArROD.LArRODFlags import larRODFlags
larRODFlags.doLArFebErrorSummary.set_Value_and_Lock(False)
#from AthenaCommon.DetFlags import DetFlags
#DetFlags.makeRIO.Calo_setOff()
#DetFlags.detdescr.Calo_setOn()
#DetFlags.Print()

# main jobOption


#include ("RecExRecoTest/RecExRecoTest_RTT_common_postOptions.py")

## get a handle on the ServiceManager
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from SGComps.SGCompsConf import SGInputLoader
topSequence += SGInputLoader()
AlgScheduler.setDataLoaderAlg("SGInputLoader")
topSequence.SGInputLoader.Load = [ ('ROIB::RoIBResult','StoreGateSvc+RoIBResult') ] #, ('AthenaAttributeList','StoreGateSvc+Input') ]
#topSequence.CondInputLoader.Load = [('AthenaAttributeList','ConditionStore+Input')]

#from IOVSvc.IOVSvcConf import CondInputLoader
#topSequence += CondInputLoader( "CondInputLoader", OutputLevel=DEBUG )

include ("RecExCommon/RecExCommon_topOptions.py")

#remove RecExCommon 
# topSequence = AlgSequence()
# for i in topSequence:
#   log.info('removing '+ i.getName())
#   if i.getName() not in [ "SGInputLoader", "CondInputLoader" ]: # list of what we want to keep
#     topSequence.remove(i)



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
  ## get a handle on the Scheduler
  from AthenaCommon.AlgScheduler import AlgScheduler
  AlgScheduler.CheckDependencies( True )

# Use McEventSelector so we can run with AthenaMP
#import AthenaCommon.AtlasUnixGeneratorJob

## get a handle on the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

#svcMgr.ByteStreamInputSvc.FullFileName = 

svcMgr.ByteStreamInputSvc.ValidateEvent = True
svcMgr.EventSelector.ProcessBadEvent = True



if not hasattr( svcMgr, "ByteStreamAddressProviderSvc" ):
    from ByteStreamCnvSvcBase.ByteStreamCnvSvcBaseConf import ByteStreamAddressProviderSvc 
    svcMgr += ByteStreamAddressProviderSvc()


from TrigConfigSvc.TrigConfigSvcConf import TrigConf__LVL1ConfigSvc
from TrigConfigSvc.TrigConfigSvcConfig import findFileInXMLPATH
from TriggerJobOpts.TriggerFlags import TriggerFlags

l1svc = TrigConf__LVL1ConfigSvc("LVL1ConfigSvc")
l1svc.XMLMenuFile = findFileInXMLPATH(TriggerFlags.inputLVL1configFile())
svcMgr += l1svc


# This is the list of proxies to set up so that retrieval attempt will trigger the BS conversion
svcMgr.ByteStreamAddressProviderSvc.TypeNames += [ "ROIB::RoIBResult/RoIBResult" ]

#--------------------------------------------------------------
# Private Application Configuration options
#--------------------------------------------------------------
# Load "user algorithm" top algorithms to be run, and the libraries that house them



#Run calo decoder
# from L1Decoder.L1DecoderConf import L1CaloDecoder
# caloDecoder = L1CaloDecoder() # by default it is steered towards the RoIBResult of the name above
# caloDecoder.OutputLevel=VERBOSE
# topSequence += caloDecoder


#from AthenaCommon.CFElements import parOR, seqAND
#TopHLTSeq = seqAND("TopHLTSeq")
#topSequence += TopHLTSeq


from L1Decoder.L1DecoderMonitoring import CTPUnpackingMonitoring, RoIsUnpackingMonitoring
from L1Decoder.L1DecoderConf import CTPUnpackingTool, EMRoIsUnpackingTool, L1Decoder, MURoIsUnpackingTool
from L1Decoder.L1DecoderConf import CTPUnpackingEmulationTool, RoIsUnpackingEmulationTool
l1Decoder = L1Decoder( OutputLevel=DEBUG )
ctpUnpacker = CTPUnpackingTool( OutputLevel =  DEBUG, ForceEnableAllChains=True )


l1Decoder.ctpUnpacker = ctpUnpacker
l1Decoder.ctpUnpacker.MonTool = CTPUnpackingMonitoring(512, 200)
l1Decoder.ctpUnpacker.CTPToChainMapping = ["0:HLT_e3",  "0:HLT_g5", "1:HLT_e7", "2:HLT_2e3", "15:HLT_mu6", "33:HLT_2mu6", "15:HLT_mu6idperf", "42:HLT_e15mu4"] # this are real IDs of L1_* items in pp_v5 menu

emUnpacker = EMRoIsUnpackingTool( OutputLevel=DEBUG )
emUnpacker.ThresholdToChainMapping = ["EM3 : HLT_e3", "EM3 : HLT_g5",  "EM7 : HLT_e7", "EM15 : HLT_e15mu4" ]
emUnpacker.MonTool = RoIsUnpackingMonitoring( prefix="EM", maxCount=30 )

topSequence += l1Decoder



#Dumper
#from ViewAlgs.ViewAlgsConf import DumpDecisions
#dumper = DumpDecisions("L1CaloDecisions")
#dumper.OutputLevel=VERBOSE
#topSequence += dumper

from RegionSelector.RegSelSvcDefault import RegSelSvcDefault
svcMgr+=RegSelSvcDefault()
svcMgr.RegSelSvc.enableCalo=True

from TrigT2CaloCommon.TrigT2CaloCommonConfig import TrigDataAccess
svcMgr.ToolSvc+=TrigDataAccess()
svcMgr.ToolSvc.TrigDataAccess.ApplyOffsetCorrection=False

from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import T2CaloEgamma_FastAlgo
algo=T2CaloEgamma_FastAlgo("testFastAlgo")
algo.RoIs="EMRoIs"
algo.OutputLevel=VERBOSE
#TopHLTSeq += algo
topSequence += algo

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

from LArROD.LArRODConf import LArRawChannelBuilderDriver
topSequence.remove(LArRawChannelBuilderDriver('LArRawChannelBuilder'))
from LArROD.LArDigits import DefaultLArDigitThinner
topSequence.remove(DefaultLArDigitThinner('LArDigitThinner'))
from TileRecUtils.TileRecUtilsConf import TileRawChannelMaker
topSequence.remove(TileRawChannelMaker('TileRChMaker'))
from TileRecAlgs.TileRecAlgsConf import TileDigitsFilter
topSequence.remove(TileDigitsFilter('TileDigitsFilter'))
from CaloRec.CaloRecConf import CaloCellMaker 
topSequence.remove(CaloCellMaker('CaloCellMaker'))
from xAODCaloEventCnv.xAODCaloEventCnvConf import ClusterCreator
topSequence.remove(ClusterCreator('CaloCluster2xAOD'))


theApp.EvtMax = 100

#print topSequence
#print fail


#
# End of job options file
#
###############################################################



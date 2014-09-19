
## -- Overview of all default local settings that one can change 
## -- The default values are also shown.

## ------------------------------------------- flags set in: RecExOnline_jobOptions.py  
isOnline          = False
isOnlineStateless = False

## ------------------------------------------- flags set in: RecExOnline_monitoring.py  
isGlobalMonitoring = False

#eMon can only be used ONLINE (please see RecExOnline_Partition_Online.py)
useEmon           = False
useAtlantisEmon   = False
evtMax            = 10 #100

## ------------------------------------------- flags set in: RecExOnline_globalconfig.py  (from RecExOnline_jobOptions.py)
#read the pickle file if you want to use the AMI tag info
#stored in ami_recotrf.pickle (produced by 'tct_getAmiTag.py f140 ami_recotrf.cmdargs ami_recotrf.pickle') 
usePickleConfig   = False
pickleconfigfile  = './ami_recotrf.pickle'
DataSource        = 'data'
InputFormat       = 'bytestream'
#fileName          = '/afs/cern.ch/user/k/koutsman/OnlineNightly/data/data12_8TeV.00208931.express_express.daq.RAW._lb0123._SFO-9._0001.data'
fileName          = 'root://eosatlas//eos/atlas/atlascerngroupdisk/proj-sit/tct/rec_input/00204416/express_express/data12_8TeV.00204416.express_express.merge.RAW._lb0015._SFO-ALL._0001.1'

doESD             = True
writeESD          = False
doAOD             = False
writeAOD          = False
IOVDbSvcMessage   = False

## ------------------------------------------ flags set in: RecExOnline_recoflags.py (from RecExOnline_jobOptions.py)
doAllReco   = True
doInDet     = doAllReco
doMuon      = doAllReco
doLArg      = doAllReco
doTile      = doAllReco
doTrigger   = False
doHist      = doAllReco
doJiveXML   = False
doEgammaTau = doAllReco

#set to True in the JO
#doCommissionig = False

## ------------------------------------------ flags set in : RecExOnline_monitoring.py (from from RecExOnline_jobOptions.py)
doAllMon  = True
doCaloMon = doAllMon
doPhysMon = doAllMon
doTrigMon = False
doIDMon   = doAllMon
doTRTMon  = doAllMon
doMuonMon = doAllMon

## ------------------------------------------ flags set in : RecExOnline_postconfig.py    (called from RecExOnline_jobOptions.py)

## main online reco scripts
include ("RecExOnline/RecExOnline_jobOptions.py")


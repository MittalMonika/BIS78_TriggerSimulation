## this will triogger IOV to retriev CaliWave
RunNumber = '00110555'
IOVBegin = 110555
Partition = "EB-EMBA"
SubDet = 'Barrel'
GainList = [ "MEDIUM" ]

#############################################################################################
## this cool channel selection is using to Read only BarrelPS Medium gain Delay from the DB
ChannelSelection='<channelSelection>385:416,1013</channelSelection>'
#############################################################################################


## where you want to put the outputfile
#OutputParamsRootFileDir= ""
#OutputPulseParamsPoolFileDir= ""
#OutputDetCellParamsPoolFileDir= ""

## DB folders for Parameters
## extracted using RTM methode :
InputCaliPulseParamsFolder     = "/LAR/ElecCalibOfl/CaliPulseParams/RTM"
InputDetCellParamsFolder       = "/LAR/ElecCalibOfl/DetCellParams/RTM"  

WriteNtuple = True

###################################
#     Extract All Parameters      #
###################################
ExtractAllParameters = True

###################################
#      Extract only tauR          #
###################################
#ExtractTauROnly = True

#########################
# to read official DB
#########################
# by default, 
# nothing to do it will pickup the GlobalTag


#######################################
# to read COOL in a local SQlite file 
#######################################


## CaliWave
#######################
## local SQlite file
#InputCaliWaveSQLiteFile = "myDB200.db"
# the folder tag
#CaliWaveLArCalibFolderTag = "-UPD3-00"
################################################################################
# >>>>>>>>>> you NEED to have your POOL files inserter in the  PoolFileCatalog
################################################################################


## CaliPulseParams
########################
## local SQlite file
#InputCaliPulseParamsSQLiteFile = "myDB200.db"
# folder tag
#CaliPulseParamsLArCalibFolderTag = "-UPD3-00"
################################################################################
# >>>>>>>>>> you NEED to have your POOL files inserter in the  PoolFileCatalog
################################################################################

## DetCellParams
########################
#InputDetCellParamsSQLiteFile = "myDB200.db"
# the folder tag
#DetCellParamsLArCalibFolderTag = "-UPD3-00"
################################################################################
# >>>>>>>>>> you NEED to have your POOL files inserter in the  PoolFileCatalog
################################################################################

## Bad Channel
#######################
#InputBadChannelSQLiteFile = "myDB200.db"
# the folder tag
#BadChannelsLArCalibFolderTag = "-UPD3-00"
#MissingFEBsLArCalibFolderTag = "-empty"

#######################################

include ( "LArCalibProcessing/LArCalib_RTMParams_jobOptions.py" )

#######################################


## LArRTMParams
LArRTMParamExtractor.OutputLevel = INFO
theLArWFParamTool.OutputLevel = INFO
LArWFParams2Ntuple.OutputLevel = INFO

svcMgr.IOVDbSvc.OutputLevel = DEBUG    

#########################
#        options
#########################
#------------
# Message
#------------
#svcMgr.MessageSvc.defaultLimit = 9999999  # all messages
#svcMgr.MessageSvc.OutputLevel = VERBOSE

#from StoreGate.StoreGateConf import StoreGateSvc
#----------------
# Dump StoreGate
#----------------
#sgStore = StoreGateSvc("StoreGateSvc")
#sgStore.Dump = True
#sgStore.OutputLevel = DEBUG

#--------------------
# Dump DetectorStore
#--------------------
#dtStore = StoreGateSvc("DetectorStore")
#dtStore.Dump = True
#dtStore.OutputLevel = DEBUG

#----------------------
# Dump ConditionStore
#----------------------
#cdStore = StoreGateSvc("ConditionStore")
#cdStore.Dump = True
#cdStore.OutputLevel = DEBUG

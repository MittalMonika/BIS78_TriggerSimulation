from GaudiSvc.GaudiSvcConf import THistSvc

# add LumiBlockMetaDataTool to ToolSvc and configure
from LumiBlockComps.LumiBlockCompsConf import LumiBlockMetaDataTool
ToolSvc += LumiBlockMetaDataTool( "LumiBlockMetaDataTool" )
LumiBlockMetaDataTool.calcLumi = True # False by default

# add ToolSvc.LumiBlockMetaDataTool to MetaDataSvc
from AthenaServices.AthenaServicesConf import MetaDataSvc
svcMgr += MetaDataSvc( "MetaDataSvc" )
svcMgr.MetaDataSvc.MetaDataTools += [ ToolSvc.LumiBlockMetaDataTool ]

# Configure the goodrunslist selector tool
from GoodRunsLists.GoodRunsListsConf import *
ToolSvc += GoodRunsListSelectorTool()

GoodRunsListSelectorTool.GoodRunsListVec = [ '$TestArea/InnerDetector/InDetMonitoring/InDetPerformanceMonitoring/share/data15_13TeV.periodAllYear_DetStatus-v69-pro19-03_DQDefects-00-01-02_PHYS_StandardGRL_All_Good.xml' ]
#GoodRunsListSelectorTool.GoodRunsListVec = [ 'data15_13TeV.periodAllYear_DetStatus-v65-pro19-01_DQDefects-00-01-02_PHYS_StandardGRL_All_Good.xml' ]

#GoodRunsListSelectorTool.GoodRunsListVec = [ 'data15_13TeV.periodAllYear_DetStatus-v63-pro18-01_DQDefects-00-01-02_PHYS_StandardGRL_All_Good.xml' ]
#GoodRunsListSelectorTool.OutputLevel = VERBOSE

## This Athena job consists of algorithms that loop over events;
## here, the (default) top sequence is used:
from AthenaCommon.AlgSequence import AlgSequence, AthSequencer
job = AlgSequence()
seq = AthSequencer("AthMasterSeq")

## AthMasterSeq is always executed before the top sequence, and is configured such that
## any follow-up sequence (eg. top sequence) is not executed in case GRLTriggerAlg1 does
## not pass the event
## In short, the sequence AthMasterSeq makes sure that all algs in the job sequence
## are skipped when an event gets rejects
from GoodRunsListsUser.GoodRunsListsUserConf import *
seq += GRLTriggerSelectorAlg('GRLTriggerAlg1')
seq.GRLTriggerAlg1.GoodRunsListArray = ['PHYS_StandardGRL_All_Good']        ## pick up correct name from inside xml file!
#seq.GRLTriggerAlg1.TriggerSelectionRegistration = 'L1_MBTS_1' ## set this to your favorite trigger, eg. L1_MBTS_1_1

## Add the ntuple dumper to the top sequence, as usual
## DummyDumperAlg1 is run in the top sequence, but is not executed in case GRLTriggerAlg1 rejects the event.
job += DummyDumperAlg('DummyDumperAlg1')
# job.DummyDumperAlg1.RootFileName = 'selection1.root'
ServiceMgr += THistSvc()
ServiceMgr.THistSvc.Output = ["new DATAFILE='selection1.root' TYP='ROOT' OPT='RECREATE'"];
job.DummyDumperAlg1.GRLNameVec = [ 'LumiBlocks_GoodDQ0', 'IncompleteLumiBlocks_GoodDQ0' ]

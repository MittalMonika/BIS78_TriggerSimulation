
#
# Properties that should be modified by the user:
#
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
if not "InputFiles" in dir():
    #InputFiles = [ "ESD.pool.root" ]
    #InputFiles = [ "/afs/cern.ch/atlas/project/trigger/pesa-sw/validation/validation-data/attila.AOD.pool.root" ]
    InputFiles = ["/afs/cern.ch/user/r/rwhite/workspace/public/tutorial/mc15_13TeV.361106.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zee.merge.AOD.e3601_s2876_r7917_r7676/AOD.08275487._000742.pool.root.1"]
if not "OutputFile" in dir():
    OutputFile = "TDTExample.root"
if not "athenaCommonFlags.EvtMax" in dir():
    athenaCommonFlags.EvtMax = 20

#if not already done, you should determine if you are running on MC or Data:
#from PyUtils import AthFile
#af = AthFile.fopen(svcMgr.EventSelector.InputCollections[0]) 
#isMC = 'IS_SIMULATION' in af.fileinfos['evt_type']
isMC = True
#must set the globalflags appropriately so that the correct conditions database is queried when you extract the bunch train
from AthenaCommon.GlobalFlags import globalflags
globalflags.DataSource = 'geant4' if isMC else 'data' 
globalflags.DatabaseInstance = 'CONDBR2' 

# Set up the needed RecEx flags:
athenaCommonFlags.FilesInput.set_Value_and_Lock( InputFiles )

# Set up what the RecExCommon job should and shouldn't do:
from RecExConfig.RecFlags import rec
rec.AutoConfiguration = [ "everything" ]
rec.doCBNT.set_Value_and_Lock( False )
rec.doWriteESD.set_Value_and_Lock( False )
rec.doWriteAOD.set_Value_and_Lock( False )
rec.doWriteTAG.set_Value_and_Lock( False )
rec.doESD.set_Value_and_Lock( False )
rec.doAOD.set_Value_and_Lock( False )
rec.doDPD.set_Value_and_Lock( False )
rec.doHist.set_Value_and_Lock( False )
rec.doPerfMon.set_Value_and_Lock( False )
rec.doForwardDet.set_Value_and_Lock( False )

# Let RecExCommon set everything up:
include( "RecExCommon/RecExCommon_topOptions.py" )

# Correctly configure the trigger via TriggerJobOpts
# The following will configure the TDT
from TriggerJobOpts.TriggerFlags import TriggerFlags
TriggerFlags.configurationSourceList.set_Value_and_Lock( [ "ds" ] )
    
from TriggerJobOpts.TriggerConfigGetter import TriggerConfigGetter
TriggerConfigGetter()
#########################################################################
#                                                                       #
#                      Now set up the example job                       #
#                                                                       #
#########################################################################
#
# Configure an instance of TrigDecisionTool:
#
#from TrigDecisionTool.TrigDecisionToolConf import Trig__TrigDecisionTool
#ToolSvc += Trig__TrigDecisionTool( "TrigDecisionTool" )
#ToolSvc.TrigDecisionTool.OutputLevel=ERROR
#ToolSvc.TrigDecisionTool.Navigation.OutputLevel = ERROR

#if "xAODConfig" in dir():
#    from TrigEDMConfig.TriggerEDM import EDMLibraries
#    ToolSvc.TrigDecisionTool.Navigation.Dlls = [e for e in  EDMLibraries if 'TPCnv' not in e]
#else:
#    from TrigEDMConfig.TriggerEDM import EDMLibraries
#    ToolSvc.TrigDecisionTool.Navigation.Dlls = EDMLibraries

ToolSvc += CfgMgr.Trig__TriggerAnalysisHelper("TriggerAnalysisHelper")
ToolSvc += CfgMgr.Trig__MatchingTool("MatchingTool",OutputLevel=DEBUG)
#from TrigBunchCrossingTool.BunchCrossingTool import BunchCrossingTool
#if isMC: bcTool = BunchCrossingTool( "MC" )
#else: bcTool = BunchCrossingTool( "LHC" )
#ToolSvc += bcTool
#
# Example Code
#
from TrigAnalysisExamples.TrigAnalysisExamplesConf import Trig__TDTExample
# the tutorial
from TrigAnalysisExamples.TrigAnalysisExamplesConf import TriggerAnalysisTutorial
elHLTList=["HLT_e26_lhtight_nod0_ivarloose","HLT_e26_lhtight_nod0_iloose","HLT_e17_lhloose_nod0"] 
stl = elHLTList
tat = TriggerAnalysisTutorial(StatTriggerChains=stl,ElectronTriggerList=elHLTList)


# Histogram routing
ServiceMgr += CfgMgr.THistSvc()
ServiceMgr.THistSvc.Output += ["file DATAFILE='TriggerAnalysisTutorial.root' TYP='ROOT' OPT='RECREATE'"]
ServiceMgr.THistSvc.Output += ["Trigger DATAFILE='TriggerAnalysisTutorial.root' TYP='ROOT' OPT='RECREATE'"]

#from TrigAnalysisExamples.TrigAnalysisExamplesConf import TagAndProbeExAlg
ex1=Trig__TDTExample( "TDTExample", TriggerList=["HLT_e26_lhtight_nod0_ivarloose","HLT_e26_lhtight_nod0_iloose","HLT_e17_lhloose_nod0"] )
#ex2=TagAndProbeExAlg( "TagAndProbeExAlg", TriggerList=["HLT_mu24_iloose","HLT_e26_lhtight_nod0_ivarloose"])
print ex1
#print ex2
#from TrigAnalysisExamples.TDTExample import TriggerTree
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()
topSequence += ex1
topSequence += tat
#topSequence += ex2
#topSequence += TriggerTree( "trigger" )

#
# Add Histograms
#
ServiceMgr += CfgMgr.THistSvc()
ServiceMgr.THistSvc.Output += [ "turnontree DATAFILE='" + OutputFile +
                                "' TYP='ROOT' OPT='RECREATE'" ]
ServiceMgr.THistSvc.OutputLevel = ERROR

# This is the configuration file to run ATLAS Digitization
# Use the following switches for:
#   athenaCommonFlags.EvtMax = <NEvents>         Number of events to digitize (set to -1 for all)
#   athenaCommonFlags.SkipEvents = <NSkip>       Number of events from input file collection to skip
#   athenaCommonFlags.PoolHitsInput=<FileName>   Input collections. The list of files with the hits to digitize
#   athenaCommonFlags.PoolRDOOutput=<FileName>   Output file name

#--------------------------------------------------------------
# AthenaCommon configuration
#--------------------------------------------------------------


import glob
import os

if 'customInput' not in locals() or 'customInput' not in globals():
    print("customInput not defined yet setting the default as input.rdo.pool.root")
    customInput='input.rdo.pool.root'

if(not os.path.isdir(customInput) and not os.path.isfile(customInput) ):
    checklist=glob.glob(customInput)
    if len(checklist)==0:
        print("Invalid INPUT : "+customInput)
        os.sys.exit()
if(os.path.isdir(customInput)):
    customInput+="/*.root"

MessageSvc.defaultLimit=100
MessageSvc.useColors = True
MessageSvc.Format = "% F%30W%S%7W%R%T %0W%M"


from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.AllowIgnoreConfigError=False #This job will stop if an include fails.
from AthenaCommon.GlobalFlags import globalflags
globalflags.ConditionsTag.set_Value_and_Lock("OFLCOND-RUN12-SDR-25")
globalflags.DetDescrVersion.set_Value_and_Lock("ATLAS-R3-2021-00-00-00")

from RecExConfig.RecFlags import rec as recFlags 
recFlags.doNameAuditor = True

globalflags.InputFormat.set_Value_and_Lock('pool')

athenaCommonFlags.EvtMax = -1
athenaCommonFlags.SkipEvents = 0




import AthenaPoolCnvSvc.ReadAthenaPool


svcMgr.EventSelector.InputCollections=glob.glob(customInput)


#svcMgr.EventSelector.InputCollections = [ "input.rdo.pool.root" ]

from AthenaCommon.DetFlags import DetFlags

#with tgc setOn we get sagfault and crash
#DetFlags.ID_setOff()
#DetFlags.Calo_setOff()
#DetFlags.Muon_setOff()
#DetFlags.MDT_setOff()
#DetFlags.CSC_setOff()
#DetFlags.TGC_setOff()
#DetFlags.RPC_setOff()
DetFlags.sTGC_setOn()
DetFlags.Micromegas_setOff()

#DetFlags.digitize.MDT_setOff() 
#DetFlags.digitize.TGC_setOff() 
#DetFlags.digitize.RPC_setOff() 
#DetFlags.digitize.CSC_setOff() 
#DetFlags.digitize.Micromegas_setOff() 
DetFlags.digitize.sTGC_setOn() 
#DetFlags.Truth_setOff()

# initialize GeoModel with layout set in globalflags.DetDescrVersion
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit

from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
GeoModelSvc = GeoModelSvc()
GeoModelSvc.MuonVersionOverride = "MuonSpectrometer-R.09.00.NSW"
from MuonGeoModel.MuonGeoModelConf import MuonDetectorTool
MuonDetectorTool=MuonDetectorTool(UseCSC=False)
DetDescrCnvSvc = Service( "DetDescrCnvSvc" )
DetDescrCnvSvc.UseCSC = False




# get AGDD service
from AGDD2GeoSvc.AGDD2GeoSvcConf import AGDDtoGeoSvc
Agdd2GeoSvc = AGDDtoGeoSvc()
from AthenaCommon import CfgGetter

# get AGDD tool for inert material
ToolSvc += CfgGetter.getPublicTool("MuonSpectrometer", checkType=True)
Agdd2GeoSvc.Builders += ["MuonAGDDTool/MuonSpectrometer"]

#get AGDD tool for NSW geometry
ToolSvc += CfgGetter.getPublicTool("NewSmallWheel", checkType=True)
Agdd2GeoSvc.Builders += ["NSWAGDDTool/NewSmallWheel"]

theApp.CreateSvc += ["AGDDtoGeoSvc"]
ServiceMgr += Agdd2GeoSvc

#-----------------------------------------------------------------------------
# Algorithms:  NSW L1 simulation
#-----------------------------------------------------------------------------
from RegionSelector.RegSelSvcDefault import RegSelSvcDefault
from AthenaCommon.AppMgr import ServiceMgr
theRegSelSvc = RegSelSvcDefault()
ServiceMgr += theRegSelSvc



include ('TrigT1NSW/TrigT1NSW_jobOptions.py')

#Switch on and off trigger simulaton components sTGC / MicroMegas
topSequence.NSWL1Simulation.DosTGC=True
topSequence.NSWL1Simulation.DoMM=False
topSequence.NSWL1Simulation.DoPadTrigger=True
topSequence.NSWL1Simulation.StripSegmentTool.rIndexScheme=0
topSequence.NSWL1Simulation.StripSegmentTool.NSWTrigRDOContainerName="NSWTRGRDO"
# Simulation parameters
#topSequence.NSWL1Simulation.PadTdsTool.VMM_DeadTime=3
#topSequence.NSWL1Simulation.PadTdsTool.ApplyVMM_DeatTime=True



#Toggle Ntuple making 
topSequence.NSWL1Simulation.DoNtuple=True
topSequence.NSWL1Simulation.PadTdsTool.DoNtuple=False
topSequence.NSWL1Simulation.PadTriggerTool.DoNtuple=True
topSequence.NSWL1Simulation.StripTdsTool.DoNtuple=False
topSequence.NSWL1Simulation.StripClusterTool.DoNtuple=False
topSequence.NSWL1Simulation.StripSegmentTool.DoNtuple=False


#Tools' Messaging Levels
topSequence.NSWL1Simulation.OutputLevel=DEBUG
topSequence.NSWL1Simulation.PadTdsTool.OutputLevel=DEBUG
topSequence.NSWL1Simulation.PadTriggerTool.OutputLevel=DEBUG
topSequence.NSWL1Simulation.StripTdsTool.OutputLevel=DEBUG
topSequence.NSWL1Simulation.StripClusterTool.OutputLevel=DEBUG
topSequence.NSWL1Simulation.StripSegmentTool.OutputLevel=DEBUG



#-----------------------------------------------------------------------------
# save ROOT histograms and Tuple
#-----------------------------------------------------------------------------

if topSequence.NSWL1Simulation.DoNtuple:

    if not hasattr( ServiceMgr, "THistSvc" ):
        from GaudiSvc.GaudiSvcConf import THistSvc
        ServiceMgr += THistSvc()
    ServiceMgr.THistSvc.Output = ["EXPERT DATAFILE='Monitoring.root' OPT='RECREATE'"];

    if not hasattr( theApp.Dlls, "RootHistCnv" ):
        theApp.Dlls += [ "RootHistCnv" ]
        theApp.HistogramPersistency = "ROOT"

    if not hasattr( ServiceMgr, "NTupleSvc" ):
        from GaudiSvc.GaudiSvcConf import NTupleSvc
        ServiceMgr += NTupleSvc()

    ServiceMgr.THistSvc.Output += [ "NSWL1Simulation DATAFILE='NSWL1Simulation.root'  OPT='RECREATE'" ]

    print ServiceMgr

else:#to avoid any possible crash. If DoNtuple is set to true for a tool but false for NSWL1Simulation the code will crash
     # ideally that should have been already handled in C++ side
    topSequence.NSWL1Simulation.PadTdsTool.DoNtuple=False
    topSequence.NSWL1Simulation.PadTriggerTool.DoNtuple=False
    topSequence.NSWL1Simulation.StripTdsTool.DoNtuple=False
    topSequence.NSWL1Simulation.StripClusterTool.DoNtuple=False
    topSequence.NSWL1Simulation.StripSegmentTool.DoNtuple=False

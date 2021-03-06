from AthenaCommon.Constants import *
from AthenaCommon.AppMgr import theApp
from AthenaCommon.AppMgr import ServiceMgr
import AthenaPoolCnvSvc.ReadAthenaPool

ServiceMgr.AuditorSvc.Auditors += ["ChronoAuditor"]

AthenaPoolCnvSvc = Service("AthenaPoolCnvSvc")
AthenaPoolCnvSvc.UseDetailChronoStat = TRUE

from PartPropSvc.PartPropSvcConf import PartPropSvc

include("ParticleBuilderOptions/McAOD_PoolCnv_jobOptions.py")
include("EventAthenaPool/EventAthenaPool_joboptions.py")

#----------------------------
# Input Dataset
#----------------------------
import os
from glob import glob
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.FilesInput = glob("/tmp/" + os.environ['USER'] + "/" + "RDO*.root*")
#athenaCommonFlags.FilesInput = ['/afs/cern.ch/work/c/ctreado/RDOAnalysis/datasets/mc15_13TeV.110401.PowhegPythia_P2012_ttbar_nonallhad.recon.RDO.e4032_s2608_s2183_r6790_tid05706569_00/RDO.05706569._000011.pool.root.1']
ServiceMgr.EventSelector.InputCollections = athenaCommonFlags.FilesInput()

theApp.EvtMax = -1
#theApp.EvtMax = 100

#----------------------------
# Message Service
#----------------------------
# set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)
ServiceMgr.MessageSvc.OutputLevel = INFO
ServiceMgr.MessageSvc.defaultLimit = 9999999

#----------------------------
# Algorithms
#----------------------------
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()
from RDOAnalysis.RDOAnalysisConf import TRT_FastRDOAnalysis, SCT_FastRDOAnalysis, PixelFastRDOAnalysis
topSequence += TRT_FastRDOAnalysis(), SCT_FastRDOAnalysis(), PixelFastRDOAnalysis()

TRT_FastRDOAnalysis = TRT_FastRDOAnalysis()
TRT_FastRDOAnalysis.NtupleFileName = '/TRT_FastRDOAnalysis/'
TRT_FastRDOAnalysis.HistPath = '/TRT_FastRDOAnalysis/'

SCT_FastRDOAnalysis = SCT_FastRDOAnalysis()
SCT_FastRDOAnalysis.NtupleFileName = '/SCT_FastRDOAnalysis/'
SCT_FastRDOAnalysis.HistPath = '/SCT_FastRDOAnalysis/'

PixelFastRDOAnalysis = PixelFastRDOAnalysis()
PixelFastRDOAnalysis.NtupleFileName = '/PixelFastRDOAnalysis/'
PixelFastRDOAnalysis.HistPath = '/PixelFastRDOAnalysis/'

#----------------------------
# Histogram and Tree Service
#----------------------------
from GaudiSvc.GaudiSvcConf import THistSvc
ServiceMgr += THistSvc()
ServiceMgr.THistSvc.Output += ["TRT_FastRDOAnalysis DATAFILE='TRT_FastRDOAnalysis.root' OPT='RECREATE'"]
ServiceMgr.THistSvc.Output += ["SCT_FastRDOAnalysis DATAFILE='SCT_FastRDOAnalysis.root' OPT='RECREATE'"]
ServiceMgr.THistSvc.Output += ["PixelFastRDOAnalysis DATAFILE='PixelFastRDOAnalysis.root' OPT='RECREATE'"]


#---------------------------
# Detector geometry
#---------------------------
from RecExConfig.AutoConfiguration import *
ConfigureFieldAndGeo()
include("RecExCond/AllDet_detDescr.py")

import MuonCnvExample.MuonCablingConfig





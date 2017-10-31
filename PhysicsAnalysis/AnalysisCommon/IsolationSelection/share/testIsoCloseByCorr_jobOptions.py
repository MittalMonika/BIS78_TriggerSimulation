# Input file

# n events ( use -1 to process all events )
from GaudiSvc.GaudiSvcConf import THistSvc
from AthenaCommon.JobProperties import jobproperties
import AthenaPoolCnvSvc.ReadAthenaPool
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags as acf
from AthenaServices.AthenaServicesConf import AthenaEventLoopMgr
from AthenaCommon.AppMgr import ServiceMgr
from PathResolver import PathResolver
import AthenaPoolCnvSvc.ReadAthenaPool

ServiceMgr += AthenaEventLoopMgr(EventPrintoutInterval = 5000)
ServiceMgr += THistSvc()
OutFileName = "AnalysisOutput.root" if not "outFile" in globals() else outFile
ServiceMgr.THistSvc.Output += ["ISOCORRECTION DATAFILE='{}' OPT='RECREATE'".format(OutFileName)]
ROOTFiles = []

#Cmd to execute the tester:
# athena.py -c "inputFile='/ptmp/mpp/junggjo9/Datasets/mc16_13TeV.344235.PowhegPy8EG_NNPDF30_AZNLOCTEQ6L1_VBFH125.DAOD.e5500_e5984_s3126_r9781_r9778_21.2_r05_EXT0/user.schaffer/user.schaffer.12283539.EXT0._000004.DAOD_HIGG2D1.pool.root'" IsolationSelection/testIsoCloseByCorr_jobOptions.py
#








if "inputFile" in globals():
    print "Use the following %s as input" % (inputFile)
    ROOTFiles = []
    ResolvedInFile = PathResolver.FindCalibFile(inputFile)
    ReolvedInDir = PathResolver.FindCalibDirectory(inputFile)
                                         
    if os.path.isdir(ReolvedInDir):
      for DirEnt in os.listdir(ReolvedInDir):
          if DirEnt.endswith(".root"):
              ROOTFiles.append(DirEnt)
    else: ROOTFiles.append(ResolvedInFile)
    if len(ROOTFiles)==0: raise RuntimeError("No ROOT files could be loaded as input")
    ServiceMgr.EventSelector.InputCollections = ROOTFiles
    acf.FilesInput = ROOTFiles


if "nevents" in globals():
    print "Only run on %i events"%( int(nevents))
    theApp.EvtMax = int (nevents)
## Configure an isolation selection tool with your desired working points
ToolSvc += CfgMgr.CP__IsolationSelectionTool("MySelectionTool", MuonWP = "FixedCutLoose", ElectronWP = "Loose", PhotonWP = "FixedCutTightCaloOnly")


## Configure CorrectionTool, feeding it our selection tool
ToolSvc += CfgMgr.CP__IsolationCloseByCorrectionTool("IsolationCloseByCorrectionTool",
                                                     IsolationSelectionTool=ToolSvc.MySelectionTool,
                                                     SelectionDecorator = "isCloseByObject",
#                                                      PassOverlapDecorator = "passOR",
                                                      IsolationSelectionDecorator = "correctedIsol" ,
                                                      BackupPrefix = "default")


## Test algorithm

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()
from IsolationSelection.IsolationSelectionConf import CP__TestIsolationCloseByCorrAthenaAlg 
job += CfgMgr.CP__TestIsolationCloseByCorrAthenaAlg("TestAlg",IsoSelectorTool = ToolSvc.MySelectionTool, 
                                                       IsoCloseByCorrTool=ToolSvc.IsolationCloseByCorrectionTool)


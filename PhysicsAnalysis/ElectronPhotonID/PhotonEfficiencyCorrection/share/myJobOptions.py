#specify how many events it should process. Set it equal to -1 to process all events
theApp.EvtMax = 100

#set input files - svcMgr has a property that lets you specify the input files, as a list:
import AthenaPoolCnvSvc.ReadAthenaPool
svcMgr.EventSelector.InputCollections = [ " /afs/cern.ch/atlas/project/PAT/xAODs/r6630/mc15_13TeV.361106.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zee.recon.AOD.e3601_s2576_s2132_r6630_tid05358812_00/AOD.05358812._000010.pool.root.1" ]

#One of the existing master sequences where one should attach all algorithms
algSeq = CfgMgr.AthSequencer("AthAlgSeq")

# Add the test algorithm. the alg is a part of the tool, so it should be imported:
from PhotonEfficiencyCorrection.PhotonEfficiencyCorrectionConf import testAthenaPhotonAlg
alg = testAthenaPhotonAlg()

#define input files, please note that the current recomendation of PhotonID WG can be found here: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PhotonEfficiencyRun2

#Set Properties for photonID_SF tool - setup the map.txt file
alg.PhotonEfficiencyCorrectionTool.MapFilePath = "PhotonEfficiencyCorrection/2015_2016/rel20.7/Moriond2017_v1/map2.txt";

#Set DataType: for data use 0 (or don't run the tool - faster), for FULLSIM use 1, and for FASTSIM use 3, please note that the input files are also should be different
alg.PhotonEfficiencyCorrectionTool.ForceDataType = 1


#comment this line
alg.OutputLevel = DEBUG
algSeq += alg

from OutputStreamAthenaPool.MultipleStreamManager import MSMgr
outStream = MSMgr.NewPoolRootStream( "MyXAODStream", "myXAOD.pool.root" )
outStream.AddItem(['xAOD::PhotonContainer_v1#MyPhotons'])
outStream.AddItem(['xAOD::PhotonAuxContainer_v1#MyPhotonsAux'])

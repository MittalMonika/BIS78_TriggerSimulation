from AthenaCommon.AppMgr                    import theApp
from AthenaCommon.AppMgr                    import ServiceMgr as svcMgr
from AthenaCommon.AlgSequence               import AlgSequence
from GaudiSvc.GaudiSvcConf                  import THistSvc

import AthenaRootComps.ReadAthenaxAODHybrid

# ART File
testFile = '/afs/cern.ch/work/m/mscodegg/public/QualiTask/athena/Reconstruction/egamma/egammaValidation/test/ART_Baseline_Checker/rootART/ART_Rec.root'

svcMgr.EventSelector.InputCollections = [testFile]

job = CfgMgr.AthSequencer("AthAlgSeq")
job += CfgMgr.ArtTest()

theApp.EvtMax = 2000

print ( "\n\nALL OK\n\n" )

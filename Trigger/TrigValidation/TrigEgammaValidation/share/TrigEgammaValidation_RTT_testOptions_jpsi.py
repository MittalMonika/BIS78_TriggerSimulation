WhichInput = "RDO"
RunOffline = False

DetDescrVersion = "ATLAS-CSC-02-01-00"
EvtMax = -1
OutputLevel = INFO

include("TrigEgammaValidation/generalJobOption.py")

theApp.serviceMgr().IOVDbSvc.GlobalTag="OFLCOND-CSC-00-01-00"

# Now load Validation algorithm:
from TrigEgammaValidation.TrigEgammaValidationConf import JpsiValidation
myJpsiValidation = JpsiValidation()
topSequence += myJpsiValidation
#myJpsiValidation.OutputLevel = DEBUG

# setup TTree registration Service
# save ROOT histograms and Tuple
from GaudiSvc.GaudiSvcConf import THistSvc
svcMgr += THistSvc()
tHistSvc = svcMgr.THistSvc
tHistSvc.Output += [ "Validation DATAFILE='Validation_jpsi.root' OPT='RECREATE'" ]
theApp.HistogramPersistency = "ROOT"

# To Dump Truth:
#theApp.Dlls  += [ "TruthExamples" ]   
#theApp.TopAlg += ["DumpMC"]
#Algorithm("DumpMC").McEventKey = "TruthEvent"
#Algorithm("DumpMC").McEventOutputKey = "TruthEvent"

#svcMgr.StoreGateSvc.Dump = True

DetFlags.Print()

print theApp.TopAlg
print theApp.Dlls

import os;
f = file('file.txt', 'w');
f.write('Validation_jpsi.root\n');
f.write('/afs/cern.ch/user/d/dferreir/public/references_new_dataset/TrigEgammaValidation_RTT_testOptions_jpsi/reference.root\n');
f.close();


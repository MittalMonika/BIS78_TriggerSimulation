from G4AtlasApps.SimFlags import simFlags
simFlags.OptionalUserActionList.addAction('G4UA::SG_StepNtupleTool', ['General'])

theApp.HistogramPersistency = "ROOT"
NTupleSvc = Service( "NTupleSvc" )
NTupleSvc.Output = [ "FILE1 DATAFILE='SG_StepNtuple.root' TYP='ROOT' OPT='NEW'" ]

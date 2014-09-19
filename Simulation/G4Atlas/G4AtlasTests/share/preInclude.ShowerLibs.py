#Memory benchmarking.  LAr parameterisation set to 1.

from PerfMonComps.PerfMonFlags import jobproperties
jobproperties.PerfMonFlags.doMonitoring = True
jobproperties.PerfMonFlags.doMallocMonitoring = True
jobproperties.PerfMonFlags.doPostProcessing = True

from AthenaCommon.DetFlags import DetFlags
DetFlags.ID_setOn()
DetFlags.Calo_setOn()
DetFlags.Muon_setOn()

from G4AtlasApps.SimFlags import simFlags
#simFlags.load_atlas_flags()
simFlags.SimLayout.set_On() # uses the default value

simFlags.CalibrationRun.set_Off()
simFlags.EventFilter.set_On()
simFlags.LArParameterization = 1

simFlags.MagneticField.set_Off()

simFlags.ReleaseGeoModel.set_Off()

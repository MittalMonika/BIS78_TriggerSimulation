# Use Global flags and DetFlags.
from AthenaCommon.DetFlags import DetFlags
from AthenaCommon.GlobalFlags import globalflags

# Just the pixel and SCT
DetFlags.ID_setOn()
DetFlags.detdescr.pixel_setOn()
DetFlags.detdescr.SCT_setOn()

# MC or data - affects which conditions database instance is used
globalflags.DataSource='geant4'
#globalflags.DataSource='data'

# Select the geometry version. 
globalflags.DetDescrVersion = 'ATLAS-R2-2016-00-00-00'

# THIS ACTUALLY DOES STUFF!!
from AtlasGeoModel import GeoModelInit
from AtlasGeoModel import SetGeometryVersion

# For misalignments
from IOVDbSvc.CondDB import conddb
conddb.setGlobalTag('OFLCOND-SIM-00-00-00')
conddb.addOverride("/Indet/Align", "InDetAlign_R2_Nominal")

from AthenaCommon.AppMgr import ServiceMgr
from GeomACTS.GeomACTSConfig import TrackingGeometrySvc
ServiceMgr += TrackingGeometrySvc()

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

from GeomACTS.GeomACTSConf import ActsWriteTrackingGeometry
alg = ActsWriteTrackingGeometry()

alg.ObjWriterTool.OutputDirectory = "obj"
alg.ObjWriterTool.SubDetectors = [
    "Pixel", 
    "SCT",
    "TRT",
]


job += alg

theApp.EvtMax = 1


# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

#
# Joboptions for bootstrapping the loading of the DetectorStore with
# the DetDescrCnvSvc
#

def _setupDetStoreConfig():
    from AthenaCommon.AppMgr import theApp
    from AthenaCommon.AppMgr import ServiceMgr as svcMgr

    # first, make sure we create a DetectorStore
    import AthenaCommon.AtlasUnixStandardJob
    #theApp.CreateSvc += [ svcMgr.DetectorStore.getFullName() ]

    # now configure the Detector Description converter service
    from DetDescrCnvSvc.DetDescrCnvSvcConf import DetDescrCnvSvc
    from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags
    from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
    # Create DetDescrCnvSvc and add to SvcMgr
    # Specify primary Identifier dictionary to be used
    # and whether CSC/sTgc/MM muon chambers are part of the detector
    svcMgr += DetDescrCnvSvc(IdDictName = "IdDictParser/ATLAS_IDS.xml",
                             HasCSC=MuonGeometryFlags.hasCSC(),
                             HasSTgc=(CommonGeometryFlags.Run() in ["RUN3", "RUN4"]),
                             HasMM=(CommonGeometryFlags.Run() in ["RUN3", "RUN4"]))

    #theApp.CreateSvc += [ svcMgr.DetDescrCnvSvc.getFullName() ]
    svcMgr.EventPersistencySvc.CnvServices += [ "DetDescrCnvSvc" ]

    # Always include the IdDict jobOptions
    from AthenaCommon.Include import include
    include( "IdDictDetDescrCnv/IdDictDetDescrCnv_joboptions.py" )

    pass

# run this function at module's load
_setupDetStoreConfig()

# avoid running this method multiple times
del _setupDetStoreConfig


################################################################################
# Package: TrkParticleCreator
################################################################################

# Declare the package name:
atlas_subdir( TrkParticleCreator )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC
   Control/AthContainers
   Control/AthenaBaseComps
   Event/EventPrimitives
   Event/xAOD/xAODTracking
   GaudiKernel
   InnerDetector/InDetDetDescr/InDetIdentifier
   InnerDetector/InDetDetDescr/PixelGeoModel
   MuonSpectrometer/MuonReconstruction/MuonRecTools/MuonRecToolInterfaces
   Tracking/TrkEvent/TrkEventPrimitives
   Tracking/TrkEvent/TrkParameters
   Tracking/TrkEvent/TrkParticleBase
   Tracking/TrkEvent/TrkTrack
   Tracking/TrkEvent/TrkTrackSummary
   Tracking/TrkTools/TrkToolInterfaces
   PRIVATE
   Control/DataModel
   DetectorDescription/AtlasDetDescr
   DetectorDescription/GeoPrimitives
   DetectorDescription/IdDictDetDescr
   InnerDetector/InDetConditions/InDetBeamSpotService
   InnerDetector/InDetRecEvent/InDetPrepRawData
   InnerDetector/InDetRecEvent/InDetRIO_OnTrack
   MagneticField/MagFieldInterfaces
   Reconstruction/Particle
   Reconstruction/RecoTools/ITrackToVertex
   Tracking/TrkDetDescr/TrkSurfaces
   Tracking/TrkEvent/TrkPseudoMeasurementOnTrack
   Tracking/TrkEvent/VxVertex
   Tracking/TrkExtrapolation/TrkExInterfaces )

# Component(s) in the package:
atlas_add_library( TrkParticleCreatorLib
   TrkParticleCreator/*.h
   INTERFACE
   PUBLIC_HEADERS TrkParticleCreator
   LINK_LIBRARIES TrkToolInterfaces AthenaBaseComps GaudiKernel TrkParticleBase
   TrkParameters TrkTrackSummary TrkTrack TrkEventPrimitives
   MuonRecToolInterfaces TrkTrackSummary xAODTracking AthContainers
   EventPrimitives InDetIdentifier PixelGeoModelLib )

atlas_add_component( TrkParticleCreator
   src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AthContainers AthenaBaseComps EventPrimitives xAODTracking
   GaudiKernel InDetIdentifier MuonRecToolInterfaces TrkEventPrimitives
   TrkParameters TrkParticleBase TrkTrack TrkTrackSummary TrkToolInterfaces
   DataModel AtlasDetDescr GeoPrimitives IdDictDetDescr InDetPrepRawData
   InDetRIO_OnTrack MagFieldInterfaces Particle ITrackToVertex TrkSurfaces
   TrkPseudoMeasurementOnTrack VxVertex TrkExInterfaces PixelGeoModelLib
   TrkParticleCreatorLib )

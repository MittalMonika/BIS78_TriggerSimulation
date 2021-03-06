################################################################################
# Package: MuonSegmentMomentum
################################################################################

# Declare the package name:
atlas_subdir( MuonSegmentMomentum )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          Control/AthenaBaseComps
                          GaudiKernel
                          MagneticField/MagFieldInterfaces
                          MuonSpectrometer/MuonReconstruction/MuonRecTools/MuonRecToolInterfaces
                          PRIVATE
                          DetectorDescription/GeoPrimitives
                          MuonSpectrometer/MuonIdHelpers
                          MuonSpectrometer/MuonReconstruction/MuonRecEvent/MuonSegment
                          Tracking/TrkDetDescr/TrkGeometry
                          Tracking/TrkEvent/TrkCompetingRIOsOnTrack
                          Tracking/TrkEvent/TrkParameters
                          Tracking/TrkEvent/TrkRIO_OnTrack
                          Tracking/TrkExtrapolation/TrkExInterfaces
                          Tracking/TrkExtrapolation/TrkExUtils )

# External dependencies:
find_package( Eigen )

# Component(s) in the package:
atlas_add_library( MuonSegmentMomentumLib
                   src/*.cxx
                   PUBLIC_HEADERS MuonSegmentMomentum
                   PRIVATE_INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaBaseComps GaudiKernel MagFieldInterfaces MuonRecToolInterfaces MuonIdHelpersLib
                   PRIVATE_LINK_LIBRARIES ${EIGEN_LIBRARIES} GeoPrimitives MuonSegment TrkGeometry TrkCompetingRIOsOnTrack TrkParameters TrkRIO_OnTrack TrkExInterfaces TrkExUtils )

atlas_add_component( MuonSegmentMomentum
                     src/components/*.cxx
                     INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS}
                     LINK_LIBRARIES ${EIGEN_LIBRARIES} AthenaBaseComps GaudiKernel MagFieldInterfaces MuonRecToolInterfaces GeoPrimitives MuonIdHelpersLib MuonSegment TrkGeometry TrkCompetingRIOsOnTrack TrkParameters TrkRIO_OnTrack TrkExInterfaces TrkExUtils MuonSegmentMomentumLib )


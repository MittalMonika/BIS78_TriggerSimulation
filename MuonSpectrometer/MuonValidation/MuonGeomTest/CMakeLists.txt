################################################################################
# Package: MuonGeomTest
################################################################################

# Declare the package name:
atlas_subdir( MuonGeomTest )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PRIVATE
                          Control/AthenaBaseComps
                          Control/StoreGate
                          DetectorDescription/GeoPrimitives
                          GaudiKernel
                          MuonSpectrometer/MuonDetDescr/MuonReadoutGeometry
                          MuonSpectrometer/MuonIdHelpers
                          Tracking/TrkDetDescr/TrkSurfaces
                          Tracking/TrkUtilityPackages/TrkValidationUtils )

# External dependencies:
find_package( Eigen )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( MuonGeomTest
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${EIGEN_LIBRARIES} AthenaBaseComps StoreGateLib SGtests GeoPrimitives GaudiKernel MuonReadoutGeometry MuonIdHelpersLib TrkSurfaces TrkValidationUtils )

# Install files from the package:
atlas_install_joboptions( share/*.py )


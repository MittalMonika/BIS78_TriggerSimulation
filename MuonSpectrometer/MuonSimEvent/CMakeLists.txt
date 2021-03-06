################################################################################
# Package: MuonSimEvent
################################################################################

# Declare the package name:
atlas_subdir( MuonSimEvent )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          Control/CLIDSvc
                          Control/SGTools
                          Database/AtlasSealCLHEP
                          DetectorDescription/GeoPrimitives
                          DetectorDescription/Identifier
                          Generators/GeneratorObjects
                          MuonSpectrometer/MuonIdHelpers
                          Simulation/HitManagement )

# External dependencies:
find_package( Eigen )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( MuonSimEvent
                   src/*.cxx
                   PUBLIC_HEADERS MuonSimEvent
                   INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${EIGEN_LIBRARIES} SGTools GeoPrimitives Identifier GeneratorObjects HitManagement MuonIdHelpersLib
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} )

atlas_add_dictionary( MuonSimEventDict
                      MuonSimEvent/MuonSimEventDict.h
                      MuonSimEvent/selection.xml
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} ${EIGEN_LIBRARIES} SGTools GeoPrimitives Identifier GeneratorObjects MuonIdHelpersLib HitManagement MuonSimEvent )


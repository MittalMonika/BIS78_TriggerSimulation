################################################################################
# Package: SiPropertiesSvc
################################################################################

# Declare the package name:
atlas_subdir( SiPropertiesSvc )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          Control/AthenaBaseComps
                          Control/AthenaKernel
                          GaudiKernel
                          InnerDetector/InDetDetDescr/InDetReadoutGeometry
                          PRIVATE
                          Control/StoreGate
                          DetectorDescription/Identifier
                          InnerDetector/InDetConditions/InDetConditionsSummaryService
                          InnerDetector/InDetDetDescr/InDetIdentifier )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_library( SiPropertiesSvcLib
                   src/*.cxx
                   PUBLIC_HEADERS SiPropertiesSvc
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel GaudiKernel InDetReadoutGeometry StoreGateLib SGtests
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} Identifier InDetIdentifier )

atlas_add_component( SiPropertiesSvc
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel InDetReadoutGeometry StoreGateLib SGtests Identifier InDetIdentifier SiPropertiesSvcLib )


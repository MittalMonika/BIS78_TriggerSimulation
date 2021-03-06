# $Id: CMakeLists.txt 732131 2016-03-24 11:03:29Z krasznaa $
################################################################################
# Package: VP1GeometrySystems
################################################################################

# Declare the package name:
atlas_subdir( VP1GeometrySystems )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC
   DetectorDescription/GeoModel/GeoModelKernel
   MuonSpectrometer/MuonDetDescr/MuonReadoutGeometry
   graphics/VP1/VP1Base
   PRIVATE
   DetectorDescription/GeoModel/GeoModelUtilities
   graphics/VP1/VP1HEPVis
   graphics/VP1/VP1Utils )

# External dependencies:
find_package( CLHEP )
find_package( Coin3D )
find_package( Qt4 COMPONENTS QtCore QtGui )

# Generate UI files automatically:
set( CMAKE_AUTOUIC TRUE )
# Generate MOC files automatically:
set( CMAKE_AUTOMOC TRUE )

# Component(s) in the package:
atlas_add_library( VP1GeometrySystems
   VP1GeometrySystems/*.h src/*.cxx
   PUBLIC_HEADERS VP1GeometrySystems
   INCLUDE_DIRS ${COIN3D_INCLUDE_DIRS}
   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}
   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
   LINK_LIBRARIES ${COIN3D_LIBRARIES} GeoModelKernel MuonReadoutGeometry
   VP1Base GL Qt4::QtCore
   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} GeoModelUtilities VP1HEPVis
   VP1Utils Qt4::QtGui )

################################################################################
# Package: VP1CaloReadoutSystems
################################################################################

# Declare the package name:
atlas_subdir( VP1CaloReadoutSystems )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          graphics/VP1/VP1Base
                          PRIVATE
                          LArCalorimeter/LArGeoModel/LArHV
                          LArCalorimeter/LArGeoModel/LArReadoutGeometry
                          graphics/VP1/VP1Utils )

# External dependencies:
find_package( Coin3D )
find_package( Qt4 COMPONENTS QtCore QtOpenGL QtGui )

include_directories( /usr/X11R6/include )

# Generate UI files automatically:
set( CMAKE_AUTOUIC TRUE )
# Generate MOC files automatically:
set( CMAKE_AUTOMOC TRUE )
# Generate resource files automatically:
set( CMAKE_AUTORCC TRUE )
# to let CMake find .h files automatically-generated from .ui files
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# Component(s) in the package:
atlas_add_library( VP1CaloReadoutSystems VP1CaloReadoutSystems/*.h src/*.h src/*.cxx src/*.qrc  
                   PUBLIC_HEADERS VP1CaloReadoutSystems
                   PRIVATE_INCLUDE_DIRS ${COIN3D_INCLUDE_DIRS}
                   LINK_LIBRARIES VP1Base GL
                   PRIVATE_LINK_LIBRARIES ${COIN3D_LIBRARIES} LArHV LArReadoutGeometry VP1Utils )


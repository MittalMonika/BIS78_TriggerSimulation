################################################################################
# Package: VP1VertexSystems
################################################################################

# Declare the package name:
atlas_subdir( VP1VertexSystems )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          graphics/VP1/VP1Base
                          PRIVATE
                          DetectorDescription/GeoPrimitives
                          GaudiKernel
                          Generators/GeneratorObjects
                          Tracking/TrkEvent/TrkParticleBase
                          Tracking/TrkEvent/TrkTrack
                          Tracking/TrkEvent/TrkTrackLink
                          Tracking/TrkEvent/VxVertex
                          graphics/VP1/VP1Utils )

# External dependencies:
find_package( Coin3D )
find_package( Eigen )
find_package( HepMC )
find_package( Qt4 COMPONENTS QtCore QtOpenGL QtGui )

# Generate UI files automatically:
set( CMAKE_AUTOUIC TRUE )
# Generate MOC files automatically:
set( CMAKE_AUTOMOC TRUE )
# Generate resource files automatically:
set( CMAKE_AUTORCC TRUE )
# to let CMake find .h files automatically-generated from .ui files
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories( /usr/X11R6/include )

# Component(s) in the package:
atlas_add_library( VP1VertexSystems VP1VertexSystems/*.h src/*.h src/*.cxx src/*.qrc  
                   PUBLIC_HEADERS VP1VertexSystems
                   INCLUDE_DIRS ${QT4_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${COIN3D_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS} ${HEPMC_INCLUDE_DIRS}
                   LINK_LIBRARIES VP1Base ${QT4_LIBRARIES} GL
                   PRIVATE_LINK_LIBRARIES ${COIN3D_LIBRARIES} ${EIGEN_LIBRARIES} ${HEPMC_LIBRARIES} GeoPrimitives GaudiKernel GeneratorObjects TrkParticleBase TrkTrack VxVertex VP1Utils )


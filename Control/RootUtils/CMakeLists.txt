################################################################################
# Package: RootUtils
################################################################################

# Declare the package name:
atlas_subdir( RootUtils )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PRIVATE
                          AtlasTest/TestTools )

# External dependencies:
find_package( Boost COMPONENTS filesystem thread system )
find_package( PythonLibs )
find_package( ROOT COMPONENTS RIO Hist Tree PyROOT Core MathCore pthread )

# Component(s) in the package:
atlas_add_library( RootUtils
                   src/*.cxx
                   PUBLIC_HEADERS RootUtils
                   INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${PYTHON_LIBRARIES} ${Boost_LIBRARIES}
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} TestTools )

atlas_add_library( RootUtilsPyROOT
                   src/pyroot/*.cxx
                   PUBLIC_HEADERS RootUtils
                   INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${PYTHON_LIBRARIES} ${Boost_LIBRARIES} RootUtils
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} TestTools )

atlas_add_dictionary( RootUtilsDict
                      RootUtils/RootUtilsDict.h
                      RootUtils/selection.xml
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} ${PYTHON_LIBRARIES} ${Boost_LIBRARIES} TestTools RootUtils RootUtilsPyROOT )

atlas_add_dictionary( RootUtilsPyROOTDict
                      RootUtils/RootUtilsPyROOTDict.h
                      RootUtils/selection_PyROOT.xml
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} ${PYTHON_LIBRARIES} ${Boost_LIBRARIES} TestTools RootUtils RootUtilsPyROOT )

atlas_add_test( Type_test
                SOURCES
                test/Type_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} ${PYTHON_LIBRARIES} ${Boost_LIBRARIES} TestTools RootUtils RootUtilsPyROOT )

# Install files from the package:
atlas_install_python_modules( python/*.py )



# Set the minimum required CMake version:
cmake_minimum_required( VERSION 3.2 FATAL_ERROR )

# Read in the project's version from a file called version.txt. But let it be
# overridden from the command line if necessary.
file( READ ${CMAKE_SOURCE_DIR}/version.txt _version )
string( STRIP ${_version} _version )
set( ATHDATAQUALITY_PROJECT_VERSION ${_version}
   CACHE STRING "Version of the AthDataQuality project to build" )
unset( _version )

# Set the version of tdaq-common to use for the build:
set( TDAQ-COMMON_VERSION "02-03-00" )
set( TDAQ-COMMON_ROOT 
    "$ENV{TDAQ_RELEASE_BASE}/tdaq-common/tdaq-common-${TDAQ-COMMON_VERSION}" )

set( DQM-COMMON_VERSION "01-03-00" )
set( DQM-COMMON_ROOT
   "$ENV{TDAQ_RELEASE_BASE}/dqm-common/dqm-common-${DQM-COMMON_VERSION}" )

# we need this in order to avoid linking Gaudi in AtlasTest/TestTools
set( XAOD_STANDALONE TRUE CACHE BOOL
   "Flag specifying that this is a standalone build" )

# Pick up the AtlasCMake code:
find_package( AtlasCMake REQUIRED )

# Build the project against LCG:
set( LCG_VERSION_POSTFIX ""
   CACHE STRING "Version postfix for the LCG release to use" )
set( LCG_VERSION_NUMBER 88
   CACHE STRING "Version number for the LCG release to use" )
find_package( LCG ${LCG_VERSION_NUMBER} EXACT REQUIRED )

# Extra packages to find, for a "healthy" runtime environment:
find_package( Xrootd )
find_package( GSL )
find_package( Frontier_Client )
find_package( XercesC )

# Make CMake find the project specific code we have here:
list( INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/modules )

# Set up CTest:
atlas_ctest_setup()

# Set up a work directory project:
atlas_project( AthDataQuality ${ATHDATAQUALITY_PROJECT_VERSION}
   PROJECT_ROOT ${CMAKE_SOURCE_DIR}/../../ )

# Generate the environment setup for the externals, to be used during the build:
lcg_generate_env( SH_FILE ${CMAKE_BINARY_DIR}/${ATLAS_PLATFORM}/env_setup.sh )

# Generate replacement rules for the installed paths:
set( _replacements )
if( NOT "$ENV{NICOS_PROJECT_HOME}" STREQUAL "" )
   get_filename_component( _buildDir $ENV{NICOS_PROJECT_HOME} PATH )
   list( APPEND _replacements ${_buildDir} "\${Athena_DIR}/../../../.." )
endif()
if( NOT "$ENV{NICOS_PROJECT_RELNAME}" STREQUAL "" )
   list( APPEND _replacements $ENV{NICOS_PROJECT_RELNAME}
      "\${Athena_VERSION}" )
endif()
if( NOT "$ENV{TDAQ_RELEASE_BASE}" STREQUAL "" )
   list( APPEND _replacements $ENV{TDAQ_RELEASE_BASE}
      "\${TDAQ_RELEASE_BASE}" )
endif()

# Now generate and install the installed setup files:
lcg_generate_env( SH_FILE
   ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/env_setup_install.sh
   REPLACE ${_replacements} )
install( FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/env_setup_install.sh
   DESTINATION . RENAME env_setup.sh )

# Configure and install the post-configuration file:
string( REPLACE "$ENV{TDAQ_RELEASE_BASE}" "\$ENV{TDAQ_RELEASE_BASE}"
   TDAQ-COMMON_ROOT "${TDAQ-COMMON_ROOT}" )
string( REPLACE "${TDAQ-COMMON_VERSION}" "\${TDAQ-COMMON_VERSION}"
   TDAQ-COMMON_ROOT "${TDAQ-COMMON_ROOT}" )
string( REPLACE "$ENV{TDAQ_RELEASE_BASE}" "\$ENV{TDAQ_RELEASE_BASE}"
   DQM-COMMON_ROOT "${DQM-COMMON_ROOT}" )
string( REPLACE "${DQM-COMMON_VERSION}" "\${DQM-COMMON_VERSION}"
   DQM-COMMON_ROOT "${DQM-COMMON_ROOT}" )
configure_file( ${CMAKE_SOURCE_DIR}/PreConfig.cmake.in
   ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PreConfig.cmake @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/PostConfig.cmake.in
   ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PostConfig.cmake @ONLY )
install( FILES
   ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PreConfig.cmake
   ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PostConfig.cmake
   DESTINATION ${CMAKE_INSTALL_CMAKEDIR} )

# Set up CPack:
atlas_cpack_setup()

#!/bin/bash
#
# Script used for building Gaudi.
#

# Don't stop on errors:
set +e

# Function printing the usage information for the script
usage() {
    echo "Usage: build_Gaudi.sh <-s source dir> <-b build dir> " \
        "<-i install dir> <-e externals dir> <-p externals project name> " \
        "<-f platform name> [-r RPM dir] [-t build type]"
}

# Parse the command line arguments:
SOURCEDIR=""
BUILDDIR=""
INSTALLDIR=""
EXTDIR=""
EXTPROJECT=""
PLATFORM=""
RPMDIR=""
BUILDTYPE="Release"
while getopts ":s:b:i:e:p:f:r:t:h" opt; do
    case $opt in
        s)
            SOURCEDIR=$OPTARG
            ;;
        b)
            BUILDDIR=$OPTARG
            ;;
        i)
            INSTALLDIR=$OPTARG
            ;;
        e)
            EXTDIR=$OPTARG
            ;;
        p)
            EXTPROJECT=$OPTARG
            ;;
        f)
            PLATFORM=$OPTARG
            ;;
        r)
            RPMDIR=$OPTARG
            ;;
        t)
            BUILDTYPE=$OPTARG
            ;;
        h)
            usage
            exit 0
            ;;
        :)
            echo "Argument -$OPTARG requires a parameter!"
            usage
            exit 1
            ;;
        ?)
            echo "Unknown argument: -$OPTARG"
            usage
            exit 1
            ;;
    esac
done

# Make sure that the required options were all specified:
if [ "$SOURCEDIR" = "" ] || [ "$BUILDDIR" = "" ] || [ "$INSTALLDIR" = "" ] \
    || [ "$EXTDIR" = "" ] || [ "$EXTPROJECT" = "" ] \
    || [ "$PLATFORM" = "" ]; then
    echo "Not all required parameters received!"
    usage
    exit 1
fi

# Create the build directory if it doesn't exist, and move to it:
mkdir -p ${BUILDDIR}
cd ${BUILDDIR}

# Set up the externals project:
source ${EXTDIR}/setup.sh

# Configure the build:
cmake -DCMAKE_BUILD_TYPE:STRING=${BUILDTYPE} -DCTEST_USE_LAUNCHERS:BOOL=TRUE \
    -DGAUDI_ATLAS:BOOL=TRUE -DGAUDI_ATLAS_BASE_PROJECT:STRING=${EXTPROJECT} \
    -DCMAKE_INSTALL_PREFIX:PATH=/InstallArea/${PLATFORM} \
    ${SOURCEDIR} 2>&1 | tee cmake_config.log

# Build it:
make -k

# Install it:
make -k install/fast DESTDIR=${INSTALLDIR}

# If no RPM directory was specified, stop here:
if [ "$RPMDIR" = "" ]; then
    exit 0
fi

# Build the RPM for the project:
cpack
mkdir -p ${RPMDIR}
cp GAUDI*.rpm ${RPMDIR}

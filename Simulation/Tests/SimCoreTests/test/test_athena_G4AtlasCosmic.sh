#!/bin/sh
#
# art-description: Run cosmics simulation outside ISF from the athena command-line, generating events on-the-fly
# art-include: 21.0/Athena
# art-include: 21.3/Athena
# art-include: 21.9/Athena
# art-include: master/Athena
# art-type: grid

athena --preloadlib=${ATLASMKLLIBDIR_PRELOAD}/libintlc.so.5:${ATLASMKLLIBDIR_PRELOAD}/libimf.so G4AtlasApps/jobOptions.G4Cosmic.py

# TODO This is a regression test I think. 
ArtPackage=$1
ArtJobName=$2
art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName}

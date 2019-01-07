# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#
# This module is used to set up the environment for Photospp
# 
#

# Set the environment variable(s):
find_package( Photospp )

if( PHOTOSPP_FOUND )
  set( PHOTOSPPENVIRONMENT_ENVIRONMENT 
        FORCESET PHOTOSPPVER ${PHOTOSPP_VERSION})
endif()

# Silently declare the module found:
set( PHOTOSPPENVIRONMENT_FOUND TRUE )


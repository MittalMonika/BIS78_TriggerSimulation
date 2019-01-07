# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#
# This module is used to set up the environment for QGSJet
# 
#

# Set the environment variable(s):
find_package( CRMC )

if( CRMC_FOUND  )
  set( QGSJETENVIRONMENT_ENVIRONMENT 
        FORCESET QGSJETVER ${CRMC_VERSION} )
endif()

# Silently declare the module found:
set( QGSJETENVIRONMENT_FOUND TRUE )



# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# Powheg_ZZ setup with sensible defaults
#--------------------------------------------------------------
include('PowhegControl/PowhegControl_ZZ_Common.py')
PowhegConfig.generateRunCard()
PowhegConfig.generateEvents()

#--------------------------------------------------------------
# Fortran Herwig showering with Jimmy MPI
#--------------------------------------------------------------
include("MC12JobOptions/PowhegJimmy_AUET2_CT10_Common.py")
# Beware! Only used here because Fortran Herwig does not have a native model for QED FSR off charged leptons 
include("MC12JobOptions/Jimmy_Photos.py") 

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = 'POWHEG+Herwig ZZ production with AU2 CT10 tune'
evgenConfig.keywords    = [ 'SM', 'zz' ]
evgenConfig.contact     = [ 'daniel.hayden@cern.ch' ]
evgenConfig.generators += [ 'Powheg', 'Herwig', 'Jimmy' ]
evgenConfig.minevents   = 10


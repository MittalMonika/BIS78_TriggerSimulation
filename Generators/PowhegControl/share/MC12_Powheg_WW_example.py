#--------------------------------------------------------------
# Powheg_WW setup with sensible defaults
#--------------------------------------------------------------
include('PowhegControl/PowhegControl_WW_Common.py')
PowhegConfig.generateRunCard()
PowhegConfig.generateEvents()

#--------------------------------------------------------------
# Pythia8 showering with new, main31-style shower
#--------------------------------------------------------------
include('MC12JobOptions/Pythia8_AU2_CT10_Common.py')
include('MC12JobOptions/Pythia8_Powheg.py')
topAlg.Pythia8.Commands += [ 'SpaceShower:pTmaxMatch = 2'
                           , 'TimeShower:pTmaxMatch = 2'
                           ]
topAlg.Pythia8.UserHook  = 'Main31'

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = 'POWHEG+Pythia8 WW production with AU2 CT10 tune'
evgenConfig.keywords    = [ 'SM', 'ww' ]
evgenConfig.contact     = [ 'daniel.hayden@cern.ch' ]
evgenConfig.generators += [ 'Pythia8' ]
evgenConfig.minevents   = 10

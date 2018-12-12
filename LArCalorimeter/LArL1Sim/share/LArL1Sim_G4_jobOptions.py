from LArL1Sim.LArTTL1Getter import *
theLArTTL1Getter = LArTTL1Getter()

from LArL1Sim.LArL1SimConf import *
theLArTTL1Maker=LArTTL1Maker()
from Digitization.DigitizationFlags import digitizationFlags
theLArTTL1Maker.RndmSvc = digitizationFlags.rndmSvc.get_Value()
digitizationFlags.rndmSeedList.addSeed("LArTTL1Maker", 335242, 7306589 )
#include( "LArDetDescr/LArDetDescr_joboptions.py" )

#include( "CaloConditions/CaloConditions_jobOptions.py" )

if digitizationFlags.PileUpPremixing and 'OverlayMT' in digitizationFlags.experimentalDigi():
    from OverlayCommonAlgs.OverlayFlags import overlayFlags
    theLArTTL1Maker.EmTTL1ContainerName = overlayFlags.bkgPrefix() + "LArTTL1EM"
    theLArTTL1Maker.HadTTL1ContainerName = overlayFlags.bkgPrefix() + "LArTTL1HAD"

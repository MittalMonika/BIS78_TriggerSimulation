#--------------------------------------------------------------
# SLHC setup
#--------------------------------------------------------------

# build GeoModel geometry
from InDetSLHC_Example.SLHC_Setup_InclBrl_4_OptRing_25x100 import SLHC_Setup
SLHC_Setup = SLHC_Setup()

include("InDetSLHC_Example/postInclude.SLHC_Setup_Common.py")

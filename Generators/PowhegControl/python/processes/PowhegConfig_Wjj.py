# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# @PowhegControl PowhegConfig_Wjj
#  Powheg configuration for Wjj subprocess
#
#  Authors: James Robinson  <james.robinson@cern.ch>
#           Daniel Hayden   <danhayden0@googlemail.com>

#! /usr/bin/env python
from ..PowhegConfig_base import PowhegConfig_base

# Default Powheg configuration for Wjj generation
#
#  Create a full configurable with all available Powheg options
class PowhegConfig_Wjj(PowhegConfig_base):

    # Constructor: set process-dependent executable path here
    # @param runArgs athena run arguments
    # @param opts athena run options
    def __init__(self, runArgs=None, opts=None):
        super(PowhegConfig_Wjj, self).__init__(runArgs, opts)
        self._powheg_executable += "/W2jet/pwhg_main"

        # Logger warnings for unvalidated process
        self.logger.warning("Integration parameters have not been validated - see https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/PowhegForATLAS#Changing_POWHEG_BOX_integration")

        # Add process specific options
        self.add_parameter("vdecaymodeW", -11, desc="(-11:e+; -13:mu+; 11:e-; 13:mu-). PDG ID of W decay")

        # Decorate with generic option sets
        self.add_parameter_set("Breit-Wigner width")
        self.add_parameter_set("dilepton mass")
        self.add_parameter_set("electroweak properties", boson="w")
        self.add_parameter_set("extra tests")
        self.add_parameter_set("generic scale")
        self.add_parameter_set("old Dij")
        self.add_parameter_set("PDF reweighting")
        self.add_parameter_set("radiation parametrisation")
        self.add_parameter_set("running scales")
        self.add_parameter_set("upper bound")
        self.add_parameter_set("v2")
        self.add_parameter_set("Vector boson + two jets")

        # Set optimised integration parameters
        self.itmx1    = 4
        self.itmx2    = 4
        self.ncall1   = 15000000
        self.ncall2   = 30000000
        self.nubound  = 10000000
        self.xupbound = 5
        self.foldx    = 25
        self.foldy    = 25
        self.foldphi  = 25

        # Override defaults
        self.bornktmin = 10.0
        self.minlo     = 1
        self.withdamp  = 1

        self.populate_default_strings()

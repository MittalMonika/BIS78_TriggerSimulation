# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# @PowhegControl PowhegConfig_Hjj
#  Powheg configuration for Hjj subprocess
#
#  Authors: James Robinson  <james.robinson@cern.ch>

#! /usr/bin/env python
from ..PowhegConfig_base import PowhegConfig_base

# Default Powheg configuration for Hjj generation
#
#  Create a full configurable with all available Powheg options
class PowhegConfig_Hjj(PowhegConfig_base):

    # Constructor: set process-dependent executable path here
    # @param runArgs athena run arguments
    # @param opts athena run options
    def __init__(self, runArgs=None, opts=None):
        super(PowhegConfig_Hjj, self).__init__(runArgs, opts)
        self._powheg_executable += "/HJJ/pwhg_main"

        # Logger warnings for unvalidated process
        self.logger.warning("Integration parameters have not been validated - see https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/PowhegForATLAS#Changing_POWHEG_BOX_integration")

        # Add process specific options
        self.add_parameter("polecheck", -1, desc="(-1:Powheg default)")

        # Decorate with generic option sets
        self.add_parameter_set("Breit-Wigner width")
        self.add_parameter_set("CKKW")
        self.add_parameter_set("generic scale")
        self.add_parameter_set("Higgs fixed width")
        self.add_parameter_set("Higgs properties")
        self.add_parameter_set("old Dij")
        self.add_parameter_set("PDF reweighting")
        self.add_parameter_set("radiation parametrisation")
        self.add_parameter_set("running scales")
        self.add_parameter_set("upper bound")
        self.add_parameter_set("v2")

        # Set optimised integration parameters
        self.itmx1    = 2
        self.itmx2    = 2
        self.ncall1   = 40000
        self.ncall2   = 400000
        self.nubound  = 600000
        self.xupbound = 4
        self.foldx    = 5
        self.foldy    = 5
        self.foldphi  = 5

        # NB. These settings give the following output
        # Cross-section test : 0.00% PASSED
        # Neg. weight test   : 5.41% FAILED
        # Upper bound test   : 0.55% PASSED

        # Override defaults
        self.bornktmin    = 0.26
        self.bornzerodamp = 1
        self.minlo        = 1
        self.par_diexp    = 2
        self.par_dijexp   = 2

        self.populate_default_strings()

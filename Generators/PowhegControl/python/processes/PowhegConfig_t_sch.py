# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# @PowhegControl PowhegConfig_t_sch
#  Powheg configuration for t_sch subprocess
#
#  Authors: James Robinson  <james.robinson@cern.ch>

#! /usr/bin/env python
from ..PowhegConfig_base import PowhegConfig_base

# Default Powheg configuration for t_sch generation
#
#  Create a full configurable with all available Powheg options
class PowhegConfig_t_sch(PowhegConfig_base):

    # Constructor: set process-dependent executable path here
    # @param runArgs athena run arguments
    # @param opts athena run options
    def __init__(self, runArgs=None, opts=None):
        super(PowhegConfig_t_sch, self).__init__(runArgs, opts)
        self._powheg_executable += "/ST_sch/pwhg_main"

        # Decorate with generic option sets
        self.add_parameter_set("CKM")
        self.add_parameter_set("electroweak properties", boson="w")
        self.add_parameter_set("generic scale")
        self.add_parameter_set("lepton mass", prefix="tdec")
        self.add_parameter_set("sin**2 theta W")
        self.add_parameter_set("top decay branching", prefix="tdec")
        self.add_parameter_set("top decay mode")
        self.add_parameter_set("top properties")
        self.add_parameter_set("upper bound")

        # Set optimised integration parameters
        self.ncall1  = 50000
        self.ncall2  = 50000
        self.nubound = 50000

        # Override defaults
        self.topdecaymode = 11111
        self.withdamp     = 1

        self.populate_default_strings()

# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from ..powheg_V2 import PowhegV2

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class HWj(PowhegV2):
    """! Default Powheg configuration for associated Higgs boson and W-boson production plus one jet, with quark mass effects.

    Create a configurable object with all applicable Powheg options.
    @author James Robinson  <james.robinson@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory  path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        super(self.__class__, self).__init__(base_directory, "HWJ", **kwargs)

        ## List of allowed decay modes
        self.allowed_decay_modes = ["w+ > e+ ve", "w- > e- ve~", "w+ > mu+ vm", "w- > mu- vm~", "w+ > tau+ vt",
                                    "w- > tau- vt~", "w+ > j j", "w- > j j", "w+ > l+ vl", "w- > l- vl~", "w+ > all",
                                    "w- > all"]

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("bmass")
        self.add_keyword("bornktmin", 0.26)
        self.add_keyword("bornonly")
        self.add_keyword("bornsuppfact")
        self.add_keyword("bornsuppfactV")
        self.add_keyword("bornsuppfactW")
        self.add_keyword("bornzerodamp")
        self.add_keyword("bottomthr")
        self.add_keyword("bottomthrpdf")
        self.add_keyword("btildeborn")
        self.add_keyword("btildecoll")
        self.add_keyword("btildereal")
        self.add_keyword("btildevirt")
        self.add_keyword("btlscalect")
        self.add_keyword("btlscalereal")
        self.add_keyword("charmthr")
        self.add_keyword("charmthrpdf")
        self.add_keyword("check_bad_st2")
        self.add_keyword("clobberlhe")
        self.add_keyword("colltest")
        self.add_keyword("complexpolescheme")
        self.add_keyword("compress_lhe")
        self.add_keyword("compress_upb")
        self.add_keyword("compute_rwgt")
        self.add_keyword("doublefsr", 1)
        self.add_keyword("evenmaxrat")
        self.add_keyword("facscfact")
        self.add_keyword("factsc2min")
        self.add_keyword("fastbtlbound")
        self.add_keyword("fixedgrid")
        self.add_keyword("flg_debug")
        self.add_keyword("foldcsi", 5)
        self.add_keyword("foldphi", 5)
        self.add_keyword("foldy", 5)
        self.add_keyword("frensc2min")
        self.add_keyword("fullrwgt")
        self.add_keyword("fullrwgtmode")
        self.add_keyword("hdamp")
        self.add_keyword("hfact")
        self.add_keyword("hmass")
        self.add_keyword("hwidth")
        self.add_keyword("icsimax")
        self.add_keyword("idvecbos", hidden=True)
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("itmx1", 5)
        self.add_keyword("itmx1rm")
        self.add_keyword("itmx2", 6)
        self.add_keyword("itmx2rm")
        self.add_keyword("iupperfsr")
        self.add_keyword("iupperisr")
        self.add_keyword("iymax")
        self.add_keyword("kappa_ghb")
        self.add_keyword("kappa_ght")
        self.add_keyword("kappa_ghw")
        self.add_keyword("lhans1")
        self.add_keyword("lhans2")
        self.add_keyword("lhapdf6maxsets")
        self.add_keyword("lhrwgt_descr")
        self.add_keyword("lhrwgt_group_combine")
        self.add_keyword("lhrwgt_group_name")
        self.add_keyword("lhrwgt_id")
        self.add_keyword("LOevents")
        self.add_keyword("manyseeds")
        self.add_keyword("massivebottom")
        self.add_keyword("massivetop")
        self.add_keyword("max_h_mass")
        self.add_keyword("max_w_mass", 2.0 * self.parameters_by_name("beam_energy")[0].value)
        self.add_keyword("maxseeds")
        self.add_keyword("min_h_mass")
        self.add_keyword("min_w_mass", 2.5)
        self.add_keyword("minlo_nnll")
        self.add_keyword("minlo", 1)
        self.add_keyword("mintupbratlim")
        self.add_keyword("mintupbxless")
        self.add_keyword("ncall1", 70000)
        self.add_keyword("ncall1rm")
        self.add_keyword("ncall2", 40000)
        self.add_keyword("ncall2rm")
        self.add_keyword("ncallfrominput")
        self.add_keyword("noevents")
        self.add_keyword("novirtual")
        self.add_keyword("nubound", 200000)
        self.add_keyword("olddij")
        self.add_keyword("par_2gsupp")
        self.add_keyword("par_diexp")
        self.add_keyword("par_dijexp")
        self.add_keyword("parallelstage")
        self.add_keyword("pdfreweight")
        self.add_keyword("ptsqmin")
        self.add_keyword("ptsupp")
        self.add_keyword("ptVhigh")
        self.add_keyword("ptVlow")
        self.add_keyword("radregion")
        self.add_keyword("rand1")
        self.add_keyword("rand2")
        self.add_keyword("renscfact")
        self.add_keyword("runningscales")
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_format_rwgt")
        self.add_keyword("skipextratests")
        self.add_keyword("smartsig")
        self.add_keyword("softtest")
        self.add_keyword("stage2init")
        self.add_keyword("storeinfo_rwgt")
        self.add_keyword("storemintupb")
        self.add_keyword("sudscalevar")
        self.add_keyword("testplots")
        self.add_keyword("testsuda")
        self.add_keyword("tmass")
        self.add_keyword("ubexcess_correct")
        self.add_keyword("ubsigmadetails")
        self.add_keyword("use-old-grid")
        self.add_keyword("use-old-ubound")
        self.add_keyword("vdecaymode", self.allowed_decay_modes[0], name="decay_mode", description="W boson decay mode. [{}]".format("; ".join(self.allowed_decay_modes)))
        self.add_keyword("Vstep")
        self.add_keyword("withdamp")
        self.add_keyword("withnegweights")
        self.add_keyword("withsubtr")
        self.add_keyword("xgriditeration")
        self.add_keyword("xupbound", 20)

        # Add parameter validation functions
        self.validation_functions.append("validate_decays")

    def validate_decays(self):
        """! Validate idvecbos and vdecaymode keywords."""
        self.expose()  # convenience call to simplify syntax
        self.check_decay_mode(self.decay_mode, self.allowed_decay_modes)
        # Calculate appropriate decay mode numbers
        self.parameters_by_keyword("idvecbos")[0].value = 24 * [-1, +1][self.decay_mode.startswith("w+")]
        __decay_mode_lookup = {"e ve": 1, "mu vm": 2, "tau vt": 3, "j j": 0, "l vl": 11, "all": 10}
        __decay_mode = self.decay_mode.split("> ")[1].replace("~", "").replace("+", "").replace("-", "")
        self.parameters_by_keyword("vdecaymode")[0].value = __decay_mode_lookup[__decay_mode]

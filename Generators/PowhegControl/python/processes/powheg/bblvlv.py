# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from ..powheg_RES import PowhegRES


class bblvlv(PowhegRES):
    """! Default Powheg configuration for top pair and Wt production including non-resonant and interference effects.

    Create a configurable object with all applicable Powheg options.

    @author James Robinson  <james.robinson@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        super(self.__class__, self).__init__(base_directory, "b_bbar_4l", **kwargs)

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("allrad", 1)
        self.add_keyword("alpha")
        self.add_keyword("bmass")
        self.add_keyword("bornktmin")
        self.add_keyword("bornonly")
        self.add_keyword("bornsuppfact")
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
        self.add_keyword("chklimseed")
        self.add_keyword("clobberlhe")
        self.add_keyword("colltest")
        self.add_keyword("complexGFermi")
        self.add_keyword("compress_lhe")
        self.add_keyword("compress_upb")
        self.add_keyword("compute_rwgt")
        self.add_keyword("doublefsr")
        self.add_keyword("enhancereg")
        self.add_keyword("evenmaxrat")
        self.add_keyword("ewscheme")
        self.add_keyword("facscfact")
        self.add_keyword("fastbtlbound")
        self.add_keyword("fixedscale")
        self.add_keyword("flg_debug")
        self.add_keyword("foldcsi")
        self.add_keyword("foldphi")
        self.add_keyword("foldy")
        self.add_keyword("for_reweighting", 1)
        self.add_keyword("fullrwgt")
        self.add_keyword("hdamp", 172.5)
        self.add_keyword("hfact")
        self.add_keyword("hmass")
        self.add_keyword("hwidth")
        self.add_keyword("icsimax")
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("itmx1")
        self.add_keyword("itmx1btl")
        self.add_keyword("itmx1btlbrn")
        self.add_keyword("itmx1rm")
        self.add_keyword("itmx2", 10)
        self.add_keyword("itmx2btl")
        self.add_keyword("itmx2btlbrn")
        self.add_keyword("itmx2rm")
        self.add_keyword("iupperfsr")
        self.add_keyword("iupperisr")
        self.add_keyword("iymax")
        self.add_keyword("lhans1")
        self.add_keyword("lhans2")
        self.add_keyword("LOevents")
        self.add_keyword("manyseeds")
        self.add_keyword("max_io_bufsize")
        self.add_keyword("maxseeds")
        self.add_keyword("minlo")
        self.add_keyword("mint_density_map")
        self.add_keyword("mintupbratlim")
        self.add_keyword("MSbarscheme")
        self.add_keyword("ncall1", 150000)
        self.add_keyword("ncall1btl")
        self.add_keyword("ncall1btlbrn")
        self.add_keyword("ncall1rm")
        self.add_keyword("ncall2", 1000000)
        self.add_keyword("ncall2btl")
        self.add_keyword("ncall2btlbrn")
        self.add_keyword("ncall2rm")
        self.add_keyword("ncallfrominput")
        self.add_keyword("noevents")
        self.add_keyword("nores")
        self.add_keyword("novirtual")
        self.add_keyword("nubound", 200000)
        self.add_keyword("olpreset")
        self.add_keyword("olverbose")
        self.add_keyword("openloops-stability")
        self.add_keyword("openloopsreal")
        self.add_keyword("openloopsvirtual")
        self.add_keyword("par_2gsupp")
        self.add_keyword("par_diexp")
        self.add_keyword("par_dijexp")
        self.add_keyword("parallelstage")
        self.add_keyword("pdfreweight")
        self.add_keyword("ptsqmin")
        self.add_keyword("ptsupp")
        self.add_keyword("radregion")
        self.add_keyword("rand1")
        self.add_keyword("rand2")
        self.add_keyword("regridfix")
        self.add_keyword("renscfact")
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_file")
        self.add_keyword("rwl_format_rwgt")
        self.add_keyword("rwl_group_events")
        self.add_keyword("smartsig")
        self.add_keyword("softmismch")
        self.add_keyword("softonly")
        self.add_keyword("softtest")
        self.add_keyword("stage2init")
        self.add_keyword("storeinfo_rwgt")
        self.add_keyword("storemintupb")
        self.add_keyword("testplots")
        self.add_keyword("testsuda")
        self.add_keyword("tmass_phsp")
        self.add_keyword("tmass")
        self.add_keyword("twidth")
        self.add_keyword("ubexcess_correct")
        self.add_keyword("ubsigmadetails", -1)  # disable cross-section output to avoid Fortran crash
        self.add_keyword("use-old-grid")
        self.add_keyword("use-old-ubound")
        self.add_keyword("user_reshists_sep")
        self.add_keyword("verytinypars")
        self.add_keyword("virtonly")
        self.add_keyword("whichpwhgevent")
        self.add_keyword("withbtilde")
        self.add_keyword("withdamp", 1)
        self.add_keyword("withnegweights")
        self.add_keyword("withremnants")
        self.add_keyword("withsubtr")
        self.add_keyword("wmass")
        self.add_keyword("wwidth")
        self.add_keyword("xgriditeration")
        self.add_keyword("xupbound", 4)
        self.add_keyword("zerowidth")
        self.add_keyword("zmass")
        self.add_keyword("zwidth")
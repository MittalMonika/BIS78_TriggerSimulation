#! /usr/bin/env python

import os,sys,time,subprocess,shutil,glob
from AthenaCommon import Logging
mglog = Logging.logging.getLogger('ConfigureHVT')

from MadGraphControl.MadGraphUtils import new_process, generate, arrange_output, build_run_card

def HVT_Generation(run_number=100000,
                   gentype="Wh",
                   decaytype="jjjj",
                   scenario="Agv1",
                   stringy="126",
                   mass=1000,
                   xmass=300,
                   nevts=5000,
                   rand_seed=1234,
                   beamEnergy=6500.,
                   scalevariation=1.0,
                   alpsfact=1.0,
                   pdf='nn23lo',
                   lhaid=247000):

    # -----------------------------------------------------------------------
    # Need to arrange several things:
    # -- proc_card.dat, this will be determined mostly by the run number.
    #    once we have this, we can also initialize the process in madgraph
    #
    # -- param_card.dat, this is mostly invariant, but does depend on the model scenario
    #
    # -- run_card.dat, this is also mostly invariant
    #
    # At that point we can generate the events, arrange the output, and cleanup.
    # -----------------------------------------------------------------------
    
    # -----------------------------------------------------------------------
    # proc card and process generation
    #
    # first, figure out what process we should run.
    # Note that the syntax 'p p > vz, (vz > z h)' means that the vz is always
    # on shell, with mass within some multiple of the width of the vz (usually 15, see 
    # bwcutoff in the run card).  If we want to allow the low-mass tail as well as the
    # peak, then generate 'p p > vz > z h' instead.
    processes=[]
    addHbb=False
    addHcc=False
    if gentype == "Zh" or gentype == "ZH" or gentype == "zh" or gentype == "zH":
        addHbb=True
        if decaytype == "lljj" or decaytype == "llqq":
            processes.append("p p > vz, (vz > z h, z > l+ l-)")
            addHcc=True
        elif decaytype == "jjjj" or decaytype == "qqqq":
            processes.append("p p > vz, (vz > z h, z > j j)") 
            addHcc=True
        elif decaytype == "vvqq" or decaytype == "vvjj":
            processes.append("p p > vz, (vz > z h, z > vl vl~)")
            addHcc=True
        elif decaytype == "llbb" or decaytype == "llbb":
            processes.append("p p > vz, (vz > z h, z > l+ l-)")
        elif decaytype == "jjbb" or decaytype == "qqbb":
            processes.append("p p > vz, (vz > z h, z > j j)")
        elif decaytype == "vvbb" or decaytype == "vvbb":
            processes.append("p p > vz, (vz > z h, z > vl vl~)")
        elif decaytype == "all":
            processes.append("p p > vz, (vz > z h)")
            addHbb=False
            addHcc=False
        else:
            mglog.error('Could not configure process for %s > %s' % (gentype, decaytype))
            return -1,''
    elif gentype == "WW" or gentype == "ww":
        if decaytype == "lvlv" or decaytype == "llvv":
            processes.append("p p > vz, (vz > w+ w-, w+ > l+ vl, w- > l- vl~)")
        elif decaytype == "jjjj" or decaytype == "qqqq":
            processes.append("p p > vz, (vz > w+ w-, w+ > j j, w- > j j)")
        elif decaytype == "lvqq" or decaytype == "lvjj":
            processes.append("p p > vz, (vz > w+ w-, w+ > j j, w- > l- vl~)")
            processes.append("p p > vz, (vz > w+ w-, w- > j j, w+ > l+ vl)")
        elif decaytype == "all":
            processes.append("p p > vz, (vz > w+ w-)")
        else:
            mglog.error('Could not configure process for %s > %s' % (gentype, decaytype))
            return -1,''
    elif gentype == "XH" : #added
        processes.append("p p > vc-" )
        processes.append("p p > vc+" )
        addHbb=True
    elif gentype == "Wh" or gentype == "WH" or gentype == "wh" or gentype == "wH":
        addHbb=True
        if decaytype == "lvqq" or decaytype == "lvjj":
            processes.append("p p > vc+, (vc+ > w+ h, w+ > l+ vl)")
            processes.append("p p > vc-, (vc- > w- h, w- > l- vl~)")
            addHcc=True
        elif decaytype == "qqqq" or decaytype == "qqjj":
            processes.append("p p > vc+, (vc+ > w+ h, w+ > j j)")
            processes.append("p p > vc-, (vc- > w- h, w- > j j)")
            addHcc=True
        elif decaytype == "lvbb" or decaytype == "lvbb":
            processes.append("p p > vc+, (vc+ > w+ h, w+ > l+ vl)")
            processes.append("p p > vc-, (vc- > w- h, w- > l- vl~)")
        elif decaytype == "qqbb" or decaytype == "qqbb":
            processes.append("p p > vc+, (vc+ > w+ h, w+ > j j)")
            processes.append("p p > vc-, (vc- > w- h, w- > j j)")
        elif decaytype == "all":
            processes.append("p p > vc+, (vc+ > w+ h)")
            processes.append("p p > vc-, (vc- > w- h)")
            addHbb=False
            addHcc=False
        else:
            mglog.error('Could not configure process for %s > %s' % (gentype, decaytype))
            return -1,''
    elif gentype == "WZ" or gentype == "wz":
        if decaytype == "qqqq" or decaytype == "jjjj": 
            processes.append("p p > vc+, (vc+ > w+ z, w+ > j j, z > j j)")
            processes.append("p p > vc-, (vc- > w- z, w- > j j, z > j j)")
        elif decaytype == "lvqq" or decaytype == "lvjj": 
            processes.append("p p > vc+, (vc+ > w+ z, w+ > l+ vl, z > j j)")
            processes.append("p p > vc-, (vc- > w- z, w- > l- vl~, z > j j)")
        elif decaytype == "lvll" or decaytype == "lnull": 
            processes.append("p p > vc+, (vc+ > w+ z, w+ > l+ vl, z > l+ l-)")
            processes.append("p p > vc-, (vc- > w- z, w- > l- vl~, z > l+ l-)")
        elif decaytype == "llqq" or decaytype == "lljj" or decaytype == "qqll" or decaytype == "jjll": 
            processes.append("p p > vc+, (vc+ > w+ z, w+ > j j, z > l+ l-)")
            processes.append("p p > vc-, (vc- > w- z, w- > j j, z > l+ l-)")
        elif decaytype == "vvqq" or decaytype == "vvjj" or decaytype == "nunuqq" or decaytype == "nunujj":
            processes.append("p p > vc+, (vc+ > w+ z, w+ > j j, z > vl vl~)")
            processes.append("p p > vc-, (vc- > w- z, w- > j j, z > vl vl~)")
        elif decaytype == "lvvv" or decaytype == "lnununu": 
            processes.append("p p > vc+, (vc+ > w+ z, w+ > l+ vl, z > vl vl~)")
            processes.append("p p > vc-, (vc- > w- z, w- > l- vl~, z > vl vl~)")
        elif decaytype == "all": 
            processes.append("p p > vc+, (vc+ > w+ z)")
            processes.append("p p > vc-, (vc- > w- z)")
        else:
            mglog.error('Could not configure process for %s > %s' % (gentype, decaytype))
            return -1,''
    elif gentype == "Wg" or gentype == "Wgamma" or gentype == "wg" or gentype == "wgamma" or gentype == "Wa" or gentype == "wa" or gentype == "Wy" or gentype == "wy":
        if ("lv" in decaytype) or ("lnu" in decaytype):
            processes.append("p p > vc+, (vc+ > w+ a, w+ > l+ vl)")
            processes.append("p p > vc-, (vc- > w- a, w- > l- vl~)")
        elif ("qq" in decaytype) or ("jj" in decaytype):
            processes.append("p p > vc+, (vc+ > w+ a, w+ > j j)")
            processes.append("p p > vc-, (vc- > w- a, w- > j j)")
        else:
            mglog.error('Could not configure process for %s > %s' % (gentype, decaytype))
            return -1,''
    else:
        mglog.error('Could not configure process for %s > %s' % (gentype, decaytype))
        return -1,''
            
    #Pythia
    pythiachans=[]

    #added tell pythia to decay vc+...
    pythiachans.append("9000002:onMode off ")
    pythiachans.append("-9000002:onMode off ")
    pythiachans.append("9000002:oneChannel = on 1.0 100 25 +6666 ") #onMode bRatio meMode products (meMode = 0:isotropic phase space)
    pythiachans.append("-9000002:oneChannel = on 1.0 100 25 -6666 ") #onMode bRatio meMode products (meMode = 0:isotropic phase space)


    #If we've got Higgs, and we only want H->bb or H->bb/cc, tell Pythia:
    if addHbb:
        pythiachans.append("25:oneChannel = on 0.569 100 5 -5 ")
        if addHcc:
            pythiachans.append("25:addChannel = on 0.0287 100 4 -4 ")
    elif addHcc:
        pythiachans.append("25:oneChannel = on 0.0287 100 4 -4 ")


    proccard = subprocess.Popen(['get_files','-data','MadGraph_proc_card_HVT.dat'])
    proccard.wait()
    
    processstring=""
    processcount=1
    for i in processes:
        if processstring=="":
            processstring="generate %s QED=99 QCD=99 @%d " % (i, processcount)
        else:
            processstring+="\nadd process %s QED=99 QCD=99 @%d" % (i, processcount)
        processcount+=1

    if not os.access('MadGraph_proc_card_HVT.dat',os.R_OK):
        mglog.error('Could not retrieve process card template')
        return -1,''
    if os.access('proc_card_mg5.dat',os.R_OK):
        mglog.error('Old process card in the current directory.  Do not want to clobber it.  Please move it first.')
        return -1,''
    oldcard = open('MadGraph_proc_card_HVT.dat','r')
    newcard = open('proc_card_mg5.dat','w')
    for line in oldcard.readlines():
        if 'generate' in line: newcard.write(processstring)
        else: newcard.write(line)

    oldcard.close()
    newcard.close()

    # Generate the new process!
    thedir = new_process(card_loc='proc_card_mg5.dat')
    if 1==thedir:
        mglog.error('Error in process generation!')
        return -1,''
    
    # See if things are where we expected them to be...
    if not os.access(thedir,os.R_OK):
        mglog.error('Could not find process directory '+thedir+' !!  Bailing out!!')
        return -1,''
    else:
        mglog.info('Using process directory '+thedir)
    # -----------------------------------------------------------------------


    # -----------------------------------------------------------------------
    # param card
    str_param_card='MadGraph_param_card_HVT_%s_M%04d.dat' % (scenario,mass)
    proc_paramcard = subprocess.Popen(['get_files','-data',str_param_card])
    proc_paramcard.wait()
    if not os.access(str_param_card,os.R_OK):
        mglog.info('Could not get param card '+str_param_card)


    if gentype == "Wg" or gentype == "Wgamma" or gentype == "wg" or gentype == "wgamma" or gentype == "Wa" or gentype == "wa" or gentype == "Wy" or gentype == "wy":
        mglog.info('Change default coupling value of Cvvw to deal with Wgamma ')
        old_str_param_card='MadGraph_param_card_HVT_Wgamma_%s_M0500.dat' % scenario
        old_proc_paramcard = subprocess.Popen(['get_files','-data',old_str_param_card])
        old_proc_paramcard.wait()
        if not os.access(old_str_param_card,os.R_OK):
            mglog.info('Could not get param card '+str_param_card)
        new_str_param_card='MadGraph_param_card_HVT_Wgamma_%s_M%04d_local.dat' % (scenario,mass)
        oldcard_param = open(old_str_param_card,'r')
        newcard_param = open(new_str_param_card,'w')
        for line_param in oldcard_param.readlines():
            if '  9000002 499.962 # vc+ ' in line_param: newcard_param.write('  9000002 %d # vc+ : cmath.sqrt(16*mV__exp __ 2 + 16*MWt__exp __ 2 + 16*cvvhh*gst__exp __ 2*vv__exp __ 2 + cmath.sqrt((-16*mV__exp __ 2 - 16*MWt__exp __ 2 - 16*cvvhh*gst__exp __ 2*vv__exp __ 2)**2 - 64*(16*mV__exp __ 2*MWt__exp __ 2 + 16*cvvhh*gst__exp __ 2*MWt__exp __ 2*vv__exp __ 2 - ch__exp __ 2*gst__exp __ 2*gw__exp __ 2*vv__exp __ 4)))/(4.*sqrt__ 2)\n' % mass)
            elif '  9000001 500 # vz : MVz' in line_param: newcard_param.write('  9000001 %d # vz : MVz\n' % mass)
            elif '    2 500 # MVz' in line_param: newcard_param.write('    2 %d # MVz\n' % mass)
#            elif '# cvvw' in line_param: newcard_param.write('    3 5. # cvvw\n')
            elif '# cq' in line_param: newcard_param.write('    4 0.01 # cq\n')
            elif '# cl' in line_param: newcard_param.write('    5 0. # cl\n')
            elif '# c3' in line_param: newcard_param.write('    6 0. # c3\n')
            elif '# ch' in line_param: newcard_param.write('    7 0.02 # ch\n')
            else: newcard_param.write(line_param)
        oldcard.close()
        newcard.close()
        str_param_card = new_str_param_card
        if not os.access(str_param_card,os.R_OK):
            mglog.info('Could not access param card '+str_param_card)
    else:
        mglog.info('This is VV default parameter card, NOT Wgamma. So no need to change anything in the parameter card. ')

    # -----------------------------------------------------------------------

    #Added
    #Add properties to the param card 
    os.system("cp %s temp.data"%str_param_card)
    tempParamCardO = open(str_param_card,'r')
    tempParamCardN = open('temp.dat','w')

    for line in tempParamCardO:
        if 'Block QNUMBERS 9000001  # vz ' in line:
            tempParamCardN.write( 'Block QNUMBERS 6666  #X \n        1 3  # 3 times electric charge \n        2 3  # number of spin states (2S+1) \n        3 1  # colour rep (1: singlet, 3: triplet, 8: octet) \n        4 1  # Particle/Antiparticle distinction (0=own anti)\n' )
            tempParamCardN.write(line)
        elif 'Block mass' in line:
            tempParamCardN.write(line)
            tempParamCardN.write('    6666 %d #MX \n' % xmass ) 
        elif 'DECAY  22 0.000000 # a : 0.0' in line:
             tempParamCardN.write(line)
             tempParamCardN.write('DECAY  6666 2.0 #X \n') #width 0.0
             tempParamCardN.write('#  BR             NDA  ID1    ID2   ... \n')
             tempParamCardN.write('        1.0    2     2        -1   # BR(X -> quarks) \n')
        else:
            tempParamCardN.write(line)
    
    tempParamCardO.close()
    tempParamCardN.close()

    os.system("mv temp.dat %s"%str_param_card)
    print 'modification end'

    # -----------------------------------------------------------------------
    # run card
    extras={}
    extras["scale"] = mass
    extras["dsqrt_q2fact1"] = mass
    extras["dsqrt_q2fact2"] = mass
    extras["pdlabel"]=pdf
    extras["lhaid"]=lhaid
    build_run_card(run_card_old='MadGraph_run_card_HVT.dat',
                   run_card_new='run_card.dat',
                   xqcut=0,
                   nevts=nevts,
                   rand_seed=rand_seed,
                   beamEnergy=beamEnergy, 
                   scalefact=scalevariation, 
                   alpsfact=alpsfact,
                   extras=extras)
    # -----------------------------------------------------------------------

    # -----------------------------------------------------------------------
    # Generation
    if generate(run_card_loc='run_card.dat',
                param_card_loc=str_param_card,
                mode=0,
                njobs=1,
                run_name='Test',
                proc_dir=thedir,
                grid_pack=False):
        mglog.error('Error generating events!')
        return -1
    # -----------------------------------------------------------------------
    # -----------------------------------------------------------------------
    # Cleanup
    outputDS=""
    try:
        outputDS=arrange_output(run_name='Test',
                                proc_dir=thedir,
                                outputDS='madgraph.%i.madgraph_HVT_%s_%s_%s_%s._00001.events.tar.gz'%(run_number,scenario,gentype,decaytype,stringy))
    except:
        mglog.error('Error arranging output dataset!')
        return -1

    keepOutput=True
    if not keepOutput:
        mglog.info('Removing process directory...')
        shutil.rmtree(thedir,ignore_errors=True)

    mglog.info('All done generating events!!')

    return outputDS,pythiachans


    # -----------------------------------------------------------------------


# multi-core running, if allowed!
if 'ATHENA_PROC_NUMBER' in os.environ:
    evgenLog.info('Noticed that you have run with an athena MP-like whole-node setup.  Will re-configure now to make sure that the remainder of the job runs serially.')
    njobs = os.environ.pop('ATHENA_PROC_NUMBER')
    # Try to modify the opts underfoot
    if not hasattr(opts,'nprocs'): mglog.warning('Did not see option!')
    else: opts.nprocs = 0
    print opts

#added some modifications
shortname=runArgs.jobConfig[0].split('/')[-1].split('.')[2].split('_')
scenario='Agv1'
HVTboson=shortname[2][:2]
HVTdecay=shortname[2][2:]
VVdecay=shortname[3]
mass=int(shortname[4][1:])
xmass=int(shortname[5][1:])

# PDF information, in MadGraph's PDF naming scheme.  
# Should probably extract this from shortname[1] in some fancy way.
# For now, specify lhapdf and the ID so that madgraph doesn't get 
# confused trying to interpret some shorthand name (which it can).
# Note that if you change these numbers, you'll probably want to 
# change the "sys_pdf" tag in the run card too.  That's not done
# automatically yet.
pdf='lhapdf'
lhaid=247000 # NNPDF23_lo_as_0130_qed

# Run MadGraph!
runArgs.inputGeneratorFile,pythiachans=HVT_Generation(runArgs.runNumber,     # run number
                                                      HVTdecay,              # How the W'/Z' decay
                                                      VVdecay,               # How the W and Z decay
                                                      scenario,              # 'Agv1', or similar
                                                      ("m%d" % mass),        # dataset tag (production and decay not needed here)
                                                      mass,                  # HVT mass
                                                      xmass,                 # X mass
                                                      runArgs.maxEvents*2,   # number of events for MadGraph to generate
                                                      1234,                  # random seed
                                                      6500,                  # beam energy
                                                      1.0,                   # scale variation
                                                      1.0,                   # PS variation
                                                      pdf,                   # PDF information
                                                      lhaid
                                                      )

# Some more information
evgenConfig.inputfilecheck = 'HVT'
evgenConfig.description = "HVT Signal Point"
evgenConfig.keywords = ["exotic"] 
evgenConfig.contact = ["Kalliopi Iordanidou <Kalliopi.Iordanidou@cern.ch>" ] #added
evgenConfig.process = "pp>%s>%s>%s" % (HVTboson,HVTdecay,VVdecay) # e.g. pp>Vc>WZ>qqqq

# Finally, run the parton shower and configure MadGraph+Pythia
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_MadGraph.py")

# Make Pythia 8.2 happy with our new particles....
genSeq.Pythia8.Commands += [" 9000001:all vz vz 3 0 0 1000",
                            " 9000002:all vc+ vc- 3 3 0 1000",
                            "Init:showAllParticleData = on"]
genSeq.Pythia8.Commands += pythiachans

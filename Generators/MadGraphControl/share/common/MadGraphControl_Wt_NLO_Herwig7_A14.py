from MadGraphControl.MadGraphUtils import *

# General settings
name=''
parton_shower='HERWIGPP'
maxjetflavor=5
zdecay="decay z > all all"
if runArgs.runNumber==412047:
    name='Wt_DR1_NLO_inclusive_H7'
    decay_mode = 'inclusive'
elif runArgs.runNumber==412050:
    name='Wt_DR1_NLO_dilepton_H7'
    decay_mode = 'dilepton'
runMode=0 # 0 = single machine, 1 = cluster, 2 = multicore
nevents=int(1.1*runArgs.maxEvents)

# MG Particle cuts
mllcut=-1

### PROCESS
mgproc="""generate p p > ttbar w [QCD]"""

stringy = 'madgraph.'+str(runArgs.runNumber)+'.MadGraph_'+str(name)

fcard = open('proc_card_mg5.dat','w')
fcard.write("""
import model loop_sm-no_b_mass
define p = g u c d s b u~ c~ d~ s~ b~
define j = g u c d s b u~ c~ d~ s~ b~
define w = w+ w-
define ttbar = t t~
"""+mgproc+"""
output -f
""")
fcard.close()


beamEnergy=-999
if hasattr(runArgs,'ecmEnergy'):
    beamEnergy = runArgs.ecmEnergy / 2.
else: 
    raise RuntimeError("No center of mass energy found.")


# --------------------------------------------------------------
#  Start building the cards
# --------------------------------------------------------------

process_dir = new_process()

mglog.info('Replacing matrix files.')
##
get_matrix_card = subprocess.Popen(['get_files','-data','bg_matrix_2.f'])
get_matrix_card.wait()
shutil.copy("bg_matrix_2.f",process_dir+"/SubProcesses/P0_bg_wmt/matrix_2.f")
get_matrix_card = subprocess.Popen(['get_files','-data','bg_matrix_5.f'])
get_matrix_card.wait()
shutil.copy("bg_matrix_5.f",process_dir+"/SubProcesses/P0_bg_wmt/matrix_5.f")
get_matrix_card = subprocess.Popen(['get_files','-data','bxg_matrix_2.f'])
get_matrix_card.wait()
shutil.copy("bxg_matrix_2.f",process_dir+"/SubProcesses/P0_bxg_wptx/matrix_2.f")
get_matrix_card = subprocess.Popen(['get_files','-data','bxg_matrix_6.f'])
get_matrix_card.wait()
shutil.copy("bxg_matrix_6.f",process_dir+"/SubProcesses/P0_bxg_wptx/matrix_6.f")
get_matrix_card = subprocess.Popen(['get_files','-data','gb_matrix_4.f'])
get_matrix_card.wait()
shutil.copy("gb_matrix_4.f",process_dir+"/SubProcesses/P0_gb_wmt/matrix_4.f")
get_matrix_card = subprocess.Popen(['get_files','-data','gb_matrix_6.f'])
get_matrix_card.wait()
shutil.copy("gb_matrix_6.f",process_dir+"/SubProcesses/P0_gb_wmt/matrix_6.f")
get_matrix_card = subprocess.Popen(['get_files','-data','gbx_matrix_5.f'])
get_matrix_card.wait()
shutil.copy("gbx_matrix_5.f",process_dir+"/SubProcesses/P0_gbx_wptx/matrix_5.f")
get_matrix_card = subprocess.Popen(['get_files','-data','gbx_matrix_6.f'])
get_matrix_card.wait()
shutil.copy("gbx_matrix_6.f",process_dir+"/SubProcesses/P0_gbx_wptx/matrix_6.f")
##
mglog.info('Matrix files replaced.')

beamEnergy=-999
if hasattr(runArgs,'ecmEnergy'):
    beamEnergy = runArgs.ecmEnergy / 2.
else:
    raise RuntimeError("No center of mass energy found.")


lhaid=11000
#pdflabel='lhapdf'

#Fetch default LO run_card.dat and set parameters
extras = { 'lhaid'         : lhaid,
        #'pdflabel'       : pdflabel,
        'maxjetflavor'  : maxjetflavor,
        'parton_shower' : parton_shower,
        'fixed_ren_scale' : "True",
        'fixed_fac_scale' : "True",
        'muR_ref_fixed' : 172.5,
        'muF1_ref_fixed' : 172.5,
        'muF2_ref_fixed' : 172.5,
        'fixed_QES_scale' : "True",
        'QES_ref_fixed' : 172.5
        }

doSyst=False

if doSyst:
    lhe_version=3
    extras.update({'reweight_scale': '.true.',
                   'rw_Rscale_down':  0.5,
                   'rw_Rscale_up'  :  2.0,
                   'rw_Fscale_down':  0.5,
                   'rw_Fscale_up'  :  2.0,
                   'reweight_PDF'  : '.true.',
                   'PDF_set_min'   : 11001,
                   'PDF_set_max'   : 11052,
                   'mll_sf'        : mllcut})
else:
    lhe_version=1
    extras.update({'reweight_scale': '.false.',
                   'reweight_PDF'  : '.false.'})




madspin_card_loc='madspin_card.dat'

mscard = open(madspin_card_loc,'w')
if decay_mode=='inclusive':
    mscard.write("""#************************************************************
#*                        MadSpin                           *
#*                                                          *
#*    P. Artoisenet, R. Frederix, R. Rietkerk, O. Mattelaer *
#*                                                          *
#*    Part of the MadGraph5_aMC@NLO Framework:              *
#*    The MadGraph5_aMC@NLO Development Team - Find us at   *
#*    https://server06.fynu.ucl.ac.be/projects/madgraph     *
#*                                                          *
#************************************************************
#Some options (uncomment to apply)
#
# set seed 1
# set Nevents_for_max_weigth 75 # number of events for the estimate of the max. weight
# set BW_cut 15                # cut on how far the particle can be off-shell
 set max_weight_ps_point 400  # number of PS to estimate the maximum for each event
#
set seed %i
# specify the decay for the final state particles
define q = u d s c b
define q~ = u~ d~ s~ c~ b~
define l+ = e+ mu+ ta+
define l- = e- mu- ta-
define v = ve vm vt
define v~ = ve~ vm~ vt~
decay t > w+ b, w+ > all all
decay t~ > w- b~, w- > all all
decay w+ > all all
decay w- > all all
%s
# running the actual code
launch"""%(runArgs.randomSeed,zdecay))
    mscard.close()
elif decay_mode=='dilepton':
    mscard.write("""#************************************************************
#*                        MadSpin                           *
#*                                                          *
#*    P. Artoisenet, R. Frederix, R. Rietkerk, O. Mattelaer *
#*                                                          *
#*    Part of the MadGraph5_aMC@NLO Framework:              *
#*    The MadGraph5_aMC@NLO Development Team - Find us at   *
#*    https://server06.fynu.ucl.ac.be/projects/madgraph     *
#*                                                          *
#************************************************************
#Some options (uncomment to apply)
#
# set seed 1
# set Nevents_for_max_weigth 75 # number of events for the estimate of the max. weight
# set BW_cut 15                # cut on how far the particle can be off-shell
 set max_weight_ps_point 400  # number of PS to estimate the maximum for each event
#
set seed %i
# specify the decay for the final state particles
define q = u d s c b
define q~ = u~ d~ s~ c~ b~
define l+ = e+ mu+ ta+
define l- = e- mu- ta-
define v = ve vm vt
define v~ = ve~ vm~ vt~
decay t > w+ b, w+ > l+ v
decay t~ > w- b~, w- > l- v~
decay w+ > l+ v
decay w- > l- v~
%s
# running the actual code
launch"""%(runArgs.randomSeed,zdecay))
    mscard.close()



build_run_card(run_card_old=get_default_runcard(proc_dir=process_dir),run_card_new='run_card.dat',
               nevts=nevents,rand_seed=runArgs.randomSeed,beamEnergy=beamEnergy,xqcut=0.,
               extras=extras)

print_cards()

param_card_loc='aMcAtNlo_param_card_loopsmnobmass.dat'
paramcard = subprocess.Popen(['get_files','-data',param_card_loc])
paramcard.wait()
if not os.access(param_card_loc,os.R_OK):
    raise RuntimeError("ERROR: Could not get %s"%param_card.loop_sm-no_b_mass.dat)

generate(run_card_loc='run_card.dat', param_card_loc=param_card_loc,
        mode=runMode, proc_dir=process_dir, madspin_card_loc=madspin_card_loc)

outputDS=arrange_output(proc_dir=process_dir,
        outputDS=stringy+'._00001.events.tar.gz',
        lhe_version=lhe_version,
        saveProcDir=True)
runArgs.inputGeneratorFile=outputDS


from MadGraphControl.MadGraphUtils import *

#---------------------------------------------------------------------------------------------------
# Set parameters
#---------------------------------------------------------------------------------------------------
lhe_version = 3.0

# Turn off systematics for certain DSIDs - until systematics calculation issue in MG is solved
DSID=runArgs.runNumber
DSID_withoutSyst=[436473, 436474, 436475, 436476]
if DSID in DSID_withoutSyst:
   useSysts='F'
else: 
   useSysts='T'

extras = {'dynamical_scale_choice':3,
          'auto_ptj_mjj':'False',
          'maxjetflavor':'4',
          'event_norm':'sum',
          'pdlabel':'lhapdf',
          'lhaid':315200,
          'use_syst' :useSysts,
          'sys_scalefact' :'0.5 1 2',
          'sys_alpsfact' : 'None',
          'sys_pdf' : 'NNPDF31_lo_as_0130',
          }

parameters = {'YUKAWA':{'invMscale':invMscale,'c1':c1,'c2':c2}}

#---------------------------------------------------------------------------------------------------
# MadGraph process
#---------------------------------------------------------------------------------------------------
fcard = open('proc_card_mg5.dat','w')
fcard.write("""
set group_subprocesses Auto
set ignore_six_quark_processes False
set loop_optimized_output True
set loop_color_flows False
set gauge unitary
set complex_mass_scheme False
set max_npoint_for_channel 0
import model sm
define p = g u c d s u~ c~ d~ s~
define j = g u c d s u~ c~ d~ s~
define l+ = e+ mu+
define l- = e- mu-
define vl = ve vm vt
define vl~ = ve~ vm~ vt~
import model Standard_Model_cosmo_no_c10_UFO --modelname
generate p p > t t~ phinew phinew NEW=1 QED=0
output -f""")
fcard.close()

if hasattr(runArgs,'ecmEnergy'):
  beamEnergy = runArgs.ecmEnergy / 2.
else:
  raise RuntimeError("No center of mass energy found.")

# evt_multiplier ensure sufficient events generated to be passed to Pythia8 for showering
if evt_multiplier>0:
  if runArgs.maxEvents>0:
    nevents=runArgs.maxEvents*evt_multiplier
  else:
    nevents=5000*evt_multiplier

process_dir = new_process()

#---------------------------------------------------------------------------------------------------
# Build run card
#---------------------------------------------------------------------------------------------------
build_run_card(run_card_old=get_default_runcard(proc_dir=process_dir),run_card_new='run_card.dat',
               nevts=nevents,rand_seed=runArgs.randomSeed,beamEnergy=beamEnergy,extras=extras)

#---------------------------------------------------------------------------------------------------
# Build parameter card
#---------------------------------------------------------------------------------------------------
build_param_card(param_card_old='MadGraph_param_card_DarkEnergy.dat',param_card_new='param_card.dat',params=parameters)

print_cards()

mode=0
runName='run_01' 
generate(run_card_loc='run_card.dat',param_card_loc='param_card.dat',mode=mode,proc_dir=process_dir,run_name=runName)
arrange_output(run_name=runName,proc_dir=process_dir,outputDS=runName+'._00001.events.tar.gz',lhe_version=lhe_version)

#---------------------------------------------------------------------------------------------------
# Shower with Pythia8
#---------------------------------------------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")                                                                                                               
include("Pythia8_i/Pythia8_MadGraph.py")   

#---------------------------------------------------------------------------------------------------
# Filters
#---------------------------------------------------------------------------------------------------
if (filter_string == "1LMET60orMET150"):
  evgenLog.info('1lepton and MET 60 filter or MET 150 is applied')
  include ( 'GeneratorFilters/LeptonFilter.py' )
  filtSeq.LeptonFilter.Ptcut  = 20000.
  filtSeq.LeptonFilter.Etacut = 2.8 
     
  include('GeneratorFilters/MissingEtFilter.py')
  filtSeq.MissingEtFilter.METCut = 60000.
 
  filtSeq += MissingEtFilter("MissingEtFilterHard")
  filtSeq.MissingEtFilterHard.METCut = 150000.
 
  filtSeq.Expression = "(LeptonFilter and MissingEtFilter) or MissingEtFilterHard"   

if (filter_string == "2L"):
  include ( 'GeneratorFilters/MultiElecMuTauFilter.py' )
  filtSeq.MultiElecMuTauFilter.MinPt  = 18000.
  filtSeq.MultiElecMuTauFilter.MaxEta = 2.8
  filtSeq.MultiElecMuTauFilter.NLeptons = 2
  filtSeq.MultiElecMuTauFilter.IncludeHadTaus = 0
 
  filtSeq.Expression = "MultiElecMuTauFilter"   

#---------------------------------------------------------------------------------------------------
# EVGEN Configuration
#---------------------------------------------------------------------------------------------------
evgenConfig.generators = ["MadGraph", "Pythia8", "EvtGen"]
evgenConfig.process = "pp -> ttbar + phinew phinew"
evgenConfig.description = "Scalar dark energy produced in association with ttbar"
evgenConfig.keywords = ["BSM","exotic","ttbar","lepton","jets"]

evgenConfig.contact = ['Michaela Queitsch-Maitland <michaela.queitsch-maitland@cern.ch>','Spyridon Argyropoulos <spyridon.argyropoulos@cern.ch>']
evgenConfig.inputfilecheck = runName
runArgs.inputGeneratorFile=runName+'._00001.events.tar.gz'

# Phinew is non-interacting
bonus_file = open('pdg_extras.dat','w')
bonus_file.write('1000022 phinew 100.0 (MeV/c) boson phinew 0\n')
bonus_file.close()
testSeq.TestHepMC.G4ExtraWhiteFile='pdg_extras.dat'

# Pythia decays
pythiachans=[]
pythiachans.append("1000022:all phinew phinew 1 0 0 0.1")
pythiachans.append("1000022:mayDecay off")

genSeq.Pythia8.Commands += pythiachans

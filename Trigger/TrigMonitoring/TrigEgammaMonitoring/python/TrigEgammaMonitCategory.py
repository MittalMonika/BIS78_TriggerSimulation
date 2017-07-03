# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

#Categories currently used by offline Egamma TO monitoring tool
#Important to keep lists updated
primary_single_ele = ['HLT_e24_lhtight_nod0','HLT_e26_lhtight_nod0','HLT_e60_lhmedium_nod0_L1EM24VHI','HLT_e140_lhloose_nod0_L1EM24VHI']
primary_single_ele_iso =['HLT_e24_lhtight_nod0_ivarloose','HLT_e26_lhtight_nod0_ivarloose','HLT_e28_lhtight_nod0_ivarloose']
monitoring_Zee = ['HLT_e28_lhtight_nod0_e15_etcut_L1EM7_Zee']
monitoring_Jpsiee = ['HLT_e5_lhtight_nod0_e4_etcut','HLT_e5_lhtight_nod0_e4_etcut_Jpsiee']
primary_single_pho = ['HLT_g140_tight','HLT_g200_loose']

# primary_single_ele = ['HLT_e24_lhtight_nod0','HLT_e26_lhtight_nod0','HLT_e60_lhmedium_nod0','HLT_e120_lhloose_nod0','HLT_e140_lhloose_nod0']
# primary_single_ele_iso =['HLT_e24_lhtight_nod0_ivarloose','HLT_e26_lhtight_nod0_ivarloose','HLT_e28_lhtight_nod0_ivarloose']
# monitoring_Zee = ['HLT_e26_lhtight_nod0_e15_etcut_Zee']
# monitoring_Jpsiee = ['HLT_e5_lhtight_nod0_e4_etcut','HLT_e5_lhtight_nod0_e4_etcut_Jpsiee']
# primary_single_pho = ['HLT_g120_loose','g140_loose']


#Other categories

primary_double_ele = ['HLT_2e17_lhvloose_nod0_L1EM15VHI','HLT_2e24_lhvloose_nod0']
monitoring_ele_idperf = ['HLT_e28_lhtight_idperf']
primary_double_pho = ['HLT_g35_medium_g25_medium_L12EM20VH','HLT_2g20_tight_icalovloose_L12EM15VHI','HLT_2g20_tight_icalotight_L12EM15VHI']


#Old categories
primary_single_ele_cutbased = []
primary_single_ele_cutbased_iso = []
primary_double_ele_cutbased = []
monitoring_ele_idperf_cutbased = []
# Create a single list for other groups
monitoring_egamma = primary_single_ele + primary_double_ele + monitoring_ele_idperf + monitoring_Zee + monitoring_Jpsiee + primary_single_pho + primary_double_pho

# The following is our trigger T0 monitoring configuration
# Additional trigger-only lists for monitoring tools that will evolve with lumi
#
# Maintain lists for different running modes, e.g
# pp 'collisions'
# Heavy Ion 'heavyioncollisions'
# cosmics 'cosmics
# MC 'monteCarlo'

# Define the set of histograms to be monitored by shifters
plots_efficiency = ["eff_et","eff_highet","eff_eta","eff_mu"]
plots_distribution = ["et","eta","Reta","Rphi","Rhad","f1","f3","eratio","deta2","eprobHT","npixhits","nscthits","ptvarcone20"]
plots_resolution = ["res_et","res_Rphi","res_Reta","res_Rhad","res_ptvarcone20","res_deta2"]

# Define triggers for tagging 
monitoring_tags = ['HLT_e24_lhtight_nod0_ivarloose',
                   'HLT_e26_lhtight_nod0_ivarloose',
                   'HLT_e28_lhtight_nod0_ivarloose']

monitoring_jpsitags = [
    'HLT_e5_lhtight_nod0_e4_etcut',
    'HLT_e5_lhtight_nod0_e4_etcut_Jpsiee',
    'HLT_e9_lhtight_nod0_e4_etcut_Jpsiee',
    'HLT_e5_lhtight_nod0_e9_etcut_Jpsiee',
    'HLT_e14_lhtight_nod0_e4_etcut_Jpsiee',
    'HLT_e5_lhtight_nod0_e14_etcut_Jpsiee'
    ]


# pp 'collisions' monitoring configuration, default
# L1 trigger items to monitor from inclusive offline electrons
#monitoring_L1Calo = ['L1_EM22VHI','L1_EM24VHI']
monitoring_L1Calo = []
# Startup and high-pt electrons to monitor from inclusive electrons
commission_electron = [ 'HLT_e60_lhmedium_nod0_ringer_L1EM24VHI',
                       'HLT_e140_lhloose_nod0_ringer_L1EM24VHI']
monitoring_electron = ['HLT_e60_lhmedium_nod0_L1EM24VHI',
                       'HLT_e140_lhloose_nod0_L1EM24VHI']
# monitoring_electron += commission_electron

# 2017 single electrons triggers to monitor from Zee TP, includes rerun

commissionTP_electron = ['HLT_e17_lhvloose_nod0_ringer_L1EM15VHI',
                         'HLT_e24_lhvloose_nod0_L1EM20VH',
                         'HLT_e26_lhtight_nod0_ringer_ivarloose',
                         'HLT_e26_lhtight_cutd0dphideta_ivarloose',
                         'HLT_e28_lhtight_nod0_ringer_ivarloose',
                         'HLT_e26_lhtight_nod0_ivarloose_L1EM22VHIM',
                         'HLT_e28_lhtight_nod0_ivarloose_L1EM24VHIM',
                         ]
monitoringTP_electron = [ 'HLT_e17_lhvloose_nod0_L1EM15VHI',
                          'HLT_e26_lhtight_nod0_ivarloose',
                          'HLT_e28_lhtight_nod0_ivarloose',
                          'HLT_e28_lhtight_nod0_ivarloose_L1EM24VHIM',
                          'HLT_e28_lhtight_nod0_noringer_ivarloose'
                          ]
# monitoringTP_electron += commissionTP_electron

monitoringTP_Jpsiee = ["HLT_e5_lhtight_nod0"]    
monitoring_photon = [ 'HLT_g25_medium_L1EM20VH',
                      'HLT_g200_loose',
                      'HLT_g22_tight' ,
                      'HLT_g22_tight_icalovloose_L1EM15VHI' ,
                      'HLT_g22_tight_L1EM15VHI',
                      ]
#Add the Menu-Aware monitoring; maps a trigger to a category
monitoring_mam = {'primary_single_ele_iso':'HLT_e28_lhtight_nod0_ivarloose',
                  'primary_double_pho':'HLT_g25_medium_L1EM20VH',
                  'primary_single_pho':'HLT_g200_loose',
                  'monitoring_Jpsiee' :'HLT_e5_lhtight_nod0_e4_etcut_Jpsiee'}

# Cosmic items
monitoring_electron_cosmic=['HLT_e0_perf_L1EM3_EMPTY']
monitoring_photon_cosmic=['HLT_g0_perf_L1EM3_EMPTY']
monitoring_mam_cosmic = {'primary_single_ele':'HLT_e0_perf_L1EM3_EMPTY',
                         'primary_single_pho':'HLT_g0_perf_L1EM3_EMPTY'}

# Validation Items for MC
validation_electron=[
    'HLT_e60_lhmedium_nod0',
    'HLT_e60_lhmedium_nod0_L1EM24VHI',
    'HLT_e140_lhloose_nod0',
    'HLT_e140_lhloose_nod0_L1EM24VHI',
    'HLT_e60_lhmedium_nod0_ringer_L1EM24VHI',
    'HLT_e140_lhloose_nod0_ringer_L1EM24VHI',
    'HLT_e60_lhloose',
    'HLT_e70_lhloose_nod0',
    ]
validation_electron.extend(monitoring_electron)
validationTP_Jpsiee = ['HLT_e5_lhtight_nod0','HLT_e9_lhtight_nod0','HLT_e14_lhtight_nod0']

validationTP_electron = [
    'HLT_e0_perf_L1EM15',
    'HLT_e7_lhmedium_nod0',
    'HLT_e12_lhloose_nod0',
    'HLT_e12_lhmedium_nod0',
    'HLT_e17_lhloose_nod0',
    'HLT_e17_lhvloose_nod0',
    'HLT_e19_lhvloose_nod0',
    'HLT_e19_lhvloose_nod0_ringer',
    'HLT_e24_lhmedium_nod0_L1EM20VH',
    'HLT_e26_lhloose_nod0',
    'HLT_e26_lhmedium_nod0',
    'HLT_e26_lhtight_nod0',
    'HLT_e26_lhloose_nod0_ringer',
    'HLT_e26_lhmedium_nod0_ringer',
    'HLT_e26_lhtight_nod0_ringer',
    'HLT_e26_lhtight_idperf',
    'HLT_e26_lhmedium_nod0_ivarmedium_icalomedium',
    'HLT_e26_lhmedium_nod0_ringer_ivarmedium_icalomedium',
    'HLT_e28_lhmedium_nod0_ivarmedium_icalomedium',
    'HLT_e28_lhmedium_nod0_ringer_ivarmedium_icalomedium',
    'HLT_e28_lhmedium_nod0',
    'HLT_e28_lhtight_nod0',
    'HLT_e28_lhtight_idperf'
    ]
validationTP_electron.extend(monitoringTP_electron)
validation_photon = [
    'HLT_g0_perf_L1EM15',
    'HLT_g10_loose',
    'HLT_g10_medium',
    'HLT_g15_loose_L1EM8VH',
    'HLT_g20_tight_icalovloose_L1EM15VHI',
    'HLT_g20_tight_icalotight_L1EM15VHI',
    'HLT_g22_tight_icalovloose_L1EM15VHI',
    'HLT_g22_tight_icalotight_L1EM15VHI',
    'HLT_g25_loose',
    'HLT_g25_loose_L1EM20VH',
    'HLT_g25_medium',
    'HLT_g25_medium_L1EM20VH',
    'HLT_g25_tight_L1EM20VH',
    'HLT_g35_loose',
    'HLT_g35_loose_L1EM20VH',
    'HLT_g35_medium',
    'HLT_g25_medium_L1EM20VH',
    'HLT_g35_tight_icalotight_L1EM24VHI',
    'HLT_g40_tight_icalotight_L1EM24VHI',
    'HLT_g45_tight_L1EM24VHI',
    'HLT_g50_loose_L1EM20VH',
    'HLT_g60_loose_L1EM20VH',
    'HLT_g80_loose_icalovloose_L1EM24VHI',
    'HLT_g85_tight_icalovloose_L1EM24VHI',
    'HLT_g100_tight_L1EM24VHI',
    'HLT_g140_tight',
    'HLT_g200_loose'
    ]
validation_photon.extend(monitoring_photon)
validation_mam=monitoring_mam

# Heavy Ion items
monitoring_electron_hi=['HLT_e15_etcut','HLT_e15_loose','HLT_e15_lhloose','HLT_e15_lhloose_nod0','HLT_e17_lhloose','HLT_e17_lhloose_nod0']
monitoring_photon_hi=['HLT_g13_etcut','HLT_g18_etcut','HLT_g15_loose','HLT_g20_loose']
monitoring_mam_hi = {'primary_single_ele_cutbased':'HLT_e15_loose',
                     'primary_single_ele':'HLT_e15_lhloose_nod0',
                     'primary_single_pho':'HLT_g15_loose'}


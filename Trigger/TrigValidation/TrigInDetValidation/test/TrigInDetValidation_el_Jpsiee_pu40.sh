#!/bin/bash
# art-description: art job for el_Jpsiee_pu40
# art-type: grid
# art-output: HLTEF-plots
# art-output: HLTL2-plots
# art-output: times
# art-output: times-FTF
# art-output: cost-perCall
# art-output: cost-perEvent
# art-output: cost-perCall-chain
# art-output: cost-perEvent-chain
# art-input:  mc15_13TeV.129190.Pythia8_AU2CTEQ6L1_ppToJpsie3e3.recon.RDO.e3802_s2608_s2183_r7042
# art-output: *.dat 
# art-output: *.root
# art-output: *.log

export RTTJOBNAME=TrigInDetValidation_el_Jpsiee_pu40

get_files -jo            TrigInDetValidation/TrigInDetValidation_RTT_topOptions_ElectronSlice.py
athena.py  -c 'ARTConfig=["/eos/atlas/atlascerngroupdisk/proj-sit/trigindet/mc15_13TeV.129190.Pythia8_AU2CTEQ6L1_ppToJpsie3e3.recon.RDO.e3802_s2608_s2183_r7042/RDO.06758877._000006.pool.root.1"];           EventMax=10000;runMergedChain=True'            TrigInDetValidation/TrigInDetValidation_RTT_topOptions_ElectronSlice.py
get_files -data TIDAdata11-rtt.dat
get_files -data TIDAdata_cuts.dat
get_files -data TIDAdata_chains.dat
get_files -data TIDAbeam.dat
get_files -data Test_bin.dat
TIDArdict.exe TIDAdata11-rtt.dat -f data-electron-merge.root -p 11 -b Test_bin.dat

TIDArun-art.sh data-electron-merge.root data-el_Jpsiee_pu40-reference.root HLT_e5_loose_idperf_InDetTrigTrackingxAODCnv_Electron_FTF  HLT_e5_loose_idperf_InDetTrigTrackingxAODCnv_Electron_IDTrig -d HLTEF-plots

TIDArun-art.sh data-electron-merge.root data-el_Jpsiee_pu40-reference.root HLT_e5_loose_idperf_InDetTrigTrackingxAODCnv_Electron_FTF  -d HLTL2-plots

TIDArun-art.sh expert-monitoring.root  expert-monitoring*-ref.root --auto -o times

TIDArun-art.sh expert-monitoring.root  expert-monitoring*-ref.root --auto -p FastTrack -o times-FTF

RunTrigCostD3PD.exe -f trig_cost.root  --outputTagFromAthena --costMode --linkOutputDir

TIDAcpucost.exe costMon/TrigCostRoot_Results.root costMon/TrigCostRoot_Results.root -o cost-perCall --auto -d "/Algorithm" -p "_Time_perCall"

TIDAcpucost.exe costMon/TrigCostRoot_Results.root costMon/TrigCostRoot_Results.root -o cost-perEvent --auto -d "/Algorithm" -p "_Time_perEvent"

TIDAcpucost.exe costMon/TrigCostRoot_Results.root costMon/TrigCostRoot_Results.root -o cost-perCall-chain --auto -d "/Chain_Algorithm" -p "_Time_perCall"

TIDAcpucost.exe costMon/TrigCostRoot_Results.root costMon/TrigCostRoot_Results.root -o cost-perEvent-chain --auto -d "/Chain_Algorithm" -p "_Time_perEvent"


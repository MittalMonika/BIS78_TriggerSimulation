#!/bin/bash
# art-description: athenaMT trigger test using the LS2_v1 jet menu from TriggerMenuMT 
# art-type: build
# art-include: master/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

export EVENTS=20
export THREADS=1
export SLOTS=1
export JOBOPTION="TrigUpgradeTest/full_menu.py"
export REGTESTEXP="TrigSignatureMoniMT.*HLT_.*"
export EXTRA="isOnline=True;doEmptyMenu=True;doJetSlice=True;doWriteBS=False;doWriteRDOTrigger=True;"

# Find the regtest reference installed with the release
export REGTESTREF=`find_data.py TrigUpgradeTest/slice_jet.ref`

source exec_TrigUpgradeTest_art_athenaMT.sh
source exec_TrigUpgradeTest_art_post.sh

#!/bin/sh
# art-description: athenaMT trigger test using the MET menu from TrigUpgradeTest job options
# art-type: build
# art-include: master/Athena
# art-output: *.log
# art-output: *.root
# art-output: *.regtest.new

export NAME="TrigUpgradeTest_met_menu"
export EVENTS=50
export THREADS=1
export SLOTS=1
export JOBOPTION="TrigUpgradeTest/full_menu.py"
export REGTESTEXP="TrigSignatureMoniMT.*HLT_.*"
export EXTRA="doEmptyMenu=True;doMETSlice=True"

# Find the regtest reference installed with the release
export REGTESTREF=`find_data.py TrigUpgradeTest/met_menu.ref`

source exec_TrigUpgradeTest_art_athenaMT.sh
source exec_TrigUpgradeTest_art_post.sh


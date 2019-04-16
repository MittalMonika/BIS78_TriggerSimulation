#!/bin/sh
# art-description: athenaMT trigger test using the full menu from TrigUpgradeTest job options
# art-type: grid
# art-include: master/Athena
# art-output: *.log
# art-output: *.new
# art-output: *.txt
# art-output: *.root

export NAME="TrigUpgradeTest_full_menu_grid"
export INPUT="run2dataFTK" # FTK doesn't matter here - using this dataset because of larger stats
export EVENTS=1000
export THREADS=1
export SLOTS=1
export JOBOPTION="TrigUpgradeTest/full_menu.py"
export REGTESTEXP="TriggerSummaryStep.*HLT_.*|TriggerMonitorFinal.*HLT_.*|TrigSignatureMoniMT.*HLT_.*"

source exec_TrigUpgradeTest_art_athenaMT.sh
source exec_TrigUpgradeTest_art_post.sh


#!/bin/bash
# art-description: athenaMT trigger test of the data scouting (TLA) infrastructure
# art-type: build
# art-include: master/Athena
# art-output: *.log
# art-output: *.new
# art-output: *.txt
# art-output: *.root

export SKIPEVENTS=10
export EVENTS=20
export THREADS=1
export SLOTS=1
export JOBOPTION="TrigUpgradeTest/dataScoutingTest.py"
export REGTESTEXP="obeysLB=.*robs=.*dets = |adds PEBInfo|TriggerSummary.*HLT_.*|Serialiser.*Type|Serialiser.*ROBFragments with IDs|{module.*words}"

# Find the regtest reference installed with the release
export REGTESTREF=`find_data.py TrigUpgradeTest/dataScouting.ref`

source exec_TrigUpgradeTest_art_athenaMT.sh
source exec_TrigUpgradeTest_art_post.sh

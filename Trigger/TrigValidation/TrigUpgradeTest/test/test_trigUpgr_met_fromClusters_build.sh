#!/bin/bash
# art-description: athenaMT trigger test of MET from clusters algorithm
# art-type: build
# art-include: master/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

export EVENTS=10
export THREADS=1
export SLOTS=1
export JOBOPTION="TrigUpgradeTest/met.fromClusters.py"
export REGTESTEXP="METHypoAlg.*MET.*value"

# Find the regtest reference installed with the release
export REGTESTREF=`find_data.py TrigUpgradeTest/met_fromClusters.ref`

# Skip dumping chain counts because this test doesn't produce the histogram including them
export SKIP_CHAIN_DUMP=1

source exec_TrigUpgradeTest_art_athenaMT.sh
source exec_TrigUpgradeTest_art_post.sh
#!/bin/bash

# art-description: Bphysics slice TriggerTest on MC using Jpsimu6mu6 to AOD
# art-type: grid
# art-include: 21.1/AthenaP1
# art-include: 21.0/Athena
# art-include: 21.3/Athena
# art-include: master/Athena
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.new
# art-output: *.json
# art-output: *.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: costMonitoring_*
# art-output: HLTconfig*.xml
# art-output: L1Topoconfig*.xml
# art-output: LVL1config*.xml

export NAME="slice_bphysics_grid"
export SLICE="bphysics"
export INPUT="bphysics"

source exec_athena_art_trigger_validation.sh
source exec_art_triggertest_post.sh

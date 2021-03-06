#!/bin/bash

# art-description: Trigger Physics pp v8 RDO to BS and BS to AOD test
# art-type: grid
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
# art-output: *.check*
# art-output: HLTconfig*.xml
# art-output: L1Topoconfig*.xml
# art-output: LVL1config*.xml

export NAME="physics_pp_v8_rdotobstoesd_grid"
export COST_MONITORING="False"
export TEST="TrigAnalysisTest"
export MENU="Physics_pp_v8"
export EVENTS="100"
export JOBOPTION="TrigAnalysisTest/testAthenaTrigRDOtoBS.py"

source exec_athena_art_trigger_validation.sh

echo $(date "+%FT%H:%M %Z")"     Running checklog for ${JOB_LOG}"
timeout 5m check_log.py --errors --config checklogTriggerTest.conf --showexcludestats ${JOB_LOG}  2>&1 | tee checklog_BS.log

echo "art-result: ${PIPESTATUS[0]} CheckLog BS"

export JOB_LOG="athena_ESD.log"

#the ConditionsTag should match what was used in the RDOtoBS step
athena.py -c "jp.AthenaCommonFlags.BSRDOInput=['raw.data'];from AthenaCommon.GlobalFlags import globalflags;globalflags.ConditionsTag.set_Value_and_Lock('OFLCOND-MC16-SDR-25')" TrigAnalysisTest/testAthenaTrigBStoESD.py | tee ${JOB_LOG%%.*}.${JOB_LOG#*.}
echo "art-result: ${PIPESTATUS[0]} ${JOB_LOG%%.*}"


source exec_art_triggertest_post.sh



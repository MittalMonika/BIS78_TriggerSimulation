#!/bin/bash

echo  $(date "+%FT%H:%M %Z")"     Execute TrigP1Test post processing"

### DEFAULTS

if [ -z ${JOB_LOG} ]; then
  export JOB_LOG="atn_test.log"
fi

if [ -z ${TEST} ]; then
  export TEST="TrigP1Test"
fi

if [ -z ${REF_FOLDER} ]; then
  export REF_FOLDER="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/${TEST}/ref/${AtlasBuildBranch}/${NAME}"
fi

###

echo $(date "+%FT%H:%M %Z")"     Running checklog"
timeout 1m check_log.pl --config checklogTriggerTest.conf --showexcludestats ${JOB_LOG} | tee checklog.log

echo "art-result: ${PIPESTATUS[0]} CheckLog"

# TODO
# add check_statuscode.py ${JOB_LOG}
# RunMsgFinder

# Run perfmon
timeout 1m perfmon.py -f 0.90 ntuple.pmon.gz
timeout 1m convert -density 300 -trim ntuple.perfmon.pdf -quality 100 -resize 50% ntuple.perfmon.png

echo $(date "+%FT%H:%M %Z")"     Running chainDump"
timeout 1m chainDump.py -S --rootFile=expert-monitoring.root

#export JOB_LOG_TAIL=${JOB_LOG%%.*}.tail.${JOB_LOG#*.}
export JOB_LOG_TAIL="atn_tail.log"
tail -10000  ${JOB_LOG} > ${JOB_LOG_TAIL}

grep REGTEST ${JOB_LOG} > athena.regtest
if [ -f ${REF_FOLDER}/athena.regtest ]; then
  echo $(date "+%FT%H:%M %Z")"     Running regtest"
  timeout 1m regtest.pl --inputfile athena.regtest --reffile ${REF_FOLDER}/athena.regtest | tee regtest.log
  echo "art-result: ${PIPESTATUS[0]} RegTest"
else
  echo $(date "+%FT%H:%M %Z")"     No reference athena.regtest found in ${REF_FOLDER}"
  echo "art-result: 999 RegTest"
fi

mv athena.regtest athena.regtest.new

if [ -f ${REF_FOLDER}/expert-monitoring.root ]; then
  echo $(date "+%FT%H:%M %Z")"     Running rootcomp"
  timeout 10m rootcomp.py ${REF_FOLDER}/expert-monitoring.root | tee rootcompout.log
  echo "art-result: ${PIPESTATUS[0]} RootComp"
  echo $(date "+%FT%H:%M %Z")"     Running checkcounts"
  timeout 10m trigtest_checkcounts.sh 0 expert-monitoring.root ${REF_FOLDER}/expert-monitoring.root HLT | tee checkcountout.log
  echo "art-result: ${PIPESTATUS[0]} CheckCounts"
else
  echo $(date "+%FT%H:%M %Z")"     No reference expert-monitoring.root found in ${REF_FOLDER}"
  echo "art-result:  999 RootComp"
  echo "art-result:  999 CheckCounts"
fi

if [ -f trig_cost.root ]; then 
  echo $(date "+%FT%H:%M %Z")"     Running CostMon"
#  timeout 2h RunTrigCostD3PD -f trig_cost.root --outputTagFromAthena --costMode --ratesMode > costMon.log 2>&1
  timeout 2h RunTrigCostD3PD -f trig_cost.root --outputTagFromAthena --costMode --monitorRates --isCPUPrediction --useEBWeight --patternsMonitor HLT_costmonitor HLT_mistimemonj400 HLT_mistimemoncaltimenomu HLT_mistimemoncaltime HLT_l1topodebug HLT_l1calooverflow HLT_e5_lhvloose_nod0_bBeexM6000t HLT_2e5_lhvloose_nod0_bBeexM6000t HLT_cscmon_L1All HLT_j0_perf_ds1_L1J100 --patternsInvert  --predictionLumi 1.50e34 > costMon.log 2>&1
else 
  echo $(date "+%FT%H:%M %Z")"     file trig_cost.root does not exist thus RunTrigCostD3PD will not be run"
fi

echo $(date "+%FT%H:%M %Z")"     Running check for zero L1, HLT or TE counts"
export COUNT_EXIT=0
if [[ `sed 's|.*\(.* \)|\1|' L1AV.txt | sed 's/^[ \t]*//' |  sed '/^0/'d | wc -l` == 0 ]]; then 
  echo "L1 counts   ERROR  : all entires are ZERO please consult L1AV.txt"
  (( COUNT_EXIT = COUNT_EXIT || 1 ))
fi
if [[ `sed 's|.*\(.* \)|\1|' HLTChain.txt | sed 's/^[ \t]*//' |  sed '/^0/'d | wc -l` == 0 ]]; then 
  echo "HLTChain counts   ERROR  : all entires are ZERO please consult HLTChain.txt"
  (( COUNT_EXIT = COUNT_EXIT || 1 ))
fi
if [[ `sed 's|.*\(.* \)|\1|' HLTTE.txt | sed 's/^[ \t]*//' |  sed '/^0/'d | wc -l` == 0 ]]; then 
  echo "HLTTE counts   ERROR  : all entires are ZERO please consult HLTTE.txt"
  (( COUNT_EXIT = COUNT_EXIT || 1 ))
fi
echo "art-result: ${COUNT_EXIT} ZeroCounts"


if [ -f ESD.pool.root ]; then 
  echo $(date "+%FT%H:%M %Z")"     Running CheckFile on ESD"
  timeout 10m checkFile.py ESD.pool.root | tee ESD.pool.root.checkFile
  echo "art-result: ${PIPESTATUS[0]} CheckFileESD"
else 
  echo $(date "+%FT%H:%M %Z")"     No ESD.pool.root to check"
fi

if [ -f AOD.pool.root ]; then 
  echo $(date "+%FT%H:%M %Z")"     Running CheckFile on AOD"
  timeout 10m checkFile.py AOD.pool.root | tee AOD.pool.root.checkFile
  echo "art-result: ${PIPESTATUS[0]} CheckFileAOD"
  echo $(date "+%FT%H:%M %Z")"     Running CheckxAOD AOD"
  timeout 10m checkxAOD.py AOD.pool.root | tee AOD.pool.root.checkxAODFile
  echo "art-result: ${PIPESTATUS[0]} CheckXAOD"
else 
  echo $(date "+%FT%H:%M %Z")"     No AOD.pool.root to check"
fi

echo  $(date "+%FT%H:%M %Z")"     Files in directory:"
ls -lh

echo  $(date "+%FT%H:%M %Z")"     Finished TrigP1Test post processing"




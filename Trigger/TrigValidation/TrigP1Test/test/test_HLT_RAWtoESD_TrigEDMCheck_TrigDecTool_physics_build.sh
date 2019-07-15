#!/bin/bash

# art-description: athenaHLT on EB data with MC_pp_v8 menu (temporarily replaced from physics menu), RAWtoESD on output, EDM checker on ESD, and Check of TrigDec 
# art-type: build
# art-include: 21.1/AthenaP1
# art-include: 21.3/Athena
# art-include: master/Athena

if [ -z ${TEST} ]; then
  export TEST="TrigP1Test"
fi

export NAME=HLT_mcV8
export JOB_LOG="${NAME}.log"

timeout 100m trigtest_ART.pl --cleardir --test ${NAME} --rundir ${NAME} --conf TrigP1Test_ART.conf | tee ${JOB_LOG}

ATH_RETURN=${PIPESTATUS[0]}
echo "art-result: ${ATH_RETURN} ${NAME}"

export NAME=RAWtoESD_mcV8
export JOB_LOG="${NAME}.log"

timeout 100m trigtest_ART.pl --cleardir  --test ${NAME} --rundir ${NAME} --conf TrigP1Test_ART.conf | tee ${JOB_LOG}

ATH_RETURN=${PIPESTATUS[0]}
echo "art-result: ${ATH_RETURN} ${NAME}"

export NAME=TrigEDMCheck_mcV8
export JOB_LOG="${NAME}.log"

timeout 50m trigtest_ART.pl --cleardir  --test ${NAME} --rundir ${NAME} --conf TrigP1Test_ART.conf | tee ${JOB_LOG}

ATH_RETURN=${PIPESTATUS[0]}
echo "art-result: ${ATH_RETURN} ${NAME}"

export NAME=TrigDecTool_mcV8
export JOB_LOG="${NAME}.log"

timeout 50m trigtest_ART.pl --cleardir  --test ${NAME} --rundir ${NAME} --conf TrigP1Test_ART.conf | tee ${JOB_LOG}

ATH_RETURN=${PIPESTATUS[0]}
echo "art-result: ${ATH_RETURN} ${NAME}"



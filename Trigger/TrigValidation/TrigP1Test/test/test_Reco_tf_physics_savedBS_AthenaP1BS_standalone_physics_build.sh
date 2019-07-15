#!/bin/bash

# art-description: athenaHLT on EB data with Physics_pp menu, Reco_tf on saved output, and Standalone HLT running in athena and compare to athenaHLT 
# art-type: build
# art-include: 21.1/AthenaP1
# art-include: 21.3/Athena
# art-include: master/Athena

if [ -z ${TEST} ]; then
  export TEST="TrigP1Test"
fi

export NAME=HLT_physicsV8
export JOB_LOG="${NAME}.log"

timeout 100m trigtest_ART.pl --cleardir --test ${NAME} --rundir ${NAME} --conf TrigP1Test_ART.conf | tee ${JOB_LOG}

ATH_RETURN=${PIPESTATUS[0]}
echo "art-result: ${ATH_RETURN} ${NAME}"

export NAME=Reco_tf_physicsV8_savedBS
export JOB_LOG="${NAME}.log"

timeout 100m trigtest_ART.pl --cleardir  --test ${NAME} --rundir ${NAME} --conf TrigP1Test_ART.conf | tee ${JOB_LOG}

ATH_RETURN=${PIPESTATUS[0]}
echo "art-result: ${ATH_RETURN} ${NAME}"

export NAME=AthenaP1BS_standalone
export JOB_LOG="${NAME}.log"

timeout 100m trigtest_ART.pl --cleardir  --test ${NAME} --rundir ${NAME} --conf TrigP1Test_ART.conf | tee ${JOB_LOG}

ATH_RETURN=${PIPESTATUS[0]}
echo "art-result: ${ATH_RETURN} ${NAME}"




#!/bin/bash

# art-description: athenaHLT standalone with HLT_HIV5 menu on a HI sample 
# art-type: build
# art-include: 21.1/AthenaP1
# art-include: 21.3/Athena
# art-include: master/Athena

if [ -z ${TEST} ]; then
  export TEST="TrigP1Test"
fi

export NAME=HLT_MCHIV5
export JOB_LOG="${NAME}.log"

timeout 100m trigtest_ART.pl --cleardir --test ${NAME} --rundir ${NAME} --conf TrigP1Test_ART.conf | tee ${JOB_LOG}

ATH_RETURN=${PIPESTATUS[0]}
echo "art-result: ${ATH_RETURN} ${NAME}"




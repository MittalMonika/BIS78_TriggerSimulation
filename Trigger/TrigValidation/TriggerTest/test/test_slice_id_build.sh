#!/bin/bash

# art-description: ID TriggerTest over e28_lhtight_idperf on MC
# art-type: build

export EVENTS="10"
export JOBOPTIONS="testIDAthenaTrigRDO.py"

source exec_athena_art_trigger_validation.sh
source exec_art_triggertest_post.sh

return $ATH_EXIT

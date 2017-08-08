#!/bin/bash

# art-description: Tau slice TriggerTest on MC
# art-type: build

export SLICE="tau"
export EVENTS="10"

source exec_athena_art_trigger_validation.sh
source exec_art_triggertest_post.sh

return $ATH_EXIT

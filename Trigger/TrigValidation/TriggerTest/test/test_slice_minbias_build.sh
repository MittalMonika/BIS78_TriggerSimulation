#!/bin/bash

# art-description: MinBias slice TriggerTest on MinBias MC
# art-type: build

export SLICE="minbias"
export MENU="Physics_pp_v7"
export EVENTS="10"
export INPUT="minbias"

source exec_athena_art_trigger_validation.sh
source exec_art_triggertest_post.sh

return $ATH_EXIT

#!/bin/bash

# art-description: Heavy ion physics v3 TriggerTest on MC
# art-type: grid

export MENU="Physics_HI_v3"
export EVENTS="500"
export INPUT="pbpb"

source exec_athena_art_trigger_validation.sh
source exec_art_triggertest_post.sh

return $ATH_EXIT

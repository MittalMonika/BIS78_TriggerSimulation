#!/bin/bash

# art-description: Events processed by hardcoded chains with data requests, accepted randomly and sent to Full Event build, PEB and DS streams in athenaHLT (--nprocs 2 --threads 2 --concurrent-events 2)
# art-type: build
# art-include: master/Athena

export NAME=CalibPeb_forkAndMT
# use export ART_FILE_NAME=... to set the INPUT_FILE value 
export ART_CMD="athenaHLT.py --nprocs 2 --threads 2 --concurrent-events 2 -n 50 -f INPUT_FILE TrigExPartialEB/MTCalibPeb.py"
export ART_TIMEOUT="20m"

exec_art_trigp1test.sh


#!/bin/sh
#
# art-description: Athena runs topoclustering from an ESD file
# art-type: grid
# art-include: master/Athena
# art-athena-mt: 4

unset ATHENA_PROC_NUMBER

art.py createpoolfile

athena --threads=2 tauRec/run_tau_standalone.py | tee temp.log
echo "art-result: ${PIPESTATUS[0]}"

test_postProcessing_Errors.sh temp.log


#!/bin/sh

# art-description: DAOD building HIGG1D1 mc16
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt
# art-output: checkxAOD.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.12169019._004055.pool.root.1 --outputDAODFile art.pool.root --reductionConf HIGG1D1 --maxEvents -1 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12-08-40" ' 

echo "art-result: $? reco"

DAODMerge_tf.py --inputDAOD_HIGG1D1File DAOD_HIGG1D1.art.pool.root --outputDAOD_HIGG1D1_MRGFile art_merged.pool.root

echo "art-result: $? merge"

checkFile.py DAOD_HIGG1D1.art.pool.root > checkFile.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HIGG1D1.art.pool.root > checkxAOD.txt

echo "art-result: $?  checkxAOD"

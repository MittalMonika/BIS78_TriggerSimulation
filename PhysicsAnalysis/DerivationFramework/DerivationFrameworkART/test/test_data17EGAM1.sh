#!/bin/sh

# art-description: DAOD building EGAM1 data17
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt
# art-output: checkxAOD.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.12818484._004694.pool.root.1 --outputDAODFile art.pool.root --reductionConf EGAM1 --maxEvents -1 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12Onl-08-40" ' 

echo "art-result: $? reco"

DAODMerge_tf.py --inputDAOD_EGAM1File DAOD_EGAM1.art.pool.root --outputDAOD_EGAM1_MRGFile art_merged.pool.root

echo "art-result: $? merge"

checkFile.py DAOD_EGAM1.art.pool.root > checkFile.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_EGAM1.art.pool.root > checkxAOD.txt

echo "art-result: $?  checkxAOD"

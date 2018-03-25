#!/bin/sh

# art-description: DAOD building HION2 mc16
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt
# art-output: checkxAOD.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.12169019._004055.pool.root.1 --outputDAODFile art.pool.root --reductionConf HION2 --maxEvents -1 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12-08-40" ' 

DAODMerge_tf.py --inputDAOD_HION2File DAOD_HION2.art.pool.root --outputDAOD_HION2_MRGFile art_merged.pool.root
checkFile.py DAOD_HION2.art.pool.root > checkFile.txt
checkxAOD.py DAOD_HION2.art.pool.root > checkxAOD.txt
#!/bin/sh

# art-description: DAOD building TOPQ2 mc16
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.12169019._004055.pool.root.1 --outputDAODFile art.pool.root --reductionConf TOPQ2 --maxEvents 10000 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12-08-40" ' 

DAODMerge_tf.py --maxEvents 5 --inputDAOD_TOPQ2File DAOD_TOPQ2.art.pool.root --outputDAOD_TOPQ2_MRGFile art_merged.pool.root
checkFile.py DAOD_TOPQ2.art.pool.root > checkFile.txt
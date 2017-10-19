#!/bin/sh

# art-description: DAOD building HIGG4D3 mc16
# art-type: grid
# art-output: *.pool.root

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11866988._000378.pool.root.1 --outputDAODFile art.pool.root --reductionConf HIGG4D3 --maxEvents 10000 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12-08-40" ' 

DAODMerge_tf.py --maxEvents 5 --inputDAOD_HIGG4D3File DAOD_HIGG4D3.art.pool.root --outputDAOD_HIGG4D3_MRGFile art_merged.pool.root
checkFile.py DAOD_HIGG4D3.art.pool.root > checkFile.txt
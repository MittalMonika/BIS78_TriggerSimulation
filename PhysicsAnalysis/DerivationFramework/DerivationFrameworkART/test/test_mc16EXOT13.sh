#!/bin/sh

# art-description: DAOD building EXOT13 mc16
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt
# art-output: checkxAOD.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.12169019._004055.pool.root.1 --outputDAODFile art.pool.root --reductionConf EXOT13 --maxEvents -1 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12-08-40" ' 

DAODMerge_tf.py --inputDAOD_EXOT13File DAOD_EXOT13.art.pool.root --outputDAOD_EXOT13_MRGFile art_merged.pool.root
checkFile.py DAOD_EXOT13.art.pool.root > checkFile.txt
checkxAOD.py DAOD_EXOT13.art.pool.root > checkxAOD.txt
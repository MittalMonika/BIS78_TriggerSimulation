#!/bin/sh

# art-description: DAOD building BPHY10 data17BPHYSLS
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data17_13TeV.00337491.physics_BphysLS.merge.AOD.f873_m1885._lb0100._0001.1 --outputDAODFile art.pool.root --reductionConf BPHY10 --maxEvents -1 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12Onl-08-40" ' 

DAODMerge_tf.py --inputDAOD_BPHY10File DAOD_BPHY10.art.pool.root --outputDAOD_BPHY10_MRGFile art_merged.pool.root
checkFile.py DAOD_BPHY10.art.pool.root > checkFile.txt
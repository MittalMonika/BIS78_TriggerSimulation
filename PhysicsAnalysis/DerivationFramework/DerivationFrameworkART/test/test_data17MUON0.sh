#!/bin/sh

# art-description: DAOD building MUON0 data17
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data17_13TeV.00327342.physics_Main.merge.AOD.f838_m1824._lb0300._0001.1 --outputDAODFile art.pool.root --reductionConf MUON0 --maxEvents -1 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12Onl-08-40" ' 

DAODMerge_tf.py --inputDAOD_MUON0File DAOD_MUON0.art.pool.root --outputDAOD_MUON0_MRGFile art_merged.pool.root
checkFile.py DAOD_MUON0.art.pool.root > checkFile.txt
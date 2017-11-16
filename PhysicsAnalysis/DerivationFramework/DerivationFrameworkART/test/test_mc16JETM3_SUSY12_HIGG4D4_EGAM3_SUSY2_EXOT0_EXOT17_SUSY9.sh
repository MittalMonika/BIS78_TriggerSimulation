#!/bin/sh

# art-description: DAOD building JETM3 SUSY12 HIGG4D4 EGAM3 SUSY2 EXOT0 EXOT17 SUSY9 mc16
# art-type: grid
# art-output: *.pool.root

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.12169019._004055.pool.root.1 --outputDAODFile art.pool.root --reductionConf JETM3 SUSY12 HIGG4D4 EGAM3 SUSY2 EXOT0 EXOT17 SUSY9 --maxEvents 500  --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12-08-40" '  --passThrough True 

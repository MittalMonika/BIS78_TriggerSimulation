#!/bin/sh

# art-include
# art-description: DAOD building EGAM9 EXOT20 SUSY11 EXOT6 SUSY2 HIGG4D1 BPHY1 BPHY4 mc16
# art-type: grid
# art-output: *.pool.root

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.14795494._005958.pool.root.1 --outputDAODFile art.pool.root --reductionConf EGAM9 EXOT20 SUSY11 EXOT6 SUSY2 HIGG4D1 BPHY1 BPHY4 --maxEvents 500  --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12-08-46" '  --passThrough True 

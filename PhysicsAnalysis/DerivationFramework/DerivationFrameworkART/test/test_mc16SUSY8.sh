#!/bin/sh

# art-description: DAOD building SUSY8 mc16
# art-type: grid
# art-output: *.pool.root

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11866988._000378.pool.root.1 --outputDAODFile art.pool.root --reductionConf SUSY8 --maxEvents 5000

DAODMerge_tf.py --maxEvents 5 --inputDAOD_SUSY8File DAOD_SUSY8.art.pool.root --outputDAOD_SUSY8_MRGFile art_merged.pool.root

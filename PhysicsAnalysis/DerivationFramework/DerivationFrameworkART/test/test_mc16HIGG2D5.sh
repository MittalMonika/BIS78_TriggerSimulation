#!/bin/sh

# art-description: DAOD building HIGG2D5 mc16
# art-type: grid
# art-output: *.pool.root

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11866988._000378.pool.root.1 --outputDAODFile art.pool.root --reductionConf HIGG2D5 --maxEvents 5000

DAODMerge_tf.py --maxEvents 5 --inputDAOD_HIGG2D5File DAOD_HIGG2D5.art.pool.root --outputDAOD_HIGG2D5_MRGFile art_merged.pool.root

#!/bin/sh

# art-description: DAOD building JETM7 mc16
# art-type: grid
# art-output: *.pool.root

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11866988._000378.pool.root.1 --outputDAODFile art.pool.root --reductionConf JETM7 --maxEvents 5000

DAODMerge_tf.py --maxEvents 5 --inputDAOD_JETM7File DAOD_JETM7.art.pool.root --outputDAOD_JETM7_MRGFile art_merged.pool.root

#!/bin/sh

# art-description: DAOD building BPHY10 mc16
# art-type: grid
# art-output: *.pool.root

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11866988._000378.pool.root.1 --outputDAODFile art.pool.root --reductionConf BPHY10 --maxEvents 5000

DAODMerge_tf.py --maxEvents 5 --inputDAOD_BPHY10File DAOD_BPHY10.art.pool.root --outputDAOD_BPHY10_MRGFile art_merged.pool.root

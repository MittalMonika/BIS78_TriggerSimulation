#!/bin/sh

# art-description: DAOD building BPHY11 mc16
# art-type: grid
# art-output: *.pool.root

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11866988._000378.pool.root.1 --outputDAODFile art.pool.root --reductionConf BPHY11 --maxEvents 5000

DAODMerge_tf.py --maxEvents 5 --inputDAOD_BPHY11File DAOD_BPHY11.art.pool.root --outputDAOD_BPHY11_MRGFile art_merged.pool.root

#!/bin/sh

# art-description: DAOD building HIGG4D5 mc16
# art-type: grid
# art-output: *.pool.root

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11866988._000378.pool.root.1 --outputDAODFile art.pool.root --reductionConf HIGG4D5 --maxEvents 5000

DAODMerge_tf.py --maxEvents 5 --inputDAOD_HIGG4D5File DAOD_HIGG4D5.art.pool.root --outputDAOD_HIGG4D5_MRGFile art_merged.pool.root

#!/bin/sh

# art-description: DAOD building EGAM7 data17
# art-type: grid
# art-output: *.pool.root

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data17_13TeV.00327342.physics_Main.merge.AOD.f838_m1824._lb0300._0001.1 --outputDAODFile art.pool.root --reductionConf EGAM7 --maxEvents 5000

DAODMerge_tf.py --maxEvents 5 --inputDAOD_EGAM7File DAOD_EGAM7.art.pool.root --outputDAOD_EGAM7_MRGFile art_merged.pool.root

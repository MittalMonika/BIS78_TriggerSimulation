#!/bin/sh
set -e

Reco_tf.py  --outputHISTFile=myMergedMonitoring_Egamma_0.root --maxEvents=300 --outputESDFile=myESD_Egamma_0.pool.root --geometryVersion=ATLAS-R1-2012-03-00-00 --outputAODFile=myAOD_Egamma_0.AOD.pool.root --outputTAGFile=myTAG_Egamma_0.root --ignoreErrors=False --conditionsTag=COMCOND-BLKPA-RUN1-05 --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data12_8TeV.00204073.physics_Egamma.merge.RAW._lb0144._SFO-5._0001.1,/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data12_8TeV.00204073.physics_Egamma.merge.RAW._lb0145._SFO-5._0001.1 --autoConfiguration=everything --preExec='rec.doTrigger=False;'


#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid

Reco_tf.py  --outputHISTFile=myMergedMonitoring_express_1.root --AMI=f628 --maxEvents=25 --outputESDFile=myESD_express_1.pool.root --outputAODFile=myAOD_express_1.AOD.pool.root --outputTAGFile=myTAG_express_1.root --ignoreErrors=False --inputBSFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data15_13TeV.00276689.express_express.merge.RAW._lb0222._SFO-ALL._0001.1,/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data15_13TeV.00276689.express_express.merge.RAW._lb0223._SFO-ALL._0001.1

Reco_tf.py --inputAODFile myAOD_express_1.AOD.pool.root --outputTAGFile myTAG_express_1.root


SCRIPT_DIRECTORY=$1
PACKAGE=$2
TYPE=$3
TEST_NAME=$4
NIGHTLY_RELEASE=$5
PROJECT=$6
PLATFORM=$7
NIGHTLY_TAG=$8

art.py compare grid $NIGHTLY_RELEASE $PROJECT $PLATFORM $NIGHTLY_TAG $PACKAGE $TEST_NAME myAOD_express_1.AOD.pool.root myESD_express_1.pool.root

#!/bin/sh
#
# art-description: Run digitization of an MC15 ttbar sample with 2015 geometry and conditions, 25ns pile-up, switching off noise and using digitizationFlags.doXingByXingPileUp=True
# art-type: grid


Digi_tf.py --inputHITSFile HITS.04919495.*.pool.root.? --conditionsTag default:OFLCOND-RUN12-SDR-25 --digiSeedOffset1 170 --digiSeedOffset2 170 --doAllNoise False --geometryVersion ATLAS-R2-2015-03-01-00 --inputHighPtMinbiasHitsFile HITS.05098387.*.pool.root.? --inputLowPtMinbiasHitsFile HITS.05098374.*.pool.root.? --jobNumber 1 --maxEvents 25 --numberOfCavernBkg 0 --numberOfHighPtMinBias 0.12268057 --numberOfLowPtMinBias 39.8773194 --outputRDOFile mc15_2015_ttbar.RDO.pool.root  --postExec 'all:CfgMgr.MessageSvc().setError+=["HepMcParticleLink"]' 'HITtoRDO:from AthenaCommon.CfgGetter import getPublicTool;getPublicTool("MergeMcEventCollTool").OnlySaveSignalTruth=True;ToolSvc.LArAutoCorrTotalToolDefault.deltaBunch=1'  --postInclude 'default:PyJobTransforms/UseFrontier.py' --pileupFinalBunch 6 --preExec 'all:from AthenaCommon.BeamFlags import jobproperties;jobproperties.Beam.numberOfCollisions.set_Value_and_Lock(20.0);from LArROD.LArRODFlags import larRODFlags;larRODFlags.NumberOfCollisions.set_Value_and_Lock(20);larRODFlags.nSamples.set_Value_and_Lock(4);larRODFlags.doOFCPileupOptimization.set_Value_and_Lock(True);larRODFlags.firstSample.set_Value_and_Lock(0);larRODFlags.useHighestGainAutoCorr.set_Value_and_Lock(True)'  --preInclude 'HITtoRDO:Digitization/ForceUseOfPileUpTools.py,SimulationJobOptions/preInclude.PileUpBunchTrainsMC15_2015_25ns_Config1.py,RunDependentSimData/configLumi_run222525_v1.py' --skipEvents 0

SCRIPT_DIRECTORY=$1
PACKAGE=$2
TYPE=$3
TEST_NAME=$4
NIGHTLY_RELEASE=$5
PROJECT=$6
PLATFORM=$7
NIGHTLY_TAG=$8

# TODO This is a regression test I think. We would also need to compare these files to fixed references and add DCube tests
art.py compare grid $NIGHTLY_RELEASE $PROJECT $PLATFORM $NIGHTLY_TAG $PACKAGE $TEST_NAME mc15_2015_ttbar.RDO.pool.root

#!/bin/sh
#
# art-description: RecoTrf
# art-type: grid

Reco_tf.py --inputHITSFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs/valid1.110401.PowhegPythia_P2012_ttbar_nonallhad.simul.HITS.e3099_s2578_tid04919495_00/HITS.04919495._0004*.pool.root*" --AMITag="r6594" --autoConfiguration="everything" --conditionsTag "default:OFLCOND-RUN12-SDR-28" --digiSeedOffset1="64" --digiSeedOffset2="64" --geometryVersion="default:ATLAS-R2-2015-03-01-00" --inputHighPtMinbiasHitsFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs/mc15_valid.361035.Pythia8EvtGen_A2MSTW2008LO_minbias_inelastic_high.merge.HITS.e3581_s2578_s2169_tid05098387_00/HITS.05098387._0023*.pool.root*" --inputLowPtMinbiasHitsFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs/mc15_valid.361034.Pythia8EvtGen_A2MSTW2008LO_minbias_inelastic_low.merge.HITS.e3581_s2578_s2169_tid05098374_00/HITS.05098374._0021*.pool.root*" --jobNumber="64" --maxEvents="150" --numberOfCavernBkg="0" --numberOfHighPtMinBias="0.12268057" --numberOfLowPtMinBias="39.8773194" --outputAODFile="myAOD.pool.root" --outputESDFile="myESD.pool.root" --outputRDOFile="myRDO.pool.root" --outputTAGFile=myTAG.root --postExec "all:CfgMgr.MessageSvc().setError+=[\"HepMcParticleLink\"]" "HITtoRDO:job.StandardPileUpToolsAlg.PileUpTools[\"MergeMcEventCollTool\"].OnlySaveSignalTruth=True" "RAWtoESD:ToolSvc.LArAutoCorrTotalToolDefault.deltaBunch="1"" "HITtoRDO:ToolSvc.LArAutoCorrTotalToolDefault.deltaBunch=1" --postInclude "default:RecJobTransforms/UseFrontier.py" --preExec "all:rec.Commissioning.set_Value_and_Lock(True);rec.doTrigger.set_Value_and_Lock(True);from AthenaCommon.BeamFlags import jobproperties;jobproperties.Beam.numberOfCollisions.set_Value_and_Lock(20.0);from LArROD.LArRODFlags import larRODFlags;larRODFlags.NumberOfCollisions.set_Value_and_Lock(20);larRODFlags.nSamples.set_Value_and_Lock(4);larRODFlags.doOFCPileupOptimization.set_Value_and_Lock(True);larRODFlags.firstSample.set_Value_and_Lock(0);larRODFlags.useHighestGainAutoCorr.set_Value_and_Lock(True)" "RAWtoESD:from TriggerJobOpts.TriggerFlags import TriggerFlags;TriggerFlags.triggerMenuSetup=\"MC_pp_v6_loose_mc_prescale\";from CaloRec.CaloCellFlags import jobproperties;jobproperties.CaloCellFlags.doLArCellEmMisCalib=False" "ESDtoAOD:TriggerFlags.AODEDMSet=\"AODFULL\"" "RDOtoRDOTrigger:from TriggerJobOpts.TriggerFlags import TriggerFlags;TriggerFlags.triggerMenuSetup=\"MC_pp_v6_loose_mc_prescale\";" "HITtoRDO:from AthenaCommon.BeamFlags import jobproperties;jobproperties.Beam.bunchSpacing.set_Value_and_Lock(50);" --preInclude "HITtoRDO:Digitization/ForceUseOfPileUpTools.py,SimulationJobOptions/preInclude.PileUpBunchTrainsMC15_2015_50ns_Config1.py,RunDependentSimData/configLumi_run222510.py" "RDOtoRDOTrigger:RecExPers/RecoOutputMetadataList_jobOptions.py" --runNumber="110401" --skipEvents="0" --steering "RAWtoESD:in-RDO,in+RDO_TRIG,in-BS" --pileupFinalBunch 6

export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
alias setupATLAS='source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh'
setupATLAS --quiet
lsetup rucio
SCRIPT_DIRECTORY=$1
PACKAGE=$2
TYPE=$3
TEST_NAME=$4
NIGHTLY_RELEASE=$5
PROJECT=$6
PLATFORM=$7
NIGHTLY_TAG=$8

art.py compare grid $NIGHTLY_RELEASE $PROJECT $PLATFORM $NIGHTLY_TAG $PACKAGE $TEST_NAME myAOD.pool.root myESD.pool.root

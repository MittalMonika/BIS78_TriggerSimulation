#!/bin/sh
#
# art-description: Test of DigiMReco from HITS to AOD/TAG
# art-type: grid

export TRF_ECHO=True; Reco_tf.py --inputHITSFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/HITS.508808._000857.pool.root.1 --outputESDFile my.ESD.pool.root --outputHISTFile my.HIST.pool.root --outputAODFile my.ADO.pool.root --outputTAGFile my.TAG.pool.root --maxEvents 10 --triggerConfig MCRECO:MC_pp_v5 --preInclude 'h2r:Digitization/ForceUseOfPileUpTools.py,SimulationJobOptions/preInclude.PileUpBunchTrains2011Config8_DigitConfig.py,RunDependentSimData/configLumi_mc11b_v1.py' --preExec 'h2r:from Digitization.DigitizationFlags import digitizationFlags;digitizationFlags.overrideMetadata+=["SimLayout","PhysicsList"];from AthenaMonitoring.DQMonFlags import DQMonFlags; DQMonFlags.doCaloMon.set_Value_and_Lock(False); DQMonFlags.doLArMon.set_Value_and_Lock(False);' --postExec 'h2r:ToolSvc.LArAutoCorrTotalToolDefault.NMinBias=0' 'e2a:TriggerFlags.AODEDMSet "AODSLIM";rec.Commissioning.set_Value_and_Lock(True);jobproperties.Beam.energy.set_Value_and_Lock(3500*Units.GeV);muonRecFlags.writeSDOs=True;jobproperties.Beam.numberOfCollisions.set_Value_and_Lock(8.0);jobproperties.Beam.bunchSpacing.set_Value_and_Lock(50)' 'r2e:RegSelSvc=topSequence.allConfigurables.get("RegSelSvcDefault");RegSelSvc.DeltaZ=225*Units.mm' 'r2e:rec.Commissioning.set_Value_and_Lock(True);jobproperties.Beam.energy.set_Value_and_Lock(3500*Units.GeV);muonRecFlags.writeSDOs=True;jobproperties.Beam.numberOfCollisions.set_Value_and_Lock(8.0);jobproperties.Beam.bunchSpacing.set_Value_and_Lock(50);from InDetTrigRecExample.ConfiguredNewTrackingTrigCuts import L2IDTrackingCuts;L2IDTrackingCuts.setRegSelZmax(225*Units.mm)'  --RunNumber 105200 --autoConfiguration everything --conditionsTag=OFLCOND-RUN12-SDR-22 --geometryVersion ATLAS-R2-2015-03-01-00 --numberOfLowPtMinBias 29.966 --numberOfHighPtMinBias 0.034 --LowPtMinbiasHitsFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/HITS.500617._000382.pool.root.1 --HighPtMinbiasHitsFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/HITS.500616._001496.pool.root.1 --DataRunNumber -1 --jobNumber 41 --digiSeedOffset1 41 --digiSeedOffset2 41


#!/usr/bin/env bash
# art-description: test for job configuration ttFC_fastSim_fulldigi (Sim/Digi job)
# art-type: grid
#

FastChain_tf.py --simulator ATLFASTIIF_PileUp --digiSteeringConf "SplitNoMerge" --useISF True --randomSeed 123 --enableLooperKiller True --inputEVNTFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastChainPileup/ttbar_muplusjets-pythia6-7000.evgen.pool.root --outputRDOFile RDO_pileup_fastsim_fulldigi.pool.root --maxEvents 100 --skipEvents 0 --geometryVersion ATLAS-R2-2015-03-01-00 --conditionsTag OFLCOND-RUN12-SDR-31 --preSimExec 'from TrkDetDescrSvc.TrkDetDescrJobProperties import TrkDetFlags;TrkDetFlags.TRT_BuildStrawLayers=True' --preSimInclude FastChainPileup/FastPileup.py --postInclude='PyJobTransforms/UseFrontier.py,G4AtlasTests/postInclude.DCubeTest_FCpileup.py,DigitizationTests/postInclude.RDO_Plots.py' --postExec 'from AthenaCommon.ConfigurationShelve import saveToAscii;saveToAscii("config.txt")' --DataRunNumber '284500' --postSimExec='genSeq.Pythia8.NCollPerEvent=10;' 

echo "art-result: $? evgen step"

ArtPackage=$1
ArtJobName=$2
art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName}  
echo  "art-result: $? regression"
#rootcomp.py -o comparison -c /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/ISF_G4_WriteCalHitsTest.truth.root truth.root
echo  "art-result: $? histcomp"

#!/bin/sh

# art-include: 21.2/AthDerivation
# art-description: DAOD building HION3 data18
# art-type: grid
# art-output: *.pool.root
# art-output: checkFile.txt
# art-output: checkxAOD.txt

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data17_5TeV.00340910.physics_Main.merge.AOD.f911_m1917._lb0525._0003.1 --outputDAODFile art.pool.root --reductionConf HION3 --maxEvents -1 --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12Onl-08-46"; from AthenaCommon.AlgSequence import AlgSequence; topSequence = AlgSequence(); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( "InDetTrackParticlesFixer", Containers = [ "InDetTrackParticlesAux." ] )' 

echo "art-result: $? reco"

DAODMerge_tf.py --inputDAOD_HION3File DAOD_HION3.art.pool.root --outputDAOD_HION3_MRGFile art_merged.pool.root

echo "art-result: $? merge"

checkFile.py DAOD_HION3.art.pool.root > checkFile.txt

echo "art-result: $?  checkfile"

checkxAOD.py DAOD_HION3.art.pool.root > checkxAOD.txt

echo "art-result: $?  checkxAOD"

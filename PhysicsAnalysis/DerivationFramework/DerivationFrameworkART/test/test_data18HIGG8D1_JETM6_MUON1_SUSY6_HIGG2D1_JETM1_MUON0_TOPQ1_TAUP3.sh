#!/bin/sh

# art-include: 21.2/AthDerivation
# art-description: DAOD building HIGG8D1 JETM6 MUON1 SUSY6 HIGG2D1 JETM1 MUON0 TOPQ1 TAUP3 data18
# art-type: grid
# art-output: *.pool.root

set -e

Reco_tf.py --inputAODFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data18_13TeV.00364292.physics_Main.merge.AOD.f1002_m2037._lb0163._0006.1 --outputDAODFile art.pool.root --reductionConf HIGG8D1 JETM6 MUON1 SUSY6 HIGG2D1 JETM1 MUON0 TOPQ1 TAUP3 --maxEvents 500  --preExec 'rec.doApplyAODFix.set_Value_and_Lock(True); from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = "BTagCalibRUN12Onl-08-49"; from AthenaCommon.AlgSequence import AlgSequence; topSequence = AlgSequence(); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( "InDetTrackParticlesFixer", Containers = [ "InDetTrackParticlesAux." ] )'  --passThrough True 

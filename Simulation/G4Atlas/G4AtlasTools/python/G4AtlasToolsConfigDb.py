# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addTool
addTool("G4AtlasTools.G4AtlasToolsConfig.getFastSimulationMasterTool","FastSimulationMasterTool")
addTool("G4AtlasTools.G4AtlasToolsConfig.getEmptyFastSimulationMasterTool","EmptyFastSimulationMasterTool")
addTool("G4AtlasTools.G4AtlasToolsConfig.getEmptySensitiveDetectorMasterTool","EmptySensitiveDetectorMasterTool")
addTool("G4AtlasTools.G4AtlasToolsConfig.getSensitiveDetectorMasterTool","SensitiveDetectorMasterTool")
addTool("G4AtlasTools.G4AtlasToolsConfig.getPhysicsListToolBase","PhysicsListToolBase")
addTool("G4AtlasTools.G4AtlasToolsConfig.getReducedStepSizeUserLimitsTool","ReducedStepSizeUserLimitsTool")

addTool("G4AtlasTools.G4GeometryConfig.getBeamPipeGeoDetectorTool", 'BeamPipe')
addTool("G4AtlasTools.G4GeometryConfig.getPixelGeoDetectorTool", 'Pixel')
addTool("G4AtlasTools.G4GeometryConfig.getSCTGeoDetectorTool", 'SCT')
addTool("G4AtlasTools.G4GeometryConfig.getTRTGeoDetectorTool", 'TRT')
addTool("G4AtlasTools.G4GeometryConfig.getIDetServicesMatGeoDetectorTool", 'IDetServicesMat')
addTool("G4AtlasTools.G4GeometryConfig.getLArMgrGeoDetectorTool", 'LArMgr')
addTool("G4AtlasTools.G4GeometryConfig.getTileGeoDetectorTool", 'Tile')
addTool("G4AtlasTools.G4GeometryConfig.getLucidGeoDetectorTool", 'Lucid')
addTool("G4AtlasTools.G4GeometryConfig.getALFAGeoDetectorTool", 'ALFA')
addTool("G4AtlasTools.G4GeometryConfig.getZDCGeoDetectorTool", 'ZDC')
addTool("G4AtlasTools.G4GeometryConfig.getAFPGeoDetectorTool", 'AFP')
addTool("G4AtlasTools.G4GeometryConfig.getFwdRegionGeoDetectorTool", 'FwdRegion')
addTool("G4AtlasTools.G4GeometryConfig.getMuonGeoDetectorTool", 'Muon')
addTool("G4AtlasTools.G4GeometryConfig.getCavernInfraGeoDetectorTool", 'CavernInfra')
addTool("G4AtlasTools.G4GeometryConfig.getIDETEnvelope", "IDET")
addTool("G4AtlasTools.G4GeometryConfig.getCALOEnvelope", "CALO")
addTool("G4AtlasTools.G4GeometryConfig.getForwardRegionEnvelope", 'ForwardRegion')
addTool("G4AtlasTools.G4GeometryConfig.getMUONEnvelope", "MUONQ02") #FIXME rename to MUON when safe
addTool("G4AtlasTools.G4GeometryConfig.getCosmicShortCut", "CosmicShortCut")
addTool("G4AtlasTools.G4GeometryConfig.getATLAS", "Atlas")
addTool("G4AtlasTools.G4GeometryConfig.getCavernWorld", "Cavern")
addTool("G4AtlasTools.G4GeometryConfig.getG4AtlasDetectorConstructionTool", "G4AtlasDetectorConstructionTool")
addTool("G4AtlasTools.G4GeometryConfig.getMaterialDescriptionTool", "MaterialDescriptionTool")

addTool("G4AtlasTools.G4TestBeamGeometryConfig.getTileTB_Beampipe1", "TileTB_BeamPipe1")
addTool("G4AtlasTools.G4TestBeamGeometryConfig.getTileTB_Beampipe2", "TileTB_BeamPipe2")
addTool("G4AtlasTools.G4TestBeamGeometryConfig.getTileTB_MYLAREQUIV", "TileTB_MYLAREQUIV")
addTool("G4AtlasTools.G4TestBeamGeometryConfig.getTileTB_S1", "TileTB_S1")
addTool("G4AtlasTools.G4TestBeamGeometryConfig.getTileTB_S2", "TileTB_S2")
addTool("G4AtlasTools.G4TestBeamGeometryConfig.getTileTB_S3", "TileTB_S3")
addTool("G4AtlasTools.G4TestBeamGeometryConfig.getTileTB_CALOEnvelope", "TileTB_CALO")
addTool("G4AtlasTools.G4TestBeamGeometryConfig.getTileTB_WorldEnvelope", "TileTB_World")
addTool("G4AtlasTools.G4TestBeamGeometryConfig.getLArTB_WorldEnvelope", "LArTB_World")

addTool("G4AtlasTools.G4PhysicsRegionConfig.getBeampipeFwdCutPhysicsRegionTool", 'BeampipeFwdCutPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getFWDBeamLinePhysicsRegionTool", 'FWDBeamLinePhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getFwdRegionPhysicsRegionTool", 'FwdRegionPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getPixelPhysicsRegionTool", 'PixelPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getSCTPhysicsRegionTool", 'SCTPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getTRTPhysicsRegionTool", 'TRTPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getTRT_ArPhysicsRegionTool", 'TRT_ArPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getTRT_KrPhysicsRegionTool", 'TRT_KrPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getEMBPhysicsRegionTool", 'EMBPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getEMECPhysicsRegionTool", 'EMECPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getHECPhysicsRegionTool", 'HECPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getFCALPhysicsRegionTool", 'FCALPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getEMECParaPhysicsRegionTool", 'EMECParaPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getFCALParaPhysicsRegionTool", 'FCALParaPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getFCAL2ParaPhysicsRegionTool", 'FCAL2ParaPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getPreSampLArPhysicsRegionTool", 'PreSampLArPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getDeadMaterialPhysicsRegionTool", 'DeadMaterialPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getDriftWallPhysicsRegionTool", 'DriftWallPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getDriftWall1PhysicsRegionTool", 'DriftWall1PhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getDriftWall2PhysicsRegionTool", 'DriftWall2PhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getMuonSystemFastPhysicsRegionTool", 'MuonSystemFastPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getMuonPhysicsRegionTool", "MuonPhysicsRegionTool")
addTool("G4AtlasTools.G4PhysicsRegionConfig.getSX1PhysicsRegionTool", 'SX1PhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getBedrockPhysicsRegionTool", 'BedrockPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getCavernShaftsConcretePhysicsRegionTool", 'CavernShaftsConcretePhysicsRegionTool')
##addTool("G4AtlasTools.G4PhysicsRegionConfig.getCavernShaftsAirPhysicsRegionTool", 'CavernShaftsAirPhysicsRegionTool')
addTool("G4AtlasTools.G4PhysicsRegionConfig.getSCTSiliconPhysicsRegionTool", 'SCTSiliconPhysicsRegionTool')

addTool("G4AtlasTools.G4FieldConfig.getATLASFieldManagerTool", 'ATLASFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getTightMuonsATLASFieldManagerTool", 'TightMuonsATLASFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getClassicFieldManagerTool", 'ClassicFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getBeamPipeFieldManagerTool", 'BeamPipeFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getInDetFieldManagerTool", 'InDetFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getMuonsOnlyInCaloFieldManagerTool", 'MuonsOnlyInCaloFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getMuonFieldManagerTool", 'MuonFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ1FwdFieldMangerTool", 'Q1FwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ2FwdFieldMangerTool", 'Q2FwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ3FwdFieldMangerTool", 'Q3FwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getD1FwdFieldMangerTool", 'D1FwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getD2FwdFieldMangerTool", 'D2FwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ4FwdFieldMangerTool", 'Q4FwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ5FwdFieldMangerTool", 'Q5FwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ6FwdFieldMangerTool", 'Q6FwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ7FwdFieldMangerTool", 'Q7FwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ1HKickFwdFieldMangerTool", 'Q1HKickFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ1VKickFwdFieldMangerTool", 'Q1VKickFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ2HKickFwdFieldMangerTool", 'Q2HKickFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ2VKickFwdFieldMangerTool", 'Q2VKickFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ3HKickFwdFieldMangerTool", 'Q3HKickFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ3VKickFwdFieldMangerTool", 'Q3VKickFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ4VKickAFwdFieldMangerTool", 'Q4VKickAFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ4HKickFwdFieldMangerTool", 'Q4HKickFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ4VKickBFwdFieldMangerTool", 'Q4VKickBFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ5HKickFwdFieldMangerTool", 'Q5HKickFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getQ6VKickFwdFieldMangerTool", 'Q6VKickFwdFieldManager')
addTool("G4AtlasTools.G4FieldConfig.getFwdRegionFieldMangerTool", 'FwdRegionFieldManager')

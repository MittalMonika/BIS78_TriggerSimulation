# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from ISF_Algorithms.collection_merger_helpersNew import CollectionMergerCfg

from LArG4SD.LArG4SDConf import LArG4__EMBSDTool
from LArG4SD.LArG4SDConf import LArG4__EMECSDTool
from LArG4SD.LArG4SDConf import LArG4__FCALSDTool
from LArG4SD.LArG4SDConf import LArG4__HECSDTool
from LArG4SD.LArG4SDConf import LArG4__MiniFCALSDTool
from LArG4SD.LArG4SDConf import LArG4__DeadSDTool
from LArG4SD.LArG4SDConf import LArG4__ActiveSDTool
from LArG4SD.LArG4SDConf import LArG4__InactiveSDTool

#to be migrated: getCalibrationDefaultCalculator, getDeadMaterialCalibrationHitMerger

def LArActiveSensitiveDetectorToolCfg(ConfigFlags, name="LArActiveSensitiveDetector", **kwargs):
    ## Main configuration
    if ConfigFlags.GeoModel.AtlasVersion not in ["tb_LArH6_2003","tb_LArH6_2002"]:
        kwargs.setdefault("StacVolumes",["LArMgr::LAr::EMB::STAC"])
        kwargs.setdefault("PresamplerVolumes",["LArMgr::LAr::Barrel::Presampler::Module"])
        kwargs.setdefault("NegIWVolumes",["LArMgr::LAr::EMEC::Neg::InnerWheel"])
        kwargs.setdefault("NegOWVolumes",["LArMgr::LAr::EMEC::Neg::OuterWheel"])
        kwargs.setdefault("NegBOBarretteVolumes",["LArMgr::LAr::EMEC::Neg::BackOuterBarrette::Module::Phidiv"])
        kwargs.setdefault("MiniVolumes",["LArMgr::MiniFCAL::Wafer"])
    if ConfigFlags.GeoModel.AtlasVersion!="tb_LArH6_2003":
        kwargs.setdefault("PosIWVolumes",["LArMgr::LAr::EMEC::Pos::InnerWheel"])
        kwargs.setdefault("PosOWVolumes",["LArMgr::LAr::EMEC::Pos::OuterWheel"])
        kwargs.setdefault("PosBOBarretteVolumes",["LArMgr::LAr::EMEC::Pos::BackOuterBarrette::Module::Phidiv"])
        kwargs.setdefault("PresVolumes", ["LArMgr::LAr::Endcap::Presampler::LiquidArgon"])
        kwargs.setdefault("SliceVolumes",["LArMgr::LAr::HEC::Module::Depth::Slice"])
    if ConfigFlags.GeoModel.AtlasVersion not in ["tb_LArH6_2002"]:
        kwargs.setdefault("FCAL1Volumes",["LArMgr::LAr::FCAL::Module1::Gap"])
        kwargs.setdefault("FCAL2Volumes",["LArMgr::LAr::FCAL::Module2::Gap"])
        kwargs.setdefault("FCAL3Volumes",["LArMgr::LAr::FCAL::Module3::Gap"])
    # Running PID calibration hits?
    kwargs.setdefault("ParticleID",ConfigFlags.Sim.ParticleID)
    # No effect currently
    kwargs.setdefault("OutputCollectionNames", ["LArCalibrationHitActive"])
    return LArG4__ActiveSDTool(name, **kwargs)

def LArDeadSensitiveDetectorToolCfg(ConfigFlags, name="LArDeadSensitiveDetector", **kwargs):
    ## Main configuration
    kwargs.setdefault("BarrelCryVolumes",   ["LArMgr::LAr::Barrel::Cryostat::InnerWall::Vis",
                                             "LArMgr::LAr::Barrel::Cryostat::Sector::*",
                                             "LArMgr::LAr::Barrel::Cryostat::InnerWall",
                                             "LArMgr::LAr::Barrel::Cryostat::Cylinder::*"])
    kwargs.setdefault("BarrelCryLArVolumes",["LArMgr::LAr::Barrel::Cryostat::ExtraMat*",
                                             "LArMgr::LAr::Barrel::Cryostat::HalfLAr*",
                                             "LArMgr::LAr::Barrel::Cryostat::TotalLAr",
                                             "LArMgr::LAr::Barrel::Cryostat::MotherVolume"])
    kwargs.setdefault("BarrelCryMixVolumes",["LArMgr::LAr::Barrel::Cryostat::InnerEndWall",
                                             "LArMgr::LAr::Barrel::Cryostat::OuterWall",
                                             "LArMgr::LAr::Barrel::Cryostat::Mixed::Cylinder::*"])
    kwargs.setdefault("DeadMaterialVolumes",["LArMgr::LAr::DM::*"])
    kwargs.setdefault("BarrelPresVolumes",  ["LArMgr::LAr::Barrel::Presampler",
                                             "LArMgr::LAr::Barrel::Presampler::Sector",
                                             "LArMgr::LAr::Barrel::Presampler::ProtectionShell",
                                             "LArMgr::LAr::Barrel::Presampler::MotherBoard",
                                             "LArMgr::LAr::Barrel::Presampler::Connectics",
                                             "LArMgr::LAr::Barrel::Presampler::Rail",
                                             "LArMgr::LAr::Barrel::Presampler::ProtectionPlate"])
    kwargs.setdefault("BarrelVolumes",      ["LArMgr::LAr::EMB::ExtraMat*",
                                             "LArMgr::LAr::EMB::FrontBack::Absorber",
                                             "LArMgr::LAr::EMB::FrontBack::Absorber2",
                                             "LArMgr::LAr::EMB::FrontBack::Steel",
                                             "LArMgr::LAr::EMB::FrontBack::G10",
                                             "LArMgr::LAr::EMB::FrontBack::Electrode",
                                             "LArMgr::LAr::EMB::GTENB",
                                             "LArMgr::LAr::EMB::GTENF",
                                             "LArMgr::LAr::EMB::SUMB",
                                             "LArMgr::LAr::EMB::CAAC",
                                             "LArMgr::LAr::EMB::MOAC",
                                             "LArMgr::LAr::EMB::TELB",
                                             "LArMgr::LAr::EMB::TELF",
                                             "LArMgr::LAr::EMB::ECAM"])
    kwargs.setdefault("ECCryVolumes",       ["LArMgr::LAr::Endcap::Cryostat::FcalLAr::Cylinder",
                                             "LArMgr::LAr::Endcap::Cryostat::EmecHecLAr::Sector",
                                             "LArMgr::LAr::Endcap::Cryostat::EmecHecLAr::Cylinder",
                                             "LArMgr::LAr::Endcap::Cryostat::Sector",
                                             "LArMgr::LAr::Endcap::Cryostat::Cone",
                                             "LArMgr::LAr::Endcap::Cryostat::Cylinder"])
    kwargs.setdefault("ECCryLArVolumes",    ["LArMgr::Moderator*",
                                             "LArMgr::LAr::Endcap::Cryostat::FcalLAr",
                                             "LArMgr::LAr::Endcap::Cryostat::EmecHecLAr",
                                             "LArMgr::LAr::Endcap::Cryostat::MotherVolume"])
    kwargs.setdefault("ECCryMixVolumes",    ["LArMgr::LAr::FCAL::LiquidArgonC",
                                             "LArMgr::LAr::Endcap::Cryostat::EmecHecLAr::Sector::Mixed",
                                             "LArMgr::LAr::Endcap::Cryostat::Sector::Mixed",
                                             "LArMgr::LAr::Endcap::Cryostat::Cone::Mixed",
                                             "LArMgr::LAr::Endcap::Cryostat::ExtraCyl_beforePS",
                                             "LArMgr::LAr::Endcap::Cryostat::Cylinder::Mixed"])
    kwargs.setdefault("ECSupportVolumes",   ["LArMgr::LAr::EMEC::ExtraCyl_afterPS",
                                             "LArMgr::LAr::EMEC::InnerTransversalBars",
                                             "LArMgr::LAr::EMEC::InnerAluCone::*",
                                             "LArMgr::LAr::EMEC::OuterTransversalBars",
                                             "LArMgr::LAr::EMEC::OuterSupportMother",
                                             "LArMgr::LAr::EMEC::*Stretchers",
                                             "LArMgr::LAr::EMEC::Top*",
                                             "LArMgr::LAr::EMEC::Back*GTen",
                                             "LArMgr::LAr::EMEC::Back*Hole",
                                             "LArMgr::LAr::EMEC::Back*Bar",
                                             "LArMgr::LAr::EMEC::Back*Ring",
                                             "LArMgr::LAr::EMEC::Back*Ele",
                                             "LArMgr::LAr::EMEC::Back*Abs",
                                             "LArMgr::LAr::EMEC::BackInnerBarrette::Module::Phidiv",
                                             "LArMgr::LAr::EMEC::Back*Barrette::Module",
                                             "LArMgr::LAr::EMEC::Back*Barrettes",
                                             "LArMgr::LAr::EMEC::BackSupport*",
                                             "LArMgr::LAr::EMEC::Front*",
                                             "LArMgr::LAr::EMEC::Mother*"])
    kwargs.setdefault("HECWheelVolumes",    ["LArMgr::LAr::HEC::Mother",
                                             "LArMgr::LAr::HEC::LiquidArgon",
                                             "LArMgr::LAr::HEC::Clamp",
                                             "LArMgr::LAr::HEC::Clamp::LiquidArgon",
                                             "LArMgr::LAr::HEC::Module",
                                             "LArMgr::LAr::HEC::FrontModule",
                                             "LArMgr::LAr::HEC::RearModule",
                                             "LArMgr::LAr::HEC::Module::Depth",
                                             "LArMgr::LAr::HEC::Module::Depth::FirstAbsorber",
                                             "LArMgr::LAr::HEC::Module::Depth::FirstAbsorber::TieRod"])
    # Running PID calibration hits?
    kwargs.setdefault("ParticleID", ConfigFlags.Sim.ParticleID)
    kwargs.setdefault("doEscapedEnergy",ConfigFlags.Sim.CalibrationRun  !='DeadLAr')
    # No effect currently
    outputCollectionName = "LArCalibrationHitDeadMaterial"
    if ConfigFlags.Sim.CalibrationRun in ['LAr', 'LAr+Tile']:
        outputCollectionName = "LArCalibrationHitDeadMaterial_DEAD"
    kwargs.setdefault("HitCollectionName", outputCollectionName)
    return LArG4__DeadSDTool(name, **kwargs)

def LArEMBSensitiveDetectorCfg(ConfigFlags,name="LArEMBSensitiveDetector", **kwargs):

    result = ComponentAccumulator()
    bare_collection_name = "LArHitEMB"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "LArEMBHits"
    acc, hits_collection_name = CollectionMergerCfg(ConfigFlags, bare_collection_name,
                                                              mergeable_collection_suffix,
                                                              merger_input_property)
    ## Main configuration
    kwargs.setdefault("StacVolumes",["LArMgr::LAr::EMB::STAC"])
    kwargs.setdefault("PresamplerVolumes",["LArMgr::LAr::Barrel::Presampler::Module"])
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    # Hook for fast simulation
    #from G4AtlasApps.SimFlags import simFlags
    #kwargs.setdefault("UseFrozenShowers", simFlags.LArParameterization()>0)

    #Note - frozen showers to be migrated later

    result.merge(acc)
    return result, LArG4__EMBSDTool(name, **kwargs)

def LArEMECSensitiveDetectorCfg(ConfigFlags, name="LArEMECSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    bare_collection_name = "LArHitEMEC"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "LArEMECHits"
    acc, hits_collection_name = CollectionMergerCfg(ConfigFlags, bare_collection_name,
                                                              mergeable_collection_suffix,
                                                              merger_input_property)

    if ConfigFlags.GeoModel.AtlasVersion not in ["tb_LArH6_2002","tb_LArH6EC_2002"]:
        kwargs.setdefault("NegIWVolumes",["LArMgr::LAr::EMEC::Neg::InnerWheel"])
        kwargs.setdefault("NegOWVolumes",["LArMgr::LAr::EMEC::Neg::OuterWheel"])
        kwargs.setdefault("NegBOBarretteVolumes",["LArMgr::LAr::EMEC::Neg::BackOuterBarrette::Module::Phidiv"])
    if ConfigFlags.GeoModel.AtlasVersion !="tb_LArH6EC_2002":
        kwargs.setdefault("PosIWVolumes",["LArMgr::LAr::EMEC::Pos::InnerWheel"])
        kwargs.setdefault("PosOWVolumes",["LArMgr::LAr::EMEC::Pos::OuterWheel"])
        kwargs.setdefault("PosBOBarretteVolumes",["LArMgr::LAr::EMEC::Pos::BackOuterBarrette::Module::Phidiv"])
    kwargs.setdefault("PresVolumes", ["LArMgr::LAr::Endcap::Presampler::LiquidArgon"])
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    # Hook for fast simulation
    #kwargs.setdefault("UseFrozenShowers", simFlags.LArParameterization()>0)

    #Note - frozen showers to be migrated later

    result.merge(acc)
    return result, LArG4__EMECSDTool(name, **kwargs)

def LArFCALSensitiveDetectorCfg(ConfigFlags, name="LArFCALSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    bare_collection_name = "LArHitFCAL"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "LArFCALHits"
    acc, hits_collection_name = CollectionMergerCfg(ConfigFlags, bare_collection_name,
                                                              mergeable_collection_suffix,
                                                              merger_input_property)
    kwargs.setdefault("FCAL1Volumes",["LArMgr::LAr::FCAL::Module1::Gap"])
    kwargs.setdefault("FCAL2Volumes",["LArMgr::LAr::FCAL::Module2::Gap"])
    kwargs.setdefault("FCAL3Volumes",["LArMgr::LAr::FCAL::Module3::Gap"])
    # No effect currently
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    # Hook for fast simulation
    #from G4AtlasApps.SimFlags import simFlags
    #kwargs.setdefault("UseFrozenShowers", simFlags.LArParameterization()>0)

    #Note - frozen showers to be migrated later

    result.merge(acc)
    return result, LArG4__FCALSDTool(name, **kwargs)

def LArHECSensitiveDetectorCfg(ConfigFlags, name="LArHECSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    bare_collection_name = "LArHitHEC"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "LArHECHits"
    acc, hits_collection_name = CollectionMergerCfg(ConfigFlags, bare_collection_name,
                                                              mergeable_collection_suffix,
                                                              merger_input_property)


    kwargs.setdefault("WheelVolumes",["LArMgr::LAr::HEC::Module::Depth::Slice"])
    #kwargs.setdefault("SliceVolumes",["LAr::HEC::Module::Depth::Slice"])
    #kwargs.setdefault("LocalVolumes",["LAr::HEC::Module::Depth::Slice::Local"])
    #  You might think this should go here, but we don't think so!  LAr::HEC::Module::Depth::Slice::Wheel"])
    # No effect currently
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    result.merge(acc)
    return result, LArG4__HECSDTool(name, **kwargs)

def LArInactiveSensitiveDetectorToolCfg(ConfigFlags, name="LArInactiveSensitiveDetector", **kwargs):
    ## Main configuration
    if ConfigFlags.GeoModel.AtlasVersion not in ["tb_LArH6_2003","tb_LArH6_2002"]:
        kwargs.setdefault("BarrelPreVolumes",["LArMgr::LAr::Barrel::Presampler::Cathode*",
                                              "LArMgr::LAr::Barrel::Presampler::Anode*",
                                              "LArMgr::LAr::Barrel::Presampler::Prep*"])
        kwargs.setdefault("BarrelVolumes",["LArMgr::LAr::EMB::*::Straight",
                                           "LArMgr::LAr::EMB::*::*Fold"])
        kwargs.setdefault("ECPosInVolumes", ["LArMgr::LAr::EMEC::Pos::InnerWheel::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::InnerWheel::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::InnerWheel::Glue",
                                             "LArMgr::LAr::EMEC::Pos::InnerWheel::Lead",
                                             "LArMgr::LAr::EMEC::Pos::InnerCone::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::InnerCone::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::InnerCone::Glue",
                                             "LArMgr::LAr::EMEC::Pos::InnerCone::Lead",
                                             "LArMgr::LAr::EMEC::Pos::InnerSlice*::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::InnerSlice*::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::InnerSlice*::Glue",
                                             "LArMgr::LAr::EMEC::Pos::InnerSlice*::Lead"])
        kwargs.setdefault("ECPosOutVolumes",["LArMgr::LAr::EMEC::Pos::OuterWheel::Lead",
                                             "LArMgr::LAr::EMEC::Pos::OuterWheel::Glue",
                                             "LArMgr::LAr::EMEC::Pos::OuterWheel::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::OuterWheel::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::Outer*Cone::Lead",
                                             "LArMgr::LAr::EMEC::Pos::Outer*Cone::Glue",
                                             "LArMgr::LAr::EMEC::Pos::Outer*Cone::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::Outer*Cone::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::OuterSlice*::Lead",
                                             "LArMgr::LAr::EMEC::Pos::OuterSlice*::Glue",
                                             "LArMgr::LAr::EMEC::Pos::OuterSlice*::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::OuterSlice*::Absorber"])
        kwargs.setdefault("ECNegInVolumes", ["LArMgr::LAr::EMEC::Neg::InnerWheel::Absorber",
                                             "LArMgr::LAr::EMEC::Neg::InnerWheel::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::InnerWheel::Glue",
                                             "LArMgr::LAr::EMEC::Neg::InnerWheel::Lead",
                                             "LArMgr::LAr::EMEC::Neg::InnerCone::Absorber",
                                             "LArMgr::LAr::EMEC::Neg::InnerCone::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::InnerCone::Glue",
                                             "LArMgr::LAr::EMEC::Neg::InnerCone::Lead",
                                             "LArMgr::LAr::EMEC::Neg::InnerSlice*::Absorber",
                                             "LArMgr::LAr::EMEC::Neg::InnerSlice*::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::InnerSlice*::Glue",
                                             "LArMgr::LAr::EMEC::Neg::InnerSlice*::Lead"])
        kwargs.setdefault("ECNegOutVolumes",["LArMgr::LAr::EMEC::Neg::OuterWheel::Lead",
                                             "LArMgr::LAr::EMEC::Neg::OuterWheel::Glue",
                                             "LArMgr::LAr::EMEC::Neg::OuterWheel::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::OuterWheel::Absorber",
                                             "LArMgr::LAr::EMEC::Neg::Outer*Cone::Lead",
                                             "LArMgr::LAr::EMEC::Neg::Outer*Cone::Glue",
                                             "LArMgr::LAr::EMEC::Neg::Outer*Cone::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::Outer*Cone::Absorber",
                                             "LArMgr::LAr::EMEC::Neg::OuterSlice*::Lead",
                                             "LArMgr::LAr::EMEC::Neg::OuterSlice*::Glue",
                                             "LArMgr::LAr::EMEC::Neg::OuterSlice*::Electrode",
                                             "LArMgr::LAr::EMEC::Neg::OuterSlice*::Absorber"])
        #kwargs.setdefault("HECVolumes",["LAr::HEC::Inactive"])
        #kwargs.setdefault("HECLocalVolumes",["LAr::HEC::Local::Inactive"])
        kwargs.setdefault("HECWheelVolumes",["LArMgr::LAr::HEC::Module::Depth::Absorber::TieRod",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::TieRodDead",
                                             "LArMgr::LAr::HEC::Module::Depth::Absorber",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::TieRod",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::Electrode::Copper",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::Electrode"])
        kwargs.setdefault("MiniMomVolumes",["LArMgr::MiniFCALMother"])
        kwargs.setdefault("MiniVolumes",["LArMgr::MiniFCAL"])
        kwargs.setdefault("MiniLayVolumes",["LArMgr::MiniFCAL::Layer"])
    if ConfigFlags.GeoModel.AtlasVersion=="tb_LArH6_2002":
        kwargs.setdefault("ECPosInVolumes", ["LArMgr::LAr::EMEC::Pos::InnerWheel::Absorber",
                                             "LArMgr::LAr::EMEC::Pos::InnerWheel::Electrode"])
        kwargs.setdefault("ECPosOutVolumes",["LArMgr::LAr::EMEC::Pos::OuterWheel::Electrode",
                                             "LArMgr::LAr::EMEC::Pos::OuterWheel::Absorber"])
        kwargs.setdefault("HECWheelVolumes",["LArMgr::LAr::HEC::Module::Depth::Absorber",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::Electrode",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::Electrode::Copper",
                                             "LArMgr::LAr::HEC::Module::Depth::Slice::TieRod"])
    if ConfigFlags.GeoModel.AtlasVersion!="tb_LArH6_2002":
        kwargs.setdefault("FCAL1Volumes",["LArMgr::LAr::FCAL::Module1::CableTrough",
                                          "LArMgr::LAr::FCAL::Module1::Absorber"])
        kwargs.setdefault("FCAL2Volumes",["LArMgr::LAr::FCAL::Module2::CableTrough",
                                          "LArMgr::LAr::FCAL::Module2::Absorber",
                                          "LArMgr::LAr::FCAL::Module2::Rod"])
        kwargs.setdefault("FCAL3Volumes",["LArMgr::LAr::FCAL::Module3::CableTrough",
                                          "LArMgr::LAr::FCAL::Module3::Absorber",
                                          "LArMgr::LAr::FCAL::Module3::Rod"])
    # Running PID calibration hits?
    kwargs.setdefault("ParticleID",ConfigFlags.Sim.ParticleID)
    # No effect currently
    kwargs.setdefault("OutputCollectionNames", ["LArCalibrationHitInactive"])
    return LArG4__InactiveSDTool(name, **kwargs)

def LArMiniFCALSensitiveDetectorToolCfg(ConfigFlags, name="LArMiniFCALSensitiveDetector", **kwargs):
    kwargs.setdefault("MiniVolumes",["LArMgr::MiniFCAL::Wafer"])
    # No effect currently
    kwargs.setdefault("OutputCollectionNames", ["LArHitMiniFCAL"])

    return LArG4__MiniFCALSDTool(name, **kwargs)
################################################################################
# Package: InDetPrepRawDataFormation
################################################################################

# Declare the package name:
atlas_subdir( InDetPrepRawDataFormation )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          AtlasPolicy
                          Control/AthenaBaseComps
                          Control/StoreGate
                          DetectorDescription/IRegionSelector
                          DetectorDescription/Identifier
                          GaudiKernel
                          InnerDetector/InDetRawEvent/InDetRawData
                          InnerDetector/InDetRecEvent/InDetPrepRawData
                          InnerDetector/InDetRecTools/SiClusterizationTool
                          Trigger/TrigEvent/TrigSteeringEvent
                          PRIVATE
                          Control/CxxUtils
                          DetectorDescription/AtlasDetDescr
                          InnerDetector/InDetConditions/InDetConditionsSummaryService
                          InnerDetector/InDetConditions/SCT_ConditionsServices
                          InnerDetector/InDetDetDescr/InDetIdentifier
                          InnerDetector/InDetDetDescr/InDetReadoutGeometry
                          InnerDetector/InDetRecTools/TRT_DriftCircleTool )

# Component(s) in the package:
atlas_add_component( InDetPrepRawDataFormation
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps StoreGateLib SGtests IRegionSelector Identifier GaudiKernel InDetRawData InDetPrepRawData SiClusterizationToolLib TrigSteeringEvent CxxUtils AtlasDetDescr InDetIdentifier InDetReadoutGeometry )

# Install files from the package:
atlas_install_headers( InDetPrepRawDataFormation )


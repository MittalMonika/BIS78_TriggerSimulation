################################################################################
# Package: SCT_Digitization
################################################################################

# Declare the package name:
atlas_subdir( SCT_Digitization )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          Commission/CommissionEvent
                          Control/AthenaBaseComps
                          Control/AthenaKernel
                          Control/PileUpTools
                          DetectorDescription/Identifier
                          Event/xAOD/xAODEventInfo
                          GaudiKernel
                          InnerDetector/InDetDigitization/SiDigitization
                          InnerDetector/InDetRawEvent/InDetRawData
                          InnerDetector/InDetSimEvent
                          Simulation/HitManagement
                          PRIVATE
                          Generators/GeneratorObjects
                          InnerDetector/InDetConditions/InDetConditionsSummaryService
                          InnerDetector/InDetConditions/SCT_ConditionsServices
                          InnerDetector/InDetConditions/SiPropertiesSvc
                          InnerDetector/InDetDetDescr/InDetIdentifier
                          InnerDetector/InDetDetDescr/InDetReadoutGeometry
                          InnerDetector/InDetDetDescr/SCT_ModuleDistortions
                          InnerDetector/InDetRawEvent/InDetSimData
                          Simulation/Tools/AtlasCLHEP_RandomGenerators )

# External dependencies:
find_package( Boost COMPONENTS filesystem thread system )
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( SCT_Digitization
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${Boost_LIBRARIES} ${CLHEP_LIBRARIES} CommissionEvent AthenaBaseComps AthenaKernel PileUpToolsLib Identifier xAODEventInfo GaudiKernel SiDigitization InDetRawData InDetSimEvent HitManagement GeneratorObjects SiPropertiesSvcLib InDetIdentifier InDetReadoutGeometry InDetSimData AtlasCLHEP_RandomGenerators )

# Install files from the package:
atlas_install_headers( SCT_Digitization )
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py )


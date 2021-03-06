# $Id: CMakeLists.txt 740701 2016-04-15 18:22:55Z ssnyder $
################################################################################
# Package: CaloTools
################################################################################

# Declare the package name:
atlas_subdir( CaloTools )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC
   Calorimeter/CaloConditions
   Calorimeter/CaloDetDescr
   Calorimeter/CaloEvent
   Calorimeter/CaloGeoHelpers
   Calorimeter/CaloIdentifier
   Calorimeter/CaloInterface
   Control/AthenaBaseComps
   Control/AthenaKernel
   Control/AthAllocators
   Control/StoreGate
   Database/AthenaPOOL/AthenaPoolUtilities
   DetectorDescription/AtlasDetDescr
   DetectorDescription/Identifier
   GaudiKernel
   LArCalorimeter/LArElecCalib
   LArCalorimeter/LArIdentifier
   LArCalorimeter/LArRecEvent
   LArCalorimeter/LArCabling
   LumiBlock/LumiBlockComps
   TileCalorimeter/TileConditions
   TileCalorimeter/TileEvent
   Trigger/TrigAnalysis/TrigAnalysisInterfaces
   PRIVATE
   AtlasTest/TestTools
   Calorimeter/CaloCondBlobObjs
   Control/AthAllocators
   DetectorDescription/GeoModel/GeoModelInterfaces
   DetectorDescription/IdDictParser
   Event/xAOD/xAODBase
   Event/xAOD/xAODEventInfo
   TileCalorimeter/TileIdentifier )

# External dependencies:
find_package( Boost )
find_package( CLHEP )
find_package( CORAL COMPONENTS CoralBase )
find_package( ROOT COMPONENTS Core MathCore )

# Component(s) in the package:
atlas_add_component( CaloTools
   CaloTools/*.h CaloTools/*.icc src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS}
   ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${Boost_LIBRARIES} ${CORAL_LIBRARIES}
   ${CLHEP_LIBRARIES} CaloConditions CaloDetDescrLib CaloEvent CaloGeoHelpers
   CaloIdentifier AthenaBaseComps AthenaKernel DataModel StoreGateLib
   AthenaPoolUtilities AtlasDetDescr Identifier GaudiKernel LArIdentifier
   LArRecEvent LArToolsLib LumiBlockCompsLib TileConditionsLib TileEvent
   CaloCondBlobObjs AthAllocators IdDictParser xAODBase xAODEventInfo
   TileIdentifier )

# Test(s) in the package:
atlas_add_test( CaloCellPackerUtils_test
   SOURCES test/CaloCellPackerUtils_test.cxx
   LINK_LIBRARIES GaudiKernel )

atlas_add_test( CaloCompactCellTool_test
   SOURCES test/CaloCompactCellTool_test.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} CaloDetDescrLib CaloEvent CaloIdentifier
   AthenaKernel StoreGateLib GaudiKernel LArRecEvent TileEvent TestTools
   IdDictParser
   EXTRA_PATTERNS "SGAudSvc +INFO|^lar decode|^HistogramPersis.* INFO|initialize_from_dict|mask/zero|^JobOptionsSvc +INFO|^AtlasDetectorID::"
   ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share" )

# Install files from the package:
atlas_install_headers( CaloTools )
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.txt share/*.py )

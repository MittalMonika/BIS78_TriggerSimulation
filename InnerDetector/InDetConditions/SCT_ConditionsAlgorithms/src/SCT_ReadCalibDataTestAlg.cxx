/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/** @file SCT_ReadCalibDataTestAlg.cxx Implementation file for SCT_ReadCalibDataTestAlg class
    @author: Per Johansson, based on Jorgen Dalmau TestReadSCT_CalibData example
    adapted first to test a tool and the a service
*/

// Include SCT_ReadCalibDataTestAlg and Svc
#include "SCT_ReadCalibDataTestAlg.h"

// Include Athena stuff
#include "Identifier/IdentifierHash.h"
#include "SCT_Cabling/ISCT_CablingSvc.h"
#include "InDetIdentifier/SCT_ID.h"

// Include Gaudi stuff

// Include STL stuff
#include <vector>
#include <string>

//----------------------------------------------------------------------
SCT_ReadCalibDataTestAlg::SCT_ReadCalibDataTestAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_sc{StatusCode::SUCCESS, true},
  m_id_sct{nullptr},
  m_moduleId{0},
  m_waferId{0},
  m_stripId{0},
  m_cabling{"SCT_CablingSvc", name},
  m_doTestmyConditionsSummary{false},
  m_doTestmyDefectIsGood{false},
  m_doTestmyDefectType{false},
  m_doTestmyDefectsSummary{false},
  m_doTestmyDefectList{false},
  m_moduleOfflinePosition{}
{
  declareProperty("DoTestmyConditionsSummary",   m_doTestmyConditionsSummary = false, "Test return bool conditions summary?");
  declareProperty("DoTestmyDefectIsGood",        m_doTestmyDefectIsGood      = false, "Test return defect type summary?");
  declareProperty("DoTestmyDefectType",          m_doTestmyDefectType        = false, "Test return defect type summary?");
  declareProperty("DoTestmyDefectsSummary",      m_doTestmyDefectsSummary    = false, "Test return module defects summary?");
  declareProperty("DoTestmyDefectList",          m_doTestmyDefectList        = false, "Test return defectList?");
  declareProperty("ModuleOfflinePosition",       m_moduleOfflinePosition            , "Offline pos. as: B-EC,layer-disk,phi,eta");
}

//----------------------------------------------------------------------
StatusCode SCT_ReadCalibDataTestAlg::initialize()
{
  // Print where you are
  ATH_MSG_DEBUG("in initialize()");
  
  // Get SCT ID helper
  m_sc = detStore()->retrieve(m_id_sct, "SCT_ID");
  if (m_sc.isFailure()) {
    ATH_MSG_FATAL("Failed to get SCT ID helper");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG("Found SCT detector manager");
  }
  
  // Process jobOption properties
  m_sc = processProperties();
  if (m_sc.isFailure()) {
    ATH_MSG_ERROR("Failed to process jobOpt properties");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG("Processed jobOpt properties");
  }

  // Get the SCT_ReadCaliDataTool
  m_sc = m_ReadCalibDataTool.retrieve();
  if (m_sc.isFailure()) {
    ATH_MSG_FATAL("Cannot locate CalibData service");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG( "CalibData Service located ");
  }

  // Retrieve SCT Cabling service
  m_sc = m_cabling.retrieve();
  if (m_sc.isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve SCT cabling service");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
} // SCT_ReadCalibDataTestAlg::initialize()

//----------------------------------------------------------------------
StatusCode SCT_ReadCalibDataTestAlg::processProperties()
{
  // Print where you are
  ATH_MSG_DEBUG("in processProperties()");
  
  // Get module position from jobOpt property
  std::vector<int>::const_iterator itLoop{m_moduleOfflinePosition.value().begin()};
  int offlineBarrelEC{*itLoop};  ++itLoop;
  int offlineLayerDisk{*itLoop}; ++itLoop;
  int offlineEta{*itLoop};       ++itLoop;
  int offlinePhi{*itLoop};       ++itLoop;
  int offlineSide{*itLoop};      ++itLoop;
  int offlineStrip{*itLoop};     ++itLoop;
  
  ATH_MSG_DEBUG("Module positions from jobOpt property:");
  ATH_MSG_DEBUG("B-EC/layer-disk/eta/phi: "
		<< offlineBarrelEC  << "/"
		<< offlineLayerDisk << "/"
		<< offlineEta       << "/"
		<< offlinePhi       << "/"
		<< offlineSide      << "/"
		<< offlineStrip);
  
  // Create offline Identifier for this module position, wafer,chip and strip
  m_moduleId = m_id_sct->module_id(offlineBarrelEC, offlineLayerDisk, offlinePhi, offlineEta);
  m_waferId = m_id_sct->wafer_id(offlineBarrelEC, offlineLayerDisk, offlinePhi, offlineEta, offlineSide);
  m_stripId = m_id_sct->strip_id(offlineBarrelEC, offlineLayerDisk, offlinePhi, offlineEta, offlineSide, offlineStrip);

  // Debug output
  ATH_MSG_DEBUG("id-getString : " << m_moduleId.getString());   // hex format
  ATH_MSG_DEBUG("id-getCompact: " << m_moduleId.get_compact()); // dec format
  ATH_MSG_DEBUG("id-getCompact2: " << m_stripId.get_compact()); // dec format
  ATH_MSG_DEBUG("Module Id: " << m_id_sct->print_to_string(m_moduleId));
  ATH_MSG_DEBUG("Strip Id: " << m_id_sct->print_to_string(m_stripId));
  
  return StatusCode::SUCCESS;
} // SCT_ReadCalibDataTestAlg::processProperties()

//----------------------------------------------------------------------
StatusCode SCT_ReadCalibDataTestAlg::execute()
{
  //This method is only used to test the summary service, and only used within this package,
  // so the INFO level messages have no impact on performance of these services when used by clients
  
  // Print where you are
  ATH_MSG_DEBUG("in execute()");
  
  //Test ConditionsSummary
  if (m_doTestmyConditionsSummary) {
    // Test summmary, ask status of strip in module
    Identifier IdM{m_moduleId};
    Identifier IdS{m_stripId};
    bool Sok{m_ReadCalibDataTool->isGood(IdS, InDetConditions::SCT_STRIP)};
    ATH_MSG_INFO("Strip " << IdS << " on module " << IdM << " is " << (Sok?"good":"bad"));
  }

  // Loop over all strips and check if good or not using isGood, and print the bad ones
  if (m_doTestmyDefectIsGood) {
    int ngood{0};
    int nbad{0};
    //Loop over all wafers using hashIds from the cabling service
    std::vector<boost::uint32_t> listOfRODs;
    m_cabling->getAllRods(listOfRODs);
    std::vector<boost::uint32_t>::iterator rodIter{listOfRODs.begin()};
    std::vector<boost::uint32_t>::iterator rodEnd{listOfRODs.end()};
    for (; rodIter != rodEnd; ++rodIter) {
      std::vector<IdentifierHash> listOfHashes;
      m_cabling->getHashesForRod(listOfHashes, *rodIter);
      std::vector<IdentifierHash>::iterator hashIt{listOfHashes.begin()};
      std::vector<IdentifierHash>::iterator hashEnd{listOfHashes.end()};
      for (; hashIt != hashEnd; ++hashIt) {
        Identifier waferId{m_id_sct->wafer_id(*hashIt)};
        //loop over all strips and check if good or not
        for (unsigned int stripIndex{0}; stripIndex<768; ++stripIndex) {
          Identifier IdS{m_id_sct->strip_id(waferId,stripIndex)};
          const int stripId{m_id_sct->strip(IdS)};
          const int side{m_id_sct->side(IdS)};
          const bool stripOk{m_ReadCalibDataTool->isGood(IdS, InDetConditions::SCT_STRIP)};
          if (stripOk) ++ngood;
	  else ++nbad; 
          if (not stripOk) { // Print info on all bad strips
            ATH_MSG_INFO("ModuleId/side/strip: " << m_id_sct->module_id(waferId) << "/" << side << "/" << stripId << " is bad");
          }
        }
      }
    }
    ATH_MSG_INFO("Number of good strips: " << ngood << " number of bad strips: " << nbad);
  }
               
  return StatusCode::SUCCESS;
} // SCT_ReadCalibDataTestAlg::execute()

//----------------------------------------------------------------------
StatusCode SCT_ReadCalibDataTestAlg::finalize()
{
  // Print where you are
  ATH_MSG_DEBUG("in finalize()");
  
  return StatusCode::SUCCESS;
} // SCT_ReadCalibDataTestAlg::finalize()

//----------------------------------------------------------------------

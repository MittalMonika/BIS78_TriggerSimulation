/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_ByteStreamErrorsSvc.cxx
 * implementation file for service that keeps track of errors in the bytestream
 * @author nbarlow@cern.ch
 **/

/// header file for this class.
#include "SCT_ByteStreamErrorsSvc.h"

///Athena includes
#include "InDetIdentifier/SCT_ID.h"

#include "SCT_Cabling/ISCT_CablingSvc.h"
#include "SCT_ConditionsServices/ISCT_ConfigurationConditionsSvc.h"
#include "InDetReadoutGeometry/SCT_DetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"

///Read Handle
#include "StoreGate/ReadHandle.h"

/** Constructor */
SCT_ByteStreamErrorsSvc::SCT_ByteStreamErrorsSvc( const std::string& name, ISvcLocator* pSvcLocator ) : 
  AthService(name, pSvcLocator), 
  m_detStore{"DetectorStore", name},
  m_config{"InDetSCT_ConfigurationConditionsSvc", name},
  m_sct_id{nullptr},
  m_pManager{nullptr},
  m_tempMaskedChips{},
  m_abcdErrorChips{},
  m_mutex{},
  m_cache{}
{
  declareProperty("DetectorStore", m_detStore);
  declareProperty("ContainerName", m_bsErrContainerName=std::string{"SCT_ByteStreamErrs"});
  declareProperty("FracContainerName", m_bsFracContainerName=std::string{"SCT_ByteStreamFrac"});
  declareProperty("ConfigService", m_config);
}

/** Initialize */
StatusCode 
SCT_ByteStreamErrorsSvc::initialize() {
  StatusCode sc{StatusCode::SUCCESS};

  for (int errorType{0}; errorType<SCT_ByteStreamErrors::NUM_ERROR_TYPES; errorType++) {
    m_bsErrors[errorType].clear();
  }

  /**  Get a detector store */
  if (m_detStore.retrieve().isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve service " << m_detStore);
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_INFO("Retrieved service " << m_detStore);
  }
 
  sc = m_detStore->retrieve(m_sct_id, "SCT_ID") ;
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Cannot retrieve SCT ID helper!");
    return StatusCode::FAILURE;
  } 
  m_cntx_sct = m_sct_id->wafer_context();

  sc = m_config.retrieve() ;
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Cannot retrieve ConfigurationConditionsSvc!");
    return StatusCode::FAILURE;
  } 
  
  ATH_CHECK(m_detStore->retrieve(m_pManager, "SCT"));

  // Read Handle Key
  ATH_CHECK(m_bsErrContainerName.initialize());
  ATH_CHECK(m_bsFracContainerName.initialize());

  return sc;
}

/** Finalize */
StatusCode
SCT_ByteStreamErrorsSvc::finalize() {
  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////

/** allow the Service to be retrieved via a ServiceHandle. */

StatusCode 
SCT_ByteStreamErrorsSvc::queryInterface(const InterfaceID& riid, void** ppvInterface) 
{
  if ( ISCT_ByteStreamErrorsSvc::interfaceID().versionMatch(riid) ) {
    *ppvInterface = this;
  } else if (ISCT_ConditionsSvc::interfaceID().versionMatch(riid) ) {
    /** Interface is not directly available : try out a base class */
    *ppvInterface =  dynamic_cast<ISCT_ConditionsSvc*>(this);
  } else {
    return AthService::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}

bool
SCT_ByteStreamErrorsSvc::isCondensedReadout() const {
  const SCT_ByteStreamFractionContainer* fracData{getFracData()};
  if (fracData==nullptr) return false;
  return fracData->majority(SCT_ByteStreamFractionContainer::CondensedMode);
}

bool
SCT_ByteStreamErrorsSvc::HVisOn() const {
  const SCT_ByteStreamFractionContainer* fracData{getFracData()};
  if (fracData==nullptr) return true;
  return fracData->majority(SCT_ByteStreamFractionContainer::HVOn);
}

/////////////////////////////////////////////////////////////////////////////

/** Used by ConditionsSummaySvc to decide whether to call isGood() for a particular
 * detector element.
 * Iin principle we could report about modules and/or strips too, and
 * use the id helper to navigate up or down the hierarchy to the wafer,
 * but in practice we don't want to do the time-consuming isGood() for 
 * every strip, so lets only report about wafers..
 */

bool 
SCT_ByteStreamErrorsSvc::canReportAbout(InDetConditions::Hierarchy h) {
  return (h==InDetConditions::SCT_SIDE or h==InDetConditions::SCT_CHIP);
}

///////////////////////////////////////////////////////////////////////////

/** this is the principle method which can be accessed via 
 * the ConditionsSummarySvc to decide if a wafer is good - in this
 * case we want to return false if the wafer has an error that would 
 * result in bad hits or no hits for that event */
 
bool 
SCT_ByteStreamErrorsSvc::isGood(const IdentifierHash& elementIdHash) {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  
  if (isRODSimulatedData()) return false;
  
  bool result{true};

  const std::vector<SCT_ByteStreamErrors::errorTypes> errorsToBeChecked{
      SCT_ByteStreamErrors::TimeOutError,
      SCT_ByteStreamErrors::BCIDError,
      SCT_ByteStreamErrors::LVL1IDError,
      SCT_ByteStreamErrors::MaskedLink,
      SCT_ByteStreamErrors::ROBFragmentError,
      SCT_ByteStreamErrors::MissingLinkHeaderError,
      SCT_ByteStreamErrors::MaskedROD};
  for (unsigned int i{0}; i<errorsToBeChecked.size(); i++) {
    const std::set<IdentifierHash>& errorSet{getErrorSet(errorsToBeChecked[i], ctx)};
    result = (std::find(errorSet.begin(), errorSet.end(), elementIdHash) == errorSet.end());
    if (not result) return result;
  }
  
  // If all 6 chips of a link issue ABCD errors or are bad chips or temporarily masked chips, the link is treated as bad one. 
  const Identifier wafer_id{m_sct_id->wafer_id(elementIdHash)};
  const Identifier module_id{m_sct_id->module_id(wafer_id)};
  unsigned int badChips{m_config->badChips(module_id)};
  unsigned int abcdErrorChips2{abcdErrorChips(module_id)};
  unsigned int tempMaskedChips2{tempMaskedChips(module_id)};
  const int side{m_sct_id->side(wafer_id)};
  bool allChipsBad{true};
  const int idMax{static_cast<short>(side==0 ? 6 : 12)};
  for (int id{idMax-6}; id<idMax; id++) {
    bool issueABCDError{((abcdErrorChips2 >> id) & 0x1) != 0};
    bool isBadChip{((badChips >> id) & 0x1) != 0};
    bool isTempMaskedChip{((tempMaskedChips2 >> id) & 0x1) != 0};
    allChipsBad = (issueABCDError or isBadChip or isTempMaskedChip);
    if (not allChipsBad) break;
  }
  if (allChipsBad) return false;
  
  return result;
}

bool 
SCT_ByteStreamErrorsSvc::isGood(const Identifier& elementId, InDetConditions::Hierarchy h) {
  if (not canReportAbout(h)) return true;
  
  if (isRODSimulatedData()) return false;

  if (h==InDetConditions::SCT_SIDE) {
    const IdentifierHash elementIdHash{m_sct_id->wafer_hash(elementId)};
    return isGood(elementIdHash);
  }
  if (h==InDetConditions::SCT_CHIP) {
    return isGoodChip(elementId);
  }

  return true;
}

bool
SCT_ByteStreamErrorsSvc::isGoodChip(const Identifier& stripId) {
  // This check assumes present SCT.
  // Get module number
  const Identifier moduleId{m_sct_id->module_id(stripId)};
  if (not moduleId.is_valid()) {
    ATH_MSG_WARNING("moduleId obtained from stripId " << stripId << " is invalid.");
    return false;
  }

  // tempMaskedChips and abcdErrorChips hold 12 bits.
  // bit 0 (LSB) is chip 0 for side 0.
  // bit 5 is chip 5 for side 0.
  // bit 6 is chip 6 for side 1.
  // bit 11 is chip 11 for side 1.
  // Temporarily masked chip information
  const unsigned int v_tempMaskedChips{tempMaskedChips(moduleId)};
  // Information of chips with ABCD errors
  const unsigned int v_abcdErrorChips{abcdErrorChips(moduleId)};
  // Take 'OR' of tempMaskedChips and abcdErrorChips
  const unsigned int badChips{v_tempMaskedChips | v_abcdErrorChips};

  // If there is no bad chip, this check is done.
  if (badChips==0) return true;

  const int side{m_sct_id->side(stripId)};
  // Check the six chips on the side
  // 0x3F  = 0000 0011 1111
  // 0xFC0 = 1111 1100 0000
  // If there is no bad chip on the side, this check is done.
  if ((side==0 and (badChips & 0x3F)==0) or (side==1 and (badChips & 0xFC0)==0)) return true;

  int chip{getChip(stripId)};
  if (chip<0 or chip>=12) {
    ATH_MSG_WARNING("chip number is invalid: " << chip);
    return false;
  }

  // Check if the chip is bad
  const bool badChip{static_cast<bool>(badChips & (1<<chip))};

  return (not badChip);
}

int
SCT_ByteStreamErrorsSvc::getChip(const Identifier& stripId) const {
  const InDetDD::SiDetectorElement* siElement{m_pManager->getDetectorElement(stripId)};
  if (siElement==nullptr) {
    ATH_MSG_DEBUG ("InDetDD::SiDetectorElement is not obtained from stripId " << stripId);
    return -1;
  }

  // Get strip number
  const int strip{m_sct_id->strip(stripId)};
  if (strip<0 or strip>=768) {
    // This check assumes present SCT.
    ATH_MSG_WARNING("strip number is invalid: " << strip);
    return -1;
  }

  // Conversion from strip to chip (specific for present SCT)
  int chip{strip/128}; // One ABCD chip reads 128 strips
  // Relation between chip and offline strip is determined by the swapPhiReadoutDirection method.
  // If swap is false
  //  offline strip:   0            767
  //  chip on side 0:  0  1  2  3  4  5
  //  chip on side 1: 11 10  9  8  7  6
  // If swap is true
  //  offline strip:   0            767
  //  chip on side 0:  5  4  3  2  1  0
  //  chip on side 1:  6  7  8  9 10 11
  const bool swap{siElement->swapPhiReadoutDirection()};
  const int side{m_sct_id->side(stripId)};
  if (side==0) {
    chip = swap ?  5 - chip :     chip;
  } else {
    chip = swap ? 11 - chip : 6 + chip;
  }

  return chip;
}

/////////////////////////////////////////////////////////////////////////

/** reset everything at the start of every event. */

void 
SCT_ByteStreamErrorsSvc::resetSets(const EventContext& ctx) {

  for (int errType{0}; errType<SCT_ByteStreamErrors::NUM_ERROR_TYPES; errType++) {
    m_bsErrors[errType][ctx.slot()].clear();
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////////////

/** The accessor method that can be used by clients to 
 * retrieve a set of IdHashes of wafers with a given type of error.
 * e.g. for monitoring plots.
 */

const std::set<IdentifierHash>* 
SCT_ByteStreamErrorsSvc::getErrorSet(int errorType) {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  if (errorType>=0 and errorType<SCT_ByteStreamErrors::NUM_ERROR_TYPES) {
    return &getErrorSet(static_cast<SCT_ByteStreamErrors::errorTypes>(errorType), ctx);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////

/** this function is used to populate the data of this service from 
 * the InDetBSErrContainer in StoreGate, loops through container,
 * calls addError() for each entry.
 */

StatusCode
SCT_ByteStreamErrorsSvc::fillData() {
  ATH_MSG_ERROR("fillData() is obsolete");
  return StatusCode::FAILURE;
}

StatusCode
SCT_ByteStreamErrorsSvc::fillData(const EventContext& ctx) {
  EventContext::ContextID_t slot{ctx.slot()};
  EventContext::ContextEvt_t evt{ctx.evt()};
  
  if (slot<m_cache.size() and m_cache[slot]==evt) {
    // Cache isvalid
    return StatusCode::SUCCESS;
  }

  std::lock_guard<std::mutex> lock{m_mutex};

  static const EventContext::ContextEvt_t invalidValue{EventContext::INVALID_CONTEXT_EVT};  
  if (slot>=m_cache.size()) {
    m_cache.resize(slot+1, invalidValue); // Store invalid values in order to go to the next IF statement.
    for (int errType{0}; errType<SCT_ByteStreamErrors::NUM_ERROR_TYPES; errType++) {
      m_bsErrors[errType].resize(slot+1);
    }
    m_tempMaskedChips.resize(slot+1);
    m_abcdErrorChips.resize(slot+1);
  }
  resetSets(ctx);
  m_tempMaskedChips[slot].clear();
  m_abcdErrorChips[slot].clear();

  SG::ReadHandle<InDetBSErrContainer> errCont{m_bsErrContainerName};

  /** When running over ESD files without BSErr container stored, don't 
   * want to flood the user with error messages.  Should just have a bunch
   * of empty sets, and keep quiet.
   */
  if (not errCont.isValid()) {
    ATH_MSG_INFO("Failed to retrieve BS error container "
                 << m_bsErrContainerName.key()
                 << " from StoreGate.  ");
    return StatusCode::SUCCESS;
  }

  /** OK, so we found the StoreGate container, now lets iterate
   * over it to populate the sets of errors owned by this Svc.
   */
  ATH_MSG_DEBUG("size of error container is " << errCont->size());
  for (const auto* elt : *errCont) {
    addError(elt->first,elt->second, ctx);
    Identifier wafer_id{m_sct_id->wafer_id(elt->first)};
    Identifier module_id{m_sct_id->module_id(wafer_id)};
    int side{m_sct_id->side(m_sct_id->wafer_id(elt->first))};
    if ((elt->second>=SCT_ByteStreamErrors::ABCDError_Chip0 and elt->second<=SCT_ByteStreamErrors::ABCDError_Chip5)) {
      m_abcdErrorChips[slot][module_id] |= (1 << (elt->second-SCT_ByteStreamErrors::ABCDError_Chip0 + side*6));
    } else if (elt->second>=SCT_ByteStreamErrors::TempMaskedChip0 and elt->second<=SCT_ByteStreamErrors::TempMaskedChip5) {
      m_tempMaskedChips[slot][module_id] |= (1 << (elt->second-SCT_ByteStreamErrors::TempMaskedChip0 + side*6));
    } else {
      std::pair<bool, bool> badLinks{m_config->badLinks(module_id)};
      bool result{(side==0 ? badLinks.first : badLinks.second) and (badLinks.first xor badLinks.second)};
      if (result) {
        /// error in a module using RX redundancy - add an error for the other link as well!!
        /// However, ABCDError_Chip0-ABCDError_Chip5 and TempMaskedChip0-TempMaskedChip5 are not common for two links.
        if (side==0) {
          IdentifierHash otherSide{IdentifierHash(elt->first  + 1)};
          addError(otherSide,elt->second, ctx);
          ATH_MSG_DEBUG("Adding error to side 1 for module with RX redundancy" << otherSide);
        } else if (side==1) {
          IdentifierHash otherSide{IdentifierHash(elt->first  - 1)};
          addError(otherSide,elt->second, ctx);
          ATH_MSG_DEBUG("Adding error to side 0 for module with RX redundancy" << otherSide);
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}

bool
SCT_ByteStreamErrorsSvc::filled() const{
  return true;
}

/** The following method is used to populate 
 *  the sets of IdHashes for wafers with errors. 
 *  It is called by the fillData() method, which reads the 
 *  InDetBSErrContainer from StoreGate.
 */

void 
SCT_ByteStreamErrorsSvc::addError(IdentifierHash id, int errorType, const EventContext& ctx) {
  if (errorType>=0 and errorType<SCT_ByteStreamErrors::NUM_ERROR_TYPES) {
    m_bsErrors[errorType][ctx.slot()].insert(id);
  }
}

///////////////////////////////////////////////////////////////////////////////

/** A bit from a particular word in the ByteStream if the data
 * is coming from the ROD simulator rather than real modules. */
bool
SCT_ByteStreamErrorsSvc::isRODSimulatedData() const {
  const SCT_ByteStreamFractionContainer* fracData{getFracData()};
  if (fracData==nullptr) return false;
  return fracData->majority(SCT_ByteStreamFractionContainer::SimulatedData);
}

///////////////////////////////////////////////////////////////////////////////

unsigned int SCT_ByteStreamErrorsSvc::tempMaskedChips(const Identifier& moduleId) {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const std::map<Identifier, unsigned int>& v_tempMaskedChips{getTempMaskedChips(ctx)};
  std::map<Identifier, unsigned int>::const_iterator it{v_tempMaskedChips.find(moduleId)};
  if (it!=v_tempMaskedChips.end()) return it->second;
  return 0;
}

unsigned int SCT_ByteStreamErrorsSvc::abcdErrorChips(const Identifier& moduleId) {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const std::map<Identifier, unsigned int>& v_abcdErrorChips{getAbcdErrorChips(ctx)};
  std::map<Identifier, unsigned int>::const_iterator it{v_abcdErrorChips.find(moduleId)};
  if (it!=v_abcdErrorChips.end()) return it->second;
  return 0;
}

const SCT_ByteStreamFractionContainer* SCT_ByteStreamErrorsSvc::getFracData() const {
  SG::ReadHandle<SCT_ByteStreamFractionContainer> fracCont{m_bsFracContainerName};
  if (not fracCont.isValid()) {
    ATH_MSG_INFO(m_bsFracContainerName.key() << " cannot be retrieved");
    return nullptr;
  }
  return fracCont.ptr();
}

const std::set<IdentifierHash>& SCT_ByteStreamErrorsSvc::getErrorSet(SCT_ByteStreamErrors::errorTypes errorType, const EventContext& ctx) {
  StatusCode sc{fillData(ctx)};
  if (sc.isFailure()) {
    ATH_MSG_ERROR("fillData in getErrorSet fails");
  }

  return m_bsErrors[errorType][ctx.slot()];
}

const std::map<Identifier, unsigned int>& SCT_ByteStreamErrorsSvc::getTempMaskedChips(const EventContext& ctx) { 
  StatusCode sc{fillData(ctx)};
  if (sc.isFailure()) {
    ATH_MSG_ERROR("fillData in getTempMaskedChips fails");
  }

  return m_tempMaskedChips[ctx.slot()];
}

const std::map<Identifier, unsigned int>& SCT_ByteStreamErrorsSvc::getAbcdErrorChips(const EventContext& ctx) {
  StatusCode sc{fillData(ctx)};
  if (sc.isFailure()) {
    ATH_MSG_ERROR("fillData in getAbcdErrorChips fails");
  }

  return m_abcdErrorChips[ctx.slot()];
}

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_ConfigurationConditionsTool.h"

// Athena includes
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "StoreGate/ReadCondHandle.h"

// Constructor
SCT_ConfigurationConditionsTool::SCT_ConfigurationConditionsTool(const std::string& type, const std::string& name, const IInterface* parent) :
  base_class(type, name, parent)
{ 
}

// Initialize
StatusCode SCT_ConfigurationConditionsTool::initialize() {
  ATH_MSG_INFO("Initializing configuration");

  ATH_CHECK(detStore()->retrieve(m_pHelper, "SCT_ID"));

  // Read Cond Handle Key
  ATH_CHECK(m_condKey.initialize());
  ATH_CHECK(m_SCTDetEleCollKey.initialize());

  return StatusCode::SUCCESS;
}

// Finalize
StatusCode SCT_ConfigurationConditionsTool::finalize() {
  ATH_MSG_INFO("Configuration finalize");

  return StatusCode::SUCCESS;
}

// What level of element can this service report about
bool SCT_ConfigurationConditionsTool::canReportAbout(InDetConditions::Hierarchy h) const {
  return (h == InDetConditions::SCT_STRIP or
          h == InDetConditions::SCT_CHIP or
          h == InDetConditions::SCT_SIDE or
          h == InDetConditions::SCT_MODULE or
          h == InDetConditions::DEFAULT);
}

// Is an element with this Identifier and hierachy good?
bool SCT_ConfigurationConditionsTool::isGood(const Identifier& elementId, const EventContext& ctx, InDetConditions::Hierarchy h) const {
  if (not canReportAbout(h)) return true;

  const SCT_ConfigurationCondData* condData{getCondData(ctx)};
  if (condData==nullptr) {
    ATH_MSG_ERROR("In isGood, SCT_ConfigurationCondData pointer cannot be retrieved");
    return false;
  }

  bool result{true};
  if (h == InDetConditions::SCT_STRIP) {
    result = (not condData->isBadStrip(m_pHelper->wafer_hash(m_pHelper->wafer_id(elementId)),
                                       m_pHelper->strip(elementId)));
    // If strip itself is not bad, check if it's in a bad module
    if (result and m_checkStripsInsideModules) result = (not isStripInBadModule(elementId, ctx));
  } else if (h == InDetConditions::SCT_MODULE) {
    result = (not condData->isBadModuleId(elementId));
  } else if (h == InDetConditions::SCT_SIDE or h == InDetConditions::DEFAULT) {
    result = (not condData->isBadWaferId(elementId));
  } else if (h == InDetConditions::SCT_CHIP) {
    result = isGoodChip(elementId, ctx);
  }
  return result;
}

bool SCT_ConfigurationConditionsTool::isGood(const Identifier& elementId, InDetConditions::Hierarchy h) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};

  return isGood(elementId, ctx, h);
}

// Is a wafer with this IdentifierHash good?
bool SCT_ConfigurationConditionsTool::isGood(const IdentifierHash& hashId, const EventContext& ctx) const {
  const Identifier elementId{m_pHelper->wafer_id(hashId)};
  return isGood(elementId, ctx);
}

bool SCT_ConfigurationConditionsTool::isGood(const IdentifierHash& hashId) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return isGood(hashId, ctx);
}

// Is a chip with this Identifier good?
bool SCT_ConfigurationConditionsTool::isGoodChip(const Identifier& stripId, const EventContext& ctx) const {
  // This check assumes present SCT.
  // Get module number
  const Identifier moduleId{m_pHelper->module_id(stripId)};
  if (not moduleId.is_valid()) {
    ATH_MSG_WARNING("moduleId obtained from stripId " << stripId << " is invalid.");
    return false;
  }

  // badChips word for the module
  const unsigned int v_badChips{badChips(moduleId, ctx)};
  // badChips holds 12 bits.
  // bit 0 (LSB) is chip 0 for side 0.
  // bit 5 is chip 5 for side 0.
  // bit 6 is chip 6 for side 1.
  // bit 11 is chip 11 for side 1.

  // If there is no bad chip, this check is done.
  if (v_badChips==0) return true;

  const int side{m_pHelper->side(stripId)};
  // Check the six chips on the side
  // 0x3F  = 0000 0011 1111
  // 0xFC0 = 1111 1100 0000
  // If there is no bad chip on the side, this check is done.
  if ((side==0 and (v_badChips & 0x3F)==0) or (side==1 and (v_badChips & 0xFC0)==0)) return true;

  int chip{getChip(stripId, ctx)};
  if (chip<0 or chip>=12) {
    ATH_MSG_WARNING("chip number is invalid: " << chip);
    return false;
  }

  // Check if the chip is bad
  const bool badChip{static_cast<bool>(v_badChips & (1<<chip))};

  return (not badChip);
}

// Check if a strip is within a bad module
bool SCT_ConfigurationConditionsTool::isStripInBadModule(const Identifier& stripId, const EventContext& ctx) const {
  const SCT_ConfigurationCondData* condData{getCondData(ctx)};
  if (condData==nullptr) {
    ATH_MSG_ERROR("In isStripInBadModule, SCT_ConfigurationCondData pointer cannot be retrieved");
    return true;
  }

  const Identifier moduleId(m_pHelper->module_id(m_pHelper->wafer_id(stripId)));
  return condData->isBadModuleId(moduleId);
}

// Check if a wafer is within a bad module
bool SCT_ConfigurationConditionsTool::isWaferInBadModule(const Identifier& waferId, const EventContext& ctx) const {
  const SCT_ConfigurationCondData* condData{getCondData(ctx)};
  if (condData==nullptr) {
    ATH_MSG_ERROR("In isWaferInBadModule, SCT_ConfigurationCondData pointer cannot be retrieved");
    return true;
  }

  const Identifier moduleId{m_pHelper->module_id(waferId)};
  return condData->isBadModuleId(moduleId);
}

// Find the chip number containing a particular strip Identifier
int SCT_ConfigurationConditionsTool::getChip(const Identifier& stripId, const EventContext& ctx) const {
  // Find side and strip number
  const int side{m_pHelper->side(stripId)};
  int strip{m_pHelper->strip(stripId)};

  // Check for swapped readout direction
  const IdentifierHash waferHash{m_pHelper->wafer_hash(m_pHelper->wafer_id(stripId))};
  const InDetDD::SiDetectorElement* pElement{getDetectorElement(waferHash, ctx)};
  if (pElement==nullptr) {
    ATH_MSG_FATAL("Element pointer is NULL in 'badStrips' method");
    return invalidChipNumber;
  }
  strip = (pElement->swapPhiReadoutDirection()) ? lastStrip - strip: strip;

  // Find chip number
  return (side==0 ? strip/stripsPerChip : strip/stripsPerChip + 6);
}

int SCT_ConfigurationConditionsTool::getChip(const Identifier& stripId) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return getChip(stripId, ctx);
}

const std::set<Identifier>* SCT_ConfigurationConditionsTool::badModules(const EventContext& ctx) const {
  const SCT_ConfigurationCondData* condData{getCondData(ctx)};
  if (condData==nullptr) {
    ATH_MSG_ERROR("In badModules, SCT_ConfigurationCondData pointer cannot be retrieved");
    return nullptr;
  }

  return condData->getBadModuleIds();
}

const std::set<Identifier>* SCT_ConfigurationConditionsTool::badModules() const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return badModules(ctx);
}

void SCT_ConfigurationConditionsTool::badStrips(const Identifier& moduleId, std::set<Identifier>& strips, const EventContext& ctx, bool ignoreBadModules, bool ignoreBadChips) const {
  const SCT_ConfigurationCondData* condData{getCondData(ctx)};
  if (condData==nullptr) {
    ATH_MSG_ERROR("In badStrips, SCT_ConfigurationCondData pointer cannot be retrieved");
    return;
  }

  // Bad strips for a given module
  if (ignoreBadModules) {
    // Ignore strips in bad modules
    if (condData->isBadModuleId(moduleId)) return;
  }

  for (const Identifier& badStripId: *(condData->getBadStripIds())) {
    if (ignoreBadChips) {
      // Ignore strips in bad chips
      const int chip{getChip(badStripId)};
      if (chip!=invalidChipNumber) {
        unsigned int chipStatusWord{condData->getBadChips(moduleId)};
        if ((chipStatusWord & (1 << chip)) != 0) continue;
      }   
    }
    if (m_pHelper->module_id(m_pHelper->wafer_id(badStripId)) == moduleId) strips.insert(badStripId);
  }
}

void SCT_ConfigurationConditionsTool::badStrips(const Identifier& moduleId, std::set<Identifier>& strips, bool ignoreBadModules, bool ignoreBadChips) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return badStrips(moduleId, strips, ctx, ignoreBadModules, ignoreBadChips);
}
       
std::pair<bool, bool> SCT_ConfigurationConditionsTool::badLinks(const IdentifierHash& hash, const EventContext& ctx) const {
  // Bad links for a given module
  // Bad convetion is used. true is for good link and false is for bad link...
  const SCT_ConfigurationCondData* condData{getCondData(ctx)};
  if (condData==nullptr) {
    ATH_MSG_ERROR("In badLinks, SCT_ConfigurationCondData pointer cannot be retrieved");
    return std::pair<bool, bool>{false, false};
  }

  return condData->areBadLinks(hash);
}

std::pair<bool, bool> SCT_ConfigurationConditionsTool::badLinks(const IdentifierHash& hash) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return badLinks(hash, ctx);
}

const std::map<IdentifierHash, std::pair<bool, bool>>* SCT_ConfigurationConditionsTool::badLinks(const EventContext& ctx) const {
  const SCT_ConfigurationCondData* condData{getCondData(ctx)};
  if (condData==nullptr) {
    ATH_MSG_ERROR("In badLinks, SCT_ConfigurationCondData pointer cannot be retrieved");
    return nullptr;
  }

  return condData->getBadLinks();
}

const std::map<IdentifierHash, std::pair<bool, bool>>* SCT_ConfigurationConditionsTool::badLinks() const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return badLinks(ctx);
}

const std::map<Identifier, unsigned int>* SCT_ConfigurationConditionsTool::badChips(const EventContext& ctx) const {
  const SCT_ConfigurationCondData* condData{getCondData(ctx)};
  if (condData==nullptr) {
    ATH_MSG_ERROR("In badChips, SCT_ConfigurationCondData pointer cannot be retrieved");
    return nullptr;
  }

  return condData->getBadChips();
}

const std::map<Identifier, unsigned int>* SCT_ConfigurationConditionsTool::badChips() const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return badChips(ctx);
}

unsigned int SCT_ConfigurationConditionsTool::badChips(const Identifier& moduleId, const EventContext& ctx) const {
  // Bad chips for a given module
  const SCT_ConfigurationCondData* condData{getCondData(ctx)};
  if (condData==nullptr) {
    ATH_MSG_ERROR("In badChips, SCT_ConfigurationCondData pointer cannot be retrieved");
    return 0xFFF; // 12 bad chips
  }

  return condData->getBadChips(moduleId);
}

unsigned int SCT_ConfigurationConditionsTool::badChips(const Identifier& moduleId) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return badChips(moduleId, ctx);
}
void 
SCT_ConfigurationConditionsTool::badStrips(std::set<Identifier>& strips, const EventContext& ctx, bool ignoreBadModules, bool ignoreBadChips) const {
  const SCT_ConfigurationCondData* condData{getCondData(ctx)};
  if (condData==nullptr) {
    ATH_MSG_ERROR("In badStrips, SCT_ConfigurationCondData pointer cannot be retrieved");
    return;
  }

  if (ignoreBadModules == false and ignoreBadChips == false) {
    std::copy(condData->getBadStripIds()->begin(), condData->getBadStripIds()->end(), std::inserter(strips,strips.begin()));
    return;
  }
  for (const Identifier& badStripId: *(condData->getBadStripIds())) {
    const Identifier moduleId{m_pHelper->module_id(m_pHelper->wafer_id(badStripId))};
    // Ignore strips in bad modules
    if (ignoreBadModules) {
      if (condData->isBadModuleId(moduleId)) continue;
    }
    // Ignore strips in bad chips
    if (ignoreBadChips) {
      const int chip{getChip(badStripId)};
      if (chip!=invalidChipNumber) {
        unsigned int chipStatusWord{condData->getBadChips(moduleId)};
        if ((chipStatusWord & (1 << chip)) != 0) continue;
      }
    }
    strips.insert(badStripId);
  }
}

void 
SCT_ConfigurationConditionsTool::badStrips(std::set<Identifier>& strips, bool ignoreBadModules, bool ignoreBadChips) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  badStrips(strips, ctx, ignoreBadModules, ignoreBadChips);
}

const SCT_ConfigurationCondData*
SCT_ConfigurationConditionsTool::getCondData(const EventContext& ctx) const {
  SG::ReadCondHandle<SCT_ConfigurationCondData> condData{m_condKey, ctx};
  return condData.retrieve();
}

const InDetDD::SiDetectorElement* SCT_ConfigurationConditionsTool::getDetectorElement(const IdentifierHash& waferHash, const EventContext& ctx) const {
  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> condData{m_SCTDetEleCollKey, ctx};
  if (not condData.isValid()) return nullptr;
  return condData->getDetectorElement(waferHash);
}

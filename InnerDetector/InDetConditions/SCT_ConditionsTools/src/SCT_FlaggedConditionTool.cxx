/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_FlaggedConditionTool.h"

// Athena
#include "InDetIdentifier/SCT_ID.h"

// Constructor
SCT_FlaggedConditionTool::SCT_FlaggedConditionTool(const std::string& type, const std::string& name, const IInterface* parent):
  base_class(type, name, parent)
{
}

// Initialize
StatusCode SCT_FlaggedConditionTool::initialize() {
  ATH_MSG_INFO("SCT_FlaggedConditionTool::initialize()");

  // Retrieve SCT helper
  if (detStore()->retrieve(m_sctID, "SCT_ID").isFailure()) {
    ATH_MSG_ERROR("SCT helper failed to retrieve");
    return StatusCode::FAILURE;
  }

  // A dirty fix of a configuration of InDetTrigInDetSCT_FlaggedConditionTool for InDetTrigInDetSCT_ConditionsSummaryTool
  if (name().find("InDetTrigInDetSCT_FlaggedConditionTool")!=std::string::npos) {
    if (m_badIds.key()=="SCT_FlaggedCondData") {
      m_badIds = SG::ReadHandleKey<SCT_FlaggedCondData>("SCT_FlaggedCondData_TRIG");
      ATH_MSG_INFO("Key of SCT_FlaggedCondData is changed to SCT_FlaggedCondData_TRIG in " << name());
    }
  }

  ATH_CHECK(m_badIds.initialize());

  return StatusCode::SUCCESS;
}

// Finalize
StatusCode SCT_FlaggedConditionTool::finalize() {
  ATH_MSG_INFO("SCT_FlaggedConditionTool::finalize()");
  return StatusCode::SUCCESS;
}

// Detector elements that this service can report about
bool SCT_FlaggedConditionTool::canReportAbout(InDetConditions::Hierarchy h) const {
  return (h == InDetConditions::SCT_SIDE or h == InDetConditions::DEFAULT);
}

// Is this element good (by Identifier)?
bool SCT_FlaggedConditionTool::isGood(const Identifier& elementId, const EventContext& ctx, InDetConditions::Hierarchy h) const {
  if (not canReportAbout(h)) return true;
  const IdentifierHash hashId{m_sctID->wafer_hash(elementId)};
  return isGood(hashId, ctx);
}

bool SCT_FlaggedConditionTool::isGood(const Identifier& elementId, InDetConditions::Hierarchy h) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};

  return isGood(elementId, ctx, h);
}

// Is this element good (by IdentifierHash)?
bool SCT_FlaggedConditionTool::isGood(const IdentifierHash& hashId, const EventContext& ctx) const {
  const SCT_FlaggedCondData* badIds{getCondData(ctx)};
  if (badIds==nullptr) {
    ATH_MSG_ERROR("SCT_FlaggedCondData cannot be retrieved. (isGood)");
    return false;
  }

  return (badIds->find(hashId) == badIds->end());
}

bool SCT_FlaggedConditionTool::isGood(const IdentifierHash& hashId) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};

  return isGood(hashId, ctx);
}

// Retrieve the reason why the wafer is flagged as bad (by IdentifierHash)
// If wafer is not found return a null string
const std::string& SCT_FlaggedConditionTool::details(const IdentifierHash& hashId, const EventContext& ctx) const {
  static const std::string nullString;

  const SCT_FlaggedCondData* badIds{getCondData(ctx)};
  if (badIds==nullptr) {
    ATH_MSG_ERROR("SCT_FlaggedCondData cannot be retrieved. (details)");
    return nullString;
  }

  std::map<IdentifierHash, std::string>::const_iterator itr(badIds->find(hashId));
  return ((itr != badIds->end()) ? (*itr).second : nullString);
}

const std::string& SCT_FlaggedConditionTool::details(const IdentifierHash& hashId) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return details(hashId, ctx);
}

// Retrieve the reason why the wafer is flagged as bad (by Identifier)
// If wafer is not found return a null string
const std::string& SCT_FlaggedConditionTool::details(const Identifier& Id, const EventContext& ctx) const {
  const IdentifierHash hashId{m_sctID->wafer_hash(Id)};
  return details(hashId, ctx);
}

const std::string& SCT_FlaggedConditionTool::details(const Identifier& Id) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return details(Id, ctx);
}

int SCT_FlaggedConditionTool::numBadIds(const EventContext& ctx) const {
  const SCT_FlaggedCondData* badIds{getCondData(ctx)};
  if (badIds==nullptr) {
    ATH_MSG_ERROR("SCT_FlaggedCondData cannot be retrieved. (numBadIds)");
    return -1;
  }

  return badIds->size();
}

int SCT_FlaggedConditionTool::numBadIds() const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return numBadIds(ctx);
}

const SCT_FlaggedCondData* SCT_FlaggedConditionTool::getBadIds(const EventContext& ctx) const {
  return getCondData(ctx);
}

const SCT_FlaggedCondData* SCT_FlaggedConditionTool::getBadIds() const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return getBadIds(ctx);
}

const SCT_FlaggedCondData* SCT_FlaggedConditionTool::getCondData(const EventContext& ctx) const {
  SG::ReadHandle<SCT_FlaggedCondData> condData{m_badIds, ctx};
  if (not condData.isValid()) {
    ATH_MSG_ERROR("Failed to get " << m_badIds.key());
    return nullptr;
  }
  return condData.cptr();
}

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_RODVetoCondAlg.cxx
 *
 * @brief Implementation file for the SCT_RODVetoCondAlg class 
 * in package SCT_ConditionsAlgorithms
 *
 * @author Susumu Oda
 **/

#include "SCT_RODVetoCondAlg.h"

#include <algorithm>
#include <ios>

#include "InDetIdentifier/SCT_ID.h"

SCT_RODVetoCondAlg::SCT_RODVetoCondAlg(const std::string& name, 
                                       ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_pHelper{nullptr},
  m_badIds{"BadSCTModuleIds_RODVeto"},
  m_badRODElementsInput{}
{
  declareProperty("BadModuleIds", m_badIds, "Write key for bad module identifiers");
  declareProperty("BadRODIds", m_badRODElementsInput, "Input list of RODs to be vetoed");
}

StatusCode SCT_RODVetoCondAlg::initialize() {
  ATH_CHECK(m_cabling.retrieve());
  ATH_CHECK(detStore()->retrieve(m_pHelper, "SCT_ID"));
  ATH_CHECK(m_badIds.initialize());

  return StatusCode::SUCCESS;
}

StatusCode SCT_RODVetoCondAlg::execute() {
  ATH_MSG_INFO(m_badRODElementsInput.size() <<" RODs were declared bad");

  std::vector<unsigned int> allRods;
  m_cabling->getAllRods(allRods);
  
  SG::WriteHandle<IdentifierSet> badIds{m_badIds};
  ATH_CHECK(badIds.record(std::make_unique<IdentifierSet>()));

  for (unsigned int thisRod: m_badRODElementsInput) {
    ATH_MSG_DEBUG("This rod is " << std::hex << "0x" << thisRod << std::dec);

    //check whether rod exists
    if (std::find(allRods.begin(), allRods.end(), thisRod)==allRods.end()) {
      ATH_MSG_WARNING("Your vetoed selection " << std::hex << "0x" << thisRod << " does not exist." << std::dec);
      continue;
    }

    std::vector<IdentifierHash> listOfHashes;
    m_cabling->getHashesForRod(listOfHashes, thisRod);
    // Two consecutive hashes may produce the same module id, since they will be two sides
    // of the same module. We avoid invalid inserts by guarding against this.
    Identifier previousId; //constructor produces an invalid one
    for (const IdentifierHash& thisHash: listOfHashes) {
      Identifier wafId{m_pHelper->wafer_id(thisHash)};
      Identifier modId{m_pHelper->module_id(wafId)};
      const bool alreadyInserted{modId==previousId};
      previousId = modId;
      if (alreadyInserted) continue;
      ATH_MSG_VERBOSE("This module Id is " << modId);
      const bool thisInsertionOk{badIds->insert(modId).second};
      if (not thisInsertionOk) {
        ATH_MSG_WARNING("Insertion failed for rod " << std::hex << "0x" << thisRod << std::dec << " and module (hash,id): " << thisHash << ", " << modId);
      }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode SCT_RODVetoCondAlg::finalize() {
  return StatusCode::SUCCESS;
}

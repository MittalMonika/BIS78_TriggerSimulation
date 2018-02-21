/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_ModuleVetoTestAlg.cxx
 *
 * @brief Implementation file for the SCT_ModuleVetoTestAlg class 
 * in package SCT_ConditionsAlgorithms
 *
 * @author Shaun Roe
 **/

#include "SCT_ModuleVetoTestAlg.h"


#include "Identifier/IdentifierHash.h"
#include "Identifier/Identifier.h"

SCT_ModuleVetoTestAlg::SCT_ModuleVetoTestAlg(const std::string& name, ISvcLocator* pSvcLocator) : 
  AthAlgorithm(name, pSvcLocator), 
  m_pModuleVetoSvc("SCT_ModuleVetoSvc", name) {
  //nop
}

//Initialize
StatusCode 
SCT_ModuleVetoTestAlg::initialize() {
  StatusCode sc{StatusCode::SUCCESS};
  ATH_MSG_INFO("Calling initialize");
  sc = m_pModuleVetoSvc.retrieve();
  if (StatusCode::SUCCESS not_eq sc) {
    ATH_MSG_ERROR("Could not retrieve the veto service");
  }
  return sc;
}

//Execute
StatusCode 
SCT_ModuleVetoTestAlg::execute() {
  //This method is only used to test the summary service, and only used within this package,
  // so the INFO level messages have no impact on performance of these services when used by clients

  ATH_MSG_INFO("Calling execute");

  ATH_MSG_INFO("Dummy call to module id 0: module is ");
  bool result{m_pModuleVetoSvc->isGood(Identifier{0})};
  ATH_MSG_INFO((result ? "good" : "bad"));

  ATH_MSG_INFO("Dummy call to module id 1:  module is ");
  result=m_pModuleVetoSvc->isGood(Identifier{1});
  ATH_MSG_INFO((result ? "good" : "bad"));

  ATH_MSG_INFO("Dummy call to module id 2:  module is ");
  result=m_pModuleVetoSvc->isGood(Identifier{2});
  ATH_MSG_INFO((result ? "good" : "bad"));

  ATH_MSG_INFO("Dummy call to module id 3:  module is ");
  result=m_pModuleVetoSvc->isGood(Identifier{3});
  ATH_MSG_INFO((result ? "good" : "bad"));

  ATH_MSG_INFO("Dummy call to module id 151040000:  module is ");
  result=m_pModuleVetoSvc->isGood(Identifier{151040000});
  ATH_MSG_INFO((result ? "good" : "bad"));

  ATH_MSG_INFO("Using Identifier Hash method:  with number 2137 ");
  result=m_pModuleVetoSvc->isGood(IdentifierHash{2137});
  ATH_MSG_INFO((result ? "good" : "bad"));

  return StatusCode::SUCCESS;
}


//Finalize
StatusCode
SCT_ModuleVetoTestAlg::finalize() {
  ATH_MSG_INFO("Calling finalize");

  return StatusCode::SUCCESS;
}

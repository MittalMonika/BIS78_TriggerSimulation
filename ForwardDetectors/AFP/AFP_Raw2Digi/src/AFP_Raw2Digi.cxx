/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"

// AFP_Raw2Digi includes
#include "AFP_Raw2Digi/AFP_Raw2Digi.h"
#include "xAODForward/AFPSiHit.h"
#include "xAODForward/AFPSiHitContainer.h"
#include "xAODForward/AFPSiHitAuxContainer.h"

#include "TBranch.h"

//////////////////////////
//// constructor
//////////////////////////

AFP_Raw2Digi::AFP_Raw2Digi(const std::string &name, ISvcLocator *pSvcLocator)
  : AthAlgorithm(name, pSvcLocator),
    m_DigiTool( "AFP_Raw2DigiTool")
{
  declareProperty("DigiTool", m_DigiTool , "Tool to translate RawData to xAOD");
}

//////////////////////////
//// destructor
//////////////////////////

AFP_Raw2Digi::~AFP_Raw2Digi() {}

//////////////////////////
//// initialize
//////////////////////////

StatusCode AFP_Raw2Digi::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "...");

  CHECK( m_DigiTool.retrieve() );

  return StatusCode::SUCCESS;
}

//////////////////////////
//// finalize
//////////////////////////

StatusCode AFP_Raw2Digi::finalize() {
  ATH_MSG_INFO("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

//////////////////////////
//// execute
//////////////////////////

StatusCode AFP_Raw2Digi::execute() {

  ATH_MSG_DEBUG("Executing " << name() << "...");

  CHECK (m_DigiTool->recoSiHits() );

  return StatusCode::SUCCESS;
}

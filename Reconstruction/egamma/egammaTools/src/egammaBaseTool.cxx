/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaBaseTool.h"

// INCLUDE Header Files:

// ====================================================================================
egammaBaseTool::egammaBaseTool(const std::string& type,
			       const std::string& name,
			       const IInterface* parent):
  AthAlgTool(type, name, parent)
{ 
  declareInterface<IegammaBaseTool>(this);
}

// ====================================================================================
egammaBaseTool::~egammaBaseTool() { 
}

// ====================================================================================
StatusCode egammaBaseTool::initialize() {  
  return StatusCode::SUCCESS;
}

// =========================================================================================
StatusCode egammaBaseTool::finalize(){
  return StatusCode::SUCCESS;
}


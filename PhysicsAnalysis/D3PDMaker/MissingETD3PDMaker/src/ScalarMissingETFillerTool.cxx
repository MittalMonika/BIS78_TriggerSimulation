/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file MissingETD3PDMaker/src/ScalarMissingETFillerTool.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2009
 * @brief Block filler tool for Missing ET objects.
 */


#include "MissingETD3PDMaker/ScalarMissingETFillerTool.h"
#include "MissingETEvent/MissingET.h"
#include "AthenaKernel/errorcheck.h"


namespace D3PD {


/**
 * @brief Standard Gaudi tool constructor.
 * @param type The name of the tool type.
 * @param name The tool name.
 * @param parent The tool's Gaudi parent.
 */
ScalarMissingETFillerTool::ScalarMissingETFillerTool (const std::string& type,
                                          const std::string& name,
                                          const IInterface* parent)
  : BlockFillerTool<MissingET> (type, name, parent)
  ,   mLog(msgSvc(), name )
{

}


/**
 * @brief Book variables for this block.
 */
StatusCode ScalarMissingETFillerTool::book()
{
  
  CHECK( addVariable ("et",     m_et) );
  
  return StatusCode::SUCCESS;
}


StatusCode ScalarMissingETFillerTool::fill (const MissingET& p)
{
  
  *m_et    = p.et();
 
  mLog << MSG::DEBUG << "MissingETD3PDMaker::ScalarMissingETFillerTool -- MET filled" << endreq;

  return StatusCode::SUCCESS;
}


} // namespace D3PD

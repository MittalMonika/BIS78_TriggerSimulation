/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/getMessageSvc.h"
#include "LArRawConditions/LArAutoCorrMC.h" 
#include "LArElecCalib/ILArMCSymTool.h"

LArAutoCorrMC::LArAutoCorrMC() :LArAutoCorrComplete(), m_larmcsym("LArMCSymTool")
{

}

StatusCode LArAutoCorrMC::initialize()  { 

  if(m_larmcsym.retrieve().isFailure()){
      MsgStream log(Athena::getMessageSvc(), "LArAutoCorrMC");
      log << MSG::WARNING << "Could not retrieve LArMCSymTool " << endmsg;
      return (StatusCode::FAILURE);
  }

  return (CONTAINER::initialize()) ; 
}

LArAutoCorrMC::~LArAutoCorrMC() {}


/* retrieve AutoCorr ******************************************************
 */
ILArAutoCorr::AutoCorrRef_t LArAutoCorrMC::autoCorr(const HWIdentifier& CellID, int gain) const 
{ // symmetrize CellID for MC usage
  HWIdentifier SymCellID = m_larmcsym->symOnline(CellID);
  return  LArAutoCorrComplete::autoCorr(SymCellID, gain); 
}


/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "MmRdoToPrepDataTool.h"
#include "MuonPrepRawData/MMPrepDataContainer.h"

Muon::MmRdoToPrepDataTool::MmRdoToPrepDataTool(const std::string& t,
                 const std::string& n,
                 const IInterface*  p )
  :
  AthAlgTool(t,n,p),
  MmRdoToPrepDataToolCore(t,n,p)
{
}

StatusCode Muon::MmRdoToPrepDataTool::initialize()
{  
  ATH_MSG_VERBOSE("Starting init");
  ATH_CHECK( MmRdoToPrepDataToolCore::initialize() );
  ATH_MSG_DEBUG("initialize() successful in " << name());
  return StatusCode::SUCCESS;
}

Muon::MmRdoToPrepDataToolCore::SetupMM_PrepDataContainerStatus Muon::MmRdoToPrepDataTool::setupMM_PrepDataContainer() 
{
  if(!evtStore()->contains<Muon::MMPrepDataContainer>(m_mmPrepDataContainerKey.key())){    
    m_fullEventDone=false;
    
    SG::WriteHandle< Muon::MMPrepDataContainer > handle(m_mmPrepDataContainerKey);
    StatusCode status = handle.record(std::make_unique<Muon::MMPrepDataContainer>(m_idHelperSvc->mmIdHelper().module_hash_max()));
    
    if (status.isFailure() || !handle.isValid() )   {
      ATH_MSG_FATAL("Could not record container of MicroMega PrepData Container at " << m_mmPrepDataContainerKey.key()); 
      return FAILED;
    }
    m_mmPrepDataContainer = handle.ptr();
    return ADDED;
  }
  return ALREADYCONTAINED;
}

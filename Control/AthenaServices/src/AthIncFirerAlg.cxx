/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#include "AthIncFirerAlg.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventIncident.h"
#include "AthenaKernel/errorcheck.h"

AthIncFirerAlg::AthIncFirerAlg( const std::string& name, ISvcLocator* pSvcLocator ):AthReentrantAlgorithm(name,pSvcLocator),m_Serial(false){
  declareProperty("Incidents",m_incLists,"Incidents to fire");
  declareProperty("FireSerial",m_Serial,"Whether to fire serial incidents");  
}

StatusCode AthIncFirerAlg::initialize(){
  m_incSvc = service("IncidentSvc",true);
  //Copied from CPUCruncher.cpp
  if (m_incLists.value().size()==0){
    ATH_MSG_ERROR("Need to have at least one incident defined!" <<name());
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode AthIncFirerAlg::execute(const EventContext& ctx)const {
  auto ctxcp=ctx;
  for(auto & i:m_incLists.value()){
    ATH_MSG_VERBOSE("Firing incident "<<i);

    if((i=="BeginEvent")||(i=="EndEvent")){
      const EventInfo* event(0);
      evtStore()->retrieve(event).ignore();
      m_incSvc->fireIncident(std::make_unique<EventIncident>(*event, name(),i,ctxcp));
      if(m_Serial.value())m_incSvc->fireIncident(EventIncident(*event, name(),i,ctxcp));
    }else{
      m_incSvc->fireIncident(std::make_unique<Incident>(name(),i,ctxcp));
      if(m_Serial.value())m_incSvc->fireIncident(Incident( name(),i,ctxcp));
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode AthIncFirerAlg::finalize(){
  return StatusCode::SUCCESS;
}

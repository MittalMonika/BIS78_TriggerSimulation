/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "AlgT.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "GaudiKernel/ServiceHandle.h"

#include <thread>
#include <chrono>
#include <ctime>
#include <memory>

AlgT::AlgT( const std::string& name, 
		    ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator ),
  m_wrh1("t1"), m_rdh1(""),
  m_evt("McEventInfo"),
  m_tool1("HiveTool",this),
  m_tool2("HiveTool",this),
  m_tool3("HiveTool",this)
{

  declareProperty("Key_R1",m_rdh1);
  declareProperty("Key_W1",m_wrh1);
  declareProperty("EvtInfo", m_evt);

  declareProperty("Tool1",m_tool1);
  declareProperty("Tool2",m_tool2);
  declareProperty("Tool3",m_tool3);
  
}

//---------------------------------------------------------------------------

AlgT::~AlgT() {}

StatusCode AlgT::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  if (m_rdh1.key() != "") ATH_CHECK( m_rdh1.initialize() );
  ATH_CHECK( m_wrh1.initialize() );
  ATH_CHECK( m_evt.initialize() );

  if ( m_tool1.retrieve().isFailure() ) {
    ATH_MSG_FATAL 
      (m_tool1.propertyName() << ": Failed to retrieve tool "
       << m_tool1.type());
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO("retreived " << m_tool1.name());
  }

  if ( m_tool2.retrieve().isFailure() ) {
    ATH_MSG_FATAL 
      (m_tool2.propertyName() << ": Failed to retrieve tool "
       << m_tool2.type());
    return StatusCode::FAILURE;
  }else {
    ATH_MSG_INFO("retreived " << m_tool2.name());
  }

  
  if ( m_tool3.retrieve().isFailure() ) {
    ATH_MSG_FATAL 
      (m_tool3.propertyName() << ": Failed to retrieve tool "
       << m_tool3.type());
    return StatusCode::FAILURE;
  }else {
    ATH_MSG_INFO("retreived " << m_tool3.name());
  }


  
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgT::finalize() {
  ATH_MSG_DEBUG("finalize " << name());
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgT::execute() {

  ATH_MSG_DEBUG("execute " << name());

  SG::ReadHandle<EventInfo> evt(m_evt);
  ATH_MSG_INFO("   EventInfo:  r: " << evt->event_ID()->run_number()
               << " e: " << evt->event_ID()->event_number() );


  SG::WriteHandle<HiveDataObj> wh1(m_wrh1);
  ATH_CHECK( wh1.record( std::make_unique<HiveDataObj> 
                         ( HiveDataObj(10000 + 
                                       evt->event_ID()->event_number()) ) )
             );
  ATH_MSG_INFO("  write: " << wh1.key() << " = " << wh1->val() );


  m_tool1->saySomething();
  m_tool2->saySomething();
  m_tool3->saySomething();
  

  return StatusCode::SUCCESS;

}


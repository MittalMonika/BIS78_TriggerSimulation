/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/******************************************************************************
Name:        EventCleaningTool

Author:      Julia Gonski
Created:     Nov 2016

Description: Class for selecting events that pass recommended jet cleaning procedure
******************************************************************************/

// This class header and package headers
#include "JetSelectorTools/EventCleaningTool.h"
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetSelectorTools/Helpers.h"

// The xAOD jet type
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"

// xAOD/ASG includes
#include "AsgTools/AsgMessaging.h"

// STL includes
#include <iostream>
#include <cmath>
#include <cfloat>

// ROOT includes
#include "TEnv.h"

namespace ECUtils { 

//=============================================================================
// Constructors
//=============================================================================
EventCleaningTool::EventCleaningTool(const std::string& name)
  : asg::AsgTool(name)
  , m_pt()
  , m_eta()
  , m_jvt()
  , m_or()
  , m_prefix()
  , m_cleaningLevel()
{
  declareProperty( "PtCut" , m_pt = 20000.0 );
  declareProperty( "EtaCut" , m_eta = 4.5 );
  declareProperty( "JvtDecorator" , m_jvt = "passJvt" );
  declareProperty( "OrDecorator" , m_or = "passOR" );
  declareProperty( "JetCleanPrefix", m_prefix = "" );
  declareProperty( "CleaningLevel" , m_cleaningLevel = "LooseBad");
}


//=============================================================================
// Destructor  
//=============================================================================
EventCleaningTool::~EventCleaningTool() {}

//=============================================================================
// Initialize
//=============================================================================
StatusCode EventCleaningTool::initialize()
{
  if(m_jvt == "" || m_or == ""){
    ATH_MSG_ERROR( "Tool initialized with unknown decorator names." );
    return StatusCode::FAILURE;
  }
  if (m_cleaningLevel == ""){
    ATH_MSG_ERROR( "Tool initialized with unknown cleaning level." );
    return StatusCode::FAILURE;
  }
  
  //initialize jet cleaning tool
  m_tool = new JetCleaningTool("JetCleaningTool_"+m_cleaningLevel);
  ATH_CHECK(m_tool->setProperty("CutLevel", m_cleaningLevel));
  ATH_CHECK(m_tool->initialize());
  ATH_MSG_INFO( "Event cleaning tool configured with cut level " << m_cleaningLevel  );

  return StatusCode::SUCCESS;
}

bool EventCleaningTool::acceptEvent(const xAOD::JetContainer* jets) const
{
	bool pass_pt = 0;
	bool pass_eta = 0;
	bool pass_accept = 0;
	int jvtDecision = 0;
	int orDecision = 0;
	bool isThisJetGood = 0;
	bool isEventAllGood = 1;

        SG::AuxElement::Decorator<char>* dec_jetClean = new SG::AuxElement::Decorator<char>(m_prefix + "jetClean_" + m_cleaningLevel);
	const static SG::AuxElement::ConstAccessor<char> acc_passOR(m_prefix+m_or);
	const static SG::AuxElement::ConstAccessor<char> acc_passJvt(m_prefix+m_jvt);	
	ATH_MSG_DEBUG("m_or: " << m_or << ", m_jvt: " << m_jvt);

	for (auto thisJet : *jets){  //loop over decorated jet collection 
		pass_pt = thisJet->pt() > m_pt; 
		pass_eta = fabs(thisJet->eta()) < m_eta;
		pass_accept = keepJet(*thisJet); 
		jvtDecision = acc_passJvt(*thisJet);
		orDecision = !acc_passOR(*thisJet);  //recall, passOR==0 means that the jet is not an overlap and should be kept!

		ATH_MSG_DEBUG("Jet info: pT: " << pass_pt << ", eta: " << pass_eta << ", accept? " << pass_accept << ", jvt: " << jvtDecision << ", or: " << orDecision); 
		if(pass_pt && pass_eta && jvtDecision && orDecision){//only consider jets for cleaning if they pass these requirements. 
			isThisJetGood = pass_accept;
			isEventAllGood = isEventAllGood && isThisJetGood; //any event with a bad jet is rejected 
		}	
		else isThisJetGood = pass_accept;     //if it fails any one of these, it shouldn't be able to kill the whole event, but we still need to know cleaning
		ATH_MSG_DEBUG("Is jet good? " << isThisJetGood);
		(*dec_jetClean)(*thisJet) = isThisJetGood;
 	}
	ATH_MSG_DEBUG("Is event good? " << isEventAllGood);
   	delete dec_jetClean;
	return isEventAllGood;		
}

int EventCleaningTool::keepJet(const xAOD::Jet& jet) const 
{ 
	return m_tool->keep(jet); 
}

}//ECUtils



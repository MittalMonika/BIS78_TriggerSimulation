/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include <math.h>

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"

#include "xAODTrigMuon/L2StandAloneMuonContainer.h"
#include "TrigMuonHypo/TrigMufastHypoAlg.h"

#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Units/PhysicalConstants.h"

using namespace TrigCompositeUtils; 

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

TrigMufastHypoAlg::TrigMufastHypoAlg( const std::string& name,
				      ISvcLocator* pSvcLocator ) :
  ::AthReentrantAlgorithm( name, pSvcLocator )
{
   declareProperty("ViewRoIs", m_viewsCollectionKey = std::string("ViewRoIs"), "MUViewRoIs to read in");
   declareProperty("MuFastDecisions", m_muFastCollectionKey = std::string("MufastDecisions"), "xAOD::L2StandAloneMuonContainer to read in");
   declareProperty("Decisions", m_decisionsKey = std::string("TrigMufastHypoAlgDesicions"), "Decisions");
   declareProperty("L1Decisions", m_previousDecisionsKey = std::string("L1Decisions"), "Key for L1 decisions per RoI");
   declareProperty("HypoTools", m_hypoTools, "Hypo Tools");
} 

TrigMufastHypoAlg::~TrigMufastHypoAlg() 
{}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMufastHypoAlg::initialize()
{
  ATH_MSG_INFO ( "Initializing " << name() << "..." );
  ATH_CHECK(m_hypoTools.retrieve());

  ATH_CHECK(m_viewsCollectionKey.initialize());

  renounce(m_muFastCollectionKey);
  ATH_CHECK(m_muFastCollectionKey.initialize());

  ATH_CHECK(m_decisionsKey.initialize());

  ATH_CHECK(m_previousDecisionsKey.initialize());
 
  ATH_MSG_INFO( "Initialization completed successfully" );
  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMufastHypoAlg::finalize() 
{   
  ATH_MSG_INFO( "Finalizing " << name() << "..." );
  ATH_MSG_INFO( "Finalization completed successfully" );
  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMufastHypoAlg::execute_r( const EventContext& context ) const
{

  ATH_MSG_DEBUG("StatusCode TrigMufastHypoAlg::execute_r start");

  auto viewsHandle = SG::makeHandle( m_viewsCollectionKey, context );

  std::map<const TrigRoiDescriptor*, const TrigCompositeUtils::Decision* > roiToDecision;
  auto previousDecisionsHandle = SG::makeHandle( m_previousDecisionsKey, context );
  for ( auto previousDecision: *previousDecisionsHandle ) { 
    auto roiEL = previousDecision->objectLink<TrigRoiDescriptorCollection>( "initialRoI" );
    if (!roiEL.isValid()) {
      ATH_MSG_ERROR("ReadHandle for std::vector<SG::View*> key:" << m_viewsCollectionKey.key() << " is failed");
      return StatusCode::FAILURE;
    }
    const TrigRoiDescriptor* roi = *roiEL;
    roiToDecision.insert( std::make_pair( roi, previousDecision ) );
  }   
  ATH_MSG_DEBUG("REGTEST: RoI to decisions map size: " << roiToDecision.size() );

  auto decisions = std::make_unique<DecisionContainer>();
  auto aux = std::make_unique<DecisionAuxContainer>();
  decisions->setStore(aux.get());

  size_t counter = 0;
  for ( auto view: *viewsHandle ) {
    ATH_MSG_DEBUG("REGTEST: " << m_viewsCollectionKey.key()  << " = " << view );

    auto d = newDecisionIn( decisions.get() );

    // retrieve MuFastDecisions with ReadHandle
    auto muFastCollectionHandle = SG::makeHandle( m_muFastCollectionKey, context );
    if (muFastCollectionHandle.setProxyDict(view).isFailure()) {
      ATH_MSG_ERROR("ReadHandle for xAOD::L2StandAloneMuonContainer key:" << m_muFastCollectionKey.key() << " is failed");
      return StatusCode::FAILURE;
    } 
    { // retrieve MUViewRoIs
      auto element = ElementLink< std::vector<SG::View*> >( m_viewsCollectionKey.key(), counter );
      if(!element.isValid()) {
        ATH_MSG_ERROR("ReadHandle for std::vector<SG::View*> key:" << m_viewsCollectionKey.key() << " isn't Valid");
        return StatusCode::FAILURE;
      }
      d->setObjectLink( "view", element );
    }
    { // retrieve MuFastDecisions 
      auto element = ElementLink<xAOD::L2StandAloneMuonContainer>( view->name()+"_"+m_muFastCollectionKey.key(), 0 ); 
      if(!element.isValid()) {
        ATH_MSG_ERROR("ReadHandle for xAOD::L2StandAloneMuonContainer key:" << m_muFastCollectionKey.key() << " isn't Valid");
        return StatusCode::FAILURE;
      }
      d->setObjectLink( "feature", element );
      ATH_MSG_DEBUG("REGTEST: " << m_muFastCollectionKey.key() << " pT = " << (*element)->pt());
      ATH_MSG_DEBUG("REGTEST: " << m_muFastCollectionKey.key() << " eta/phi = " << (*element)->eta() << "/" << (*element)->eta());

/*      // to TrigMufastHypoTool
      bool pass = false;
      StatusCode sc = StatusCode::SUCCESS;
      for ( auto& tool: m_hypoTools ) {
        sc = tool->decide(element, pass);
        if (!sc.isSuccess()) {
          ATH_MSG_ERROR("MuonHypoTool is failed");
          return StatusCode::FAILURE;
        }
      } // End of hypoTool	*/
    }
  } // End of view loops

  auto handle =  SG::makeHandle(m_decisionsKey, context);     
  ATH_CHECK( handle.record( std::move(decisions), std::move(aux) ) );

  ATH_MSG_DEBUG("StatusCode TrigMufastHypoAlg::execute_r success");
  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------


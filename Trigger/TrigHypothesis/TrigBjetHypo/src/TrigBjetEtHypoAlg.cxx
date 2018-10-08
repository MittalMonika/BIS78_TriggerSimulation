/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/Property.h"
#include "TrigBjetEtHypoAlg.h"

using namespace TrigCompositeUtils;

TrigBjetEtHypoAlg::TrigBjetEtHypoAlg( const std::string& name, 
				      ISvcLocator* pSvcLocator ) : 
  ::HypoBase( name, pSvcLocator ) {}

TrigBjetEtHypoAlg::~TrigBjetEtHypoAlg()
{}

StatusCode TrigBjetEtHypoAlg::initialize()
{
  ATH_MSG_INFO ( "Initializing " << name() << "..." );

  ATH_MSG_DEBUG( "Initializing Tools" );
  ATH_CHECK( m_hypoTools.retrieve() );
  //  ATH_CHECK( m_etHypoTools.retrieve() );

  ATH_MSG_DEBUG( "Initializing HandleKeys" );
  CHECK( m_bTagKey.initialize() );
  CHECK( m_roisKey.initialize() );

  CHECK( m_decisionsKey.initialize() );

  ATH_MSG_INFO("Initializing TrigBjetEtHypoAlg");

  ATH_MSG_DEBUG(  "declareProperty review:"   );
  ATH_MSG_DEBUG(  "   " << m_roisKey          );
  ATH_MSG_DEBUG(  "   " << m_bTagKey          );

  return StatusCode::SUCCESS;
}

StatusCode TrigBjetEtHypoAlg::finalize() {
  return StatusCode::SUCCESS;
}

StatusCode TrigBjetEtHypoAlg::execute_r( const EventContext& context ) const {  
  ATH_MSG_DEBUG ( "Executing " << name() << "..." );
  SG::ReadHandle< xAOD::BTaggingContainer > bTagHandle = SG::makeHandle( m_bTagKey, context );
  SG::ReadHandle< TrigRoiDescriptorCollection > roisHandle = SG::makeHandle( m_roisKey, context );
  
  std::unique_ptr< DecisionContainer > decisions = std::make_unique< DecisionContainer >();
  std::unique_ptr< DecisionAuxContainer > aux = std::make_unique< DecisionAuxContainer >();
  decisions->setStore( aux.get() );

  // prepare decision storage ( we could simplify it )
  // Lidija: should be checked once more
  size_t counter = 0;
  // ---->>>>>>
  xAOD::BTaggingContainer::const_iterator bTagIter = bTagHandle->begin();
  for ( ;  bTagIter != bTagHandle->end(); ++bTagIter, ++counter ) {
    TrigCompositeUtils::Decision *d = newDecisionIn( decisions.get() );
    d->setObjectLink( "feature", ElementLink<xAOD::BTaggingContainer>( m_bTagKey.key(), counter ) );
    d->setObjectLink( "roi", ElementLink<TrigRoiDescriptorCollection>( m_roisKey.key(), counter ) );
  }


  size_t index = 0;
  for ( ; index < decisions->size(); ++index ) {

    const xAOD::BTagging *bTag = bTagHandle->at( index );
    const TrigRoiDescriptor *roiDescriptor = roisHandle->at( index );
    TrigCompositeUtils::Decision *decision = decisions->at( index );

    /* // TO BE CHANGED
    for ( const ToolHandle< TrigBjetEtHypoTool >& tool : m_hypoTools ) {
      // interface of the tool needs to be suitable for current system, so no TrigComposite
      // also no support for the multi-electrons yet ( will be additional method )
      if ( tool->decide( bTag, roiDescriptor ) ) {   
	addDecisionID( tool->decisionId(), decision );	  
	ATH_MSG_DEBUG( " + " << tool->name() );
      } else {
	ATH_MSG_DEBUG( " - " << tool->name() );
      }
    }
    */
  }

  {
    SG::WriteHandle< TrigCompositeUtils::DecisionContainer > handle =  SG::makeHandle( m_decisionsKey, context );
    CHECK( handle.record( std::move( decisions ), std::move( aux ) ) );
  }

  return StatusCode::SUCCESS;
}





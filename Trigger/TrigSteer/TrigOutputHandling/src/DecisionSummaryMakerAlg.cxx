/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#include "DecisionHandling/HLTIdentifier.h"
#include "DecisionSummaryMakerAlg.h"

DecisionSummaryMakerAlg::DecisionSummaryMakerAlg(const std::string& name, ISvcLocator* pSvcLocator) 
  : AthReentrantAlgorithm(name, pSvcLocator) {}

DecisionSummaryMakerAlg::~DecisionSummaryMakerAlg() {}

StatusCode DecisionSummaryMakerAlg::initialize() {
  //ATH_MSG_DEBUG("Use macros for logging!");
  renounceArray( m_finalDecisionKeys );
  ATH_CHECK( m_finalDecisionKeys.initialize() ); 
  ATH_CHECK( m_summaryKey.initialize() );
  
  for ( auto& pair: m_lastStepForChain ) {
    struct { std::string chain, collection; } conf { pair.first, pair.second };    
    m_collectionFilter[ conf.collection ].insert( HLT::Identifier( conf.chain).numeric() );
    ATH_MSG_DEBUG( "Final decision of the chain " << conf.chain << " will be read from " << conf.collection );
  }

  if (m_enableCostMonitoring) {
    CHECK( m_trigCostSvcHandle.retrieve() );
    CHECK( m_costWriteHandleKey.initialize() );
  }
  
  return StatusCode::SUCCESS;
}

StatusCode DecisionSummaryMakerAlg::finalize() {
  return StatusCode::SUCCESS;
}

StatusCode DecisionSummaryMakerAlg::execute(const EventContext& context) const {
  SG::WriteHandle<TrigCompositeUtils::DecisionContainer> outputHandle = TrigCompositeUtils::createAndStore( m_summaryKey, context );
  auto container = outputHandle.ptr();

  TrigCompositeUtils::Decision* output = TrigCompositeUtils::newDecisionIn( container, "HLTSummary" );
  
  for ( auto& key: m_finalDecisionKeys ) {
    auto handle{ SG::makeHandle(key, context) };
    if ( not handle.isValid() )  {
      ATH_MSG_DEBUG("missing input " <<  key.key() << " likely rejected");
      continue;
    }
    const auto thisCollFilter = m_collectionFilter.find( key.key() );
    if ( thisCollFilter == m_collectionFilter.end() ) {
      ATH_MSG_WARNING( "The colleciton " << key.key() << " is not configured to contain any final decision, remove it from the configuration of " << name() << " to save time" );
      continue;
    }

    TrigCompositeUtils::DecisionIDContainer sum;
    for ( const TrigCompositeUtils::Decision* decisionObject: *handle ) {
      sum.insert( TrigCompositeUtils::decisionIDs(decisionObject).begin(), TrigCompositeUtils::decisionIDs(decisionObject).end() );  // copy from vector
    }
    
    TrigCompositeUtils::DecisionIDContainer finalIDs;
    std::set_intersection(  sum.begin(), sum.end(),
			    thisCollFilter->second.begin(), thisCollFilter->second.end(), 
			    std::inserter(finalIDs, finalIDs.begin() ) ); // should be faster than remove_if
    
    TrigCompositeUtils::decisionIDs( output ).insert( TrigCompositeUtils::decisionIDs( output ).end(), 
						      finalIDs.begin(), finalIDs.end() );
    
  }
  if ( msgLvl( MSG::DEBUG ) ) {
    ATH_MSG_DEBUG( "Number of positive decisions " <<  TrigCompositeUtils::decisionIDs( output ).size() << " passing chains");
    for ( auto d: TrigCompositeUtils::decisionIDs( output ) ) {
      ATH_MSG_DEBUG( HLT::Identifier( d ) );
    }
  }

  // Do cost monitoring
  if (m_enableCostMonitoring) {
    SG::WriteHandle<xAOD::TrigCompositeContainer> costMonOutput = TrigCompositeUtils::createAndStore(m_costWriteHandleKey, context);
    // Populate collection (assuming monitored event, otherwise collection will remain empty)
    ATH_CHECK(m_trigCostSvcHandle->endEvent(context, costMonOutput));
  }

  return StatusCode::SUCCESS;
}


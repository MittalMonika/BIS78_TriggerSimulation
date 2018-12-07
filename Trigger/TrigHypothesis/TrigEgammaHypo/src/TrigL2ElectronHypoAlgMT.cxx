/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#include <map>
#include "GaudiKernel/Property.h"
#include "TrigL2ElectronHypoAlgMT.h"
#include "AthViews/ViewHelper.h"


using TrigCompositeUtils::DecisionContainer;
using TrigCompositeUtils::DecisionAuxContainer;
using TrigCompositeUtils::DecisionIDContainer;
using TrigCompositeUtils::decisionIDs;
using TrigCompositeUtils::newDecisionIn;
using TrigCompositeUtils::linkToPrevious;


TrigL2ElectronHypoAlgMT::TrigL2ElectronHypoAlgMT( const std::string& name, 
			  ISvcLocator* pSvcLocator ) : 
  ::HypoBase( name, pSvcLocator ) {}

TrigL2ElectronHypoAlgMT::~TrigL2ElectronHypoAlgMT() {}

StatusCode TrigL2ElectronHypoAlgMT::initialize() {
  ATH_MSG_INFO ( "Initializing " << name() << "..." );

  CHECK( m_hypoTools.retrieve() );
  
  CHECK( m_electronsKey.initialize() );
  if (  m_runInView)   renounce( m_electronsKey );// clusters are made in views, so they are not in the EvtStore: hide them

  return StatusCode::SUCCESS;
}

 StatusCode TrigL2ElectronHypoAlgMT::finalize() {
    ATH_MSG_INFO( "Finalizing " << name() << "..." );
    return StatusCode::SUCCESS;
  }

StatusCode TrigL2ElectronHypoAlgMT::execute_r( const EventContext& context ) const {  
  ATH_MSG_DEBUG ( "Executing " << name() << "..." );
  auto previousDecisionsHandle = SG::makeHandle( decisionInput(), context );
  if( not previousDecisionsHandle.isValid() ) {//implicit
    ATH_MSG_DEBUG( "No implicit RH for previous decisions "<<  decisionInput().key()<<": is this expected?" );
    return StatusCode::SUCCESS;      
  }

  ATH_MSG_DEBUG( "Running with "<< previousDecisionsHandle->size() <<" implicit ReadHandles for previous decisions");
  
  // new output
  auto decisions = std::make_unique<DecisionContainer>();
  auto aux = std::make_unique<DecisionAuxContainer>();
  decisions->setStore( aux.get() );

  // // extract mapping of cluster pointer to an index in the cluster decision collection

  // prepare decisions container and link back to the clusters, and decision on clusters  
  std::map<const xAOD::TrigEMCluster*, size_t> clusterToIndexMap;
  size_t clusterCounter = 0;
  for ( auto previousDecision : *previousDecisionsHandle){
    TrigCompositeUtils::LinkInfo<xAOD::TrigEMClusterContainer> linkInfo = 
      TrigCompositeUtils::findLink<xAOD::TrigEMClusterContainer>(previousDecision, "feature");
    ElementLink<xAOD::TrigEMClusterContainer> clusterLink = linkInfo.link;

    ATH_CHECK( clusterLink.isValid() );    
    const xAOD::TrigEMCluster* cluster = *clusterLink;
    clusterToIndexMap.insert( std::make_pair( cluster, clusterCounter ) );
    clusterCounter++;
  }
  ATH_MSG_DEBUG( "Cluster ptr to decision map has size " << clusterToIndexMap.size() );

  // prepare imput for tools
  std::vector<TrigL2ElectronHypoTool::ElectronInfo> hypoToolInput;
 
  for ( auto previousDecision: *previousDecisionsHandle ) {
      // get View
    auto viewELInfo = TrigCompositeUtils::findLink< ViewContainer >( previousDecision, "view" );
    CHECK( viewELInfo.isValid() );

    // get electron from that view:
    size_t electronCounter = 0;
    auto electronsHandle = ViewHelper::makeHandle( *viewELInfo.link, m_electronsKey, context );
    ATH_CHECK( electronsHandle.isValid() );
    ATH_MSG_DEBUG ( "electron handle size: " << electronsHandle->size() << "..." );

    for ( auto electronIter = electronsHandle->begin(); electronIter != electronsHandle->end(); ++electronIter, electronCounter++ ) {
      auto d = newDecisionIn( decisions.get() );
      d->setObjectLink( "feature", ViewHelper::makeLink<xAOD::TrigElectronContainer>( *viewELInfo.link, electronsHandle, electronCounter ) );
      
      auto clusterPtr = (*electronIter)->emCluster();
      CHECK( clusterPtr != nullptr );

      // now find matching cluster 
      // could use geometric matching but in fact the cluster owned by the decision object and the cluster owned by the electron should be the same
      // since we have a map made in advance we can make use of the index lookup w/o the need for additional loop 
      auto origCluster = clusterToIndexMap.find( clusterPtr );
      ATH_CHECK( origCluster != clusterToIndexMap.end() );
      linkToPrevious( d, decisionInput().key(), origCluster->second );
      
      // now we have DecisionObject ready to be passed to hypo tool. it has link to electron, 
      // and decisions on clusters
      // we shall avoid calling the tools for chains which were already rejected on certain cluster, but this is left to hypo tools
      DecisionIDContainer clusterDecisionIDs;
      decisionIDs( previousDecisionsHandle->at( origCluster->second ), clusterDecisionIDs );
      
      hypoToolInput.emplace_back( TrigL2ElectronHypoTool::ElectronInfo{ d, *electronIter,  origCluster->first, clusterDecisionIDs } );
    }
  }

  for ( auto & tool: m_hypoTools ) {
    ATH_CHECK( tool->decide( hypoToolInput ) );    
  } 

  auto outputHandle = SG::makeHandle(decisionOutput(), context);
  CHECK( outputHandle.record(std::move(decisions), std::move(aux) ) );
  
  ATH_MSG_DEBUG( "Exiting with "<< outputHandle->size() <<" decisions");
  //debug
  for (auto outh: *outputHandle){
    TrigCompositeUtils::DecisionIDContainer objDecisions;      
    TrigCompositeUtils::decisionIDs( outh, objDecisions );
    
    ATH_MSG_DEBUG("Number of positive decisions for this input: " << objDecisions.size() );
    
    for ( TrigCompositeUtils::DecisionID id : objDecisions ) {
      ATH_MSG_DEBUG( " --- found new decision " << HLT::Identifier( id ) );
    }  
    
  }

  return StatusCode::SUCCESS;
}





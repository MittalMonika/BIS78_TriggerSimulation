/*
  General-purpose view creation algorithm <bwynne@cern.ch>
  
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "EventViewCreatorAlgorithm.h"
#include "AthLinks/ElementLink.h"
#include "AthViews/ViewHelper.h"
#include "AthViews/View.h"
#include "DecisionHandling/TrigCompositeUtils.h"
#include "DecisionHandling/HLTIdentifier.h"


EventViewCreatorAlgorithm::EventViewCreatorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : InputMakerBase( name, pSvcLocator ) {}

EventViewCreatorAlgorithm::~EventViewCreatorAlgorithm(){}

StatusCode EventViewCreatorAlgorithm::initialize() {
  ATH_MSG_DEBUG("Will produce views=" << m_viewsKey << " roIs=" << m_inViewRoIs );
  ATH_CHECK( m_viewsKey.initialize() );
  ATH_CHECK( m_inViewRoIs.initialize() );
  ATH_CHECK( m_scheduler.retrieve() );
  return StatusCode::SUCCESS;
}

StatusCode EventViewCreatorAlgorithm::execute_r( const EventContext& context ) const { 
  auto outputHandles = decisionOutputs().makeHandles( context );     
  // make the views
  auto viewVector = std::make_unique< ViewContainer >();
  auto contexts = std::vector<EventContext>( );
  unsigned int viewCounter = 0;
  unsigned int conditionsRun = getContext().getExtension<Atlas::ExtendedEventContext>()->conditionsRun();

  //  const TrigRoiDescriptor* previousRoI = 0;
  // mapping RoI with index of the View in the vector
  // This is used to link the same view to differnt decisions that come from the same RoI
  std::map <const TrigRoiDescriptor*, int> viewMap;
  std::map <const TrigRoiDescriptor*, int>::iterator itViewMap;
  int outputIndex = -1;
 // Loop over all input containers, which are of course TrigComposites, and request their features
  // this is the same as InputMaker, apart from the view creation. The loop can be splitted in two loops, to have one common part
  for ( auto inputKey: decisionInputs() ) {
    outputIndex++;
    auto inputHandle = SG::makeHandle( inputKey, context );
    if( not inputHandle.isValid() ) {
      ATH_MSG_DEBUG( "Got no decisions from input " << inputKey.key() );
      continue;
    }
    
    if( inputHandle->size() == 0 ) { // input filtered out
      ATH_MSG_ERROR( "Got 0 decisions from valid input "<< inputKey.key()<<". Is it expected?");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG( "Got input " << inputKey.key() << " with " << inputHandle->size() << " elements" );
    
     // prepare output decisions
    auto outputDecisions = std::make_unique<TrigCompositeUtils::DecisionContainer>();    
    auto decAux = std::make_unique<TrigCompositeUtils::DecisionAuxContainer>();
    outputDecisions->setStore( decAux.get() );


    size_t inputCounter = 0;
    for ( auto inputDecision: *inputHandle ) {
      //make one TC decision output per input and connect to previous
      auto newDecision = TrigCompositeUtils::newDecisionIn( outputDecisions.get(), name() );
      TrigCompositeUtils::linkToPrevious( newDecision, inputKey.key(), inputCounter );
      insertDecisions( inputDecision, newDecision );
      
      // pull RoI descriptor
      TrigCompositeUtils::LinkInfo<TrigRoiDescriptorCollection> roiELInfo = TrigCompositeUtils::findLink<TrigRoiDescriptorCollection>(inputDecision, m_roisLink );
      ATH_CHECK( roiELInfo.isValid() );
      // associate this RoI to output decisions
      auto roiDescriptor = *roiELInfo.link;
      ATH_MSG_DEBUG( "Placing TrigRoiDescriptor " << *roiDescriptor );
      newDecision->setObjectLink( "initialRoI", roiELInfo.link );

      
      // search for existing view
      itViewMap = viewMap.find(roiDescriptor);
      if ( itViewMap != viewMap.end() ) {
	int iview = itViewMap->second;
	newDecision->setObjectLink( "view", ElementLink< ViewContainer >(m_viewsKey.key(), iview ) ); //adding view to TC
	ATH_MSG_DEBUG( "Adding already mapped view " << iview << " in ViewVector , to new decision");
	//	need to check if this View has parent views? can we have more than one parent views?
      } else {
	
	ATH_MSG_DEBUG( "Positive decisions on RoI, preparing view" );
	
	// make the view
	ATH_MSG_DEBUG( "Making the View" );
	auto newView = ViewHelper::makeView( name()+"_view", viewCounter++, m_viewFallThrough ); //pointer to the view
	viewVector->push_back( newView );
	contexts.emplace_back( context );
	contexts.back().setExtension( Atlas::ExtendedEventContext( viewVector->back(), conditionsRun ) );
	
	// link decision to this view
	newDecision->setObjectLink( "view", ElementLink< ViewContainer >(m_viewsKey.key(), viewVector->size()-1 ));//adding view to TC
      	viewMap[roiDescriptor]=viewVector->size()-1;
	ATH_MSG_DEBUG( "Adding new view to new decision; storing view in viewVector component " << viewVector->size()-1 );


	ATH_CHECK( linkViewToParent( inputDecision, viewVector->back() ) );
	ATH_CHECK( placeRoIInView( roiDescriptor, viewVector->back(), contexts.back() ) );
	
      }
      inputCounter++;      
    }

    ATH_MSG_DEBUG( "Recording output key " <<  decisionOutputs()[ outputIndex ].key() <<" of size "<< outputDecisions->size()  <<" at index "<< outputIndex);
    ATH_CHECK( outputHandles[outputIndex].record( std::move( outputDecisions ), std::move( decAux ) ) );
  }

  
  ATH_MSG_DEBUG( "Launching execution in " << viewVector->size() << " views" );
  ATH_CHECK( ViewHelper::ScheduleViews( viewVector.get(),           // Vector containing views
					m_viewNodeName,             // CF node to attach views to
					context,                    // Source context
					m_scheduler.get() ) );

  
  // store views
  auto viewsHandle = SG::makeHandle( m_viewsKey );
  ATH_CHECK( viewsHandle.record(  std::move( viewVector ) ) );
  ATH_MSG_DEBUG( "Store "<< viewsHandle->size() <<" Views");


  size_t validInputCount = countInputHandles( context );  
  size_t validOutputCount = 0;
  for ( auto outHandle: outputHandles ) {
    if( not outHandle.isValid() ) continue;
    validOutputCount++;
  }
  
  ATH_MSG_DEBUG("Produced " << validOutputCount << " decisions containers");
  if(validInputCount != validOutputCount ) {
    ATH_MSG_ERROR("Found " << validInputCount << " inputs and " << validOutputCount << " outputs");
    return StatusCode::FAILURE;
  }

  printDecisions( outputHandles );
     
  return StatusCode::SUCCESS;
}

size_t EventViewCreatorAlgorithm::countInputHandles( const EventContext& context ) const {
  size_t validInputCount=0;
  for ( auto inputKey: decisionInputs() ) {
    auto inputHandle = SG::makeHandle( inputKey, context );
    ATH_MSG_DEBUG(" " << inputKey.key() << (inputHandle.isValid()? "valid": "not valid" ) );
    if (inputHandle.isValid()) validInputCount++;
  }
  ATH_MSG_DEBUG( "number of implicit ReadHandles is " << decisionInputs().size() << ", " << validInputCount << " are valid" );
  
  return validInputCount;
}


void EventViewCreatorAlgorithm::printDecisions( const std::vector<SG::WriteHandle<TrigCompositeUtils::DecisionContainer>>& outputHandles ) const {
  if ( not msgLvl( MSG::DEBUG ) )
    return;
      
  for ( auto outHandle: outputHandles ) {
    if( not outHandle.isValid() ) continue;
    ATH_MSG_DEBUG(outHandle.key() << " with " << outHandle->size() << " decisions:");
    for ( auto outdecision:  *outHandle ) {
      TrigCompositeUtils::DecisionIDContainer objDecisions;      
      TrigCompositeUtils::decisionIDs( outdecision, objDecisions );
      
      ATH_MSG_DEBUG("Number of positive decisions for this output: " << objDecisions.size() );
      
      for ( TrigCompositeUtils::DecisionID id : objDecisions ) {
	ATH_MSG_DEBUG( " ---  decision " << HLT::Identifier( id ) );
      }  
    }
  }    
}

void EventViewCreatorAlgorithm::insertDecisions( const TrigCompositeUtils::Decision* src, TrigCompositeUtils::Decision* dest ) const  {
  using namespace TrigCompositeUtils;
  decisionIDs(dest).reserve( decisionIDs(dest).size() + decisionIDs(src).size() );
  decisionIDs(dest).insert( decisionIDs(dest).end(), decisionIDs(src).begin(), decisionIDs(src).end() );
}	

StatusCode EventViewCreatorAlgorithm::linkViewToParent( const TrigCompositeUtils::Decision* inputDecision, SG::View* newView ) const {
  // see if there is a view linked to the decision object, if so link it to the view that is just made
  TrigCompositeUtils::LinkInfo<ViewContainer> parentViewLinkInfo = TrigCompositeUtils::findLink<ViewContainer>(inputDecision, "view" );
  if ( parentViewLinkInfo.isValid() ) {
    ATH_CHECK( parentViewLinkInfo.link.isValid() );
    auto parentView = *parentViewLinkInfo.link;
    newView->linkParent( parentView );
    ATH_MSG_DEBUG( "Parent view linked" );
  } else {
    if ( m_requireParentView ) {
      ATH_MSG_ERROR( "Parent view not linked because it could not be found" );
      ATH_MSG_ERROR( TrigCompositeUtils::dump( inputDecision, [](const xAOD::TrigComposite* tc){ 
								return "TC " + tc->name() + ( tc->hasObjectLink("view") ? " has view " : " has no view " );
							      } ) );
      return StatusCode::FAILURE;
    }
    
  }
  return StatusCode::SUCCESS;
}

StatusCode EventViewCreatorAlgorithm::placeRoIInView( const TrigRoiDescriptor* roi, SG::View* view, const EventContext& context ) const {
  // fill the RoI output collection
  auto oneRoIColl = std::make_unique< ConstDataVector<TrigRoiDescriptorCollection> >();    
  oneRoIColl->clear( SG::VIEW_ELEMENTS ); //Don't delete the RoIs
  oneRoIColl->push_back( roi );
	
  //store the RoI in the view
  auto handle = SG::makeHandle( m_inViewRoIs, context );
  ATH_CHECK( handle.setProxyDict( view ) );
  ATH_CHECK( handle.record( std::move( oneRoIColl ) ) );
  return StatusCode::SUCCESS;
}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// TrigUpgradeTest includes
#include "TestInputMaker.h"
#include <xAODTrigger/TrigCompositeAuxContainer.h>
#include <TrigSteeringEvent/TrigRoiDescriptorCollection.h>
#include <DecisionHandling/TrigCompositeUtils.h>
#include <vector>

namespace HLTTest {
  
  TestInputMaker::TestInputMaker( const std::string& name, 
			    ISvcLocator* pSvcLocator )    
    : AthAlgorithm( name, pSvcLocator ),
      m_linkName("feature") {
    declareProperty( "Inputs", m_inputs, "Input Decisions (implicit)" );
    declareProperty( "Output", m_recoOutput, "name of the output collection for input to next reco alg in sequence");
    declareProperty( "OutputType", m_outputType, "reserved for future use");
    declareProperty( "LinkName", m_linkName, "name of the link to the features in the decision, e.g. 'feature', 'InitialRoI'");
  }

  TestInputMaker::~TestInputMaker() {}

  StatusCode TestInputMaker::initialize() {
    ATH_MSG_INFO ("Initializing " << name() << "...");
    CHECK( m_inputs.initialize() );
    renounceArray(m_inputs);

    ATH_MSG_DEBUG("Will consume implicit input data:" );
    for (auto& input: m_inputs){  
      ATH_MSG_DEBUG(" "<<input.key());
    }
    ATH_MSG_DEBUG(" and produce " << m_recoOutput);

    CHECK( m_recoOutput.initialize() );
    return StatusCode::SUCCESS;
  }

  StatusCode TestInputMaker::finalize() {
    ATH_MSG_INFO( "Finalizing " << name() << "..." );

    return StatusCode::SUCCESS;
  }

  StatusCode TestInputMaker::execute() {
    ATH_MSG_DEBUG( "Executing " << name() << "..." );

    //auto outputHandle = SG::makeHandle(m_recoOutput);

    // make the new output collection, as a view container so it can be given const features
    //    auto output = std::make_unique<ConstDataVector<xAOD::TrigCompositeContainer> >();
    auto output = std::make_unique<xAOD::TrigCompositeContainer>();
    auto aux = std::make_unique<xAOD::TrigCompositeAuxContainer>();
    output->setStore( aux.get() );

    //auto c = std::make_unique<ConstDataVector<TrigRoiDescriptorCollection> >();
    //output->clear( SG::VIEW_ELEMENTS );
    // view containers do not have aux stores of their own so no need to create nor call output->setStore

    ATH_MSG_DEBUG( "number of implicit ReadHandles is " << m_inputs.size() );

    // Loop over contents of this, which are of course TrigComposites, and request their features
    //    auto inputHandles = m_inputs.makeHandles();
    size_t outputIndex = 0;
    for ( auto inputKey: m_inputs ) {
      auto inputHandle = SG::makeHandle( inputKey );
      //    for (auto inputHandle: inputHandles){
      ATH_MSG_DEBUG( "Got DecisionContainer from input "<< inputKey.key()<<" with " << inputHandle->size() << " elements" );

      // loop over decisions retrieved from this input
      // The input is a DecisionContainer, alias for a TrigCompositeContainer.
      for (auto decision=inputHandle->begin(); decision!=inputHandle->end(); ++decision){
	//        ATH_MSG_DEBUG( "Got Decision (TrigComposite): " << *decision);
        // retrieve feature from TrigComposite, will in this case be a TrigRoiDescriptor
        // objectLink method is templated with container but returns single contained class
        //ElementLink<xAOD::TrigCompositeContainer> feature = (*decision)->objectLink<xAOD::TrigCompositeContainer>( m_linkName.value() );
        auto featurelink = (*decision)->objectLink<TrigRoiDescriptorCollection>( m_linkName.value() );
        if ( not featurelink.isValid() )  {
          ATH_MSG_ERROR( " Can not find reference to " + m_linkName.value() + " from the decision" );
          return StatusCode::FAILURE;
        }
        
	auto featurePtr(featurelink.cptr());
	ATH_MSG_DEBUG("    Found feature " <<m_linkName.value() <<": " << **featurePtr );

        // copy all features to a single output collection 
        // they just happen to be TrigComposites in this case but should be whatever is needed as input by the reco algorithm next in the sequence, e.g. CaloCells.
      
	//auto output = std::make_unique< ConstDataVector< xAOD::TrigCompositeContainer > > ();
	//output->clear( SG::VIEW_ELEMENTS );
	auto tc = new xAOD::TrigComposite;
	output->push_back(tc); 
	tc->setObjectLink(m_linkName.value(), featurelink);
	//output->setObjectLink(m_linkName.value(), featurelink);
        ATH_MSG_DEBUG( "    copied feature " << m_linkName.value() <<" to output " <<outputIndex);
	//	CHECK( outputHandle.record( std::move( output ) ) );
	outputIndex++;
	//        tc->setDetail( m_linkName.value(), feature );
	
        // For early tests, create TC, link to RoiD, push back onto TCC.
        // Later will output RoID collection directly via tool.
        
      } // loop over decisions
    } // loop over input keys
    

    ATH_MSG_DEBUG("Produced "<<output->size() <<" objects");
    // Finally, record output
    auto outputHandle = SG::makeHandle(m_recoOutput);
    CHECK( outputHandle.record(std::move(output), std::move(aux)) );
  
    return StatusCode::SUCCESS;
  }

} //> end namespace HLTTest

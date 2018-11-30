/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#include <algorithm>
#include "GaudiKernel/Property.h"
#include "DecisionHandling/HLTIdentifier.h"
#include "TrigSignatureMoniMT.h"



TrigSignatureMoniMT::TrigSignatureMoniMT( const std::string& name, 
			  ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator ) 
{}

StatusCode TrigSignatureMoniMT::initialize() {

  ATH_CHECK( m_l1DecisionsKey.initialize() );
  ATH_CHECK( m_finalDecisionKey.initialize() );
  ATH_CHECK( m_collectorTools.retrieve() );
  CHECK( m_histSvc.retrieve() );
      
  
  {
    const int x = nBinsX();
    const int y = nBinsY();
    ATH_MSG_DEBUG( "Histogram " << x << " x " << y << " bins");
    // this type to be replaced by LBN tagged hist available for MT
    // the new way, (does not work)
    /*
    auto hist = std::make_unique<TH2I>( "SignatureAcceptance", "Raw acceptance of signatures in;chain;step",
					x, -0.5, x -0.5,
					y, -0.5, y - 0.5 );

    std::string fullName = m_bookingPath + "/SignatureAcceptance";

    m_histSvc->regHist( fullName, std::move( hist ) );
    m_outputHistogram = m_histSvc.getHist( fullName );
    */

    m_outputHistogram = new TH2I( "SignatureAcceptance", "Raw acceptance of signatures in;chain;step",
				  x, 1, x + 1,
				  y, 1, y + 1 ); // Fill and GetBinContent use the same indexing then
    const std::string fullName = m_bookingPath + "/SignatureAcceptance";
    m_histSvc->regHist( fullName, m_outputHistogram );
    
    ATH_MSG_DEBUG( "Registerd under " << fullName );
  }
  CHECK( initHist() );
  

  return StatusCode::SUCCESS;
}

StatusCode TrigSignatureMoniMT::finalize() {
  
  auto collToString = [&]( int xbin){ 
    std::string v;
    for ( int ybin = 1; ybin <= m_outputHistogram->GetYaxis()->GetNbins(); ++ybin ) {
      v += std::to_string( int(m_outputHistogram->GetBinContent( xbin, ybin )) );
      v += std::string( 10*ybin - v.size(),  ' ' ); // fill with spaces
    }
    
    return v;
  };

  ATH_MSG_INFO("Chain name                   L1,      AfterPS, [... steps ...], Output"  );
  for ( int bin = 1; bin <= m_outputHistogram->GetXaxis()->GetNbins(); ++bin ) {
    const std::string chainName = m_outputHistogram->GetXaxis()->GetBinLabel(bin);
    ATH_MSG_INFO( chainName << std::string( 30 - chainName.size(), ' ' ) << collToString( bin ) );
  }

  return StatusCode::SUCCESS;
}

StatusCode TrigSignatureMoniMT::fillChains(const TrigCompositeUtils::DecisionIDContainer& dc, int row) {
  for ( auto id : dc )  {
    ATH_MSG_DEBUG( "row " << row << " " << HLT::Identifier(id) );
    auto id2bin = m_chainIDToBinMap.find( id );
    if ( id2bin == m_chainIDToBinMap.end() ) {
      ATH_MSG_WARNING( "HLT chain " << HLT::Identifier(id) << " not configured to be monitored" );
    } else {
      m_outputHistogram->Fill( id2bin->second, double(row) );
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode TrigSignatureMoniMT::execute()  {  

  auto l1Decisions = SG::makeHandle( m_l1DecisionsKey );
  CHECK( l1Decisions->at( 0 )->name() == "l1seeded" ); 
  CHECK( l1Decisions->at( 1 )->name() == "unprescaled" ); // see L1Decoder implementation

  auto fillL1 = [&]( int index ) -> StatusCode {    
    TrigCompositeUtils::DecisionIDContainer ids;    
    TrigCompositeUtils::decisionIDs( l1Decisions->at( index ), ids );
    ATH_MSG_DEBUG( "L1 " << index << " N positive decisions " << ids.size()  );
    CHECK( fillChains( ids, index + 1 ) );
    if ( not ids.empty() )
      m_outputHistogram->Fill( 1, double(index + 1) );
    return StatusCode::SUCCESS;
  };

  CHECK( fillL1(0) );
  CHECK( fillL1(1) );
  int step = 0;
  for ( auto& ctool: m_collectorTools ) {
    TrigCompositeUtils::DecisionIDContainer stepSum;
    ctool->getDecisions( stepSum );
    ATH_MSG_DEBUG( " Step " << step << " decisions " << stepSum.size() );
    ATH_CHECK( fillChains( stepSum, 3+step ) );    
    ++step;
  }
   
  const int row = m_outputHistogram->GetYaxis()->GetNbins();
  auto finalDecisionsHandle = SG::makeHandle( m_finalDecisionKey );
  ATH_CHECK( finalDecisionsHandle.isValid() );
  ATH_CHECK( finalDecisionsHandle->size() == 1 );
  TrigCompositeUtils::DecisionIDContainer finalIDs;
  TrigCompositeUtils::decisionIDs( finalDecisionsHandle->at(0), finalIDs );
  ATH_CHECK( fillChains( finalIDs, row ) );
  
  if ( not finalIDs.empty() ) {
    m_outputHistogram->Fill( 1, double( row ) );
  }
    
  return StatusCode::SUCCESS;
}




int TrigSignatureMoniMT::nBinsY() const {     
  return m_collectorTools.size()+3; // in, after ps, out
}

StatusCode TrigSignatureMoniMT::initHist() {

  TAxis* x = m_outputHistogram->GetXaxis();
  x->SetBinLabel(1, "All");
  int bin = 2; // 1 is for total count, (remember bins numbering in ROOT start from 1)

  std::vector<std::string> sortedChainsList( m_allChains );
  std::sort( sortedChainsList.begin(), sortedChainsList.end() );
  
  for ( auto chainName:  sortedChainsList ) {
    x->SetBinLabel( bin, chainName.c_str() );
    m_chainIDToBinMap[ HLT::Identifier( chainName ).numeric() ] = bin;
    bin++;
  }


  TAxis* y = m_outputHistogram->GetYaxis();
  y->SetBinLabel( 1, "L1" );
  y->SetBinLabel( 2, "AfterPS" );
  for ( size_t i = 0; i < m_collectorTools.size(); ++i ) {
    y->SetBinLabel( 3+i, ("Step "+std::to_string(i)).c_str() );
  }
  y->SetBinLabel( y->GetNbins(), "Output" ); // last bin

  // loop until the last bin and assign labels Step #
  for ( int bin = 3; std::string( y->GetBinLabel( bin ) ) != "Output" ; ++bin ) {
    y->SetBinLabel(bin, ( "Step " + std::to_string( bin - 2 ) ).c_str() );
  }
 
  return StatusCode::SUCCESS;
}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetPerfPlot_TrkInJet.cxx
 * @author Gabrel Facini <Gabriel.Facini@cern.ch>
 * Oct 23 15:20:45 CEST 2014
**/

#include "InDetPerfPlot_TrkInJet.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

namespace {
bool passJetCuts( const xAOD::Jet& jet ) {
		float etaMin = -2.5;
		float etaMax = 2.5;
		float jetPtMin = 100;  // in GeV
		float jetPtMax = 1000; // in GeV
		float jetPt = jet.pt()/1e3; // GeV
		if( jetPt < jetPtMin ) { return false; }
		if( jetPt > jetPtMax ) { return false; }
		float eta = jet.eta();
		if( eta < etaMin ) { return false; }
		if( eta > etaMax ) { return false; }

		return true;
	}

}

InDetPerfPlot_TrkInJet::InDetPerfPlot_TrkInJet(InDetPlotBase *pParent, std::string sDir):InDetPlotBase(pParent, sDir) {
  // these should be configurable
  m_etaMin = -2.5;
  m_etaMax = 2.5;
  m_dRMin = 0;
  m_dRMax = 0.2;
  m_dRNBins = 40;
  m_jetPtMin = 100;  // in GeV
  m_jetPtMax = 1000; // in GeV
  m_jetPtNBins = 45; // 20 GeV bins
  // find the width of the bin so can easily find appropiate bin later
  m_dRWidth = ( m_dRMax - m_dRMin ) / float(m_dRNBins);
  // one element in the array for each bin of the histogram
  for( int i=0; i<m_dRNBins; i++) {
    m_tracksVsDr.push_back(0);
    m_sumPtVsDr.push_back(0.0);
    m_tracksVsDr_reco.push_back(0);
    m_sumPtVsDr_reco.push_back(0.0);
    m_tracksVsDr_truth.push_back(0);
    m_sumPtVsDr_truth.push_back(0.0);
  }
}

void InDetPerfPlot_TrkInJet::setEtaRange(float eta_min, float eta_max) {
  m_etaMin = eta_min;
  m_etaMax = eta_max;
}

void InDetPerfPlot_TrkInJet::setJetPtRange(float jetPt_min, float jetPt_max) {
  m_jetPtMin = jetPt_min;
  m_jetPtMax = jetPt_max;
}

void InDetPerfPlot_TrkInJet::initializePlots(){

  const bool prependDirectory(false);

  m_jetPt  = Book1D("jetpT","p_{T} of jets (in GeV);p_{T}(GeV/c)",m_jetPtNBins,m_jetPtMin,m_jetPtMax, prependDirectory);
  m_recPt  = Book1D("recInJetpT","p_{T} of selected rec tracks in jets(in GeV);p_{T}(GeV/c)",200,0.,200, prependDirectory);
  m_nTrack = Book1D("nTrack","Number of Tracks in Jet;N Tracks",50,0.,50, prependDirectory);
  m_sumPt  = Book1D("sumPt","Sum p_{T} of Tracks in Jet;sum p_{T}(GeV/c)",25,0,500,prependDirectory);
  m_fracPt = Book1D("fracPt","Sum p_{T} of Tracks over jet p_{T};sum Track p_{T}/jet p_{T}", 60, 0, 3, prependDirectory);

  n_vs_jetDR_BLayerHits = BookTProfile("HitContent_vs_jetDR_NBlayerHits", 
					    "Number of B-Layer clusters;#Delta R(jet,trk);<Number of B-Layer clusters>",
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  n_vs_jetDR_PixelHits = BookTProfile("HitContent_vs_jetDR_NPixelHits",
					   "Number of Pixel clusters;#Delta R(jet,trk); <Number of Pixel clusters>", 
					   m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  n_vs_jetDR_PixelHoles = BookTProfile("HitContent_vs_jetDR_NPixelHoles",
					    "Number of Pixel holes;#Delta R(jet,trk);<Number of Pixel holes>", 
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  n_vs_jetDR_SCTHits = BookTProfile("HitContent_vs_jetDR_NSCTHits",
					 "Number of SCT clusters;#Delta R(jet,trk);<Number of SCT Clusters>", 
					 m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  n_vs_jetDR_SCTHoles = BookTProfile("HitContent_vs_jetDR_NSCTHoles",
					  "Number of SCT holes;#Delta R(jet,trk);<Number of SCT Holes>", 
					  m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  n_vs_jetDR_TRTHits = BookTProfile("HitContent_vs_jetDR_NTRTHits",
					 "Number of TRT clusters;#Delta R(jet,trk);<Number of TRT Hits>", 
					 m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  n_vs_jetDR_TRTHighThresholdHits = BookTProfile("HitContent_vs_jetDR_NTRTHighThresholdHits",
						      "Number of TRT high threshold Hits;#Delta R(jet,trk);<Number of High Thresh TRT Hits>", 
						      m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );

  //debug plots
  if (m_iDetailLevel >= 100) {
    n_vs_jetDR_BLayerOutliers = BookTProfile("HitContent_vs_jetDR_NBlayerOutliers",
						  "Number of B-layer outliers;#Delta R(jet,trk);<Number of B-Layer Outliers>", 
						  m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );  
    n_vs_jetDR_BLayerSharedHits = BookTProfile("HitContent_vs_jetDR_NBlayerSharedHits",
						    "Number of shared B-layer clusters;#Delta R(jet,trk);<Number of B-Layer Shared Clusters>", 
						    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
    n_vs_jetDR_BLayerSplitHits = BookTProfile("HitContent_vs_jetDR_NBlayerSplitHits", 
						   "Number of split B-layer clusters;#Delta R(jet,trk);<Number of B-Layer Split Clusters>", 
						   m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
    n_vs_jetDR_PixelOutliers = BookTProfile("HitContent_vs_jetDR_NPixelOutliers",
						 "Number of Pixel outliers;#Delta R(jet,trk);<Number of Pixel Outliers>", 
						 m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
    n_vs_jetDR_PixelContribLayers = BookTProfile("HitContent_vs_jetDR_NPixelContribLayers",
						      "Number of contributed Pixel layers;#Delta R(jet,trk);<Number of Pixel Layers>", 
						      m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
    n_vs_jetDR_PixelSharedHits = BookTProfile("HitContent_vs_jetDR_NPixelSharedHits",
						   "Number of shared Pixel clusters;#Delta R(jet,trk);<Number of Shared Pixel Clusters>", 
						   m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
    n_vs_jetDR_PixelSplitHits = BookTProfile("HitContent_vs_jetDR_NPixelSplitHits",
						  "Number of split Pixel clusters; #Delta R(jet,trk);<Number of Split Pixel Clusters>", 
						  m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
    n_vs_jetDR_PixelGangedHits = BookTProfile("HitContent_vs_jetDR_NPixelGangedHits",
						   "Number of ganged Pixel clusters;#Delta R(jet,trk);<Number of Ganged Pixel Clusters>", 
						   m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );    
    n_vs_jetDR_SCTOutliers = BookTProfile("HitContent_vs_jetDR_NSCTOutliers",
					       "Number of SCT Outliers;#Delta R(jet,trk);<Number of SCT Outliers>", 
					       m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
    n_vs_jetDR_SCTDoubleHoles = BookTProfile("HitContent_vs_jetDR_NSCTDoubleHoles",
						  "Number of SCT double holes;#Delta R(jet,trk);<Number of SCT Double Holes>", 
						  m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
    n_vs_jetDR_SCTSharedHits = BookTProfile("HitContent_vs_jetDR_NSCTSharedHits",
									  "Number of shared SCT clusters;#Delta R(jet,trk);<Number of Shared SCT Clusters>", 
									  m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
    n_vs_jetDR_TRTOutliers = BookTProfile("HitContent_vs_jetDR_NTRTOutliers",
					       "Number of TRT outliers;#Delta R(jet,trk);<Number of TRT Outliers>",
									 m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
    n_vs_jetDR_TRTHighThresholdOutliers = BookTProfile("HitContent_vs_jetDR_NTRTHighThresholdOutliers",
							    "Number of TRT High Threshold outliers;#Delta R(jet,trk);<Number of High Thresh TRT Outliers>", 
										      m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  }

  // counter plots
  n_vs_jetDR = BookTProfile("NTracks_vs_jetDR",
					    "Number of Tracks;#Delta R(jet,trk);<Number of Tracks>",
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  sumPt_vs_jetDR = BookTProfile("sumPt_vs_jetDR",
					    "sum trk p_{T};#Delta R(jet,trk);<Sum of trk p_{T}>",
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  fracPt_vs_jetDR = BookTProfile("fracPt_vs_jetDR",
					    "sum trk p_{T}/jet p_{T};#Delta R(jet,trk);<Sum of trk p_{T}/jet p_{T}>",
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  n_vs_jetPt = BookTProfile("NTracks_vs_jetPt",
					    "Number of Tracks;jet p_{T};<Number of Tracks>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, 0, 100, prependDirectory );
  sumPt_vs_jetPt = BookTProfile("sumPt_vs_jetPt",
					    "Sum Track p_{T};jet p_{T};<Sum Trk p_{T}>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, 0, 100, prependDirectory );
  fracPt_vs_jetPt = BookTProfile("fracPt_vs_jetPt",
					    "sum Track p_{T}/jet p_{T};jet p_{T};<sum Trk p_{T}/jet p_{T}>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, 0, 100, prependDirectory );

  n_vs_jetDR_reco = BookTProfile("NTracks_vs_jetDR_reco",
					    "Number of Tracks;#Delta R(jet,trk);<Number of Tracks>",
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  fracPt_vs_jetDR_reco = BookTProfile("fracPt_vs_jetDR_reco",
					    "sum trk p_{T};#Delta R(jet,trk);<Sum of trk p_{T}>",
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  sumPt_vs_jetDR_reco = BookTProfile("sumPt_vs_jetDR_reco",
					    "sum trk p_{T};#Delta R(jet,trk);<Sum of trk p_{T}>",
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  n_vs_jetPt_reco = BookTProfile("NTracks_vs_jetPt_reco",
					    "Number of Tracks;jet p_{T};<Number of Tracks>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, 0, 100, prependDirectory );
  fracPt_vs_jetPt_reco = BookTProfile("fracPt_vs_jetPt_reco",
					    "Sum Track p_{T}/jet p_{T};jet p_{T};<Sum Trk p_{T}/jet p_{T}>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, 0, 100, prependDirectory );
  sumPt_vs_jetPt_reco = BookTProfile("sumPt_vs_jetPt_reco",
					    "Sum Track p_{T}/jet p_{T};jet p_{T};<Sum Trk p_{T}/jet p_{T}>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, 0, 100, prependDirectory );

  n_vs_jetDR_truth = BookTProfile("NTracks_vs_jetDR_truth",
					    "Number of Tracks;#Delta R(jet,trk);<Number of Tracks>",
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  fracPt_vs_jetDR_truth = BookTProfile("fracPt_vs_jetDR_truth",
					    "sum trk p_{T};#Delta R(jet,trk);<Sum of trk p_{T}>",
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  sumPt_vs_jetDR_truth = BookTProfile("sumPt_vs_jetDR_truth",
					    "sum trk p_{T} truth;#Delta R(jet,trk);<Sum of truth trk p_{T}>",
					    m_dRNBins, m_dRMin, m_dRMax, 0, 100, prependDirectory );
  n_vs_jetPt_truth = BookTProfile("NTracks_vs_jetPt_truth",
					    "Number of Tracks;jet p_{T};<Number of Tracks>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, 0, 100, prependDirectory );
  sumPt_vs_jetPt_truth = BookTProfile("sumPt_vs_jetPt_truth",
					    "Sum Track p_{T} truth;jet p_{T};<Sum Truth Trk p_{T}>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, 0, 100, prependDirectory );
  fracPt_vs_jetPt_truth = BookTProfile("fracPt_vs_jetPt_truth",
					    "Sum Track p_{T}/jet p_{T};jet p_{T};<Sum Trk p_{T}/jet p_{T}>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, 0, 100, prependDirectory );

  n_vs_jetDR_eff = Book1D("NTracks_vs_jetDR_eff",
					    "Efficiency Number of Tracks;#Delta R(jet,trk);Efficiency <Number of Tracks>",
					    m_dRNBins, m_dRMin, m_dRMax, prependDirectory );
  sumPt_vs_jetDR_eff = Book1D("sumPt_vs_jetDR_eff",
					    "Efficiency sum trk p_{T};#Delta R(jet,trk);Efficiency <Sum of trk p_{T}>",
					    m_dRNBins, m_dRMin, m_dRMax, prependDirectory );
  n_vs_jetPt_eff = Book1D("NTracks_vs_jetPt_eff",
					    "Efficiency Number of Tracks;jet p_{T};Efficiency <Number of Tracks>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, prependDirectory );
  sumPt_vs_jetPt_eff = Book1D("sumPt_vs_jetPt_eff",
					    "Efficiency Sum Track p_{T};jet p_{T};Efficiency <Sum Trk p_{T}>",
					    m_jetPtNBins, m_jetPtMin, m_jetPtMax, prependDirectory );

  clearCounters();
  clearEffCounters();

}

void InDetPerfPlot_TrkInJet::finalizePlots() {
  
  n_vs_jetDR_eff->Sumw2();
  n_vs_jetDR_eff->Divide(n_vs_jetDR_reco, n_vs_jetDR_truth, 1., 1., "B");
  n_vs_jetPt_eff->Sumw2();
  n_vs_jetPt_eff->Divide(n_vs_jetPt_reco, n_vs_jetPt_truth, 1., 1., "B");
  sumPt_vs_jetDR_eff->Sumw2();
  sumPt_vs_jetDR_eff->Divide(sumPt_vs_jetDR_reco, sumPt_vs_jetDR_truth, 1., 1., "B");
  sumPt_vs_jetPt_eff->Sumw2();
  sumPt_vs_jetPt_eff->Divide(sumPt_vs_jetPt_reco, sumPt_vs_jetPt_truth, 1., 1., "B");
  
}

void InDetPerfPlot_TrkInJet::clearCounters() {
  for(unsigned int i=0; i<m_tracksVsDr.size(); i++) {
    m_tracksVsDr.at(i) = 0;
    m_sumPtVsDr.at(i)  = 0.0;
  }
}

void InDetPerfPlot_TrkInJet::clearEffCounters() {
  for(unsigned int i=0; i<m_tracksVsDr.size(); i++) {
    m_tracksVsDr_reco.at(i) = 0;
    m_sumPtVsDr_reco.at(i)  = 0.0;
    m_tracksVsDr_truth.at(i) = 0;
    m_sumPtVsDr_truth.at(i)  = 0.0;
  }
}


bool InDetPerfPlot_TrkInJet::fill(const xAOD::TrackParticle& trk, const xAOD::Jet& jet) {

  bool pass = passJetCuts( jet );
  if( ! pass ) { return pass; }

  float jetDR = jet.p4().DeltaR( trk.p4() );
  if(jetDR > m_dRMax) { return false; } // protection for vectors

  int idR = (int) (jetDR/m_dRWidth);
  m_tracksVsDr.at(idR)+=1.0;
  m_sumPtVsDr.at(idR) +=trk.pt();

  uint8_t iBLayerHits(0),iPixHits(0),iSctHits(0),iTrtHits(0);
  uint8_t iPixHoles(0),iSCTHoles(0),iTrtHTHits(0);  

  m_recPt->Fill( trk.pt()/1e3 );

  if (trk.summaryValue(iBLayerHits,xAOD::numberOfBLayerHits)) n_vs_jetDR_BLayerHits->Fill(jetDR, iBLayerHits);
  if (trk.summaryValue(iPixHits,xAOD::numberOfPixelHits)) n_vs_jetDR_PixelHits->Fill(jetDR, iPixHits);
  if (trk.summaryValue(iSctHits,xAOD::numberOfSCTHits)) n_vs_jetDR_SCTHits->Fill(jetDR, iSctHits);
  if (trk.summaryValue(iTrtHits,xAOD::numberOfTRTHits)) n_vs_jetDR_TRTHits->Fill(jetDR, iTrtHits);
  if (trk.summaryValue(iPixHoles,xAOD::numberOfPixelHoles)) n_vs_jetDR_PixelHoles->Fill(jetDR, iPixHoles);
  if (trk.summaryValue(iSCTHoles,xAOD::numberOfSCTHoles)) n_vs_jetDR_SCTHoles->Fill(jetDR, iSCTHoles);
  if (trk.summaryValue(iTrtHTHits,xAOD::numberOfTRTHighThresholdHits)) n_vs_jetDR_TRTHighThresholdHits->Fill(jetDR, iTrtHTHits);

  //expert plots
  if (m_iDetailLevel >= 100) {
    uint8_t iBLayerOutliers(0),iBLayerShared(0),iBLayerSplit(0);
    uint8_t iPixelOutliers(0),iPixelContribLayers(0),iPixelShared(0),iPixelSplit(0),iPixelGanged(0);
    uint8_t iSCTOutliers(0),iSCTDoubleHoles(0),iSCTShared(0);
    uint8_t iTRTOutliers(0),iTRTHTOutliers(0);
    if (trk.summaryValue(iBLayerOutliers,xAOD::numberOfBLayerOutliers)) n_vs_jetDR_BLayerOutliers->Fill(jetDR, iBLayerOutliers);
    if (trk.summaryValue(iBLayerShared,xAOD::numberOfBLayerSharedHits)) n_vs_jetDR_BLayerSharedHits->Fill(jetDR, iBLayerShared);
    if (trk.summaryValue(iBLayerSplit,xAOD::numberOfBLayerSplitHits)) n_vs_jetDR_BLayerSplitHits->Fill(jetDR, iBLayerSplit);
    if (trk.summaryValue(iPixelOutliers,xAOD::numberOfPixelOutliers)) n_vs_jetDR_PixelOutliers->Fill(jetDR, iPixelOutliers);
    if (trk.summaryValue(iPixelContribLayers,xAOD::numberOfContribPixelLayers)) n_vs_jetDR_PixelContribLayers->Fill(jetDR, iPixelContribLayers);
    if (trk.summaryValue(iPixelShared,xAOD::numberOfPixelSharedHits)) n_vs_jetDR_PixelSharedHits->Fill(jetDR, iPixelShared);
    if (trk.summaryValue(iPixelSplit,xAOD::numberOfPixelSplitHits)) n_vs_jetDR_PixelSplitHits->Fill(jetDR, iPixelSplit);
    if (trk.summaryValue(iPixelGanged,xAOD::numberOfGangedPixels)) n_vs_jetDR_PixelGangedHits->Fill(jetDR, iPixelGanged);
    if (trk.summaryValue(iSCTOutliers,xAOD::numberOfSCTOutliers)) n_vs_jetDR_PixelOutliers->Fill(jetDR, iSCTOutliers);
    if (trk.summaryValue(iSCTDoubleHoles,xAOD::numberOfSCTDoubleHoles)) n_vs_jetDR_SCTDoubleHoles->Fill(jetDR, iSCTDoubleHoles);
    if (trk.summaryValue(iSCTShared,xAOD::numberOfSCTSharedHits)) n_vs_jetDR_SCTSharedHits->Fill(jetDR, iSCTShared);
    if (trk.summaryValue(iTRTOutliers,xAOD::numberOfTRTOutliers)) n_vs_jetDR_TRTOutliers->Fill(jetDR, iTRTOutliers);
    if (trk.summaryValue(iTRTHTOutliers,xAOD::numberOfTRTHighThresholdOutliers)) n_vs_jetDR_TRTHighThresholdOutliers->Fill(jetDR, iTRTHTOutliers);
  }

  return true;
	
}

bool InDetPerfPlot_TrkInJet::fillCounter(const xAOD::Jet& jet) {

  bool pass = passJetCuts( jet );
  if( ! pass ) {
    clearCounters(); // should have never been filled if does not pass jet cuts
    return pass; 
  }


  float totalSumPt(0);
  float totalNTrk(0);
  for( unsigned int i=0; i<m_tracksVsDr.size(); i++ ) {
    // hit the bin center
    float iDR = (i+0.5)*m_dRWidth;
    n_vs_jetDR     -> Fill( iDR, m_tracksVsDr.at(i) );
    sumPt_vs_jetDR -> Fill( iDR, m_sumPtVsDr.at(i) / 1e3 ); // GeV
    fracPt_vs_jetDR-> Fill( iDR, m_sumPtVsDr.at(i) / jet.pt() );
    totalNTrk  += m_tracksVsDr.at(i);
    totalSumPt += m_sumPtVsDr.at(i);
  }

  m_jetPt             -> Fill( jet.pt()/1e3 );
  n_vs_jetPt          -> Fill( jet.pt()/1e3, totalNTrk  );
  sumPt_vs_jetPt      -> Fill( jet.pt()/1e3, totalSumPt/1e3 ); // GeV
  fracPt_vs_jetPt     -> Fill( jet.pt()/1e3, totalSumPt/jet.pt() );

  m_nTrack->Fill( totalNTrk );
  m_sumPt->Fill( totalSumPt/1e3 );
  m_fracPt->Fill( totalSumPt / jet.pt() );

  clearCounters();

  return true;

}

bool InDetPerfPlot_TrkInJet::BookEffReco(const xAOD::TruthParticle& truth, const xAOD::Jet& jet) {
  // fill vectors like above in bins of dR with number of tracks and sum track pT
  // for reco tracks
  bool pass = passJetCuts( jet );
  if( ! pass ) { return pass; }

  float jetDR = jet.p4().DeltaR( truth.p4() );
  if(jetDR > m_dRMax) { return false; } // protection for vectors

  int idR = (int) (jetDR/m_dRWidth);
  m_tracksVsDr_reco.at(idR)+=1.0;
  m_sumPtVsDr_reco.at(idR) +=truth.pt();

  return true;
}

bool InDetPerfPlot_TrkInJet::BookEffTruth(const xAOD::TruthParticle& truth, const xAOD::Jet& jet) {
  // fill vectors like above in bins of dR with number of tracks and sum track pT
  // for truth particle
  bool pass = passJetCuts( jet );
  if( ! pass ) { return pass; }

  float jetDR = jet.p4().DeltaR( truth.p4() );
  if(jetDR > m_dRMax) { return false; } // protection for vectors

  int idR = (int) (jetDR/m_dRWidth);
  m_tracksVsDr_truth.at(idR)+=1.0;
  m_sumPtVsDr_truth.at(idR) +=truth.pt();

  return true;
}

bool InDetPerfPlot_TrkInJet::fillEff(const xAOD::Jet& jet) {
  // fill plots vs dR with number of track and sum pt of tracks in this jet
  // do this for truth and reco tracks separately ( vectors filled about )
  // need some way to get the efficiency from these two sets of histograms
  bool pass = passJetCuts( jet );
  if( ! pass ) { return pass; }

  float totalSumPt_reco(0);
  float totalNTrk_reco(0);
  float totalSumPt_truth(0);
  float totalNTrk_truth(0);
  for( unsigned int i=0; i<m_tracksVsDr_reco.size(); i++ ) {
    // hit the bin center
    float iDR = (i+0.5)*m_dRWidth;
    n_vs_jetDR_reco     -> Fill( iDR, m_tracksVsDr_reco.at(i) );
    fracPt_vs_jetDR_reco -> Fill( iDR, m_sumPtVsDr_reco.at(i) / jet.pt());
    totalNTrk_reco  += m_tracksVsDr_reco.at(i);
    totalSumPt_reco += m_sumPtVsDr_reco.at(i);

    n_vs_jetDR_truth     -> Fill( iDR, m_tracksVsDr_truth.at(i) );
    fracPt_vs_jetDR_truth -> Fill( iDR, m_sumPtVsDr_truth.at(i) / jet.pt() );
    totalNTrk_truth  += m_tracksVsDr_truth.at(i);
    totalSumPt_truth += m_sumPtVsDr_truth.at(i);
  }

  n_vs_jetPt_reco          -> Fill( jet.pt()/1e3, totalNTrk_reco  );
  sumPt_vs_jetPt_reco      -> Fill( jet.pt()/1e3, totalSumPt_reco );
  fracPt_vs_jetPt_reco -> Fill( jet.pt()/1e3, totalSumPt_reco / jet.pt() );

  n_vs_jetPt_truth          -> Fill( jet.pt()/1e3, totalNTrk_truth  );
  sumPt_vs_jetPt_truth      -> Fill( jet.pt()/1e3, totalSumPt_truth );
  fracPt_vs_jetPt_truth -> Fill( jet.pt()/1e3, totalSumPt_truth / jet.pt() );

  clearEffCounters();
  
  return true;
}



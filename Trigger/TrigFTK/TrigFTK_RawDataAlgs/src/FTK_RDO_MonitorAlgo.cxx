/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "FTK_RDO_MonitorAlgo.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/ITHistSvc.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/PixelDetectorManager.h"
#include "InDetReadoutGeometry/SCT_DetectorManager.h"
#include "InDetReadoutGeometry/PixelModuleDesign.h"
#include "InDetReadoutGeometry/SCT_ModuleSideDesign.h"
#include "InDetReadoutGeometry/SCT_BarrelModuleSideDesign.h"
#include "InDetReadoutGeometry/SCT_ForwardModuleSideDesign.h"


#include "TrkSurfaces/Surface.h"

#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "InDetReadoutGeometry/PixelDetectorManager.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"

#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

#include "TrkTrack/Track.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrackSummary/TrackSummary.h"

#include "InDetRIO_OnTrack/SiClusterOnTrack.h"

#include "TMath.h"

using namespace std;


/////////////////////////////////////////////////////////////////////////////
FTK_RDO_MonitorAlgo::FTK_RDO_MonitorAlgo(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_DataProviderSvc("IFTK_DataProviderSvc/IFTK_DataProviderSvc", name),
  m_pixelId(0),
  m_sctId(0),
  m_pixelManager(0),
  m_SCT_Manager(0),
  m_id_helper(0),
  m_minPt(1000.),
  m_maxa0(1.),
  m_maxz0(100.),
  m_mineta(-2.5),
  m_maxeta(2.5),
  m_minphi(-10.),
  m_maxphi(10.),
  m_minMatches(6),
  m_reverseIBLlocx(false)
{
  declareProperty("RDO_CollectionName",m_ftk_raw_trackcollection_Name, "Collection name of RDO");
  declareProperty("offlineTracksName",m_offlineTracksName," offline tracks collection name");
  declareProperty("FTK_DataProvider", m_DataProviderSvc);
  declareProperty("minpt", m_minPt,"Minimum pT for offline track");
  declareProperty("maxa0", m_maxa0,"Maximum a0 for offline track");
  declareProperty("maxz0", m_maxz0,"Maximum z0 for offline track");
  declareProperty("mineta", m_mineta,"Minimum eta for offline track");
  declareProperty("maxeta", m_maxeta,"Maximum eta for offline track");
  declareProperty("minphi", m_minphi,"Minimum phi for offline track");
  declareProperty("maxphi", m_maxphi,"Maximum phi for offline track");
  declareProperty("minMatches", m_minMatches,"Minimum number of matched hits");
  declareProperty("reverseIBLlocx",m_reverseIBLlocx,"reverse direction of IBL locx from FTK");
}

FTK_RDO_MonitorAlgo::~FTK_RDO_MonitorAlgo()
{ }


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


StatusCode FTK_RDO_MonitorAlgo::initialize(){


  MsgStream athlog(msgSvc(), name());
  ATH_MSG_INFO("FTK_RDO_MonitorAlgo::initialize()" );


  //Set up the FTK Data Provider SVC //
  ATH_CHECK(m_DataProviderSvc.retrieve());
  /// Get Histogram Service ///
  ATH_CHECK(service("THistSvc", rootHistSvc));

  

  ///Tree
  std::string trackRootPath;

  std::vector<TH1D*> *histograms = new std::vector<TH1D*> ;

  //Book Histograms//
  Hist_Init(histograms);

  trackRootPath = "/TRACKS/";
  for( uint i = 0; i < histograms->size(); ++i){
    if ( rootHistSvc->regHist( trackRootPath + (*histograms)[i]->GetName(), (*histograms)[i] ).isFailure()) {
      std::string temp = "Failed to book Histogram: " + std::string((*histograms)[i]->GetName());
      ATH_MSG_WARNING(temp);
    } else {
      std::string temp = "Booked Histogram: " + std::string((*histograms)[i]->GetName());
      ATH_MSG_DEBUG(temp);
    }  }
  delete(histograms);

  h_missingHits = new TProfile("h_missingHits", " layer ",12,0.,12.,0.,1.);
  if (rootHistSvc->regHist( trackRootPath + h_missingHits->GetName(), h_missingHits).isFailure()) {
    std::string temp = "Failed to book Histogram: " + std::string(h_missingHits->GetName());
    ATH_MSG_WARNING(temp);
  }


  ATH_CHECK(detStore()->retrieve(m_pixelId, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_sctId, "SCT_ID"));
  ATH_CHECK(detStore()->retrieve(m_pixelManager));
  ATH_CHECK(detStore()->retrieve(m_SCT_Manager));
  ATH_CHECK(detStore()->retrieve(m_id_helper, "AtlasID"));


  ATH_MSG_INFO("RDO_CollectionName " << m_ftk_raw_trackcollection_Name);
  ATH_MSG_INFO("offlineTracksName "<<m_offlineTracksName);
  ATH_MSG_INFO("FTK_DataProvider "<< m_DataProviderSvc);
  ATH_MSG_INFO("minpt "<< m_minPt);
  ATH_MSG_INFO("maxa0 " << m_maxa0);
  ATH_MSG_INFO("maxz0 " << m_maxz0);
  ATH_MSG_INFO("mineta " << m_mineta);
  ATH_MSG_INFO("maxeta " <<  m_maxeta);
  ATH_MSG_INFO("minphi " << m_minphi);
  ATH_MSG_INFO("maxphi " << m_maxphi);
  ATH_MSG_INFO("minMatches " <<  m_minMatches);
  if ( m_reverseIBLlocx ) 
    ATH_MSG_INFO("reversing direction of IBL locx");


  return StatusCode::SUCCESS; 
}


StatusCode FTK_RDO_MonitorAlgo::execute() {
  MsgStream athlog(msgSvc(), name());
  ATH_MSG_DEBUG("FTK_RDO_MonitorAlgo::execute() start");
  
  
  const EventInfo* eventInfo;
  StatusCode scEv = evtStore()->retrieve(eventInfo);
  if (scEv.isFailure()) {
    ATH_MSG_DEBUG( "Could not retrieve event info" );
  } else {
    int eventNumber = eventInfo->event_ID()->event_number();
    ATH_MSG_DEBUG(" Event " << eventNumber);
    
    const EventID* eventID( eventInfo->event_ID());
    ATH_MSG_DEBUG( "entered execution for run" << eventID->run_number()
		   << "event" << eventID->event_number());
  }
  
  // get FTK tracks
  const FTK_RawTrackContainer *rawTracks ( nullptr );
  
  if ( evtStore()->contains<FTK_RawTrackContainer> ( m_ftk_raw_trackcollection_Name) ) {
    if ( evtStore()->retrieve ( rawTracks, m_ftk_raw_trackcollection_Name  ).isFailure() ) {
      ATH_MSG_DEBUG("Could not retrieve FTK_RawTrackContainer " <<  m_ftk_raw_trackcollection_Name << " in StoreGate.");
    }
  } else {
    ATH_MSG_DEBUG("Could not find FTK_RawTrackContainer " <<  m_ftk_raw_trackcollection_Name << " in StoreGate.");
    
  }
  
  if (rawTracks == nullptr) {
    ATH_MSG_DEBUG("No FTK tracks");
    return StatusCode::SUCCESS;
  }
  
  
  ATH_MSG_DEBUG(" Got FTK_RawTrackContainer with " << rawTracks->size() << " tracks");
  h_FTK_RawTrack_n->Fill(rawTracks->size());
  

  unsigned int pixMaxHash = m_pixelId->wafer_hash_max();
  unsigned int sctMaxHash = m_sctId->wafer_hash_max();
  

  std::vector<std::vector<unsigned int>*> pixList(pixMaxHash);
  std::vector<std::vector<unsigned int>*> sctList(sctMaxHash);

  (this)->fillMaps(rawTracks,pixList,sctList);


  const TrackCollection *offlineTracks = nullptr;
  if(evtStore()->retrieve(offlineTracks,m_offlineTracksName).isFailure()){
    ATH_MSG_DEBUG("Failed to retrieve Offline Tracks");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_VERBOSE( " Got offline collection with " << offlineTracks->size()<< " offline tracks" );
  
  auto track_it   = offlineTracks->begin();
  auto last_track = offlineTracks->end();
  
  for (int iTrack=0 ; track_it!= last_track; track_it++, iTrack++) {
  

    const Trk::TrackParameters* trackPars = (*track_it)->perigeeParameters();
    if(trackPars==nullptr) {
      continue;
    }


    float theta = trackPars->parameters()[Trk::theta]; 
    float qOverP = trackPars->parameters()[Trk::qOverP]; 
    if (qOverP==0) {
      //      ATH_MSG_DEBUG("q/p == 0, setting to 1e-12");
      qOverP = 1e-12;
    }
    float pT=sin(theta)/qOverP;
    float a0 = trackPars->parameters()[Trk::d0]; 
    float z0 = trackPars->parameters()[Trk::z0]; 
    float phi0 = trackPars->parameters()[Trk::phi0]; 
    float eta = -log(tan(0.5*theta)); 

    // TODO: handle phi-wrap-arround for phi selection
    
    if (fabs(pT>m_minPt) && fabs(a0) < m_maxa0 && fabs(z0) < m_maxz0 && eta> m_mineta && eta<m_maxeta && phi0> m_minphi && phi0<m_maxphi) {

      ATH_MSG_VERBOSE(" Offline Track " << iTrack << " pT " << pT << " eta " << eta << 
		      " phi0 " <<  phi0 << " d0 " << a0 << " z0 " << z0);


      h_offline_pt->Fill(pT);
      h_offline_eta->Fill(eta);
      h_offline_phi->Fill(phi0);
	  

      std::map<unsigned int,std::pair<double,double> > offlinetrackPixLocxLocy;
      std::map<unsigned int,double> offlinetrackSctLocx;
      bool uniqueMatch=false;

      const std::pair<unsigned int,unsigned int> ftkTrackMatch = (this)->matchTrack(*track_it,pixList, sctList, offlinetrackPixLocxLocy, offlinetrackSctLocx, uniqueMatch);

      h_FTK_nHitMatch->Fill((double)ftkTrackMatch.second);  
      if (ftkTrackMatch.second > 0) {
	ATH_MSG_DEBUG(" matched to FTK track index " << ftkTrackMatch.first << " with " << ftkTrackMatch.second << " matches");
	if (ftkTrackMatch.second > m_minMatches) { 

	  const FTK_RawTrack* ftktrack = rawTracks->at(ftkTrackMatch.first);
	  float ftkTrkTheta = std::atan2(1.0,(ftktrack)->getCotTh());
	  float ftkTrkEta = -std::log(std::tan(ftkTrkTheta/2.));
	  float ftkTrkPt=1.e10;
	  if (fabs((ftktrack)->getInvPt()) >= 1e-9) ftkTrkPt=1./(ftktrack)->getInvPt();
    
	  ATH_MSG_VERBOSE(" FTK     Track " << ftkTrackMatch.second << " pT " << ftkTrkPt << " eta " << ftkTrkEta << 
			  " phi0 " << (ftktrack)->getPhi() << " d0 " << (ftktrack)->getD0() << " z0 " << (ftktrack)->getZ0());
	 
	  //	ATH_MSG_VERBOSE( " nPix: " << (ftkTrack)->getPixelClusters().size() << " nSCT: "<< (ftkTrack)->getSCTClusters().size()<< " barCode: "<<(ftkTrack)->getBarcode()  );
	  //    ATH_MSG_VERBOSE( "     SectorID " << (ftkTrack)->getSectorID()   <<   "  RoadID "  << (ftkTrack)->getRoadID() << " LayerMap " << (ftkTrack)->getLayerMap());


	  h_FTK_pt->Fill(pT);
	  h_FTK_eta->Fill(eta);
	  h_FTK_phi->Fill(phi0);

	  if (uniqueMatch) (this)->compareTracks(ftktrack, offlinetrackPixLocxLocy, offlinetrackSctLocx);
	} else {
	  ATH_MSG_DEBUG(" not enough matched FTK hits");
	}
      } else {
	ATH_MSG_DEBUG(" no matched FTK track");
      }
    }
  }

  for ( auto& ip:pixList) if (ip) delete(ip);
  for ( auto& is:sctList) if (is) delete(is);

  
  ATH_MSG_DEBUG( "FTK_RDO_MonitorAlgo::execute() end" );
  
  return StatusCode::SUCCESS;
  
} 
 


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FTK_RDO_MonitorAlgo::finalize() {
  MsgStream athlog(msgSvc(), name());
  ATH_MSG_INFO("finalize()" );

  return StatusCode::SUCCESS;
}


void FTK_RDO_MonitorAlgo::Hist_Init(std::vector<TH1D*> *histograms){

  // FTK Raw Track Histograms //

  h_FTK_nHitMatch    = new TH1D("h_FTKMoni_nHitMatch",    "No. Hits on track matching Offline; No. Matched Hits; NTracks",      12,  0.,    12.);
  histograms->push_back(h_FTK_nHitMatch);

  h_FTK_nTrackMatch    = new TH1D("h_FTKMoni_nTrackMatch",    "No. FTK tracks matching offline;No. Matched FTK Tracks;",      12,  0.,    12.);
  histograms->push_back(h_FTK_nTrackMatch);

  h_FTK_RawTrack_n    = new TH1D("h_FTKMoni_RawTrack_n",    ";FTK Raw Track multiplicity;No. Tracks",      100,  0.,    1000.);
  histograms->push_back(h_FTK_RawTrack_n);


  h_FTK_pt    = new TH1D("h_FTKMoni_RawTrack_pt",    ";offline track pt[MeV];No. Tracks",      100, 0., 100000.);
  histograms->push_back(h_FTK_pt);
  h_offline_pt    = new TH1D("h_FTKMoni_offline_pt",    ";offline trackpt[MeV];No. Tracks",    100, 0., 100000.);
  histograms->push_back(h_offline_pt);


  h_FTK_eta    = new TH1D("h_FTKMoni_RawTrack_eta",    ";offline track eta;No. Tracks",      104,  -2.6, 2.6);
  histograms->push_back(h_FTK_eta);
  h_offline_eta    = new TH1D("h_FTKMoni_offline_eta",    ";offline track eta;No. Tracks",      104,  -2.6,    2.6);
  histograms->push_back(h_offline_eta);
  h_FTK_phi    = new TH1D("h_FTKMoni_RawTrack_phi",    ";offline track phi[Rad];No. Tracks",      64,  -3.2,    3.2);
  histograms->push_back(h_FTK_phi);
  h_offline_phi    = new TH1D("h_FTKMoni_offline_phi",    ";offline track phi[Rad]; No. Tracks",      64,  -3.2,    3.2);
  histograms->push_back(h_offline_phi);
  
  h_IBL_dlocX_fullRange= new TH1D("h_IBL_dlocX_fullRange", "PIXB0: locx(offline)-locx(ftk)", 100,  -20.,    20.);
  histograms->push_back(h_IBL_dlocX_fullRange);

  h_IBL_dlocY_fullRange= new TH1D("h_IBL_dlocY_fullRange", "PIXB0: locy(offline)-locy(ftk)", 100,  -50.,    50.);
  histograms->push_back(h_IBL_dlocY_fullRange);



  for (Int_t ilay=0; ilay<4; ilay++){

    TString histname("h_FTKMoni_PIXB");
    histname+=ilay;
    histname+="_dlocX";

    TString histtitle("PIXB");
    histtitle+=ilay;
    histtitle+=": locx(offline)-locx(ftk)";

    TH1D* pixb_dlocX = new TH1D(histname, histtitle, 100,  -0.5,    0.5);
    h_pixb_dlocX.push_back(pixb_dlocX);
    histograms->push_back(pixb_dlocX);

    histname="h_FTKMoni_PIXB";
    histname+=ilay;
    histname+="_dlocY";

    histtitle="PIXB";
    histtitle+=ilay;
    histtitle+=": locy(offline)-locy(ftk)";


    TH1D* pixb_dlocY = new TH1D(histname, histtitle, 100,  -0.5,    0.5);
    h_pixb_dlocY.push_back(pixb_dlocY);
    histograms->push_back(pixb_dlocY);

    if (ilay < 3) {

      histname="h_FTKMoni_PIXE";
      histname+=ilay;
      histname+="_dlocX";
      
      histtitle="PIXE";
      histtitle+=ilay;
      histtitle+=": locx(offline)-locx(ftk)";
      
      TH1D* pixe_dlocX = new TH1D(histname, histtitle, 100,  -0.5,    0.5);
      h_pixe_dlocX.push_back(pixe_dlocX);
      histograms->push_back(pixe_dlocX);

      histname="h_FTKMoni_PIXE";
      histname+=ilay;
      histname+="_dlocY";
      
      histtitle="PIXE";
      histtitle+=ilay;
      histtitle+=": locy(offline)-locy(ftk);locy(offline)-locy(ftk);No. Hits";
      
      TH1D* pixe_dlocY = new TH1D(histname, histtitle, 100,  -0.5,    0.5);
      h_pixe_dlocY.push_back(pixe_dlocY);
      histograms->push_back(pixe_dlocY);
    }      
    histname="h_FTKMoni_SCTB";
    histname+=ilay;
    histname+="_0_dlocX";
    
    histtitle="SCTB";
    histtitle+=ilay;
    histtitle+="(side 0): locx(offline)-locx(ftk);locx(offline)-locx(ftk);No. Hits";
    

    TH1D* sctb_0_dlocX = new TH1D(histname, histtitle, 100,  -0.5,    0.5);
    h_sctb_0_dlocX.push_back(sctb_0_dlocX);
    histograms->push_back(sctb_0_dlocX);

    histname="h_FTKMoni_SCTB";
    histname+=ilay;
    histname+="_1_dlocX";

    histtitle="SCTB";
    histtitle+=ilay;
    histtitle+="(side 1): locx(offline)-locx(ftk);locx(offline)-locx(ftk);No. Hits";


    TH1D* sctb_1_dlocX = new TH1D(histname, histtitle, 100,  -0.5,    0.5);
    h_sctb_1_dlocX.push_back(sctb_1_dlocX);
    histograms->push_back(sctb_1_dlocX);

    histname="h_FTKMoni_SCTE";
    histname+=ilay;
    histname+="_0_dlocX";
 
    histtitle="SCTE";
    histtitle+=ilay;
    histtitle+="(side 0): locx(offline)-locx(ftk);locx(offline)-locx(ftk);No. Hits ";


    TH1D* scte_0_dlocX = new TH1D(histname, histtitle, 100,  -0.5,    0.5);
    h_scte_0_dlocX.push_back(scte_0_dlocX);
    histograms->push_back(scte_0_dlocX);

    histname="h_FTKMoni_SCTE";
    histname+=ilay;
    histname+="_1_dlocX";

    histtitle="SCTE";
    histtitle+=ilay;
    histtitle+="(side 1): locx(offline)-locx(ftk);locx(offline)-locx(ftk);No. Hits";


    TH1D* scte_1_dlocX = new TH1D(histname, histtitle, 100,  -0.5,    0.5);
    h_scte_1_dlocX.push_back(scte_1_dlocX);
    histograms->push_back(scte_1_dlocX);

  }

  h_ftk_sct_clusWidth = new TH1D("h_FTKMoni_sct_clusWidth", "SCT cluster width ", 8,  -0.5,    7.5);
  histograms->push_back(h_ftk_sct_clusWidth);

  h_ftk_pix_phiClusWidth = new TH1D("h_FTKMoni_pix_phiClusWidth", "Pixel cluster width in phi", 8,  -0.5,    7.5);
  histograms->push_back(h_ftk_pix_phiClusWidth);

  h_ftk_pix_etaClusWidth = new TH1D("h_FTKMoni_pix_etaClusWidth", "Pixel cluster width in eta", 8,  -0.5,    7.5);
  histograms->push_back(h_ftk_pix_etaClusWidth);

  
  TH1D* clusWidth0 = new TH1D("h_FTKMoni_sct_dLocX_w1to3", "SCT locx(offline)-locx(ftk) cluswidth 1-4", 100,  -0.5,    0.5);
  h_ftk_sctb_dlocX.push_back(clusWidth0);
  histograms->push_back(clusWidth0);
  TH1D* clusWidth1 = new TH1D("h_FTKMoni_sct_dLocX_w1", "SCT locx(offline)-locx(ftk) cluswidth 1", 100,  -0.5,    0.5);
  h_ftk_sctb_dlocX.push_back(clusWidth1);
  histograms->push_back(clusWidth1);
  TH1D* clusWidth2 = new TH1D("h_FTKMoni_sct_dLocX_w2", "SCT locx(offline)-locx(ftk) cluswidth 2", 100,  -0.5,    0.5);
  h_ftk_sctb_dlocX.push_back(clusWidth2);
  histograms->push_back(clusWidth2);
  TH1D* clusWidth3 = new TH1D("h_FTKMoni_sct_dLocX_w3", "SCT locx(offline)-locx(ftk) cluswidth 3", 100,  -0.5,    0.5);
  h_ftk_sctb_dlocX.push_back(clusWidth3);
  histograms->push_back(clusWidth3);
  TH1D* clusWidth4 = new TH1D("h_FTKMoni_sct_dLocX_w4", "SCT locx(offline)-locx(ftk) cluswidth 4", 100,  -0.5,    0.5);
  h_ftk_sctb_dlocX.push_back(clusWidth4);
  histograms->push_back(clusWidth4);

}
		    
		    


void FTK_RDO_MonitorAlgo::fillMaps(const FTK_RawTrackContainer* rawTracks, std::vector<std::vector<unsigned int>*> &pixList, std::vector<std::vector<unsigned int>*> &sctList) {

  
  //  ATH_MSG_DEBUG("fillMaps");

  unsigned int pixMaxHash = m_pixelId->wafer_hash_max();
  unsigned int sctMaxHash = m_sctId->wafer_hash_max();
  
  
  FTK_RawTrackContainer::const_iterator pTrack = rawTracks->begin();
  FTK_RawTrackContainer::const_iterator pLastTrack = rawTracks->end();
  
  
  //    const FTK_RawTrack* highPtTrack = nullptr;


  std::vector<unsigned int>  missingPixLayer(4);
  std::vector<unsigned int> missingSctLayer(8);


  
  for ( int itr=0; pTrack!= pLastTrack; pTrack++, itr++) {
    
          
	    
	    /*
	    float ftkTrkPt=1.e10;
	    if (fabs((*pTrack)->getInvPt()) >= 1e-9) ftkTrkPt=1./(*pTrack)->getInvPt();
	    float ftkTrkTheta = std::atan2(1.0,(*pTrack)->getCotTh());
	    float ftkTrkEta = -std::log(std::tan(ftkTrkTheta/2.));
	      ATH_MSG_VERBOSE( itr << ": pT: " << ftkTrkPt  << " eta: " << ftkTrkEta << " phi: " <<  (*pTrack)->getPhi() << " d0: " << (*pTrack)->getD0() <<
	    " z0: " << (*pTrack)->getZ0()<< " cot " << (*pTrack)->getCotTh() << " theta " << ftkTrkTheta << " invPt " << (*pTrack)->getInvPt() <<
	    " nPix: " << (*pTrack)->getPixelClusters().size() << " nSCT: "<< (*pTrack)->getSCTClusters().size()<< " barCode: "<<(*pTrack)->getBarcode()  );
	    ATH_MSG_VERBOSE( "     SectorID " << (*pTrack)->getSectorID()   <<   "  RoadID "  << (*pTrack)->getRoadID() << " LayerMap " << (*pTrack)->getLayerMap());
	    */
    
    unsigned int iPlane = 0;



    for( unsigned int i = 0; i < (*pTrack)->getPixelClusters().size(); ++i,iPlane++){
      
      
      if ((*pTrack)->isMissingPixelLayer(i))  {
	//	  ATH_MSG_VERBOSE(" Missing Pixel Layer " <<  i);
	if (i<4)missingPixLayer[i]++;
	continue;
      }
      /* 
	 ATH_MSG_VERBOSE(" FTK_RawPixelCluster " << i <<
	 " Row Coord= " << (*pTrack)->getPixelClusters()[i].getRowCoord() <<
	 " Row Width= " << (*pTrack)->getPixelClusters()[i].getRowWidth() <<
	 " Col Coord= " << (*pTrack)->getPixelClusters()[i].getColCoord() <<
	 " Col Width= " << (*pTrack)->getPixelClusters()[i].getColWidth() <<
	 " Hash ID= 0x" << std::hex << (*pTrack)->getPixelClusters()[i].getModuleID() 
	 << std::dec << " Barcode= "   << (*pTrack)->getPixelClusters()[i].getBarcode() 
	 );
      */
      
      
      
      
      IdentifierHash hash = (*pTrack)->getPixelClusters()[i].getModuleID();
      
      if (hash<pixMaxHash) {
	//	ATH_MSG_DEBUG(" Adding to pixList[ "<<hash<<" ] track " << itr);
	if (!pixList[hash]) pixList[hash]= new std::vector<unsigned int>;
	pixList[hash]->push_back(itr);
      }
      
    }
    for( unsigned int isct = 0; isct < (*pTrack)->getSCTClusters().size(); ++isct){
      
      if ((*pTrack)->isMissingSCTLayer(isct)) {
	//	  ATH_MSG_VERBOSE(" Missing SCT Layer" << isct);
	if (isct<8)missingSctLayer[isct]++;
	continue;
      }
      
      /* ATH_MSG_VERBOSE(" FTK_RawSCT_Cluster " << isct <<
	 " Hit Coord= " << (*pTrack)->getSCTClusters()[isct].getHitCoord() <<
	 " Hit Width= " << (*pTrack)->getSCTClusters()[isct].getHitWidth() <<
	 " Module ID= " << (*pTrack)->getSCTClusters()[isct].getModuleID() <<
	 " Barcode= "   << (*pTrack)->getSCTClusters()[isct].getBarcode() );
      */
      
      IdentifierHash hash = (*pTrack)->getSCTClusters()[isct].getModuleID();
      
      if (hash<sctMaxHash) {
	//	ATH_MSG_DEBUG(" Adding to sctList[ "<<hash<<" ] track " << itr);
	
	if (!sctList[hash]) sctList[hash]= new std::vector<unsigned int>;
	  sctList[hash]->push_back(itr);
      }
    
    }
  } // end loop over tracks
  

  unsigned int ntracks = rawTracks->size();
  unsigned int ilay=0;
  for (auto& ipix:missingPixLayer) {
    double frac = (double) ipix/ (double) ntracks;
    ATH_MSG_DEBUG(" Fraction of Tracks Missing Hits from Layer " << ilay << " = "<< frac); 
    h_missingHits->Fill(ilay++,frac);
  }
  for (auto& isct:missingSctLayer) {
    double frac = (double) isct/ (double) ntracks;
    ATH_MSG_DEBUG(" Fraction of Tracks Missing Hits from Layer " << ilay << " = "<< frac); 
    double x = (double) ilay++;
    h_missingHits->Fill(x,frac);
  }
}



const std::pair<unsigned int, unsigned int> FTK_RDO_MonitorAlgo::matchTrack(const Trk::Track* track, 
									    std::vector<std::vector<unsigned int>*> &pixList, // list FTK tracks containing pix hashID 
									    std::vector<std::vector<unsigned int>*> &sctList, // list FTK tracks containing SCT hashID
									    std::map<unsigned int,std::pair<double,double>>& offlinetrackPixLocxLocy,
									    std::map<unsigned int,double>& offlinetrackSctLocx,
									    bool& uniqueMatch
									    ) {

  //  ATH_MSG_DEBUG("matchTrack");
  std::pair<unsigned int, unsigned int> index_freq(0,0);
  
  const DataVector<const Trk::TrackStateOnSurface>* trackStates=track->trackStateOnSurfaces();   
  if(!trackStates) {
    //    ATH_MSG_DEBUG(" No trackStates");
    return index_freq;
  }
  
  
  DataVector<const Trk::TrackStateOnSurface>::const_iterator it=trackStates->begin();
  DataVector<const Trk::TrackStateOnSurface>::const_iterator it_end=trackStates->end();
  if (!(*it)) {
    //    ATH_MSG_WARNING("TrackStateOnSurface == Null");
    return index_freq;
  }

  unsigned int pixMaxHash = m_pixelId->wafer_hash_max();
  unsigned int sctMaxHash = m_sctId->wafer_hash_max();
  
  
  std::vector<unsigned int> ftkTrackIndexList;
  
  
  unsigned int iHit=0;
  for (unsigned int iSurf=0; it!=it_end; it++, iSurf++) {
    //    ATH_MSG_DEBUG(" Surface " << iSurf);
    
    const Trk::TrackStateOnSurface* tsos=(*it);	
    if (tsos == 0) {
      //      ATH_MSG_DEBUG(" No  TrackStateOnSurface ");
      continue;
    }
    if ((*it)->type(Trk::TrackStateOnSurface::Measurement) ){
      //      ATH_MSG_DEBUG(" Measurment Surface");
      const Trk::MeasurementBase *measurement = (*it)->measurementOnTrack();
      if (!measurement) {
	//	ATH_MSG_DEBUG(" No MeasurmentOnTrack");
	continue;
      }
      
      const Trk::RIO_OnTrack* hit = dynamic_cast <const Trk::RIO_OnTrack*>(measurement);
      if (!hit) {
	//	ATH_MSG_DEBUG(" No Hit");
	continue;
      }
      //      ATH_MSG_DEBUG(" got hit - getting ID");
      const Identifier & hitId = hit->identify();
      
      const IdentifierHash hash = hit->idDE();
      
      
      //      ATH_MSG_DEBUG(" getting  locX locY");
      const float locX = (float)measurement->localParameters()[Trk::locX];
      const float locY = (float)measurement->localParameters()[Trk::locY];


      
      if (m_id_helper->is_pixel(hitId)) {
	const std::pair<double, double>  locXlocY(locX,locY);
	offlinetrackPixLocxLocy.insert({hash,locXlocY});
	
	/* 
	   bool isBarrelPixel = m_pixelId->is_barrel(hitId);
	   unsigned int layerPixel = m_pixelId->layer_disk(hitId);
	   if ( isBarrelPixel) {
	  ATH_MSG_DEBUG(iHit <<": Pixel barrel layer " << layerPixel << " locX " << locX << " locY " << locY);
	} else {
	  ATH_MSG_DEBUG(iHit <<": Pixel endcap layer " << layerPixel << " locX " << locX << " locY " << locY);
	}
	*/

	if (hash < pixMaxHash) {
	  if (pixList[hash]) {
	    //	    ATH_MSG_VERBOSE(" Pixel module " << hash << " is on the following FTK tracks:");
	    
	    for (auto& ipixtr:*pixList[hash]) {
	      //	      ATH_MSG_VERBOSE(ipixtr);
	      ftkTrackIndexList.push_back(ipixtr);
	    }
	  }
	}
	
      } else {

	  
	offlinetrackSctLocx.insert({hash,locX});

	
	/*
	  bool isBarrelSCT = m_sctId->is_barrel(hitId);
	  unsigned int layerSCT = m_sctId->layer_disk(hitId);
	  if ( isBarrelSCT) {
	  ATH_MSG_DEBUG(iHit <<": SCT barrel layer " << layerSCT << " locX " << locX);
	} else {
	  ATH_MSG_DEBUG(iHit <<": SCT endcap layer " << layerSCT << " locX " << locX);
	}
	*/
	if (hash < sctMaxHash) {
	  if (sctList[hash]) {
	    //	    ATH_MSG_VERBOSE(" SCT module " << hash << " is on the following FTK tracks:");
	    
	    for (auto& iscttr:*sctList[hash]) {
	      //	      ATH_MSG_VERBOSE(iscttr);
	      ftkTrackIndexList.push_back(iscttr);
	    }
	  }
	}
	
      }
      iHit++;
      
    } else {
      //      ATH_MSG_DEBUG(" Not a Measurment Surface");
    }
  }
  
  
  
  /*
    ATH_MSG_VERBOSE(" Offline Track is matched to the following FTK tracks");

    for ( auto& i:ftkTrackIndexList) {
      ATH_MSG_VERBOSE(i);
    }
  */

  if (ftkTrackIndexList.size()==0) return index_freq;

  std::vector<unsigned int> ftktrack;
  std::vector<unsigned int> freq;

  for ( auto& i:ftkTrackIndexList) {
    bool found=false;
    if (ftktrack.size()>0){
      auto pTrack = std::find(ftktrack.begin(), ftktrack.end(), i);
      if ( pTrack!= ftktrack.end() ) {
	unsigned int pos = std::distance(ftktrack.begin(), pTrack);
	freq[pos]++;
	found = true;
	//	ATH_MSG_VERBOSE(" found at pos " << pos);
      }
    }
    if (!found) {
      //      ATH_MSG_VERBOSE(" not found, adding at pos " << ftktrack.size()+1);
      ftktrack.push_back(i);
      freq.push_back(1);
    }
  }
  auto pMax = std::max_element(freq.begin(), freq.end());

  if (pMax == freq.end()) return index_freq;

  unsigned int nmatch = std::count(freq.begin(), freq.end(), *pMax);
  
  if (nmatch ==1 ) {
    uniqueMatch=true;
  } else {
    ATH_MSG_DEBUG(" found " << nmatch << " matching FTK tracks" );
  }


  
  unsigned int n = *pMax;

  if (n>m_minMatches) h_FTK_nTrackMatch->Fill((double)nmatch);  

  unsigned int index = ftktrack[std::distance(freq.begin(), pMax)];
  
  return std::pair<unsigned int, unsigned int> (index,n);
  
}

void FTK_RDO_MonitorAlgo::compareTracks(const FTK_RawTrack* ftkTrack, 
					std::map<unsigned int,std::pair<double,double> >& offlinetrackPixLocxLocy,
					std::map<unsigned int,double>& offlinetrackSctLocx
					) {

  //  ATH_MSG_DEBUG("compareTracks");

  
  unsigned int iPlane = 0;
  
  for( unsigned int i = 0; i < (ftkTrack)->getPixelClusters().size(); ++i,iPlane++){
    
	
    if ((ftkTrack)->isMissingPixelLayer(i))  {
      //      ATH_MSG_VERBOSE(" FTK track Missing Pixel Layer " <<  i);
      continue;
    }
    /* ATH_MSG_VERBOSE(" FTK_RawPixelCluster " << i <<
		    " Row Coord= " << (ftkTrack)->getPixelClusters()[i].getRowCoord() <<
		    " Row Width= " << (ftkTrack)->getPixelClusters()[i].getRowWidth() <<
		    " Col Coord= " << (ftkTrack)->getPixelClusters()[i].getColCoord() <<
		    " Col Width= " << (ftkTrack)->getPixelClusters()[i].getColWidth() <<
		    " Hash ID= 0x" << std::hex << (ftkTrack)->getPixelClusters()[i].getModuleID() << std::dec <<
		    " Barcode= "   << (ftkTrack)->getPixelClusters()[i].getBarcode() );*/
    
    h_ftk_pix_phiClusWidth->Fill((double)ftkTrack->getPixelClusters()[i].getRowWidth());
    h_ftk_pix_etaClusWidth->Fill((double)ftkTrack->getPixelClusters()[i].getColWidth());

  
    const IdentifierHash hash = (ftkTrack)->getPixelClusters()[i].getModuleID();
    Identifier wafer_id = m_pixelId->wafer_id(hash); // Need to set up this tool
    unsigned int layer = m_pixelId->layer_disk(wafer_id);
    bool isBarrel = (m_pixelId->barrel_ec(wafer_id)==0);

    const float rawLocalPhiCoord = ( m_reverseIBLlocx && isBarrel && layer==0)? 
      (2680. -  (ftkTrack)->getPixelClusters()[i].getRowCoord()) : (ftkTrack)->getPixelClusters()[i].getRowCoord();
    
    const float rawLocalEtaCoord = (ftkTrack)->getPixelClusters()[i].getColCoord();
    


    

    std::string layername;
    if (isBarrel){
      if (layer==0){
	layername = "IBL ";
      } else {
	layername="PIXB";
      }
    } else {
      layername="PIXE";
    }


    const std::pair<double,double> ftkLocxLocy =(this)->getPixLocXlocY(hash,rawLocalPhiCoord, rawLocalEtaCoord);
    auto p = offlinetrackPixLocxLocy.find(hash);
    if (p!=offlinetrackPixLocxLocy.end()) {
      std::pair<double, double> offlineLocxLoxy= p->second;
      ATH_MSG_DEBUG(layername << layer<< " Raw locX: " << rawLocalPhiCoord << " locY: " << rawLocalEtaCoord <<  " locX*6.25e-3 " << rawLocalPhiCoord*6.25e-3 << "locY*25.0e-3: " << rawLocalEtaCoord*25.0e-3); 
      ATH_MSG_DEBUG(layername << layer<< " locX: offline " << offlineLocxLoxy.first << " FTK " << ftkLocxLocy.first << " off-FTK " <<  offlineLocxLoxy.first - ftkLocxLocy.first);
      ATH_MSG_DEBUG(layername << layer<< " locY: offline " << offlineLocxLoxy.second << " FTK " << ftkLocxLocy.second << " off-FTK " <<  offlineLocxLoxy.second - ftkLocxLocy.second);
      if (isBarrel) {
	if (layer==0) {
	  h_IBL_dlocX_fullRange->Fill(offlineLocxLoxy.first - ftkLocxLocy.first);
	  h_IBL_dlocY_fullRange->Fill(offlineLocxLoxy.second - ftkLocxLocy.second);
	}
	h_pixb_dlocX[layer]->Fill(offlineLocxLoxy.first - ftkLocxLocy.first);
	h_pixb_dlocY[layer]->Fill(offlineLocxLoxy.second - ftkLocxLocy.second);
      } else {
	h_pixe_dlocX[layer]->Fill(offlineLocxLoxy.first - ftkLocxLocy.first);
	h_pixe_dlocY[layer]->Fill(offlineLocxLoxy.second - ftkLocxLocy.second);
      }

    } else {
      ATH_MSG_DEBUG("didn't find pixel hash " << hash << " on matched offline track");
    }
  
  }
  for( unsigned int isct = 0; isct < (ftkTrack)->getSCTClusters().size(); ++isct){
    
    if ((ftkTrack)->isMissingSCTLayer(isct)) {
      //ATH_MSG_VERBOSE(" Missing SCT Layer" << isct);
      continue;
    }
    
    /* ATH_MSG_VERBOSE(" FTK_RawSCT_Cluster " << isct <<
       " Hit Coord= " << (ftkTrack)->getSCTClusters()[isct].getHitCoord() <<
       " Hit Width= " << (ftkTrack)->getSCTClusters()[isct].getHitWidth() <<
       " Module ID= " << (ftkTrack)->getSCTClusters()[isct].getModuleID() <<
       " Barcode= "   << (ftkTrack)->getSCTClusters()[isct].getBarcode() ); */
    



    float rawStripCoord = (ftkTrack)->getSCTClusters()[isct].getHitCoord();
    float stripCoord = (ftkTrack)->getSCTClusters()[isct].getHitCoord()/2.; // rawStribCoord is in units of half a strip
    
    int strip = (int) stripCoord;
    IdentifierHash scthash = (ftkTrack)->getSCTClusters()[isct].getModuleID();
    const int clusterWidth=(ftkTrack)->getSCTClusters()[isct].getHitWidth();

    Identifier sctwafer_id = m_sctId->wafer_id(scthash); 
    
    unsigned int layer = m_sctId->layer_disk(sctwafer_id);
    bool isBarrel = (m_sctId->barrel_ec(sctwafer_id)==0);
    unsigned int side = m_sctId->side(sctwafer_id);

    std::string layername;
    if (isBarrel){
      layername="SCTB";
    } else {
      layername="SCTE";
    }
    
    
    double ftkLocX  = (this)->getSctLocX(scthash,rawStripCoord,clusterWidth);
    
    auto p = offlinetrackSctLocx.find(scthash);
    if (p!=offlinetrackSctLocx.end()) {
      double offlineLocX = p->second;
      ATH_MSG_DEBUG(layername << layer<< ":"<<side<<" Raw locX: " << rawStripCoord << " locX/2: " << strip << " clusterWidth " << clusterWidth);
      ATH_MSG_DEBUG(layername << layer<< ":"<<side<< " locX: offline " << offlineLocX << " FTK " << ftkLocX << " off-FTK " <<  offlineLocX  - ftkLocX);
      
      h_ftk_sct_clusWidth->Fill((double) clusterWidth);
      if (isBarrel && side==0 && clusterWidth>0 && clusterWidth < 5) {
	h_ftk_sctb_dlocX[0]->Fill(offlineLocX  - ftkLocX);
	h_ftk_sctb_dlocX[clusterWidth]->Fill(offlineLocX  - ftkLocX);
      }
      if (isBarrel) {
	if (side==0) {
 	  h_sctb_0_dlocX[layer]->Fill(offlineLocX  - ftkLocX);
	} else {
 	  h_sctb_1_dlocX[layer]->Fill(offlineLocX  - ftkLocX);
	}
      } else {
	unsigned int ring = m_sctId->eta_module(sctwafer_id);

	if (side==0) {
 	  h_scte_0_dlocX[ring]->Fill(offlineLocX  - ftkLocX);
	} else {
 	  h_scte_1_dlocX[ring]->Fill(offlineLocX  - ftkLocX);
	}
      }

    } else {
            ATH_MSG_DEBUG("did not find SCT hash " << scthash << " on matched offline track");
    }
    
  }
   
} 



const std::pair<double,double> FTK_RDO_MonitorAlgo::getPixLocXlocY(const IdentifierHash hash, const float rawLocalPhiCoord, const float rawLocalEtaCoord) {
	
  
  const InDetDD::SiDetectorElement* pixelDetectorElement = m_pixelManager->getDetectorElement(hash);
  const InDetDD::PixelModuleDesign* design = dynamic_cast<const InDetDD::PixelModuleDesign*>(&pixelDetectorElement->design());
  
  const InDetDD::SiCellId cornerCell(0, 0);
  const InDetDD::SiLocalPosition localPositionOfCornerCell = design->localPositionOfCell(cornerCell);
  const double phi0 = localPositionOfCornerCell.xPhi();
  const double eta0 = localPositionOfCornerCell.xEta();
  
  //  ATH_MSG_VERBOSE( " local position of pixel at (0,0) is "<<  phi0 << ",  " << eta0);
  

  // zero is center of the row coordinates, so to find the cell we can use it, units of 6.25 microns
  // zero is edge of the column coordinates, so to find the cell we add 0.5, units of 25 microns
  double phiPos = rawLocalPhiCoord * 6.25e-3 + phi0; // rawLocalPhiCoord=0 is the centre of the zeroth pixel
  double etaPos = rawLocalEtaCoord * 25.0e-3 + eta0 - 0.3; // rawLocalEtaCoord=0 is the edge (-0.3mm) of the zeroth pixel.
  
  
  //ATH_MSG_VERBOSE( "Cluster position phiPos, etaPos "<<  phiPos << ", " << etaPos);
  
  if ( etaPos >  design->length()/2.) {
    ATH_MSG_DEBUG( "Illegal position: Setting Cluster eta position to module length/2.");
    etaPos = design->length()/2.;
  } else if (  etaPos <  -design->length()/2. ) {
    etaPos = -design->length()/2.;
    ATH_MSG_DEBUG( "llegal position: Setting Cluster eta position to -(module length/2.)");
  }
  if ( phiPos >  design->width()/2.) {
    phiPos = design->width()/2.;
    ATH_MSG_DEBUG( "llegal position: Setting Cluster phi position to module width/2.");
  } else if (  phiPos <  -design->width()/2. ) {
    phiPos = -design->width()/2.;
    ATH_MSG_DEBUG( "llegal position: Setting Cluster phi position to -(module width/2.)");
  }
  
  return std::pair<double,double>(phiPos,etaPos);
}
  

double FTK_RDO_MonitorAlgo::getSctLocX(const IdentifierHash hash, const float rawstripCoord, const int clusterWidth) {


  float stripCoord = rawstripCoord/2.; // rawStribCoord is in units of half a strip

  const InDetDD::SiDetectorElement* pDE = m_SCT_Manager->getDetectorElement(hash);
	
  const InDetDD::SCT_ModuleSideDesign* design;

  Identifier wafer_id = m_sctId->wafer_id(hash);

  if (m_sctId->is_barrel(wafer_id)){
    design = (static_cast<const InDetDD::SCT_BarrelModuleSideDesign*>(&pDE->design()));
  } else{
    design = (static_cast<const InDetDD::SCT_ForwardModuleSideDesign*>(&pDE->design()));
  }
	
  
  int firstStrip = (int)(stripCoord-0.5*clusterWidth);
  int lastStrip  = (int)(stripCoord+0.5*clusterWidth);
  
  if (firstStrip < 0) {
    firstStrip = 0;
    //    ATH_MSG_VERBOSE( " firstStrip was " << firstStrip << " setting to 0");
  }
  if (lastStrip >= design->cells()) {
    lastStrip = design->cells()-1;
    //    ATH_MSG_VERBOSE( " lastStrip was " << lastStrip << " setting to " << design->cells()-1);
  }
  
  const InDetDD::SiCellId cell1(firstStrip - 1);
  const InDetDD::SiCellId cell2(lastStrip + 1);
  
  const InDetDD::SiLocalPosition firstStripPos(pDE->rawLocalPositionOfCell(cell1));
  const InDetDD::SiLocalPosition lastStripPos(pDE->rawLocalPositionOfCell(cell2));
  
  const InDetDD::SiLocalPosition centre((firstStripPos+lastStripPos)/2.0);
  
  return centre.xPhi();
}
  

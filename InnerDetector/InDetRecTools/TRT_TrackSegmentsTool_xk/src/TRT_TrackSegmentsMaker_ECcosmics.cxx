/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class TRT_TrackSegmentsMaker_ECcosmics
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////


#include <iostream>
#include <iomanip>
#include <set>

#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "InDetReadoutGeometry/TRT_DetectorManager.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TVirtualFitter.h"
#include "TRT_TrackSegmentsTool_xk/TRT_TrackSegmentsMaker_ECcosmics.h"

#include "StoreGate/ReadHandle.h"



///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::TRT_TrackSegmentsMaker_ECcosmics::TRT_TrackSegmentsMaker_ECcosmics
(const std::string& t,const std::string& n,const IInterface* p)
  : AthAlgTool(t,n,p),
    m_multiTruthCollectionTRTName("PRD_MultiTruthTRT"),
    m_phaseMode(false),
    m_riomakerD ("InDet::TRT_DriftCircleOnTrackTool/TRT_DriftCircleOnTrackTool"                      ),
    m_riomakerN ("InDet::TRT_DriftCircleOnTrackNoDriftTimeTool/TRT_DriftCircleOnTrackNoDriftTimeTool"),
    m_useDriftTime(false),
    m_removeSuspicious(false),
    m_scaleTube(2.0),
    m_scaleFactorDrift(1.0),
    m_scaleTubeNoise(1.0),
    m_cutToTLoose(7.),
    m_cutToTTight(18.),
    m_cutToTUpper(32.),
    m_minDCSeed(7)
{
  declareInterface<ITRT_TrackSegmentsMaker>(this);

  declareProperty("TrtManagerLocation"   ,m_ntrtmanager);
  
  
  declareProperty("RIOonTrackToolYesDr"  ,m_riomakerD  );
  declareProperty("RIOonTrackToolNoDr"   ,m_riomakerN  );

  declareProperty("TruthName",m_multiTruthCollectionTRTName);
  declareProperty("Phase",m_phaseMode);

  declareProperty("UseDriftTime", m_useDriftTime);
  declareProperty("RemoveSuspicious",m_removeSuspicious);
  declareProperty("ScaleFactorTube", m_scaleTube);
  declareProperty("ScaleFactorDrift", m_scaleFactorDrift);
  declareProperty("ScaleFactorTubeNoise", m_scaleTubeNoise);
  declareProperty("ToTCutLoose",m_cutToTLoose);
  declareProperty("ToTCutTight",m_cutToTTight);
  declareProperty("ToTCutUpper",m_cutToTUpper);
  declareProperty("MinDCperSeed",m_minDCSeed);

  declareProperty("HitLimit",m_hitLimit=2000);
  
  m_truthCollectionTRT = false;
  //  m_real_counter=0;
  //  m_seg_counter=0;

  // for(int i=0;i<3;i++){
  //   m_classification[i]=0;
  //   for(int j=0;j<200;j++){
  //     m_nHits[i][j]=0;
  //   }
  // }

  //  m_classification[0]=m_classification[1]=m_classification[2]=0;
}

///////////////////////////////////////////////////////////////////
// Destructor  
///////////////////////////////////////////////////////////////////

InDet::TRT_TrackSegmentsMaker_ECcosmics::~TRT_TrackSegmentsMaker_ECcosmics()
{
}


///////////////////////////////////////////////////////////////////
// Fitting functions
///////////////////////////////////////////////////////////////////

Double_t fitf_ztanphi(Double_t *x, Double_t *par)
{
  Double_t f = par[0]*(x[0]+par[1])/(x[0]+par[2]);
  return f;
}

Double_t fitf_zphi(Double_t *x, Double_t *par)
{
  Double_t f = TMath::ATan(par[0]*(x[0]+par[1])/(x[0]+par[2]));
  return f;
}

Double_t fitf_zphi_approx(Double_t *x, Double_t *par)
{
  Double_t f = par[0]+par[1]*x[0]+par[2]*x[0]*x[0];
  return f;
}

///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode InDet::TRT_TrackSegmentsMaker_ECcosmics::initialize()
{
  StatusCode sc = AthAlgTool::initialize(); 

  // Get  TRT Detector Manager
  //
  m_trtmanager = 0;
  sc = detStore()->retrieve(m_trtmanager,m_ntrtmanager);
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Could not get TRT_DetectorManager"); return sc;
  }
  
  // Initialize ReadHandle
  ATH_CHECK(m_trtname.initialize());

  //m_trtid = m_trtmanager->getIdHelper();
  // TRT
  if (detStore()->retrieve(m_trtid, "TRT_ID").isFailure()) {
    ATH_MSG_FATAL("Could not get TRT ID helper");
    return StatusCode::FAILURE;
  }

  // Get RIO_OnTrack creator with drift time information
  //
  if( m_riomakerD.retrieve().isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve tool "<< m_riomakerD);
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO("Retrieved tool " << m_riomakerD);
  }

  // Get RIO_OnTrack creator without drift time information
  //

  if( m_riomakerN.retrieve().isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve tool "<< m_riomakerN);
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO("Retrieved tool " << m_riomakerN);
  }

  // optional PRD to track association map
  //
  ATH_CHECK( m_prdToTrackMap.initialize( !m_prdToTrackMap.key().empty() ) );


  return sc;
}


void InDet::TRT_TrackSegmentsMaker_ECcosmics::setFitFunctions(TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const {
  std::string fit_name1="ztanphi";
  std::string fit_name2="zphi";
  std::string fit_name3="zphi_ap";

  if(m_phaseMode){
    fit_name1+="_phase";
    fit_name2+="_phase";
    fit_name3+="_phase";
  }

  if(!m_prdToTrackMap.key().empty()){
    fit_name1+="_asso";
    fit_name2+="_asso";
    fit_name3+="_asso";
  }

  fit_name1+=name();
  fit_name2+=name();
  fit_name3+=name();

  event_data.m_fitf_ztanphi = new TF1(fit_name1.c_str(),fitf_ztanphi,-3000,3000,3);
  event_data.m_fitf_zphi = new TF1(fit_name2.c_str(),fitf_zphi,-3000,3000,3);
  event_data.m_fitf_zphi_approx = new TF1(fit_name3.c_str(),fitf_zphi_approx,-3000,3000,3);
}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////

StatusCode InDet::TRT_TrackSegmentsMaker_ECcosmics::finalize()
{
  /* 
   if(m_truthCollectionTRT)
     ATH_MSG_INFO("NUmber of events with real tracks through the endcaps: "<<m_real_counter);
   ATH_MSG_INFO("Number of events with segments: "<<m_seg_counter);

   if(m_truthCollectionTRT)   
     ATH_MSG_INFO("Classification:\n\t Real : "<<m_classification[0]<<"\n\t Fake : "<<m_classification[1]<<"\n\t Rest : "<<m_classification[2]);

   if(m_truthCollectionTRT){
     for(int i=0;i<200;i++){
       if(m_nHits[0][i]>0)
	 ATH_MSG_INFO("Real segments with "<<i<<" Hits : "<<m_nHits[0][i]);
     }
   }

   for(int i=0;i<200;i++){
     if(m_nHits[1][i]>0){
       if(m_truthCollectionTRT)
	 ATH_MSG_INFO("Fake segments with "<<i<<" Hits : "<<m_nHits[1][i]);
       else
	 ATH_MSG_INFO("Segments with "<<i<<" Hits : "<<m_nHits[1][i]);
     }
   }

   if(m_truthCollectionTRT){
     for(int i=0;i<200;i++){
       if(m_nHits[2][i]>0)
	 ATH_MSG_INFO("Rest segments with "<<i<<" Hits : "<<m_nHits[2][i]);
     }
   }
  */

   StatusCode sc = AthAlgTool::finalize();
   return sc;
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new event 
///////////////////////////////////////////////////////////////////

std::unique_ptr<InDet::ITRT_TrackSegmentsMaker::IEventData>
InDet::TRT_TrackSegmentsMaker_ECcosmics::newEvent () const
{

  std::unique_ptr<TRT_TrackSegmentsMaker_ECcosmics::EventData>
     event_data = std::make_unique<TRT_TrackSegmentsMaker_ECcosmics::EventData>();

  setFitFunctions(*event_data);
  //sort into good/noise hits
  retrieveHits(*event_data);

  std::list<const InDet::TRT_DriftCircle*>::iterator it,itE;
  it=event_data->m_goodHits.begin();
  itE=event_data->m_goodHits.end();

  //loop over good hits and sort them into sectors

  for(;it!=itE;++it){
    
    int straw = m_trtid->straw((*it)->identify());
    const Amg::Vector3D& sc = (*it)->detectorElement()->strawCenter(straw);
    
    int phi = int((atan2(sc.y(),sc.x())+M_PI)*8./M_PI);

    if     ( phi<0  ) phi = 15;
    else if( phi>15 ) phi = 0 ;

    int z=0;
    if(sc.z()<0) z=1;
    
    int zslice=int((fabs(sc.z())-800.)/100.);

    event_data->m_sectors[z][zslice][phi].push_back(*it);

  }

  ATH_MSG_DEBUG("Number of good  driftcircles: "<<event_data->m_goodHits.size());
  ATH_MSG_DEBUG("Number of noise driftcircles: "<<event_data->m_noiseHits.size());


  //retrieve TruthInfo if presen
  /*
  if(evtStore()->contains<PRD_MultiTruthCollection>(m_multiTruthCollectionTRTName)){
    
    StatusCode sc = evtStore()->retrieve(m_truthCollectionTRT, m_multiTruthCollectionTRTName);
    if(sc.isFailure()){
      ATH_MSG_INFO(" could not open PRD_MultiTruthCollection : " <<  m_multiTruthCollectionTRTName);
      m_truthCollectionTRT=0;
    }
  }
  */

  return std::unique_ptr<InDet::ITRT_TrackSegmentsMaker::IEventData>(event_data.release());
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new region
///////////////////////////////////////////////////////////////////

std::unique_ptr<InDet::ITRT_TrackSegmentsMaker::IEventData>
InDet::TRT_TrackSegmentsMaker_ECcosmics::newRegion
(const std::vector<IdentifierHash>& vTRT) const
{
  (void) vTRT;
  std::unique_ptr<TRT_TrackSegmentsMaker_ECcosmics::EventData>
     event_data = std::make_unique<TRT_TrackSegmentsMaker_ECcosmics::EventData>();

  setFitFunctions(*event_data);
  //  std::vector<std::vector<const InDet::TRT_DriftCircle*> *>::iterator sit,sitE;

  // @TODO here the retrieve_hits method should be called with the list of the idhash

  // std::vector<IdentifierHash>::const_iterator d=vTRT.begin(),de=vTRT.end();
  // for(; d!=de; ++d) {

  // }

  //


  std::list<const InDet::TRT_DriftCircle*>::iterator it,itE;
  it=event_data->m_goodHits.begin();
  itE=event_data->m_goodHits.end();

  //loop over good hits and sort them into sectors

  for(;it!=itE;++it){
    
    int straw = m_trtid->straw((*it)->identify());
    const Amg::Vector3D& sc = (*it)->detectorElement()->strawCenter(straw);
    
    int phi = int((atan2(sc.y(),sc.x())+M_PI)*8./M_PI);

    if     ( phi<0  ) phi = 15;
    else if( phi>15 ) phi = 0 ;

    int z=0;
    if(sc.z()<0) z=1;
    int zslice=int((fabs(sc.z())-800.)/100.);

    event_data->m_sectors[z][zslice][phi].push_back(*it);

  }

  ATH_MSG_DEBUG("Number of good  driftcircles: "<<event_data->m_goodHits.size());
  ATH_MSG_DEBUG("Number of noise driftcircles: "<<event_data->m_noiseHits.size());

  return std::unique_ptr<InDet::ITRT_TrackSegmentsMaker::IEventData>(event_data.release());
}

///////////////////////////////////////////////////////////////////
// Inform tool about end of event or region investigation
///////////////////////////////////////////////////////////////////

void InDet::TRT_TrackSegmentsMaker_ECcosmics::endEvent (InDet::ITRT_TrackSegmentsMaker::IEventData &) const
{
}



///////////////////////////////////////////////////////////////////
// Helper function to sort the DC along z
///////////////////////////////////////////////////////////////////

namespace InDet{
  bool CompareDCz(const InDet::TRT_DriftCircle* x,const InDet::TRT_DriftCircle* y)
  {
    const Amg::Vector3D& sc = x->detectorElement()->center();
    double z1=sc.z();


    const Amg::Vector3D& sc2 = y->detectorElement()->center();
    double z2=sc2.z();
    
    return z1<z2;
  }

  bool CompareDCy(const InDet::TRT_DriftCircle* x,const InDet::TRT_DriftCircle* y)
  {
    const Amg::Vector3D& sc = x->detectorElement()->center();
    double y1=sc.y();
    
    
    const Amg::Vector3D& sc2 = y->detectorElement()->center();
    double y2=sc2.y();
    
    return y1>y2;
  }

  bool CompareDCzreverse(const InDet::TRT_DriftCircle* x,const InDet::TRT_DriftCircle* y)
  {
    const Amg::Vector3D& sc = x->detectorElement()->center();
    double z1=sc.z();
    
    
    const Amg::Vector3D& sc2 = y->detectorElement()->center();
    double z2=sc2.z();
    
    return z1>z2;
  }
}



///////////////////////////////////////////////////////////////////
// Methods of TRT segment reconstruction in one event
///////////////////////////////////////////////////////////////////

void InDet::TRT_TrackSegmentsMaker_ECcosmics::find(InDet::ITRT_TrackSegmentsMaker::IEventData &virt_event_data) const
{
  TRT_TrackSegmentsMaker_ECcosmics::EventData &
     event_data  = TRT_TrackSegmentsMaker_ECcosmics::EventData::getPrivateEventData(virt_event_data);

  //MsgStream log(msgSvc(), name());


  //loop over both endcaps
  for(int endcap=0;endcap<2;endcap++){

    // 1st step: find sector with the maximal number of hits until no one is left 
    //           with at least 5 hits

    int max=m_minDCSeed;

    while(max>=m_minDCSeed){
      max=-1;
      int sector=-1;
      int zslice=-1;

      for(int j=0;j<20;j++){
	for(int i=0;i<16;i++){
	  int count=event_data.m_sectors[endcap][j][i].size();

	  if(count>0)
	    ATH_MSG_VERBOSE("Endcap "<<endcap<<" zslice "<<j<<" sector "<<i<<" : "<<count);

	  if(count>max){
	    max=count;
	    sector=i;
	    zslice=j;
	  }
	}
      }

      if(sector<0) continue; // no hits in this endcap ...

      max=event_data.m_sectors[endcap][zslice][sector].size();

      //check if enough driftcircles present
      if(max>=3){
        ATH_MSG_VERBOSE("Number of DCs in seed sector: "<<max);

        //2nd step: find seed among those driftcircles

        bool found=find_seed(endcap,zslice,sector,event_data);

        //if no seed is found we clear the sector
        if(!found){
          event_data.m_sectors[endcap][zslice][sector].clear();
        }
      }

    }
  }

  ATH_MSG_DEBUG("Found "<<event_data.m_seeds.size()<<" initial seeds !");
  

  //now loop over all seeds and try to create segments out of them

  std::vector<std::vector<const InDet::TRT_DriftCircle*> *>::iterator sit,sitE;

  sit=event_data.m_seeds.begin();
  sitE=event_data.m_seeds.end();

  for(;sit!=sitE;++sit){
    create_segment(*sit, event_data);
  }


  ATH_MSG_DEBUG("Found "<<event_data.m_segments.size()<<" TRT Segments");

  //  if(event_data.m_segments.size()>0) m_seg_counter++;

  //loop over all segments and clasify them
  //  std::list<Trk::TrackSegment*>::iterator seg=event_data.m_segments.begin();
  //  std::list<Trk::TrackSegment*>::iterator segE=event_data.m_segments.end();
  
  // {
  // std::lock_guard<std::mutex> lock(m_counterMutex);
  // for(;seg!=segE;++seg){
  //   int num=0;
  //   double p=classify_segment(*seg,num);
  //   if(p>0.67){
  //     m_nHits[0][num]++;
  //     m_classification[0]++;
  //   }else if(p<0.33){
  //     m_nHits[1][num]++;
  //     m_classification[1]++;
  //   }else{
  //     m_nHits[2][num]++;
  //     m_classification[2]++;
  //   }
  // }
  // }

  event_data.m_segiterator   = event_data.m_segments.begin();
}


///////////////////////////////////////////////////////////////////
// Find seeds
///////////////////////////////////////////////////////////////////


bool InDet::TRT_TrackSegmentsMaker_ECcosmics::find_seed(int endcap,
                                                        int zslice,
                                                        int sector,
                                                        TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const
{
  double shift=0.;
  double phimin=10.;
  double phimax=-10.;

  std::vector<const InDet::TRT_DriftCircle*> dc;
  std::list<const InDet::TRT_DriftCircle*>::iterator it,itE;

  dc.reserve(event_data.m_sectors[endcap][zslice][sector].size());
  itE=event_data.m_sectors[endcap][zslice][sector].end();
  for(it=event_data.m_sectors[endcap][zslice][sector].begin();it!=itE;++it){
    dc.push_back(*it);
  }
  //sort them along z

  std::sort(dc.begin(),dc.end(), CompareDCz);

  std::vector<const InDet::TRT_DriftCircle*>::iterator vit,vitE;
  vit=dc.begin();
  vitE=dc.end();

  ATH_MSG_VERBOSE("Sorted driftcircles: "<<dc.size());

  for(;vit!=vitE;++vit){
    int straw = m_trtid->straw((*vit)->identify());
    const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);

    ATH_MSG_VERBOSE("z="<<sc.z()<<" phi="<<sc.phi());

  }


  //sanity check: if more than 40 good hits per seed -> not a pure cosmic event!

  if(dc.size()>=40) return false;

  //fill arrays for the fit
  
  vit=dc.begin();
  vitE=dc.end();


  for(;vit!=vitE;++vit){
    int straw = m_trtid->straw((*vit)->identify());
    const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw); 

    if(sc.phi()<phimin)
      phimin=sc.phi();

    if(sc.phi()>phimax)
      phimax=sc.phi();
  }

  shift=(phimax+phimin)/2.-M_PI/4.;
  ATH_MSG_VERBOSE("Phimin="<<phimin<<" Phimax="<<phimax<<" -> shift = "<<shift);

  //protection for boundary at pi,-pi
  if(phimin<-M_PI/2. && phimax>M_PI/2.){
    shift=3./4.*M_PI;
    ATH_MSG_VERBOSE("Boundary between pi and -pi!!! use the following shift: "<<shift);
  }


  int count=0;
  for(vit=dc.begin();vit!=vitE;++vit){
    int straw = m_trtid->straw((*vit)->identify());
    const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);
    
    double orig=sc.phi();

    double corr=orig-shift;

    if     (corr > M_PI) corr = fmod(corr+M_PI,2.*M_PI)-M_PI; 
    else if(corr <-M_PI) corr = fmod(corr-M_PI,2.*M_PI)+M_PI;

    if(count>500){
      return false; // a seed cannot contain that many hits
    }

    event_data.m_a_z[count]=sc.z();
    event_data.m_a_phi[count]=corr;
    count++;
  }

  int maxdc=0;
  double p_best[3]={0,0,0};
  p_best[2]=shift;

  for(int i=0;i<count;i++){
    for(int j=i+1;j<count;j++){

      double p[3];

      p[2]=shift;

      if(event_data.m_a_z[i]==event_data.m_a_z[j]){
	p[0]=1e10;
	p[1]=event_data.m_a_z[i];
      }else{
	p[0]=(event_data.m_a_phi[i]-event_data.m_a_phi[j])/(event_data.m_a_z[i]-event_data.m_a_z[j]);
	p[1]=event_data.m_a_phi[j]-p[0]*event_data.m_a_z[j];
      }


      //log<<MSG::VERBOSE<<"Investigating seed "<<p[0]<<" "<<p[1]<<endmsg;

      int match=evaluate_seed(endcap,zslice,sector,p,event_data);

      if(match>maxdc){
	maxdc=match;
	p_best[0]=p[0];
	p_best[1]=p[1];
      }
    }
  }

  ATH_MSG_VERBOSE("Number of matching dc from best seed: "<<maxdc);


  if(maxdc>=4){
    //ok, we've found a seed
    
    double zmin=1e4;
    double zmax=-1e4;
    

    int muster[3][3];
    for(int i=0;i<3;i++)
      for(int j=0;j<3;j++)
	muster[i][j]=0;
    
    std::vector<const InDet::TRT_DriftCircle*> *seed=new std::vector<const InDet::TRT_DriftCircle*>;
    
    for(int zsl=zslice-1;zsl<=zslice+1;zsl++){
      for(int ps=sector-1;ps<=sector+1;ps++){
	
	//make sure that the slices exist ...
	if(zsl<0 || zsl>=20) continue;
	
	int ps2=ps;
	if(ps2<0) ps2+=16;
	else if(ps2>15) ps2-=16;
	
	for(it=event_data.m_sectors[endcap][zsl][ps2].begin();it!=event_data.m_sectors[endcap][zsl][ps2].end();++it){
	  int straw = m_trtid->straw((*it)->identify());
	  const Amg::Vector3D& sc = (*it)->detectorElement()->strawCenter(straw);
	  
	  double pred_phi=sc.z()*p_best[0]+p_best[1];
	  double corr=sc.phi()-p_best[2];
	  
	  if(p_best[0]==1e10){
	    if(sc.z()==p_best[1]) {
	      pred_phi=corr; //z=const ...
	    }else{
	      pred_phi=corr+0.5;
	    }
	  }
	  if     (corr > M_PI) corr = fmod(corr+M_PI,2.*M_PI)-M_PI; 
	  else if(corr <-M_PI) corr = fmod(corr-M_PI,2.*M_PI)+M_PI;
	  
	  //log<<MSG::VERBOSE<<"pred_phi="<<pred_phi<<" orig="<<sc.phi()<<" corr="<<corr<<" phidiff="<<phidiff(pred_phi,corr)<<endmsg;
	  
	  if(fabs(phidiff(pred_phi,corr))<=4./650.){
	    seed->push_back(*it);
	    if(sc.z()<zmin) zmin=sc.z();
	    if(sc.z()>zmax) zmax=sc.z();
	    muster[zsl-zslice+1][ps-sector+1]=1;
	  }
	}
      }
    }

    std::vector<const InDet::TRT_DriftCircle*>::iterator vit,vitE;
    

    //do we need to extend the range ??
    
    ATH_MSG_VERBOSE("Pattern:");
    ATH_MSG_VERBOSE("\t"<<muster[0][0]<<" "<<muster[1][0]<<" "<<muster[2][0]);
    ATH_MSG_VERBOSE("\t"<<muster[0][1]<<" "<<muster[1][1]<<" "<<muster[2][1]);
    ATH_MSG_VERBOSE("\t"<<muster[0][2]<<" "<<muster[1][2]<<" "<<muster[2][2]);

    int zsl=-1;
    if(muster[2][0] || muster[2][1] || muster[2][2]){
      //extend towards higher z-values
      zsl=zslice+2;
    
      for(int ps=sector-2;ps<=sector+2;ps++){
	
	//make sure that the slices exist ...
	if(zsl<0 || zsl>=20) continue;
	
	int ps2=ps;
	if(ps2<0) ps2+=16;
	else if(ps2>15) ps2-=16;
	
	for(it=event_data.m_sectors[endcap][zsl][ps2].begin();it!=event_data.m_sectors[endcap][zsl][ps2].end();++it){
	  int straw = m_trtid->straw((*it)->identify());
	  const Amg::Vector3D& sc = (*it)->detectorElement()->strawCenter(straw);
	  
	  double pred_phi=sc.z()*p_best[0]+p_best[1];
	  double corr=sc.phi()-p_best[2];
	  
	  if(p_best[0]==1e10){
	    if(sc.z()==p_best[1]) {
	      pred_phi=corr; //z=const ...
	    }else{
	      pred_phi=corr+0.5;
	    }
	  }
	  if     (corr > M_PI) corr = fmod(corr+M_PI,2.*M_PI)-M_PI; 
	  else if(corr <-M_PI) corr = fmod(corr-M_PI,2.*M_PI)+M_PI;
	  
	  //log<<MSG::VERBOSE<<"pred_phi="<<pred_phi<<" orig="<<sc.phi()<<" corr="<<corr<<" phidiff="<<phidiff(pred_phi,corr)<<endmsg;
	  
	  if(fabs(phidiff(pred_phi,corr))<=4./650.){
	    if (msgLvl(MSG::VERBOSE)) msg()<<"extended hit at z= "<<sc.z()<<endmsg;
	    if(sc.z()<zmin) zmin=sc.z();
	    if(sc.z()>zmax) zmax=sc.z();
	    seed->push_back(*it);
	  }
	}
      }
    }

    if(muster[0][0] || muster[0][1] || muster[0][2]){
      //extend towards lower z-values
      zsl=zslice-2;
    
      for(int ps=sector-2;ps<=sector+2;ps++){
	
	//make sure that the slices exist ...
	if(zsl<0 || zsl>=20) continue;
	
	int ps2=ps;
	if(ps2<0) ps2+=16;
	else if(ps2>15) ps2-=16;
	
	for(it=event_data.m_sectors[endcap][zsl][ps2].begin();it!=event_data.m_sectors[endcap][zsl][ps2].end();++it){
	  int straw = m_trtid->straw((*it)->identify());
	  const Amg::Vector3D& sc = (*it)->detectorElement()->strawCenter(straw);
	  
	  double pred_phi=sc.z()*p_best[0]+p_best[1];
	  double corr=sc.phi()-p_best[2];
	  
	  if(p_best[0]==1e10){
	    if(sc.z()==p_best[1]) {
	      pred_phi=corr; //z=const ...
	    }else{
	      pred_phi=corr+0.5;
	    }
	  }
	  if     (corr > M_PI) corr = fmod(corr+M_PI,2.*M_PI)-M_PI; 
	  else if(corr <-M_PI) corr = fmod(corr-M_PI,2.*M_PI)+M_PI;
	  
	  //log<<MSG::VERBOSE<<"pred_phi="<<pred_phi<<" orig="<<sc.phi()<<" corr="<<corr<<" phidiff="<<phidiff(pred_phi,corr)<<endmsg;
	  
	  if(fabs(phidiff(pred_phi,corr))<=4./650.){
	    ATH_MSG_VERBOSE("extended hit at z= "<<sc.z());
	    if(sc.z()<zmin) zmin=sc.z();
	    if(sc.z()>zmax) zmax=sc.z();
	    seed->push_back(*it);
	  }
	}
      }
    }


    if(seed->size()<(size_t)m_minDCSeed){
      ATH_MSG_VERBOSE("Seed has too few hits: "<<seed->size());
      seed->clear();
      delete seed;
      return false;
    }


    vit=seed->begin();
    vitE=seed->end();
    
    for(;vit!=vitE;++vit){
      int straw = m_trtid->straw((*vit)->identify());
      const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);

      int phi = int((atan2(sc.y(),sc.x())+M_PI)*8./M_PI);
      
      if     ( phi<0  ) phi = 15;
      else if( phi>15 ) phi = 0 ;
      
      int z=0;
      if(sc.z()<0) z=1;
      int zslic=int((fabs(sc.z())-800.)/100.);
      
      event_data.m_sectors[z][zslic][phi].remove(*vit);

    }

    //just to be sure: check also the noise hits in these 9 slices

    std::list<const InDet::TRT_DriftCircle*>::iterator lit,litE;
    lit=event_data.m_noiseHits.begin();
    litE=event_data.m_noiseHits.end();
    
    for(;lit!=litE;++lit){
      
      //don't add hits that really look like noise
      if((*lit)->timeOverThreshold()<m_cutToTLoose) continue;
      
      int straw = m_trtid->straw((*lit)->identify());
      const Amg::Vector3D& sc = (*lit)->detectorElement()->strawCenter(straw);
      
      int phi = int((atan2(sc.y(),sc.x())+M_PI)*8./M_PI);
      
      if     ( phi<0  ) phi = 15;
      else if( phi>15 ) phi = 0 ;
      
      
      int zsl=int((fabs(sc.z())-800.)/100.);
      
      if(std::abs(zsl-zslice)<2){
	double pred_phi=sc.z()*p_best[0]+p_best[1];
	double corr=sc.phi()-p_best[2];
	
	if(p_best[0]==1e10){
	  if(sc.z()==p_best[1]) {
	    pred_phi=corr; //z=const ...
	  }else{
	    pred_phi=corr+0.5;
	  }
	}
	if     (corr > M_PI) corr = fmod(corr+M_PI,2.*M_PI)-M_PI; 
	else if(corr <-M_PI) corr = fmod(corr-M_PI,2.*M_PI)+M_PI;
	
	//log<<MSG::VERBOSE<<"pred_phi="<<pred_phi<<" orig="<<sc.phi()<<" corr="<<corr<<" phidiff="<<phidiff(pred_phi,corr)<<endmsg;
	
	if(fabs(phidiff(pred_phi,corr))<=4./650.){
	  if(sc.z()>zmin && sc.z()<zmax){
	    seed->push_back(*lit);
	    ATH_MSG_VERBOSE("\t\t noise hit matched seed! z="<< sc.z() <<"MC : "<<isTrueHit(*lit));
	  }
	}
	
      }
      
      
    }




    event_data.m_seeds.push_back(seed);

    return true;
  }


  return false;
  
}

int InDet::TRT_TrackSegmentsMaker_ECcosmics::evaluate_seed(int endcap,
                                                           int zslice,
                                                           int sector,
                                                           double *p,
                                                           TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const
{

  std::list<const InDet::TRT_DriftCircle*>::iterator it,itE;

  int count=0;

  //look at all nearest neighbours ...

  for(int zsl=zslice-1;zsl<=zslice+1;zsl++){
    for(int ps=sector-1;ps<=sector+1;ps++){

      //make sure that the slices exist ...
      if(zsl<0 || zsl>=20) continue;

      int ps2=ps;
      if(ps2<0) ps2+=16;
      else if(ps2>15) ps2-=16;

      //log<<MSG::VERBOSE<<"Evaluating zslice "<<zsl<<" sector "<<ps2<<endmsg;

      itE=event_data.m_sectors[endcap][zsl][ps2].end();
      for(it=event_data.m_sectors[endcap][zsl][ps2].begin();it!=itE;++it){
	int straw = m_trtid->straw((*it)->identify());
	const Amg::Vector3D& sc = (*it)->detectorElement()->strawCenter(straw);
	
	double pred_phi=sc.z()*p[0]+p[1];
	double corr=sc.phi()-p[2];
	
	if(p[0]==1e10){
	  if(sc.z()==p[1]) {
	    pred_phi=corr; //z=const ...
	  }else{
	    pred_phi=corr+0.5;
	  }
	}
	if     (corr > M_PI) corr = fmod(corr+M_PI,2.*M_PI)-M_PI; 
	else if(corr <-M_PI) corr = fmod(corr-M_PI,2.*M_PI)+M_PI;
	
	//log<<MSG::VERBOSE<<"pred_phi="<<pred_phi<<" orig="<<sc.phi()<<" corr="<<corr<<" phidiff="<<phidiff(pred_phi,corr)<<endmsg;
	
	if(fabs(phidiff(pred_phi,corr))<=4./650.){
	  count=count+1;	  
	}
      }
    }
  }

  //log<<MSG::VERBOSE<<"Total dc in this seed: "<<count<<endmsg;

  return count;
}


void InDet::TRT_TrackSegmentsMaker_ECcosmics::create_segment(std::vector<const InDet::TRT_DriftCircle*> *seed,
                                                             TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const
{

  ATH_MSG_VERBOSE("Number of hits in initial seed: "<<seed->size());
  
  double shift=0.;
  double phimin=10.;
  double phimax=-10.;

  double zmin=1e5;
  double zmax=-1e5;

  std::vector<const InDet::TRT_DriftCircle*>::iterator vit,vitE;

  vit=seed->begin();
  vitE=seed->end();

  int count=0;
  for(;vit!=vitE;++vit){
    int straw = m_trtid->straw((*vit)->identify());
    const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);
    
    if(sc.z()<zmin) zmin=sc.z();
    if(sc.z()>zmax) zmax=sc.z();

    if(count>500){
      return; // a seed cannot contain that many hits
    }

    event_data.m_a_z[count]=sc.z();
    event_data.m_a_phi[count]=sc.phi();
    event_data.m_a_tan[count]=TMath::Tan(sc.phi());
    event_data.m_a_tan_err[count]=(1.15/700.)/(TMath::Cos(sc.phi())*TMath::Cos(sc.phi()));
    event_data.m_a_z_err[count]=1.15; // 4/sqrt(12)
    event_data.m_a_phi_err[count]=1.15/700.; //take uncertainty at a radius of 700 mm

    if(sc.phi()<phimin)
      phimin=sc.phi();
    
    if(sc.phi()>phimax)
      phimax=sc.phi();
    
    count++;
  }
  
  shift=(phimax+phimin)/2.-M_PI/4.;
  ATH_MSG_VERBOSE("Phimin="<<phimin<<" Phimax="<<phimax<<" shift="<<shift);

  //protection for boundary at pi,-pi
  if(phimin<-M_PI/2. && phimax>M_PI/2.){
    shift=3./4.*M_PI;
    ATH_MSG_VERBOSE("Boundary between pi and -pi!!! use the following shift: "<<shift);
  }


  //correct the phi values
  
  for(int i=0;i<count;i++){
    double orig=event_data.m_a_phi[i];
    
    double corr=orig-shift;
    if     (corr > M_PI) corr = fmod(corr+M_PI,2.*M_PI)-M_PI; 
    else if(corr <-M_PI) corr = fmod(corr-M_PI,2.*M_PI)+M_PI;

    event_data.m_a_phi[i]=corr;
    event_data.m_a_tan[i]=TMath::Tan(corr);
    event_data.m_a_tan_err[i]=(1.15/700.)/(TMath::Cos(corr)*TMath::Cos(corr));
  }

  TF1 *fit=perform_fit(count, event_data);
  if(!fit) return;

  std::list<const InDet::TRT_DriftCircle*>::iterator lit,litE;

  //add the ones from the good list (but skip the ones that we already have)
  lit=event_data.m_goodHits.begin();
  litE=event_data.m_goodHits.end();

  std::vector<const InDet::TRT_DriftCircle*> tobeadded;


  for(;lit!=litE;++lit){
    int straw = m_trtid->straw((*lit)->identify());
    const Amg::Vector3D& sc = (*lit)->detectorElement()->strawCenter(straw);

    double fitted_phi=fit->Eval(sc.z(),0.,0.);
    fitted_phi+=shift;

    if     (fitted_phi > M_PI) fitted_phi = fmod(fitted_phi+M_PI,2.*M_PI)-M_PI; 
    else if(fitted_phi <-M_PI) fitted_phi = fmod(fitted_phi-M_PI,2.*M_PI)+M_PI;

    if(fabs(phidiff(fitted_phi,sc.phi()))<m_scaleTube*4./800.){
      //verify that this hit is not already included

      vit=seed->begin();
      vitE=seed->end();
      bool new_hit=true;
      for(;vit!=vitE && new_hit;++vit){
	if((*vit)==(*lit)){
	  new_hit=false;
	}
      }
      if(new_hit){
	ATH_MSG_VERBOSE("\t\t good hit matched! fitted_phi="<<fitted_phi<<" z=" <<sc.z()<<"MC : "<<isTrueHit(*lit));

	if(sc.z()>zmin-200 && sc.z()<zmax+200){
	  if(sc.z()<zmin) zmin=sc.z();
	  if(sc.z()>zmax) zmax=sc.z();

	  seed->push_back(*lit);
	}else{
	  //not sure if this is really a good hit yet ...
	  tobeadded.push_back(*lit);
	}
      }
    }
  }

  std::sort(tobeadded.begin(),tobeadded.end(), CompareDCz);

  int index1=-1,index2=-1;

  double z1=-1e5;

  count=0;
  ATH_MSG_VERBOSE("zmin="<<zmin<<"   zmax="<<zmax);
  for(vit=tobeadded.begin();vit!=tobeadded.end();vit++){
    int straw = m_trtid->straw((*vit)->identify());
    const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);
    ATH_MSG_VERBOSE("Shall we add hit at z="<<sc.z());

    if(sc.z()<zmin && sc.z()>z1) {
      z1=sc.z();
      index1=count;
    }
    if(sc.z()>zmax && index2==-1) {
      index2=count;
    }
    count++;
  }

  ATH_MSG_VERBOSE("index1="<<index1<<" index2="<<index2);

  if(index2>=0){
    for(int i=index2;i<count;i++){
      int straw = m_trtid->straw(tobeadded[i]->identify());
      const Amg::Vector3D& sc = tobeadded[i]->detectorElement()->strawCenter(straw);
      if(sc.z()<zmax+200){
	zmax=sc.z();
	seed->push_back(tobeadded[i]);
	ATH_MSG_VERBOSE(" added goot hit at z="<<sc.z());
      }else{
	ATH_MSG_VERBOSE(" rejected goot hit at z="<<sc.z());
	i=count;
      }
    }
  }
  if(index1>=0){
    for(int i=index1;i>=0;i--){
      int straw = m_trtid->straw(tobeadded[i]->identify());
      const Amg::Vector3D& sc = tobeadded[i]->detectorElement()->strawCenter(straw);
      if(sc.z()>zmin-200){
	zmin=sc.z();
	seed->push_back(tobeadded[i]);
	ATH_MSG_VERBOSE(" added goot hit at z="<<sc.z());
      }else{
	ATH_MSG_VERBOSE(" rejected goot hit at z="<<sc.z());
	i=-1;
      }
    }
  }
  tobeadded.clear();

  

  //fit again
  std::sort(seed->begin(),seed->end(), CompareDCz);

  vit=seed->begin();
  vitE=seed->end();

  count=0;
  for(;vit!=vitE;++vit){
    int straw = m_trtid->straw((*vit)->identify());
    const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);
    
    if(count>500){
      return; // a seed cannot contain that many hits
    }

    event_data.m_a_z[count]=sc.z();
    event_data.m_a_phi[count]=sc.phi();
    event_data.m_a_tan[count]=TMath::Tan(sc.phi());
    event_data.m_a_tan_err[count]=(1.15/700.)/(TMath::Cos(sc.phi())*TMath::Cos(sc.phi()));
    event_data.m_a_z_err[count]=1.15; // 4/sqrt(12)
    event_data.m_a_phi_err[count]=1.15/700.; //take uncertainty at a radius of 700 mm

    if(sc.phi()<phimin)
      phimin=sc.phi();
    
    if(sc.phi()>phimax)
      phimax=sc.phi();
    
    count++;
  }
  
  shift=(phimax+phimin)/2.-M_PI/4.;
  ATH_MSG_VERBOSE("Phimin="<<phimin<<" Phimax="<<phimax<<" shift="<<shift);

 //protection for boundary at pi,-pi
  if(phimin<-M_PI/2. && phimax>M_PI/2.){
    shift=3./4.*M_PI;
    ATH_MSG_VERBOSE("Boundary between pi and -pi!!! use the following shift: "<<shift);
  }

  //correct the phi values
  
  for(int i=0;i<count;i++){
    double orig=event_data.m_a_phi[i];
    double corr=orig-shift;

    if     (corr > M_PI) corr = fmod(corr+M_PI,2.*M_PI)-M_PI; 
    else if(corr <-M_PI) corr = fmod(corr-M_PI,2.*M_PI)+M_PI;

    event_data.m_a_phi[i]=corr;
    event_data.m_a_tan[i]=TMath::Tan(corr);
    event_data.m_a_tan_err[i]=(1.15/700.)/(TMath::Cos(corr)*TMath::Cos(corr));
  }

  
  fit=perform_fit(count, event_data);
  if(!fit) return;

  //add the ones from the noise list
  lit=event_data.m_noiseHits.begin();
  litE=event_data.m_noiseHits.end();
  
  for(;lit!=litE;++lit){

    //don't add hits that really look like noise
    //if((*lit)->isNoise()) continue;

    int straw = m_trtid->straw((*lit)->identify());
    const Amg::Vector3D& sc = (*lit)->detectorElement()->strawCenter(straw);

    double z=sc.z();
    double phi=sc.phi();
    double fitted_phi=fit->Eval(sc.z(),0.,0.);
    fitted_phi+=shift;

    if     (fitted_phi > M_PI) fitted_phi = fmod(fitted_phi+M_PI,2.*M_PI)-M_PI; 
    else if(fitted_phi <-M_PI) fitted_phi = fmod(fitted_phi-M_PI,2.*M_PI)+M_PI;

    //only look into the same endcap for noise hits
    if(z*event_data.m_a_z[0]<0) continue;

    if(fabs(phidiff(fitted_phi,phi))<m_scaleTubeNoise*4./800.){
      vit=seed->begin();
      vitE=seed->end();
      bool new_hit=true;
      for(;vit!=vitE && new_hit;++vit){
	if((*vit)==(*lit)){
	  new_hit=false;
	}
      }
      if(new_hit){
	if(sc.z()>zmin-50 && sc.z()<zmax+50){
	  ATH_MSG_VERBOSE("\t\t noise hit matched! fitted_phi="<<fitted_phi<<" z=" <<sc.z()<<"MC : "<<isTrueHit(*lit));
	  seed->push_back(*lit);
	}else{
	  ATH_MSG_VERBOSE("\t\t noise hit matched but too far away! fitted_phi="<<fitted_phi<<" z=" <<sc.z()<<"MC : "<<isTrueHit(*lit));
	}
      }
    }
  }

  //first: sort along y
  std::sort(seed->begin(),seed->end(), CompareDCy);

  //check z-coordinate of first and last hit
  Amg::Vector3D firststraw=((*seed)[0]->detectorElement()->strawCenter(  m_trtid->straw((*seed)[0]->identify()) ));
  Amg::Vector3D laststraw=((*seed)[seed->size()-1]->detectorElement()->strawCenter(  m_trtid->straw((*seed)[seed->size()-1]->identify()) ));
  z1=firststraw.z();
  double z2=laststraw.z();
  double yfirst=firststraw.y();
  double ylast=laststraw.y();

  ATH_MSG_VERBOSE("zfirst="<<z1<<" zlast="<<z2);

  double Theta=0;
  int state=0;

  //sort again, but this time with the correct ordering
  if (fabs(yfirst-ylast)>50){
    if(z1>z2) {
      std::sort(seed->begin(),seed->end(), CompareDCzreverse);
    }else{
      std::sort(seed->begin(),seed->end(), CompareDCz);
    }
  }
  else {
    if ((yfirst>0 && z1<0) || (yfirst<0 && z1>0)) std::sort(seed->begin(),seed->end(), CompareDCz);
    else std::sort(seed->begin(),seed->end(), CompareDCzreverse);
  }
  firststraw=(*seed)[0]->detectorElement()->strawCenter(  m_trtid->straw((*seed)[0]->identify()) );
  laststraw=(*seed)[seed->size()-1]->detectorElement()->strawCenter(  m_trtid->straw((*seed)[seed->size()-1]->identify()) );
  z1=firststraw.z();
  z2=laststraw.z();
  

  if(z1>z2) {
    Theta=M_PI-atan2(400.,fabs(z1-z2)); 
    state=2;
  }else{
    Theta=atan2(400.,fabs(z1-z2));
    state=1;
  }
  //std::cout << "theta: " << Theta << std::endl;

  double globaly=((*seed)[0]->detectorElement()->strawCenter(  m_trtid->straw((*seed)[0]->identify()) )).y();
  double firstphi=((*seed)[0]->detectorElement()->strawCenter(  m_trtid->straw((*seed)[0]->identify()) )).phi();


  ATH_MSG_VERBOSE("Number of tube hits matching straight line: "<<seed->size());


  //estimate Segment parameters from first and last hit

  const Amg::Vector3D& sc_first = ((*seed)[0]->detectorElement()->strawCenter(  m_trtid->straw((*seed)[0]->identify()) ));
  const Amg::Vector3D& sc_last = ((*seed)[seed->size()-1]->detectorElement()->strawCenter(  m_trtid->straw((*seed)[seed->size()-1]->identify()) ));


  double tx1,tx2,ty1,ty2;

  tx1=1050*cos(sc_first.phi());
  ty1=1050*sin(sc_first.phi());

  tx2=650*cos(sc_last.phi());
  ty2=650*sin(sc_last.phi());

  double tphi=atan2(ty2-ty1,tx2-tx1);


  ATH_MSG_VERBOSE("First hit: "<<sc_first<<" \nLast hit: "<<sc_last);


  ATH_MSG_VERBOSE(" (x1,y1) = ("<<tx1<<","<<ty1<<")");
  ATH_MSG_VERBOSE(" (x2,y2) = ("<<tx2<<","<<ty2<<")");



  //correct phi to point downwards
  if(tphi>0)
    tphi-=M_PI;

  //make sure that Theta is positive and less than Pi!

  while(Theta<0)
    Theta+=M_PI;

  while(Theta>M_PI)
    Theta-=M_PI;

  ATH_MSG_VERBOSE(" tphi="<<tphi);

  ATH_MSG_VERBOSE("globaly = "<<globaly<<" phi="<<firstphi<<" theta="<<Theta<<" --> state = "<<state);


  vit=seed->begin();
  vitE=seed->end();
  
  count=0;
  for(;vit!=vitE;++vit){
    count++;
    int straw = m_trtid->straw((*vit)->identify());
    const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);
    ATH_MSG_VERBOSE("Hit "<<count<<" z="<<sc.z()<<" phi="<<sc.phi()<<" y="<<sc.y());
  }


  if(m_useDriftTime){

    ATH_MSG_VERBOSE("drifttime should be used!!!");

    //refit the seed but this time including the drift time information

    //take the left-rigfht assignment from the tube fit and then fit again
    count=0;

    vit=seed->begin();
    vitE=seed->end();

    for(;vit!=vitE;++vit){
      int straw = m_trtid->straw((*vit)->identify());
      const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);
      
      double fitted_phi=fit->Eval(sc.z(),0.,0.);
      fitted_phi+=shift;

      if     (fitted_phi > M_PI) fitted_phi = fmod(fitted_phi+M_PI,2.*M_PI)-M_PI; 
      else if(fitted_phi <-M_PI) fitted_phi = fmod(fitted_phi-M_PI,2.*M_PI)+M_PI;

      int sign=1;
      if(fitted_phi<sc.phi()) sign=-1;
      

      double driftr  = (*vit)->localPosition()(Trk::driftRadius);
      double drifte  = 2*sqrt((*vit)->localCovariance()(0,0));
      
      double straw_r=640+400./fabs(z2-z1)*fabs(sc.z()-z1);
      
      double dphi=sign*driftr/straw_r;
      
      
      event_data.m_a_z[count]=sc.z();
      event_data.m_a_phi[count]=sc.phi()+dphi;
      event_data.m_a_phi_err[count]=drifte/straw_r;
      
      event_data.m_a_tan[count]=TMath::Tan(event_data.m_a_phi[count]-shift);
      event_data.m_a_tan_err[count]=(event_data.m_a_phi_err[count])/(  TMath::Cos(event_data.m_a_phi[count]-shift)
                                                                     * TMath::Cos(event_data.m_a_phi[count]-shift));

      event_data.m_a_z_err[count]=0.;
      
      ATH_MSG_VERBOSE(count<<" z="<<event_data.m_a_z[count]<<" phi="<<event_data.m_a_phi[count]<<" +- "<< event_data.m_a_phi_err[count]);
      count++;
    }

    ATH_MSG_VERBOSE("Number of hits in first fit (driftradius): "<<count);

    fit=perform_fit(count,event_data);
    if(!fit) return;


    //check if hits are matching
    vit=seed->begin();
    vitE=seed->end();

    count=0;
    for(;vit!=vitE;++vit){
      int straw = m_trtid->straw((*vit)->identify());
      const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);
      double fitted_phi=fit->Eval(sc.z(),0.,0.);
      fitted_phi+=shift;

      double driftr  = (*vit)->localPosition()(Trk::driftRadius);
      double drifte  = 2*sqrt((*vit)->localCovariance()(0,0));
      int sign=1;
      
      if(fitted_phi<sc.phi()) sign=-1;

      double straw_r=640+400./fabs(z2-z1)*fabs(sc.z()-z1);
      double dphi=sign*driftr/straw_r;

      if(count>500){
      return; // a seed cannot contain that many hits
      }

      if (msgLvl(MSG::VERBOSE)) msg()<<"z="<<sc.z()<<" phi="<<sc.phi()<<" fitted_phi = "<<fitted_phi<<" MC : "<<isTrueHit(*vit)<<endmsg;
      if(fabs(phidiff(fitted_phi,sc.phi()-dphi)) < m_scaleFactorDrift* drifte/straw_r){
        if (msgLvl(MSG::VERBOSE)) msg()<<"\t\t matched!!!"<<endmsg;

        event_data.m_a_z[count]=sc.z();
        event_data.m_a_phi[count]=sc.phi()+dphi;
        event_data.m_a_phi_err[count]=drifte/straw_r;

	event_data.m_a_tan[count]=TMath::Tan(event_data.m_a_phi[count]-shift);
	event_data.m_a_tan_err[count]=(event_data.m_a_phi_err[count])/( TMath::Cos(event_data.m_a_phi[count]-shift)
                                                                       *TMath::Cos(event_data.m_a_phi[count]-shift));

        event_data.m_a_z_err[count]=0.;
        count++;

      }
    }

    //refit

    ATH_MSG_VERBOSE("Number of hits in second fit (driftradius): "<<count);

    if(count>4){
      fit=perform_fit(count,event_data);
      if(!fit) return;
    }

    vit=seed->begin();
    vitE=seed->end();

    count=0;
    for(;vit!=vitE;++vit){
      int straw = m_trtid->straw((*vit)->identify());
      const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);
      
      double fitted_phi=fit->Eval(sc.z(),0.,0.);
      fitted_phi+=shift;

      if     (fitted_phi > M_PI) fitted_phi = fmod(fitted_phi+M_PI,2.*M_PI)-M_PI; 
      else if(fitted_phi <-M_PI) fitted_phi = fmod(fitted_phi-M_PI,2.*M_PI)+M_PI;

      double driftr  = (*vit)->localPosition()(Trk::driftRadius);
      double drifte  = 2*sqrt((*vit)->localCovariance()(0,0));
      int sign=1;
      
      if(fitted_phi<sc.phi()) sign=-1;

      double straw_r=640+400./fabs(z2-z1)*fabs(sc.z()-z1);
      double dphi=sign*driftr/straw_r;

      
      ATH_MSG_VERBOSE("z="<<sc.z()<<" phi="<<sc.phi()<<" fitted_phi = "<<fitted_phi<<" MC : "<<isTrueHit(*vit));
      if(fabs(phidiff(fitted_phi,sc.phi()-dphi)) <  m_scaleFactorDrift* drifte/straw_r){
        ATH_MSG_VERBOSE("\t\t matched!!!");
        count++;
      }
    }

    ATH_MSG_VERBOSE("Number of hits matched after second fit: "<<count);

  }


  //create the segment
  


  if(count<m_minDCSeed) return; //too short segments don't make sense

  if(count>=400) return; //too long segments don't make sense



  // RIOonTrack production
  //
  DataVector<const Trk::MeasurementBase>* rio = new DataVector<const Trk::MeasurementBase>;

  vit=seed->begin();
  vitE=seed->end();

  double chi2=0.;

  double initial_r=-10.;
  double initial_locz=0.;

  count=0;
  for(;vit!=vitE;++vit){
    const Trk::StraightLineSurface* line = (const Trk::StraightLineSurface*)
      (&((*vit)->detectorElement())->surface( (*vit)->identify()));


    int straw = m_trtid->straw((*vit)->identify());
    const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);

    double z=sc.z();
    //    double phi=sc.phi();
    
    double locz=0.;

    if(fabs(z2-z1)>0.000001){
      if(state==1){
	locz=400./fabs(z2-z1)*fabs(z-z1)-200.;
      }else{
	locz=200.-400./fabs(z2-z1)*fabs(z-z1);
      }
    }
    double fitted_phi=fit->Eval(z,0.,0.);
    fitted_phi+=shift;

    if     (fitted_phi > M_PI) fitted_phi = fmod(fitted_phi+M_PI,2.*M_PI)-M_PI; 
    else if(fitted_phi <-M_PI) fitted_phi = fmod(fitted_phi-M_PI,2.*M_PI)+M_PI;

    double fitted_r=(phidiff(fitted_phi,sc.phi()))*(840+locz);
    double temp_phi=fitted_phi;

    if(initial_r==-10. && fabs(fitted_r)<4.0){
      initial_r=fitted_r;
      initial_locz=locz;
    }

    if     (temp_phi > M_PI) temp_phi = fmod(temp_phi+M_PI,2.*M_PI)-M_PI; 
    else if(temp_phi <-M_PI) temp_phi = fmod(temp_phi-M_PI,2.*M_PI)+M_PI;

    ATH_MSG_VERBOSE(count<<" fitted_r = "<<fitted_r<<" locz="<<locz<<" temp_phi="<<temp_phi);


    fitted_r*=-1.0;

    //if(fabs(fitted_r)>=4.0) continue; //remove straws if they are too far away

    temp_phi=firstphi;

    const Trk::AtaStraightLine Tp(fitted_r,locz,tphi,Theta,.000000001,*line);


    //shall we skip this straw as it is too isolated ???
    //    if(m_removeSuspicious && is_suspicious(*vit,seed)) continue;


    //decide if drifttime should be used


    //    std::cout<<"Realsurface: "<<*line<<std::endl;


    bool useDrift=false;

    if(m_useDriftTime){
      double driftr  = (*vit)->localPosition()(Trk::driftRadius);
      double drifte  = 2*sqrt((*vit)->localCovariance()(0,0));
      int sign=1;
      
      if(fitted_phi<sc.phi()) sign=-1;
      
      double straw_r=640+400./fabs(z2-z1)*fabs(sc.z()-z1);
      double dphi=sign*driftr/straw_r;

      if(fabs(phidiff(fitted_phi,sc.phi()-dphi)) < 3* drifte/straw_r){
        useDrift=true;
        chi2+=(fabs(fitted_r)-fabs(driftr))*(fabs(fitted_r)-fabs(driftr))/(drifte*drifte);
      }

    }


    if(useDrift){
      ATH_MSG_VERBOSE("RIO using drift time!");
      rio->push_back(m_riomakerD->correct(*(*vit),Tp));
    }else{
      ATH_MSG_VERBOSE("RIO without using drift time!");
      chi2+=(fitted_r/1.15)*(fitted_r/1.15); // no drift time used
      ATH_MSG_VERBOSE(count<<"\t\t chi2 contribution: "<<(fitted_r/1.15)*(fitted_r/1.15));
      rio->push_back(m_riomakerN->correct(*(*vit),Tp));
    }

    count++;

    //add a pseudomeasurement for the first and the last hit constraining loc_z
    if(count==1 || (size_t)count==seed->size()){
    //if(count>-1){
      Trk::DefinedParameter dp(locz,Trk::locZ);

      std::vector<Trk::DefinedParameter> defPar;
      defPar.push_back(dp);

      Trk::LocalParameters par(defPar);

      /*
      Trk::CovarianceMatrix* cov = new Trk::CovarianceMatrix(1);
      cov->fast(1,1) = 1;
      */
      Amg::MatrixX cov(1,1); 
      cov<<1.;

      //create new surface
      Amg::Vector3D C = line->transform().translation();

      if(state==2 || state==3){
        C[2]-=1.;
      }else{
        C[2]+=1.;
      }
      Amg::Transform3D*            T    = new Amg::Transform3D(line->transform().rotation()*Amg::Translation3D(C));
      Trk::StraightLineSurface* surface = new Trk::StraightLineSurface(T);

      //std::cout<<"Pseudosurface: "<<*surface<<std::endl;

      Trk::PseudoMeasurementOnTrack *pseudo=new Trk::PseudoMeasurementOnTrack( par,cov,*surface);

      delete surface;

      rio->push_back(pseudo);

    }
  }


  //Track Segment production

  Trk::FitQuality      * fqu = new Trk::FitQuality(chi2,count-2);
  const Trk::Surface   * sur =  &((*seed)[0]->detectorElement())->surface((*seed)[0]->identify());


  //phi should always be negative!

  if(firstphi>0) firstphi=-firstphi;


  Trk::DefinedParameter dp0(initial_r,Trk::locR);
  Trk::DefinedParameter dp1(initial_locz,Trk::locZ);
  Trk::DefinedParameter dp2(tphi,Trk::phi);
  Trk::DefinedParameter dp3(Theta,Trk::theta);
  Trk::DefinedParameter dp4(0.00002,Trk::qOverP);

  std::vector<Trk::DefinedParameter> defPar;
  defPar.push_back(dp0);
  defPar.push_back(dp1);
  defPar.push_back(dp2);
  defPar.push_back(dp3);
  defPar.push_back(dp4);

  Trk::LocalParameters par(defPar);

  
  ATH_MSG_VERBOSE("Track parameters from segment:"<<par);

  double universal=1.15;

  /*  
  Trk::CovarianceMatrix* cov = new Trk::CovarianceMatrix(5);
  cov->fast(1,1) = universal*universal;//FIXME: need to take correct drifttime measurement into account!!
  cov->fast(2,1) = 0.;
  cov->fast(2,2) = 160000./12. ;
  cov->fast(3,1) = 0.;
  cov->fast(3,2) = 0.;
  cov->fast(3,3) = 0.1; // dummy for now. Correct value will be given by track refit 
  cov->fast(4,1) = 0.;
  cov->fast(4,2) = 0.;
  cov->fast(4,3) = 0.;
  cov->fast(4,4) = 1.0; // dummy for now. Correct value will be given by track refit 
  cov->fast(5,1) = 0.;
  cov->fast(5,2) = 0.;
  cov->fast(5,3) = 0.;
  cov->fast(5,4) = 0.;  
  cov->fast(5,5) = 1.;// dummy for now. Correct value will be given by track refit 


  Trk::ErrorMatrix* err=new Trk::ErrorMatrix(cov);
  */
  Amg::MatrixX cov(5,5);
  cov<<
    universal*universal,          0., 0.,   0., 0.,
    0.                     , 160000./12., 0.,   0., 0.,
    0.                     ,          0., 0.1,  0., 0.,
    0.                     ,          0.,  0.,  1., 0.,
    0.                     ,          0.,  0.,  0., 1.; 
  Trk::TrackSegment* segment=new Trk::TrackSegment(par,cov,sur,rio,fqu,Trk::Segment::TRT_SegmentMaker);

  
  //add segment to list of segments

  ATH_MSG_VERBOSE("Add segment to list");
  event_data.m_segments.push_back(segment);


}



///////////////////////////////////////////////////////////////////
// Pseudo iterator
///////////////////////////////////////////////////////////////////

Trk::TrackSegment* InDet::TRT_TrackSegmentsMaker_ECcosmics::next(InDet::ITRT_TrackSegmentsMaker::IEventData &virt_event_data) const
{
  TRT_TrackSegmentsMaker_ECcosmics::EventData &
     event_data  = TRT_TrackSegmentsMaker_ECcosmics::EventData::getPrivateEventData(virt_event_data);

  if(event_data.m_segiterator!=event_data.m_segments.end()) return (*event_data.m_segiterator++);
  return 0;
}

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::TRT_TrackSegmentsMaker_ECcosmics::dump( MsgStream& out ) const
{
  return out;
}


///////////////////////////////////////////////////////////////////
// Dumps relevant information into the ostream
///////////////////////////////////////////////////////////////////

std::ostream& InDet::TRT_TrackSegmentsMaker_ECcosmics::dump( std::ostream& out ) const
{
  return out;
}


///////////////////////////////////////////////////////////////////
// retrieve Hits from StoreGate and sort into noise/good hits lists
///////////////////////////////////////////////////////////////////

void InDet::TRT_TrackSegmentsMaker_ECcosmics::retrieveHits(TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const
{

  event_data.m_noiseHits.clear();
  event_data.m_goodHits.clear();


  SG::ReadHandle<InDet::TRT_DriftCircleContainer> trtcontainer(m_trtname);
  if (not trtcontainer.isValid()) {
    ATH_MSG_DEBUG("Could not get TRT_DriftCircleContainer");
    return;
  }

  SG::ReadHandle<Trk::PRDtoTrackMap> prd_to_track_map;
  const Trk::PRDtoTrackMap *prd_to_track_map_cptr=nullptr;
  if (!m_prdToTrackMap.key().empty()) {
    prd_to_track_map = SG::ReadHandle<Trk::PRDtoTrackMap>(m_prdToTrackMap);
    if (!prd_to_track_map.isValid()) {
      ATH_MSG_ERROR("Failed to read PRD to track association map: " << m_prdToTrackMap );
    }
    prd_to_track_map_cptr = prd_to_track_map.cptr();
  }
  
  const InDet::TRT_DriftCircleContainer*   mjo_trtcontainer = trtcontainer.get();
  
 
  if (!mjo_trtcontainer) return;

  // temporary for MC truth !!!
  /*
  if(evtStore()->contains<PRD_MultiTruthCollection>("PRD_MultiTruthTRT")){
    s = evtStore()->retrieve(m_truthCollectionTRT, "PRD_MultiTruthTRT");
    if(s.isFailure()){
      ATH_MSG_DEBUG(" could not open PRD_MultiTruthCollection ! ");
      return;
    }
  }else{
    m_truthCollectionTRT=0;
  }
  */
  // end temporary

  //  std::cout<<"MYTEST: ---------------------"<<std::endl;

  int realhits[2];
  realhits[0]=realhits[1]=0;

  InDet::TRT_DriftCircleContainer::const_iterator
    w = trtcontainer->begin(),we = trtcontainer->end();
  for(; w!=we; ++w) {
    
    Identifier ID = (*w)->identify();
    if(std::abs(m_trtid->barrel_ec(ID))!=2) continue; // only endcaps please!!!


    InDet::TRT_DriftCircleCollection::const_iterator 
      c  = (*w)->begin(), ce = (*w)->end();

    for(; c!=ce; ++c) {

      if(prd_to_track_map_cptr &&  prd_to_track_map_cptr->isUsed(*(*c))) continue; //don't use hits that have already been used



      //TEMPORARY !!!!!!
      //      if(!isTrueHit(*c)) continue;
      
      if(isTrueHit(*c)){
        //which endcap:

        if(m_trtid->barrel_ec((*c)->identify())==-2){
          realhits[0]++;
        }else{
          realhits[1]++;
        }
      }
      
      

      
      //      int straw = m_trtid->straw((*c)->identify());
      //      const Amg::Vector3D& sc = (*c)->detectorElement()->strawCenter(straw);
      
      //      double z=sc.z();
      //      double phi=sc.phi();

      //      std::cout<<"MYTEST: "<<z<<" "<<phi<<std::endl;
      

      if( (*c)->isNoise() || (*c)->timeOverThreshold()<m_cutToTLoose) {
        event_data.m_noiseHits.push_back(*c);
      }else{
        event_data.m_goodHits.push_back(*c);
      }

    }
  }

  //require at least 20 hits
  //  if(realhits[0]>19 || realhits[1]>19) m_real_counter++;

  ATH_MSG_DEBUG("hits in endcap C: "<<realhits[0]<<" Hits in endcap A: "<<realhits[1]);
  if(realhits[0]>19 || realhits[1]>19) { ATH_MSG_DEBUG( "This event should be reconstructed !!!!" ); }

  ATH_MSG_DEBUG("good hits in endcap: "<<event_data.m_goodHits.size());
  ATH_MSG_DEBUG("noise hits in endcap: "<<event_data.m_noiseHits.size());


  if(event_data.m_goodHits.size()>(size_t)m_hitLimit){
    // this event is too busy ...
    ATH_MSG_DEBUG("This event has more than "<<m_hitLimit<<" good hits. Aborting segment finding ...");
    event_data.m_goodHits.clear();
    event_data.m_noiseHits.clear();
    return;
  }

  //move isolated hits from the good hits vector to the noise hits vector

  std::list<const InDet::TRT_DriftCircle*>::iterator it,it2,itE,it_remove;

  it=event_data.m_goodHits.begin();
  itE=event_data.m_goodHits.end();

  bool remove=false;


  for(;it!=itE;++it){

    if(remove){
      event_data.m_goodHits.erase(it_remove);
      remove=false;
    }

    Identifier ID = (*it)->identify();
    int wheel=abs(m_trtid->layer_or_wheel(ID));
    int endcap=m_trtid->barrel_ec(ID);

    int ns = m_trtid->straw((*it)->identify());
    const Amg::Vector3D& sc = (*it)->detectorElement()->strawCenter(ns);

    double max_r=10000;
    //    double max_z=10000;
    //    double max_phi=10000;
    bool accept=false;

    for(it2=event_data.m_goodHits.begin();it2!=itE;++it2){
      if(it==it2) continue;

      Identifier ID2 = (*it2)->identify();

      int endcap2=m_trtid->barrel_ec(ID2);
      int wheel2=abs(m_trtid->layer_or_wheel(ID2));

      //only look inside the same wheel
      if(endcap!=endcap2 || wheel2!=wheel) continue;

      int ns2 = m_trtid->straw((*it2)->identify());
      const Amg::Vector3D& sc2 = (*it2)->detectorElement()->strawCenter(ns2);

      double dz=sc.z()-sc2.z();
      double dphi=phidiff(sc.phi(),sc2.phi());
      
      dphi*=300.0; // scale factor for dphi to get equal weight
      
      double dr=sqrt(dz*dz+dphi*dphi);

      
      dz=fabs(dz);
      dphi=fabs(dphi/300.0);

      if(dr<max_r) {
        max_r=dr;
        //if(max_r<m_maximald) break;
      }

      if(dphi<0.03 && dz<31)
        accept=true;

    }

    //    if(max_r>m_maximald){
    if(!accept){
      event_data.m_noiseHits.push_back(*it);
      it_remove=it;
      remove=true;
    }
  }

  if(remove){
    event_data.m_goodHits.erase(it_remove);
    remove=false;
  }



  
  //2nd pass through good hits with increased ToT cut
  
  it=event_data.m_goodHits.begin();
  itE=event_data.m_goodHits.end();
  
  remove=false;
  
  for(;it!=itE;++it){

    if(remove){
      event_data.m_goodHits.erase(it_remove);
      remove=false;
    }

    if((*it)->timeOverThreshold()<m_cutToTTight || (*it)->timeOverThreshold()>m_cutToTUpper){
      event_data.m_noiseHits.push_back(*it);
      it_remove=it;
      remove=true;
    }
  }
  if(remove){
    event_data.m_goodHits.erase(it_remove);
    remove=false;
  }


  //2nd pass through good hits with loosened isolation criteria

  it=event_data.m_goodHits.begin();
  itE=event_data.m_goodHits.end();

  remove=false;

  for(;it!=itE;++it){

    if(remove){
      event_data.m_goodHits.erase(it_remove);
      remove=false;
    }

    Identifier ID = (*it)->identify();
    int wheel=abs(m_trtid->layer_or_wheel(ID));
    int endcap=m_trtid->barrel_ec(ID);

    int ns = m_trtid->straw((*it)->identify());
    const Amg::Vector3D& sc = (*it)->detectorElement()->strawCenter(ns);

    double max_r=10000;
    //    double max_z=10000;
    //    double max_phi=10000;
    bool accept=false;

    for(it2=event_data.m_goodHits.begin();it2!=itE;++it2){
      if(it==it2) continue;

      Identifier ID2 = (*it2)->identify();

      int endcap2=m_trtid->barrel_ec(ID2);
      int wheel2=abs(m_trtid->layer_or_wheel(ID2));

      //only look inside the same wheel
      if(endcap!=endcap2 || wheel2!=wheel) continue;

      int ns2 = m_trtid->straw((*it2)->identify());
      const Amg::Vector3D& sc2 = (*it2)->detectorElement()->strawCenter(ns2);

      double dz=sc.z()-sc2.z();
      double dphi=phidiff(sc.phi(),sc2.phi());
      
      dphi*=300.0; // scale factor for dphi to get equal weight
      
      double dr=sqrt(dz*dz+dphi*dphi);

      
      dz=fabs(dz);
      dphi=fabs(dphi/300.0);

      if(dr<max_r) {
        max_r=dr;
        //if(max_r<m_maximald) break;
      }

      if(dphi<0.25 && dz<200)
        accept=true;

    }

    //    if(max_r>m_maximald){
    if(!accept){
      ATH_MSG_DEBUG("Removing hit in 2nd pass isolation: "<<isTrueHit(*it));


      event_data.m_noiseHits.push_back(*it);
      it_remove=it;
      remove=true;
    }
  }

  if(remove){
    event_data.m_goodHits.erase(it_remove);
    remove=false;
  }

  ATH_MSG_DEBUG("good hits in endcap: "<<event_data.m_goodHits.size());
  ATH_MSG_DEBUG("noise hits in endcap: "<<event_data.m_noiseHits.size());

}

///////////////////////////////////////////////////////////////////
// MC performance test: check if hit is from MC particle or noise
///////////////////////////////////////////////////////////////////


bool InDet::TRT_TrackSegmentsMaker_ECcosmics::isTrueHit(const InDet::TRT_DriftCircle*) const
{
  if(!m_truthCollectionTRT) return false;
  /*
  typedef PRD_MultiTruthCollection::const_iterator iter;
  int numBarcodes=0;
  std::pair<iter,iter> range = m_truthCollectionTRT->equal_range(dc->identify());
  for(iter i = range.first; i != range.second; i++){
    numBarcodes++;
  }
  if(numBarcodes>0) return true;
  return false;
  */
  return false; 
}


///////////////////////////////////////////////////////////////////
// Perform the actual track fit to the coordinates
///////////////////////////////////////////////////////////////////

std::mutex InDet::TRT_TrackSegmentsMaker_ECcosmics::s_fitMutex;

TF1 *InDet::TRT_TrackSegmentsMaker_ECcosmics::perform_fit(int count,
                                                          TRT_TrackSegmentsMaker_ECcosmics::EventData &event_data) const
{
  // @TODO not thread-safe enough! need a globak root lock !
  std::lock_guard<std::mutex> lock(s_fitMutex);
  TVirtualFitter::SetMaxIterations(100000);

  //save ROOT error message level
  int originalErrorLevel=gErrorIgnoreLevel;
  gErrorIgnoreLevel=10000;


  event_data.m_fitf_ztanphi->SetParameter(0,0.);
  event_data.m_fitf_ztanphi->SetParameter(1,0.);
  event_data.m_fitf_ztanphi->SetParameter(2,0.);

  TGraphErrors *gre=new TGraphErrors(count,event_data.m_a_z,event_data.m_a_tan,event_data.m_a_z_err,event_data.m_a_tan_err);
  int fitresult1=gre->Fit(event_data.m_fitf_ztanphi,"Q");

  
//   char buffer[256];
//   sprintf(buffer,"graph_analytic_%d.pdf",m_counter);
  
//   TCanvas *c=new TCanvas();
//   gre->Draw("AP");
//   c->SaveAs(buffer);
//   delete c;
  

  ATH_MSG_VERBOSE("Fit result (ztanphi): a0="<<event_data.m_fitf_ztanphi->GetParameter(0)
                  <<" a1="<<event_data.m_fitf_ztanphi->GetParameter(1)<<" a2="
                  <<event_data.m_fitf_ztanphi->GetParameter(2)<<" fitresult="<<fitresult1);

  //copy over the parameters
  event_data.m_fitf_zphi->SetParameter(0,event_data.m_fitf_ztanphi->GetParameter(0));
  event_data.m_fitf_zphi->SetParameter(1,event_data.m_fitf_ztanphi->GetParameter(1));
  event_data.m_fitf_zphi->SetParameter(2,event_data.m_fitf_ztanphi->GetParameter(2));
  
  delete gre;

  gre=new TGraphErrors(count,event_data.m_a_z,event_data.m_a_phi,event_data.m_a_z_err,event_data.m_a_phi_err);
  //  gre->Fit("pol2","+rob=0.75");
  //  int fitresult2=gre->Fit("pol2","Q");
  int fitresult2=gre->Fit(event_data.m_fitf_zphi_approx,"Q");

  /*
  TF1 *f=gre->GetFunction("pol2");

  m_fitf_zphi_approx->SetParameter(0,f->GetParameter(0));
  m_fitf_zphi_approx->SetParameter(1,f->GetParameter(1));
  m_fitf_zphi_approx->SetParameter(2,f->GetParameter(2));
  */

//   sprintf(buffer,"graph_approx_%d.pdf",m_counter++);
//   c=new TCanvas();
//   gre->Draw("AP");
//   c->SaveAs(buffer);
//   delete c;
  
  ATH_MSG_VERBOSE( "Fit result (zphi): a0="<< event_data.m_fitf_zphi_approx->GetParameter(0)
                   <<" a1="<< event_data.m_fitf_zphi_approx->GetParameter(1)<<" a2="
                   << event_data.m_fitf_zphi_approx->GetParameter(2)<<" fitresult="<<fitresult2);
  
  double p1=event_data.m_fitf_ztanphi->GetProb();
  double p2=event_data.m_fitf_zphi_approx->GetProb();

  ATH_MSG_VERBOSE("tanphi prob: "<<p1<<" pol2 prob : "<<p2);

  delete gre;


  if(fitresult1!=0 && fitresult2!=0) return NULL;
  if(fitresult1!=0 && fitresult2==0) return event_data.m_fitf_zphi_approx;
  if(fitresult1==0 && fitresult2!=0) return event_data.m_fitf_zphi;


  int match_tan=0;
  int match_phi=0;
  for(int i=0;i<count;i++){
    double phi1=event_data.m_fitf_zphi->Eval(event_data.m_a_z[i]);
    double phi2=event_data.m_fitf_zphi_approx->Eval(event_data.m_a_z[i]);

    if(fabs(phi1-event_data.m_a_phi[i])<2*event_data.m_a_phi_err[i])
      match_tan++;

    if(fabs(phi2-event_data.m_a_phi[i])<2*event_data.m_a_phi_err[i])
      match_phi++;

  }


  ATH_MSG_VERBOSE("Number of hits matching this fit: tan="<<match_tan<<" pol2="<<match_phi);

  if(match_tan>match_phi){
    return event_data.m_fitf_zphi;
  }else if(match_tan<match_phi){
    return event_data.m_fitf_zphi_approx;
  }

  gErrorIgnoreLevel=originalErrorLevel;

  if(p1>p2){
    return event_data.m_fitf_zphi;
  }else{
    return event_data.m_fitf_zphi_approx;
  }

}


///////////////////////////////////////////////////////////////////
// Check how many real and how many fake segments we have
///////////////////////////////////////////////////////////////////

double InDet::TRT_TrackSegmentsMaker_ECcosmics::classify_segment(Trk::TrackSegment* seg, int &total) const
{
  int real=0;
  int noise=0;
  total=0;

  for(unsigned int i=0;i<seg->numberOfMeasurementBases();++i){
    const Trk::RIO_OnTrack* rot=dynamic_cast<const Trk::RIO_OnTrack*>(seg->measurement(i));
    if(rot){
      const Trk::PrepRawData* prd=rot->prepRawData();
      if(prd!=0){
        const InDet::TRT_DriftCircle *dc=dynamic_cast<const InDet::TRT_DriftCircle*>(prd);
        if(dc!=0){
          if(isTrueHit(dc))
            real++;
          else
            noise++;
          total++;

        }
      }
    }
  }
  if(total==0) total = 1;
  return double(real)/double(total);
}


///////////////////////////////////////////////////////////////////
// Provide the proper subtraction of two phi values
///////////////////////////////////////////////////////////////////


// @TODO unusued
bool InDet::TRT_TrackSegmentsMaker_ECcosmics::is_suspicious(const InDet::TRT_DriftCircle* dc,
                                                            std::vector<const InDet::TRT_DriftCircle*> *seed) const
{

  double mean=0,var=0;
  double meanz=0,varz=0;

  std::vector<const InDet::TRT_DriftCircle*>::iterator vit,vitE;

  vit=seed->begin();
  vitE=seed->end();

  double dcz[500];
  int count=0;
  int index=-1;
  for(;vit!=vitE;++vit){
    int straw = m_trtid->straw((*vit)->identify());
    const Amg::Vector3D& sc = (*vit)->detectorElement()->strawCenter(straw);

    if(dc==*vit) index=count;
    dcz[count++]=sc.z();

    meanz+=sc.z();
    varz+=sc.z()*sc.z();
  }

  if(count < 2) return true;

  varz=(count*varz-meanz*meanz);
  varz/=(double)(count*(count-1));

  meanz/=(double)count;
  //  varz/=(double)count;

  varz=sqrt(varz);

  double zmin_sel=100000;

  for(int i=0;i<count;i++){
    double zmin=10000;
    
    if(i-1>=0)
      if(fabs(dcz[i]-dcz[i-1])<zmin) zmin=fabs(dcz[i]-dcz[i-1]);
    if(i+1<count)
      if(fabs(dcz[i]-dcz[i+1])<zmin) zmin=fabs(dcz[i]-dcz[i+1]);

    if(i==index)
      zmin_sel=zmin;

    mean+=zmin;
    var+=zmin*zmin;
  }

  var=(count*var-mean*mean);
  var/=(double)(count*(count-1));

  mean/=(double)count;

  var=sqrt(var);

  if(var<6) var=6;

  if(fabs(zmin_sel-mean)>2*var){// || fabs(dcz[index]-meanz)>2*varz){
    if(index>=0) {
    ATH_MSG_VERBOSE("Hit is suspicious, remove it! "<<fabs(zmin_sel-mean)<<" , "<<2*var<<" -> "<<zmin_sel<< " : "<<dcz[index]<<" <-> "<<meanz<<" ("<<varz<<")");
    }
    ATH_MSG_VERBOSE("\t -> "<<isTrueHit(dc));
    return true;
  }else{
    if(index>=0) {
      ATH_MSG_VERBOSE("Hit seems to be ok: "<<fabs(zmin_sel-mean)<<" < "<<2*var<<" && "<<fabs(dcz[index]-meanz)<<" < "<<1.5*varz);
    }
    ATH_MSG_VERBOSE("\t -> "<<isTrueHit(dc));
  }
  return false;
}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ************************************************
//
// NAME:     TrigT2HistoPrmVtx.cxx
// PACKAGE:  Trigger/TrigAlgorithms/TrigT2HistoPrmVtx
//
// AUTHOR:   Andrea Coccaro
// EMAIL:    Andrea.Coccaro@ge.infn.it
// 
// ************************************************

#include "TrigT2HistoPrmVtx/TrigT2HistoPrmVtx.h"
#include "TrigT2HistoPrmVtx/HistoVertexHelper.h"
#include "TrigT2HistoPrmVtxBase.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODBase/IParticle.h"

#include "InDetBeamSpotService/IBeamCondSvc.h"



//** ----------------------------------------------------------------------------------------------------------------- **//


TrigT2HistoPrmVtx::TrigT2HistoPrmVtx(const std::string& name, ISvcLocator* pSvcLocator) :
  HLT::FexAlgo(name, pSvcLocator),
  c( new TrigT2HistoPrmVtxBase(msg(), msgLvl()) ),
  m_constTrigT2HistoPrmVtxBase(0),
  m_trigVertexColl(0)
{
  declareProperty ("AlgoId",             c->m_algo);
  declareProperty ("Instance",           c->m_instance);

  declareProperty ("NumBins",            c->m_nBins);

  declareProperty ("UseBeamSpot",        c->m_useBeamSpot        = false);
  declareProperty ("UseEtaPhiTrackSel",  c->m_useEtaPhiTrackSel  = false);

  declareProperty ("EFTrkSel_Chi2",      c->m_efTrkSelChi2       = 0.0);
  declareProperty ("EFTrkSel_BLayer",    c->m_efTrkSelBLayer     = 1);
  declareProperty ("EFTrkSel_PixHits",   c->m_efTrkSelPixHits    = 2);
  declareProperty ("EFTrkSel_SiHits",    c->m_efTrkSelSiHits     = 7);
  declareProperty ("EFTrkSel_D0",        c->m_efTrkSelD0         = 1*CLHEP::mm);
  declareProperty ("EFTrkSel_Pt",        c->m_efTrkSelPt         = 1*CLHEP::GeV);

  declareMonitoredVariable("PrmVtx",          c->zPrmVtx[0]          = -1);
  declareMonitoredVariable("PrmVtxSigmaAll",  c->zPrmVtxSigmaAll[0]  = -1);
  declareMonitoredVariable("PrmVtxSigma2Trk", c->zPrmVtxSigma2Trk[0] = -1);
  declareMonitoredVariable("TracksPerVtx",    c->nTrackVtx[0]        = -1);
  declareMonitoredVariable("NVtxFound",       c->nVtxFound           = -1);

  declareMonitoredObject("Trk_selectedTracks", m_constTrigT2HistoPrmVtxBase, &TrigT2HistoPrmVtxBase::totSelectedTracks);
  declareMonitoredStdContainer("Trk_stepsToSelect", c->m_listCutApplied);
}


//** ----------------------------------------------------------------------------------------------------------------- **//


TrigT2HistoPrmVtx::~TrigT2HistoPrmVtx() {}


//** ----------------------------------------------------------------------------------------------------------------- **//


HLT::ErrorCode TrigT2HistoPrmVtx::hltInitialize() {

  // Get message service 
  if (msgLvl() <= MSG::INFO) 
    msg() << MSG::INFO << "Initializing TrigT2HistoPrmVtx, version " << PACKAGE_VERSION << endreq;

 if (msgLvl() <= MSG::DEBUG) {
    msg() << MSG::DEBUG << "declareProperty review:" << endreq;
    msg() << MSG::DEBUG << " AlgoId = "              << c->m_algo << endreq; 
    msg() << MSG::DEBUG << " Instance = "            << c->m_instance << endreq; 
    msg() << MSG::DEBUG << " Numbins = "             << c->m_nBins << endreq; 
    msg() << MSG::DEBUG << " UseBeamSpot = "         << c->m_useBeamSpot << endreq; 
    msg() << MSG::DEBUG << " UseEtaPhiTrackSel = "   << c->m_useEtaPhiTrackSel << endreq;

    if (c->m_instance == "_EF") {

      msg() << MSG::DEBUG << " EFTrkSel_Chi2 = "     << c->m_efTrkSelChi2 << endreq; 
      msg() << MSG::DEBUG << " EFTrkSel_BLayer = "   << c->m_efTrkSelBLayer << endreq; 
      msg() << MSG::DEBUG << " EFTrkSel_PixHits = "  << c->m_efTrkSelPixHits << endreq; 
      msg() << MSG::DEBUG << " EFTrkSel_SiHits = "   << c->m_efTrkSelSiHits << endreq;    
      msg() << MSG::DEBUG << " EFTrkSel_D0 = "       << c->m_efTrkSelD0 << endreq; 
      msg() << MSG::DEBUG << " EFTrkSel_Pt = "       << c->m_efTrkSelPt << endreq; 

    }
  }

  m_constTrigT2HistoPrmVtxBase = const_cast<const TrigT2HistoPrmVtxBase*>(c);

  return HLT::OK;
}

//** ----------------------------------------------------------------------------------------------------------------- **//


unsigned int TrigT2HistoPrmVtx::getTrackNumbers(const xAOD::TrackParticleContainer* pointerToEFTrackCollections) {

  unsigned int nEFtracks;

  if (pointerToEFTrackCollections) {
    nEFtracks = pointerToEFTrackCollections->size();
    if (msgLvl() <= MSG::DEBUG)  
      msg() << MSG::DEBUG << "Found " << nEFtracks << " tracks in the RoI" << endreq;
  } else {
    nEFtracks = 0;
    if (msgLvl() <= MSG::DEBUG)  
      msg() << MSG::DEBUG << "No tracks in the RoI" << endreq;
  }

  return nEFtracks;
}


//** ----------------------------------------------------------------------------------------------------------------- **//


HLT::ErrorCode TrigT2HistoPrmVtx::hltExecute(const HLT::TriggerElement* /*inputTE*/, HLT::TriggerElement* outputTE) {

  if (msgLvl() <= MSG::DEBUG) 
    msg() << MSG::DEBUG << "Executing TrigT2HistoPrmVtx" << endreq;    

  float zFirstTrack=0;
  float zFirstTrackError=0;

  c->m_totTracks=0;
  c->m_totSelTracks=0;

  c->m_hisVtx->reset();

  for (int i = 0; i <3; i++) {
    c->zPrmVtx.at(i)          = -200;
    c->zPrmVtxSigmaAll.at(i)  = -200;
    c->zPrmVtxSigma2Trk.at(i) = -200;
    c->nTrackVtx.at(i)        = -200;
  }

  // Retrieve beamspot information
  c->m_xBeamSpot = c->m_yBeamSpot = c->m_zBeamSpot = 0;
  c->m_xBeamSpotSigma = c->m_yBeamSpotSigma = c->m_zBeamSpotSigma = 1e-6;
  
  if(c->m_useBeamSpot) {
    
    IBeamCondSvc* m_iBeamCondSvc; 
    StatusCode scBS = service("BeamCondSvc", m_iBeamCondSvc);
    
    if (scBS.isFailure() || m_iBeamCondSvc == 0) {
      m_iBeamCondSvc = 0;
      
      if (msgLvl() <= MSG::WARNING) {
	msg() << MSG::WARNING << "Could not retrieve Beam Conditions Service. " << endreq;
	msg() << MSG::WARNING << "Using origin at ( " << c->m_xBeamSpot << " , " << c->m_yBeamSpot << " , " << c->m_zBeamSpot << " ) " << endreq;
      }
      
    } else {
      
      Amg::Vector3D m_beamSpot = m_iBeamCondSvc->beamPos();
      
      c->m_xBeamSpot = m_beamSpot.x();
      c->m_yBeamSpot = m_beamSpot.y();
      c->m_zBeamSpot = m_beamSpot.z();
      c->m_xBeamSpotSigma = m_iBeamCondSvc->beamSigma(0);
      c->m_yBeamSpotSigma = m_iBeamCondSvc->beamSigma(1);
      c->m_zBeamSpotSigma = m_iBeamCondSvc->beamSigma(2);

      if(msgLvl() <= MSG::DEBUG)
	msg() << MSG::DEBUG << "Beam spot from service: x = " << c->m_xBeamSpot << " +/- " << c->m_xBeamSpotSigma << "   "
	      << "y = " << c->m_yBeamSpot << " +/- " << c->m_yBeamSpotSigma << "   "
	      << "z = " << c->m_zBeamSpot << " +/- " << c->m_zBeamSpotSigma << endreq;
    }
  }

  // Get RoI descriptor
  const TrigRoiDescriptor* roiDescriptor = 0;
  if (getFeature(outputTE, roiDescriptor) == HLT::OK ) {
    if (msgLvl() <= MSG::DEBUG) {
      msg() << MSG::DEBUG << "Using outputTE: "
	    << "RoI id " << roiDescriptor->roiId()
	    << ", Phi = " <<  roiDescriptor->phi()
	    << ", Eta = " << roiDescriptor->eta() << endreq;
    }
  } else {
    if (msgLvl() <= MSG::WARNING) 
      msg() <<  MSG::WARNING << "No RoI for this Trigger Element " << endreq;

    return HLT::NAV_ERROR;
  }

  // Create pointers to collections 
  const xAOD::TrackParticleContainer* pointerToEFTrackCollections = 0;

  if (c->m_instance == "_EF" || c->m_instance == "_HLT") { 

    if (msgLvl() <= MSG::DEBUG)
      msg() << MSG::DEBUG << "Executing TrigT2HistoPrmVtx at HLT" << endreq;

    HLT::ErrorCode status = getFeature(outputTE, pointerToEFTrackCollections, ""); 
    if (status != HLT::OK) {
      msg() << MSG::DEBUG << "No tracks retrieved from TrigT2HistoPrmVtx" << endreq;
    } 
    else if (msgLvl() <= MSG::DEBUG)  
      msg() << MSG::DEBUG << "Got collection from TrigT2HistoPrmVtx" << endreq;   

    // Get tracks number
    c->m_totTracks = getTrackNumbers(pointerToEFTrackCollections);

    c->m_listCutApplied.clear();

    // Retrieve z0 parameters 
    for (unsigned int i = 0; i < c->m_totTracks; i++) {
      
      if (msgLvl() <= MSG::DEBUG && i == 0) 
	msg() << MSG::DEBUG << "Loop over tracks: retrieving z0" << endreq;

      const xAOD::TrackParticle* track = (*pointerToEFTrackCollections)[i];
      float z0 =  track->z0();
      const xAOD::ParametersCovMatrix_t covTrk = track->definingParametersCovMatrix();
      float ez0 = Amg::error(covTrk, 1);

      if (!c->efTrackSel(track, i, roiDescriptor->eta(), roiDescriptor->phi())) continue;

      c->m_totSelTracks++;

      if (c->m_totSelTracks == 1) {
	zFirstTrack=z0;
	zFirstTrackError=ez0;
      }

      if (msgLvl() <= MSG::VERBOSE)
	msg() << MSG::VERBOSE << "Track number " << i+1 << " with z0 = " << z0 << " and ez0 = " << ez0 << endreq;
      
      c->m_hisVtx->fill(z0);
    }  
  }

  // Get primary vertex from TrigT2HistoPrmVtx::findPrmVtx
  if (c->m_totSelTracks == 1) {
    c->zPrmVtx.at(0)          = zFirstTrack;
    c->zPrmVtxSigmaAll.at(0)  = zFirstTrackError;
    c->nTrackVtx.at(0)        = 1;
    c->nVtxFound = 1;
  } 
  else {
    c->findPrmVtx();
  }

  if (msgLvl() <= MSG::DEBUG) {
    for (int i = 0; i <3; i++)
      msg() << MSG::DEBUG << "Primary vertex " << i << ": z=" << c->zPrmVtx.at(i) << ", sigma=" << c->zPrmVtxSigmaAll.at(i) << ", nTracks=" << c->nTrackVtx.at(i) << endreq;
  }

  if (msgLvl() <= MSG::DEBUG) {

    const EventInfo* pEventInfo;
 
    if ( !store() || store()->retrieve(pEventInfo).isFailure() ) {
      msg()  << MSG::DEBUG << "Failed to get EventInfo " << endreq;
    } else {
      msg() << MSG::DEBUG << "PrmVtx summary (Run " << pEventInfo->event_ID()->run_number()
	    << "; Event "<< pEventInfo->event_ID()->event_number() << ")" << endreq;
      msg() << MSG::DEBUG << "REGTEST: RoI " << roiDescriptor->roiId() << ", Phi = "   << roiDescriptor->phi() << ", Eta = "   << roiDescriptor->eta() << endreq;
      msg() << MSG::DEBUG << "REGTEST: Tracks: " << c->m_totTracks << " reconstructed and " << c->m_totSelTracks <<" selected" << endreq;
      msg() << MSG::DEBUG << "REGTEST: Primary vertex: " << c->zPrmVtx.at(0) << " +/- " << c->zPrmVtxSigmaAll.at(0) << " with " << c->nTrackVtx.at(0) << " tracks" << endreq;
    }
  }

  // Create the xAOD container and its auxiliary store:
  m_trigVertexColl = new xAOD::VertexContainer();
  xAOD::VertexAuxContainer aux;
  m_trigVertexColl->setStore( &aux );
  xAOD::Vertex* trigVertex = new xAOD::Vertex();
  m_trigVertexColl->push_back(trigVertex); 

  trigVertex->setX(c->m_xBeamSpot);
  trigVertex->setY(c->m_yBeamSpot);
  trigVertex->setZ(c->zPrmVtx.at(0) + c->m_zBeamSpot);
  trigVertex->setVertexType(xAOD::VxType::PriVtx);

  AmgSymMatrix(3) cov;
  cov(0,0) = c->m_xBeamSpotSigma*c->m_xBeamSpotSigma;
  cov(1,1) = c->m_yBeamSpotSigma*c->m_yBeamSpotSigma;
  cov(2,2) = c->zPrmVtxSigmaAll.at(0)*c->zPrmVtxSigmaAll.at(0);
  cov(0,1) = cov(1,0) = 0.0; 
  cov(0,2) = cov(2,0) = 0.0;
  cov(1,2) = cov(2,1) = 0.0;
  trigVertex->setCovariancePosition(cov);

// What about the tracking algo ID???
  m_trigVertexColl->push_back(trigVertex);

  string key;

  if (c->m_instance == "_EF" || c->m_instance == "_HLT") key = "EFHistoPrmVtx";

  HLT::ErrorCode stat = attachFeature(outputTE, m_trigVertexColl, key);
  if (stat != HLT::OK) {
    if (msgLvl() <= MSG::WARNING) 
      msg() << MSG::WARNING << "Failed to attach xAOD::VertexContainer to navigation" << endreq;
    return stat;
  }

  return HLT::OK;
}

//** ----------------------------------------------------------------------------------------------------------------- **//


HLT::ErrorCode TrigT2HistoPrmVtx::hltFinalize() {

  if (msgLvl() <= MSG::INFO) 
    msg() << MSG::INFO << "Finalizing TrigT2HistoPrmVtx" << endreq;

  return HLT::OK;
}

 


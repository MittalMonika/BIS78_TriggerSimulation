/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ************************************************
//
// NAME:     TrigT2HistoPrmVtxAllTE.cxx
// PACKAGE:  Trigger/TrigAlgorithms/TrigT2HistoPrmVtx
//
// AUTHOR:   Andrea Coccaro
// EMAIL:    Andrea.Coccaro@ge.infn.it
// 
// ************************************************

#include "TrigT2HistoPrmVtx/TrigT2HistoPrmVtxAllTE.h"
#include "TrigT2HistoPrmVtx/HistoVertexHelper.h"
#include "TrigT2HistoPrmVtxBase.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrigTimeAlgs/TrigTimerSvc.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODBase/IParticle.h"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "GaudiKernel/SystemOfUnits.h"


//** ----------------------------------------------------------------------------------------------------------------- **//


TrigT2HistoPrmVtxAllTE::TrigT2HistoPrmVtxAllTE(const std::string& name, ISvcLocator* pSvcLocator) :
  HLT::AllTEAlgo(name, pSvcLocator),
  m_c( new TrigT2HistoPrmVtxBase(msg(), msgLvl()) ),
  m_constTrigT2HistoPrmVtxBase(0),
  m_trigVertexColl(0)
{
  declareProperty ("AlgoId",             m_c->m_algo);
  declareProperty ("Instance",           m_c->m_instance);

  declareProperty ("NumBins",            m_c->m_nBins);

  declareProperty ("UseBeamSpot",        m_c->m_useBeamSpot        = false);
  declareProperty ("UseEtaPhiTrackSel",  m_c->m_useEtaPhiTrackSel  = false);

  declareProperty ("EFTrkSel_Chi2",      m_c->m_efTrkSelChi2       = 0.0);
  declareProperty ("EFTrkSel_BLayer",    m_c->m_efTrkSelBLayer     = 1);
  declareProperty ("EFTrkSel_PixHits",   m_c->m_efTrkSelPixHits    = 2);
  declareProperty ("EFTrkSel_SiHits",    m_c->m_efTrkSelSiHits     = 7);
  declareProperty ("EFTrkSel_D0",        m_c->m_efTrkSelD0         = 1*Gaudi::Units::mm);
  declareProperty ("EFTrkSel_Pt",        m_c->m_efTrkSelPt         = 1*Gaudi::Units::GeV);

  declareMonitoredVariable("PrmVtx",          m_c->m_zPrmVtx[0]          = -1);
  declareMonitoredVariable("PrmVtxSigmaAll",  m_c->m_zPrmVtxSigmaAll[0]  = -1);
  declareMonitoredVariable("PrmVtxSigma2Trk", m_c->m_zPrmVtxSigma2Trk[0] = -1);
  declareMonitoredVariable("TracksPerVtx",    m_c->m_nTrackVtx[0]        = -1);
  declareMonitoredVariable("NVtxFound",       m_c->m_nVtxFound           = -1);

  declareMonitoredObject("Trk_selectedTracks", m_constTrigT2HistoPrmVtxBase, &TrigT2HistoPrmVtxBase::totSelectedTracks);
  declareMonitoredStdContainer("Trk_stepsToSelect", m_c->m_listCutApplied);
}


//** ----------------------------------------------------------------------------------------------------------------- **//


TrigT2HistoPrmVtxAllTE::~TrigT2HistoPrmVtxAllTE() {}


//** ----------------------------------------------------------------------------------------------------------------- **//


HLT::ErrorCode TrigT2HistoPrmVtxAllTE::hltInitialize() {

  ATH_MSG_INFO( "Initializing TrigT2HistoPrmVtxAllTE, version " << PACKAGE_VERSION  );
  
  if (msgLvl() <= MSG::DEBUG) {
    ATH_MSG_DEBUG( "declareProperty review:"  );
    ATH_MSG_DEBUG( " AlgoId = "              << m_c->m_algo  );
    ATH_MSG_DEBUG( " Instance = "            << m_c->m_instance  );
    ATH_MSG_DEBUG( " Numbins = "             << m_c->m_nBins  );
    ATH_MSG_DEBUG( " UseBeamSpot = "         << m_c->m_useBeamSpot  );
    ATH_MSG_DEBUG( " UseEtaPhiTrackSel = "   << m_c->m_useEtaPhiTrackSel  );

    if (m_c->m_instance == "_EF" || m_c->m_instance == "_HLT") {

      ATH_MSG_DEBUG( " EFTrkSel_Chi2 = "     << m_c->m_efTrkSelChi2  );
      ATH_MSG_DEBUG( " EFTrkSel_BLayer = "   << m_c->m_efTrkSelBLayer  );
      ATH_MSG_DEBUG( " EFTrkSel_PixHits = "  << m_c->m_efTrkSelPixHits  );
      ATH_MSG_DEBUG( " EFTrkSel_SiHits = "   << m_c->m_efTrkSelSiHits  );
      ATH_MSG_DEBUG( " EFTrkSel_D0 = "       << m_c->m_efTrkSelD0  );
      ATH_MSG_DEBUG( " EFTrkSel_Pt = "       << m_c->m_efTrkSelPt  );
    }
  }
  if(m_beamSpotKey.initialize().isFailure()) return HLT::BAD_JOB_SETUP;
  m_constTrigT2HistoPrmVtxBase = const_cast<const TrigT2HistoPrmVtxBase*>(m_c);

  return HLT::OK;
}

//** ----------------------------------------------------------------------------------------------------------------- **//


unsigned int TrigT2HistoPrmVtxAllTE::getTrackNumbers(const xAOD::TrackParticleContainer* pointerToEFTrackCollections) {
  
  unsigned int nEFtracks;
  
  if (pointerToEFTrackCollections) {
    nEFtracks = pointerToEFTrackCollections->size();
    ATH_MSG_DEBUG( "Found " << nEFtracks << " tracks in the RoI"  );
  } else {
    nEFtracks = 0;
    ATH_MSG_DEBUG( "No tracks in the RoI"  );
  }
  
  return nEFtracks;
}


//** ----------------------------------------------------------------------------------------------------------------- **//


HLT::ErrorCode TrigT2HistoPrmVtxAllTE::hltExecute(std::vector<std::vector<HLT::TriggerElement*> >& inputTE, unsigned int output) {

  // since this is an AllTEAlgo, we have to call the monitoring ourselves:
  beforeExecMonitors().ignore();

  ATH_MSG_DEBUG( "Executing TrigT2HistoPrmVtxAllTE"  );

  float zFirstTrack=0;
  float zFirstTrackError=0;

  m_c->m_totTracks=0;
  m_c->m_totTracks_All=0;
  m_c->m_totSelTracks=0;
  m_c->m_totSelTracks_All=0;

  m_c->m_hisVtx->reset();

  for (int i = 0; i <3; i++) {
    m_c->m_zPrmVtx.at(i) = -200;
    m_c->m_zPrmVtxSigmaAll.at(i)  = -200;
    m_c->m_zPrmVtxSigma2Trk.at(i) = -200;
    m_c->m_nTrackVtx.at(i) = -200;
  }

  // Retrieve beamspot information
  m_c->m_xBeamSpot = m_c->m_yBeamSpot = m_c->m_zBeamSpot = 0;
  m_c->m_xBeamSpotSigma = m_c->m_yBeamSpotSigma = m_c->m_zBeamSpotSigma = 1e-6;
  m_c->m_xBeamSpotTilt = m_c->m_yBeamSpotTilt = 0;
  
  if(m_c->m_useBeamSpot) {
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey };
    
    if (beamSpotHandle.isValid() == false) {

      ATH_MSG_WARNING( "Could not retrieve Beam Conditions Service. "  );
      ATH_MSG_WARNING( "Using origin at ( " << m_c->m_xBeamSpot << " , " << m_c->m_yBeamSpot << " , " << m_c->m_zBeamSpot << " ) "  );

    } else {
      
      const Amg::Vector3D &beamSpot = beamSpotHandle->beamPos();
      
      m_c->m_xBeamSpot = beamSpot.x();
      m_c->m_yBeamSpot = beamSpot.y();
      m_c->m_zBeamSpot = beamSpot.z();
      m_c->m_xBeamSpotSigma = beamSpotHandle->beamSigma(0);
      m_c->m_yBeamSpotSigma = beamSpotHandle->beamSigma(1);
      m_c->m_zBeamSpotSigma = beamSpotHandle->beamSigma(2);
      m_c->m_xBeamSpotTilt = beamSpotHandle->beamTilt(0);
      m_c->m_yBeamSpotTilt = beamSpotHandle->beamTilt(1);

      ATH_MSG_DEBUG( "Beam spot from service: x = " << m_c->m_xBeamSpot << " +/- " << m_c->m_xBeamSpotSigma << "   "
                     << "y = " << m_c->m_yBeamSpot << " +/- " << m_c->m_yBeamSpotSigma << "   "
                     << "z = " << m_c->m_zBeamSpot << " +/- " << m_c->m_zBeamSpotSigma  );
    }
  }
  
  //the outher vector has size 1 in this case
  for (HLT::TEVec::const_iterator inner_it = inputTE[0].begin(); inner_it != inputTE[0].end(); ++inner_it) {
   
    // Get RoI descriptor
    const TrigRoiDescriptor* roiDescriptor; 
    
    if ((getFeature(*inner_it, roiDescriptor) == HLT::OK) && roiDescriptor) {
      ATH_MSG_DEBUG( "Using inputTE: "
                     << "RoI id " << roiDescriptor->roiId()
                     << ", Phi = " <<  roiDescriptor->phi()
                     << ", Eta = " << roiDescriptor->eta()  );
    }
    else {
      ATH_MSG_WARNING( "No RoI for this Trigger Element "  );
      return HLT::NAV_ERROR;
    }
    
    // Create pointers to collections 
    const xAOD::TrackParticleContainer* pointerToEFTrackCollections=0;

    zFirstTrack=0;
    m_c->m_totTracks=0;
    m_c->m_totSelTracks=0;
    
    if (m_c->m_instance == "_EF") { 
      
      ATH_MSG_DEBUG( "Executing TrigT2HistoPrmVtxAllTE at EF"  );

      HLT::ErrorCode status = getFeature(*inner_it, pointerToEFTrackCollections, ""); 
      if (status != HLT::OK) {
	ATH_MSG_DEBUG( "No tracks retrieved from TrigT2HistoPrmVtxAllTE"  );
      } 
      else
	ATH_MSG_DEBUG( "Got collection from TrigT2HistoPrmVtxAllTE"  );
      
      // Get tracks number
      m_c->m_totTracks = getTrackNumbers(pointerToEFTrackCollections);
      m_c->m_totTracks_All += m_c->m_totTracks;
      m_c->m_listCutApplied.clear();

      // Retrieve z0 parameters  
      for (unsigned int i = 0; i < m_c->m_totTracks; i++) {
	
	if (i == 0) 
	  ATH_MSG_DEBUG( "Loop over tracks: retrieving z0"  );
		
	const xAOD::TrackParticle* track = (*pointerToEFTrackCollections)[i];
	float z0 =  track->z0();
	const xAOD::ParametersCovMatrix_t covTrk = track->definingParametersCovMatrix();
	float ez0 = Amg::error(covTrk, 1);

	if (!m_c->efTrackSel(track, i, roiDescriptor->eta(), roiDescriptor->phi())) continue;
	
	m_c->m_totSelTracks++;
	m_c->m_totSelTracks_All++;

	if (m_c->m_totSelTracks == 1) {
	  zFirstTrack=z0;
	  zFirstTrackError=ez0;
	}

        ATH_MSG_DEBUG( "Track number " << i+1 << " with z0 = " << z0 << " and ez0 = " << ez0  );
      
	m_c->m_hisVtx->fill(z0);
	//std::cout << "PRMVTX-USING track " << z0 << std::endl;
      }
    }
  }

  // Get primary vertex from TrigT2HistoPrmVtxAllTE::findPrmVtx
  if (m_c->m_totSelTracks_All == 1) {
    m_c->m_zPrmVtx.at(0)          = zFirstTrack;
    m_c->m_zPrmVtxSigmaAll.at(0)  = zFirstTrackError;
    m_c->m_nTrackVtx.at(0)        = 1;
    m_c->m_nVtxFound = 1;
  } else {
    m_c->findPrmVtx();
  }

  if (msgLvl() <= MSG::DEBUG) {
    for (int i = 0; i <3; i++)
      ATH_MSG_DEBUG( "Primary vertex " << i << ": z=" << m_c->m_zPrmVtx.at(i) << ", sigma=" << m_c->m_zPrmVtxSigmaAll.at(i) 
                     << ", nTracks=" << m_c->m_nTrackVtx.at(i) << ", and ntot track in all ROIs" << m_c->m_totSelTracks_All  );
  }
  
  if (msgLvl() <= MSG::DEBUG) {

    const EventInfo* pEventInfo;
    
    if ( !store() || store()->retrieve(pEventInfo).isFailure() ) {
      ATH_MSG_DEBUG( "Failed to get EventInfo "  );
    } else {
      ATH_MSG_DEBUG( "PrmVtx summary (Run " << pEventInfo->event_ID()->run_number()
                     << "; Event "<< pEventInfo->event_ID()->event_number() << ")"  );
      ATH_MSG_DEBUG( "REGTEST: Number of RoIs " << inputTE[0].size()  );
      ATH_MSG_DEBUG( "REGTEST: Tracks: " << m_c->m_totTracks_All << " reconstructed and " << m_c->m_totSelTracks_All <<" selected"  );
      ATH_MSG_DEBUG( "REGTEST: Primary vertex: " << m_c->m_zPrmVtx.at(0) << " +/ " << m_c->m_zPrmVtxSigmaAll.at(0) << " with " << m_c->m_nTrackVtx.at(0) << " tracks"  );
    }
  }

  // Create the xAOD container and its auxiliary store:
  m_trigVertexColl = new xAOD::VertexContainer();
  xAOD::VertexAuxContainer aux;
  m_trigVertexColl->setStore( &aux );
  xAOD::Vertex* trigVertex = new xAOD::Vertex();
  m_trigVertexColl->push_back(trigVertex); 

  trigVertex->setZ(m_c->m_zPrmVtx.at(0) + m_c->m_zBeamSpot);
//  trigVertex->setX(c->m_xBeamSpot);
//  trigVertex->setY(c->m_yBeamSpot);
  trigVertex->setX(m_c->m_xBeamSpot + tan(m_c->m_xBeamSpotTilt) * trigVertex->z());
  trigVertex->setY(m_c->m_yBeamSpot + tan(m_c->m_yBeamSpotTilt) * trigVertex->z());
  trigVertex->setVertexType(xAOD::VxType::PriVtx);

  AmgSymMatrix(3) cov;
  cov(0,0) = m_c->m_xBeamSpotSigma*m_c->m_xBeamSpotSigma;
  cov(1,1) = m_c->m_yBeamSpotSigma*m_c->m_yBeamSpotSigma;
  cov(2,2) = m_c->m_zPrmVtxSigmaAll.at(0)*m_c->m_zPrmVtxSigmaAll.at(0);
  cov(0,1) = cov(1,0) = 0.0; 
  cov(0,2) = cov(2,0) = 0.0;
  cov(1,2) = cov(2,1) = 0.0;
  trigVertex->setCovariancePosition(cov);

  // Create an output TE seeded by an empty vector
  HLT::TEVec allTEs;
  HLT::TriggerElement* outputTE = config()->getNavigation()->addNode(allTEs, output);
  outputTE->setActiveState(true);

  std::string key;
  
  if (m_c->m_instance == "_EF" || m_c->m_instance == "_HLT" ) key = "EFHistoPrmVtx";

  HLT::ErrorCode stat = attachFeature(outputTE, m_trigVertexColl, key);

  if (stat != HLT::OK) {
    ATH_MSG_WARNING( "Failed to attach xAOD::VertexCollection to navigation"  );
    return stat;
  }
  else {
    ATH_MSG_DEBUG( "Attached xAOD::VertexCollection to navigation"  );
  }
  
  // since this is an AllTEAlgo, we have to call the monitoring ourselves:
  afterExecMonitors().ignore();
  
  return HLT::OK;
}

//** ----------------------------------------------------------------------------------------------------------------- **//


HLT::ErrorCode TrigT2HistoPrmVtxAllTE::hltFinalize() {

  ATH_MSG_INFO( "Finalizing TrigT2HistoPrmVtxAllTE"  );
  return HLT::OK;
}

 


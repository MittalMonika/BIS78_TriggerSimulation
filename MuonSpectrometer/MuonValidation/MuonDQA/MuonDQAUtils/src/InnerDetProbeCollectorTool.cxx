/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// InnerDetProbeCollectorTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "MuonDQAUtils/InnerDetProbeCollectorTool.h"

//================ Constructor =================================================
namespace Muon {
  InnerDetProbeCollectorTool::InnerDetProbeCollectorTool(const std::string& t,
							 const std::string& n,
							 const IInterface*  p )
    :
    AthAlgTool(t,n,p),
    m_log(msgSvc(),n),
    m_InsituPerformanceTools("InsituTrackPerformanceTools/TestTool")
  {
    declareInterface<IProbeCollectorTool>(this);

    declareProperty("InnerTrackContainerName",		m_InnerTrackContainerName = "TrackParticleCandidate");
    declareProperty("MSTrackContainerName",			m_MSTrackContainerName = "MuonboyTrackParticles" );
    declareProperty("CombinedMuonTracksContainerName",	m_CombinedMuonTracksContainerName = "StacoMuonCollection");
    declareProperty("RequireTrigger",			m_RequireTrigger = false);
    declareProperty("InsituPerformanceTools",		m_InsituPerformanceTools);
  }

  //================ Destructor =================================================

  InnerDetProbeCollectorTool::~InnerDetProbeCollectorTool()
  {}


  //================ Initialisation =================================================

  StatusCode InnerDetProbeCollectorTool::initialize()
  {
    StatusCode sc = AlgTool::initialize();
    m_log.setLevel(msgLevel());
    if (sc.isFailure()) return sc;
	
    /// histogram location
    sc = service("THistSvc", m_thistSvc);
    if(sc.isFailure() ){
      m_log   << MSG::ERROR
	      << "Unable to retrieve pointer to THistSvc"
	      << endmsg;
      return sc;
    }
	
    /// get StoreGate service
    sc = service("StoreGateSvc",m_storeGate);
    if (sc.isFailure())
      {
	m_log << MSG::FATAL << "StoreGate service not found !" << endmsg;
	return StatusCode::FAILURE;
      }

    /// Getting InsituPerformanceTools
    if (m_InsituPerformanceTools.retrieve().isFailure())
      {
	m_log << MSG::FATAL << "My Tool Service not found" << endmsg;
	return StatusCode::FAILURE;
      }
    sc = m_InsituPerformanceTools->initialize();
    if (sc.isFailure())
      {
	m_log << MSG::FATAL << "InsituPerformanceTool service not found !" << endmsg;
	return StatusCode::FAILURE;
      }
    m_log << MSG::INFO << "initialize() successful in " << name() << endmsg;
    return StatusCode::SUCCESS;
  }

  //================ Finalisation =================================================

  StatusCode InnerDetProbeCollectorTool::finalize()
  {
    StatusCode sc = AlgTool::finalize();
    return sc;
  }

  //============================================================================================

  StatusCode InnerDetProbeCollectorTool::createProbeCollection()
  {
    m_log << MSG::DEBUG << "createProbeCollection() for Inner Detector" << endmsg;
	
    StatusCode sc = StatusCode::SUCCESS;
    /// Record the container of Probe Muons in StoreGate
    m_IDProbeTrackContainer = new Rec::TrackParticleContainer();
    sc = m_storeGate->record(m_IDProbeTrackContainer,"InnerDetectorProbeTracks");
    if (sc.isFailure())  
      {
	m_log << MSG::ERROR << "Unable to record InnerDetectorProbeTracks in StoreGate" 
	      << endmsg;
	return sc;
      } else
	m_log << MSG::DEBUG << "InnerDetectorProbeTracks Container recorded in StoreGate." << endmsg;

	
    /// Retrieve Muon Tracks
    const Rec::TrackParticleContainer* trackTES=0;
    sc=m_storeGate->retrieve( trackTES, m_MSTrackContainerName);
    if( sc.isFailure()  ||  !trackTES ) 
      {
	m_log << MSG::WARNING	<< "No " << m_MSTrackContainerName << " container found in TDS"	<< endmsg; 
	return StatusCode::FAILURE;
      }  
    m_log << MSG::DEBUG << "MuonTrack Container successfully retrieved" << endmsg;

    /// Retrieve Combined Tracks
    const Analysis::MuonContainer* muonTDS=0;
    sc=m_storeGate->retrieve( muonTDS, m_CombinedMuonTracksContainerName);
    if( sc.isFailure()  ||  !muonTDS ) 
      {
	m_log << MSG::WARNING	<< "No AOD "<<m_CombinedMuonTracksContainerName<<" container of muons found in TDS"<< endmsg; 
	return StatusCode::FAILURE;
      }  
    m_log << MSG::DEBUG << "MuonContainer successfully retrieved" << endmsg;
	
    /// Loop over Combined Muon Tracks
    Analysis::MuonContainer::const_iterator muonItr  = muonTDS->begin();
    Analysis::MuonContainer::const_iterator muonItrE = muonTDS->end();
    for (; muonItr != muonItrE; ++muonItr)
      {
	if (((*muonItr)->pt()>20000.) && ((*muonItr)->isCombinedMuon()))
	  {
	    const Rec::TrackParticle *muonTrack	= (*muonItr)->track();
	    if ((m_RequireTrigger==false) || (m_InsituPerformanceTools->isTriggeredMuon(const_cast<Rec::TrackParticle*>(muonTrack))==true))
	      {
		Rec::TrackParticleContainer::const_iterator MSTrackItr  = trackTES->begin();
		Rec::TrackParticleContainer::const_iterator MSTrackItrE = trackTES->end();
		for (; MSTrackItr != MSTrackItrE; ++MSTrackItr) 
		  {
		    const Rec::TrackParticle *MSTrack	= (*MSTrackItr);
		    if (fabs(MSTrack->pt())>20000.)
		      {
						
			if (m_InsituPerformanceTools->isZBosonCandidate(const_cast<Rec::TrackParticle*>(MSTrack), const_cast<Rec::TrackParticle*>(muonTrack))==true)
			  {
			    m_IDProbeTrackContainer->push_back( new Rec::TrackParticle(**MSTrackItr) );
			  }
		      }
		  }
	      }
	  }
      }
    sc = m_storeGate->setConst(m_IDProbeTrackContainer);
    if( sc.isFailure()) 
      {
	m_log << MSG::WARNING	<< "set const failed"<< endmsg; 
	return StatusCode::FAILURE;
      }  
    return StatusCode::SUCCESS;
  }
}//namespace

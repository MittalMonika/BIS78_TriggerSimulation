/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

//#include <algorithm>
#include "AthenaKernel/Timeout.h"

#include "InDetTrigAmbiguitySolver/InDetTrigAmbiguitySolver.h"
#include "TrkToolInterfaces/ITrackAmbiguityProcessorTool.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkEventUtils/PRDtoTrackMap.h"
#include "TrkToolInterfaces/ITrackAmbiguityProcessorTool.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkParameters/TrackParameters.h"
//#include "TrkTrackSummary/TrackSummary.h"
#include "TrigNavigation/NavigationCore.icc"

using namespace InDet;

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////
InDetTrigAmbiguitySolver::InDetTrigAmbiguitySolver(const std::string& name, ISvcLocator* pSvcLocator)
  : HLT::FexAlgo(name, pSvcLocator),
    m_resolveTracks(true),
    m_oldTracks(0),
    m_tracks(0),
    m_trackInCount(0),
    m_trackOutCount(0),
    m_TotalTrackInCount(0),
    m_TotalTrackOutCount(0),
    m_ambiTool("Trk::SimpleAmbiguityProcessorTool/InDetTrigAmbiguityProcessor"),
    m_inputTracksLabel("SPSeeded"),
    m_outputTracksLabel("AmbigSolv"),
    m_outputPRDMapLabel(""),
    m_ntimesInvoked(0),
    m_doTimeOutChecks(true)
{
  declareProperty("ResolveTracks", m_resolveTracks);
  declareProperty("AmbiguityProcessor", m_ambiTool);
  declareProperty("InputTracksLabel", m_inputTracksLabel);
  declareProperty("OutputTracksLabel", m_outputTracksLabel);
  declareProperty("OutputPRDMapLabel", m_outputPRDMapLabel);
  declareProperty("doTimeOutChecks",      m_doTimeOutChecks);

  //monitoring
  declareMonitoredVariable("numInTracks",m_trackInCount);
  declareMonitoredVariable("numOutTracks",m_trackOutCount);
  declareMonitoredStdContainer("chi2ndof", m_chi2ndof);
  declareMonitoredStdContainer("trk_d0",     m_d0);
  declareMonitoredStdContainer("trk_z0",     m_z0);
  declareMonitoredStdContainer("trk_phi",    m_phi);
  declareMonitoredStdContainer("trk_eta",    m_eta);
  declareMonitoredStdContainer("trk_qoverp", m_qoverp);

}

//--------------------------------------------------------------------------
InDetTrigAmbiguitySolver::~InDetTrigAmbiguitySolver(void)
{}

///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////
HLT::ErrorCode InDetTrigAmbiguitySolver::hltInitialize() {

  msg() << MSG::INFO << "InDetTrigAmbiguitySolver::initialize(). "<< endmsg;


  if (m_resolveTracks){

    StatusCode sc =  m_ambiTool.retrieve();
    if (sc.isFailure()){

      msg() << MSG::FATAL << "Failed to retrieve tool " << m_ambiTool << endmsg;
      return HLT::BAD_ALGO_CONFIG;
    } 
    else
      msg() << MSG::INFO << "Retrieved tool " << m_ambiTool << endmsg;
  } else {
    m_ambiTool.disable();
  }

  if (m_assoTool.retrieve( DisableTool{m_assoTool.name().empty()} ).isFailure()) {
     msg() << MSG::FATAL << "Failed to retrieve tool " << m_assoTool << endmsg;
     return HLT::BAD_ALGO_CONFIG;
  }

  if (m_prdToTrackMapExchange.retrieve( DisableTool{m_prdToTrackMapExchange.name().empty()} ).isFailure()) {
     msg() << MSG::FATAL << "Failed to retrieve tool " << m_prdToTrackMapExchange << endmsg;
     return HLT::BAD_ALGO_CONFIG;
  }
  return HLT::OK;
}

///////////////////////////////////////////////////////////////////
// Execute HLT Algorithm
///////////////////////////////////////////////////////////////////
HLT::ErrorCode InDetTrigAmbiguitySolver::hltExecute(const HLT::TriggerElement*, HLT::TriggerElement* outputTE)
{ 

  ++m_ntimesInvoked;
  m_trackInCount = m_trackOutCount = 0;
  m_chi2ndof.clear();
  m_d0.clear();
  m_z0.clear();
  m_phi.clear();
  m_eta.clear();
  m_qoverp.clear();

  int outputLevel = msgLvl();

  if(outputLevel <= MSG::DEBUG)
    msg() << MSG::DEBUG << "InDetTrigAmbiguitySolver::execHLTAlgorithm()" 
	<< endmsg;

  if (m_doTimeOutChecks && Athena::Timeout::instance().reached() ) {
    msg() << MSG::WARNING << "Timeout reached. Aborting sequence." << endmsg;
    return HLT::ErrorCode(HLT::Action::ABORT_CHAIN, HLT::Reason::TIMEOUT);
  }

  //----------------------------------------------------------------------
  // Trigger specific part: navigate throw the trigger element to get the
  // track collection associated to it.
  //----------------------------------------------------------------------
  m_oldTracks  = 0;
  if ( HLT::OK != getFeature(outputTE, m_oldTracks, m_inputTracksLabel) ) {
    msg() << MSG::ERROR << " Input track collection could not be found " << endmsg;
    
    return HLT::NAV_ERROR;
  }

  if(!m_oldTracks){
    if(outputLevel <= MSG::DEBUG)
      msg() << MSG::DEBUG << " Input track collection was not attached. Algorithm not executed!" << endmsg;
    
    return HLT::OK; 
  } else {
    if(outputLevel <= MSG::VERBOSE)
      msg() << MSG::VERBOSE << " Input track collection has size " << m_oldTracks->size() << endmsg;
    if ( m_oldTracks->size() == 0 ) {
      if(outputLevel <= MSG::DEBUG)
	msg() << MSG::DEBUG << " Input track collection has 0 size. Algorithm not executed!" << endmsg;
      return HLT::OK; 
    }

  }
  
  //
  //  Resolve ambiguities
  //
  if (m_resolveTracks){
    // okay, and let's call the ambiguity processor, just for a laugh.
    if(outputLevel <= MSG::DEBUG) 
      msg() << MSG::DEBUG << "REGTEST: InDetAmbiguitySolver::resolveTracks() resolving " << m_oldTracks->size()<<"  tracks"<<endmsg;
    std::unique_ptr<Trk::PRDtoTrackMap> prd_to_track_map;
    if (m_assoTool.isEnabled()) {
       // create internal PRD-to-track map
       prd_to_track_map = std::move( m_assoTool->createPRDtoTrackMap() );
    }
    m_trackInCount = m_oldTracks->size();
    m_tracks = m_ambiTool->process( m_oldTracks, prd_to_track_map.get() );
    m_trackOutCount = m_tracks->size();
    m_TotalTrackInCount += m_trackInCount;
    m_TotalTrackOutCount += m_trackOutCount;

    if (m_assoTool.isEnabled()) {
       std::unique_ptr<Trk::PRDtoTrackMap> tmp( m_assoTool->reduceToStorableMap(std::move(prd_to_track_map)));
       if (m_prdToTrackMapExchange.isEnabled()) {
          m_prdToTrackMapExchange->setPRDtoTrackMap(tmp.release());
       }
       else {
          if ( HLT::OK !=  attachFeature(outputTE, tmp.release(), m_outputPRDMapLabel) ) {
             msg() << MSG::ERROR << "Could not attache PRDtoTrackMap feature to the TE" << endmsg;
             return HLT::NAV_ERROR;
          }
       }
    }
  }
  else{
    // copy tracks. NOT a shallow copy as it was before, since
    // this causes mem leaks when reading back in again
    // see bug#9886 for details
    m_tracks = new TrackCollection;
    TrackCollection::const_iterator it = m_oldTracks->begin();
    TrackCollection::const_iterator itEnd = m_oldTracks->end();
    for (;it!=itEnd;it++) m_tracks->push_back( new Trk::Track(**it) );
  }

  //monitor chi2 of the Si tracks 
  TrackCollection::const_iterator it = m_tracks->begin();
  TrackCollection::const_iterator itEnd = m_tracks->end();
  for (;it!=itEnd;it++){
    const Trk::FitQuality* fq = (*it)->fitQuality();
    if (fq && fq->numberDoF()) {
      m_chi2ndof.push_back(fq->chiSquared() / fq->numberDoF());
    }
    const Trk::Perigee *mp = (*it)->perigeeParameters();
    //    const Trk::TrackSummary *ts = (*it)->trackSummary();
    if (mp){
      m_d0.push_back(mp->parameters()[Trk::d0]);
      m_z0.push_back(mp->parameters()[Trk::z0]);
      m_eta.push_back(mp->eta());
      m_phi.push_back(mp->parameters()[Trk::phi]);
      m_qoverp.push_back(mp->parameters()[Trk::qOverP]);
    }
  }

  //
  //  Attach resolved tracks to the trigger element. 
  if ( HLT::OK !=  attachFeature(outputTE, m_tracks, m_outputTracksLabel) ) {
    msg() << MSG::ERROR << "Could not attache feature to the TE" << endmsg;
    
    return HLT::NAV_ERROR;
  }

  // record collection in StoreGate: get a unique key for collection;

  if(outputLevel <= MSG::DEBUG){ 
    msg() << MSG::DEBUG << "REGTEST: Stored " << m_tracks->size() 
	<< " tracks in SG." << endmsg;
    msg() << MSG::DEBUG << "Track collection " << m_outputTracksLabel << endmsg; 
  }

  if (msgLvl() <= MSG::VERBOSE){
      for (size_t i=0; i<m_tracks->size() ; i++){
      msg() << MSG::VERBOSE << *(m_tracks->at(i)) << endmsg;
    }
  }

  return HLT::OK;
}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////
HLT::ErrorCode InDetTrigAmbiguitySolver::hltFinalize() {

  msg() << MSG::INFO << "REGTEST: Finalizing with "<< m_TotalTrackInCount
	<< " tracks processed and " <<  m_TotalTrackOutCount << " saved. Invoked " << m_ntimesInvoked << " times." << endmsg;

  return HLT::OK;
}
//--------------------------------------------------------------------

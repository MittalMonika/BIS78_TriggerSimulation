/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                         InDetPriVxFinder.cxx  -  Description
                             -------------------
    begin   : 28-01-2004
    authors : Andreas Wildauer (CERN PH-ATC), Fredrik Akesson (CERN PH-ATC)
    email   : andreas.wildauer@cern.ch, fredrik.akesson@cern.ch
    changes :
 ***************************************************************************/
#include "InDetPriVxFinder/InDetPriVxFinder.h"
// forward declares
#include "InDetRecToolInterfaces/IVertexFinder.h"
#include "TrkVertexFitterInterfaces/IVertexCollectionSortingTool.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

// normal includes
#include "TrkTrack/TrackCollection.h"
#include "VxVertex/VxContainer.h"
#include "TrkParticleBase/TrackParticleBaseCollection.h"
#include "TrkVxEdmCnv/IVxCandidateXAODVertex.h"

namespace InDet
{

  InDetPriVxFinder::InDetPriVxFinder ( const std::string &n, ISvcLocator *pSvcLoc )
      : AthAlgorithm ( n, pSvcLoc ),
      m_tracksName ( "Tracks" ),
      m_vxCandidatesOutputName ( "PrimaryVertices" ),
      m_vxCandidatesOutputNameAuxPostfix ( "Aux." ),
      m_VertexFinderTool ( "InDet::InDetPriVxFinderTool" ),
      m_VertexCollectionSortingTool ("Trk::VertexCollectionSortingTool"),
      m_VertexEdmFactory("Trk::VertexInternalEdmFactory"),
      m_doVertexSorting(false),
      // for summary output at the end
      m_numEventsProcessed(0),
      m_totalNumVerticesWithoutDummy(0)

  {
    declareProperty ( "TracksName",m_tracksName );
    declareProperty ( "VxCandidatesOutputName",m_vxCandidatesOutputName );
    declareProperty ( "VxCandidatesOutputNameAuxPostfix",m_vxCandidatesOutputNameAuxPostfix );
    declareProperty ( "VertexFinderTool",m_VertexFinderTool );
    declareProperty ( "VertexCollectionSortingTool",m_VertexCollectionSortingTool );
    declareProperty ( "InternalEdmFactory", m_VertexEdmFactory);
    declareProperty ( "doVertexSorting",m_doVertexSorting );
  }

  InDetPriVxFinder::~InDetPriVxFinder()
  {}

  StatusCode InDetPriVxFinder::initialize()
  {
    /* Get the VertexFinderTool */
    if ( m_VertexFinderTool.retrieve().isFailure() )
    {
      msg(MSG::FATAL) << "Failed to retrieve tool " << m_VertexFinderTool << endreq;
      return StatusCode::FAILURE;
    }
    else
    {
      msg(MSG::INFO) << "Retrieved tool " << m_VertexFinderTool << endreq;
    }

    /*Get the Vertex Collection Sorting Tool*/
    if (m_doVertexSorting)
    {
      if ( m_VertexCollectionSortingTool.retrieve().isFailure() )
      {
        msg(MSG::FATAL) << "Failed to retrieve tool " << m_VertexCollectionSortingTool << endreq;
        return StatusCode::FAILURE;
      }
      else
      {
        msg(MSG::INFO) << "Retrieved tool " << m_VertexCollectionSortingTool << endreq;
      }
    }
    if ( m_VertexEdmFactory.retrieve().isFailure() ) {
      ATH_MSG_ERROR("Failed to retrievel tool " << m_VertexEdmFactory);
      return StatusCode::FAILURE;
    }
   

    msg(MSG::INFO) << "Initialization successful" << endreq;
    return StatusCode::SUCCESS;
  }


  StatusCode InDetPriVxFinder::execute()
  {
    m_numEventsProcessed++;
    VxContainer* theVxContainer ( 0 );
    std::pair<xAOD::VertexContainer *, xAOD::VertexAuxContainer*> theXAODContainer;

    //---- First try if m_tracksName is a TrackCollection -----------------//
    if ( evtStore()->contains<TrackCollection> ( m_tracksName ) )
    {
      const TrackCollection *trackTES ( 0 );
      if ( evtStore()->retrieve ( trackTES, m_tracksName ).isFailure() )
      {
        if (msgLvl(MSG::DEBUG)) msg() << "Could not find TrackCollection " << m_tracksName << " in StoreGate." << endreq;
        return StatusCode::SUCCESS;
      }
      theVxContainer = m_VertexFinderTool->findVertex ( trackTES );
    }
    //---- Second try if m_tracksName is a xAOD::TrackParticleContainer ----//
    else if ( evtStore()->contains<xAOD::TrackParticleContainer> ( m_tracksName ) )
    {
      const xAOD::TrackParticleContainer *trackParticleCollection(0);
      if ( evtStore()->retrieve ( trackParticleCollection, m_tracksName ).isFailure() )
      {
        if (msgLvl(MSG::DEBUG)) msg() << "Could not find xAOD::TrackParticleContainer " << m_tracksName << " in StoreGate." << endreq;
        return StatusCode::SUCCESS;
      }
      theXAODContainer = m_VertexFinderTool->findVertex ( trackParticleCollection );
    }
    //---- Third try if m_tracksName is a TrackParticleBaseCollection ----//
    else if ( evtStore()->contains<Trk::TrackParticleBaseCollection> ( m_tracksName ) )
    {
      const Trk::TrackParticleBaseCollection *trackParticleBaseCollection(0);
      if ( evtStore()->retrieve ( trackParticleBaseCollection, m_tracksName ).isFailure() )
      {
        if (msgLvl(MSG::DEBUG)) msg() << "Could not find Trk::TrackParticleBaseCollection " << m_tracksName << " in StoreGate." << endreq;
        return StatusCode::SUCCESS;
      }

      theVxContainer = m_VertexFinderTool->findVertex ( trackParticleBaseCollection );
    }
    else
    {
      ATH_MSG_WARNING("Neither a TrackCollection nor a xAOD::TrackParticleContainer nor a TrackParticleBaseCollection with key " << m_tracksName << " exists in StoreGate. Mp vertexing possible.");
      return StatusCode::SUCCESS;
    }

    // now resorting the vertex container and store to SG
    std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> myVxContainer;
    VxContainer* MyTrackVxContainer = 0;
    if (theXAODContainer.first) {
      //sort xAOD::Vertex container
      if (m_doVertexSorting && theXAODContainer.first->size() > 1) {	
	myVxContainer = m_VertexCollectionSortingTool->sortVertexContainer(*theXAODContainer.first);
	delete theXAODContainer.first; //also cleans up the aux store
        delete theXAODContainer.second; 
      } else {
	myVxContainer.first = theXAODContainer.first;
	myVxContainer.second = theXAODContainer.second;
      }
      if (myVxContainer.first!=0) m_totalNumVerticesWithoutDummy += (myVxContainer.first->size()-1);      

      //---- Recording section: write the results to StoreGate ---//
      if ( evtStore()->record ( myVxContainer.first, m_vxCandidatesOutputName,false ).isFailure() ) {
	ATH_MSG_ERROR("Unable to record xAOD::VertexContainer in StoreGate");
	return StatusCode::FAILURE;
      }      
      if (not myVxContainer.first->hasStore()) {
	ATH_MSG_WARNING("Vertex container has no associated store.");
	return StatusCode::SUCCESS;
      }
      std::string vxContainerAuxName = m_vxCandidatesOutputName + m_vxCandidatesOutputNameAuxPostfix;
      if ( evtStore()->record ( myVxContainer.second, vxContainerAuxName ).isFailure() ) {
	ATH_MSG_ERROR("Unable to record xAOD::VertexAuxContainer in StoreGate");
	return StatusCode::FAILURE;
      }
    } else if (theVxContainer) {
      //sort Trk::Track container (or Rec::TrackParticle)
      if (m_doVertexSorting && theVxContainer->size() > 1) {
	MyTrackVxContainer = m_VertexCollectionSortingTool->sortVxContainer(*theVxContainer);
	delete theVxContainer;
      } else {
	MyTrackVxContainer = theVxContainer;
      }
      if (MyTrackVxContainer!=0) m_totalNumVerticesWithoutDummy += (MyTrackVxContainer->size()-1);

      //---- Recording section: write the results to StoreGate ---//
      if ( evtStore()->record ( MyTrackVxContainer, m_vxCandidatesOutputName,false ).isFailure() ) {
	ATH_MSG_ERROR("Unable to record Vx Container in TDS");
	return StatusCode::FAILURE;
      }
    } else { //nor theVxContainer or theXAODContainer are valid
      ATH_MSG_ERROR("Unexpected error. Invalid output containers.");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Successfully reconstructed " << myVxContainer.first->size()-1 << " vertices (excluding dummy)");
       
    return StatusCode::SUCCESS;
  } 
  
  StatusCode InDetPriVxFinder::finalize()
  {
    if (msgLvl(MSG::INFO))
    {
      msg() << "Summary from Primary Vertex Finder (InnerDetector/InDetRecAlgs/InDetPriVxFinder)" << endreq;
      msg() << "=== " << m_totalNumVerticesWithoutDummy << " vertices recoed in " << m_numEventsProcessed << " events (excluding dummy)." << endreq;
      if (m_numEventsProcessed!=0) msg() << "=== " << double(m_totalNumVerticesWithoutDummy)/double(m_numEventsProcessed) << " vertices per event (excluding dummy)." << endreq;
    } 
    return StatusCode::SUCCESS;
  }
  
} // end namespace InDet

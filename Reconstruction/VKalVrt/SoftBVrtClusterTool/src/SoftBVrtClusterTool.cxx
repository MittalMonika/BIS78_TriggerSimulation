/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////
// SoftBVrtClusterToolAlg.cxx
///////////////////////////////////////

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "SoftBVrtClusterTool/SoftBVrtClusterTool.h"
#include "SoftBVrtClusterTool/TrackCluster.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "TrackVertexAssociationTool/ITrackVertexAssociationTool.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace SoftBVrt {

  SoftBVrtClusterTool::SoftBVrtClusterTool( const std::string& name, ISvcLocator *pSvcLocator ) :
  
    AthAlgorithm(name, pSvcLocator),
    m_secVertexFinderTool(""),
    m_trkDistanceFinderTool(""),
    m_selTool( "InDet::InDetTrackSelectionTool/TrackSelectionTool", this )
  {
  
    declareProperty( "JetCollectionName", m_jetCollectionName = "AntiKt4EMTopoJets" );
    declareProperty( "TrackJetCollectionName", m_trackjetCollectionName = "AntiKtVR30Rmax4Rmin02TrackJets" );
  
    //track quality tool
    declareProperty( "TrackSelectionTool", m_selTool );
    
    //select operating point
    declareProperty( "OperatingPoint", m_operatingPoint = "medium" );

    //track clustering cuts (by default initialised to no cut,
    //overriden by default working point medium)
    declareProperty( "SeedPt", m_seed_pt = 0.0 );
    declareProperty( "SeedZ0SinTheta", m_seed_z0_sintheta = 1000.0 );
    declareProperty( "SeedZ0", m_seed_z0 = 1000.0 );
    declareProperty( "SeedZ0Significance", m_seed_z0_significance = 0.0 );
    declareProperty( "SeedD0Significance", m_seed_d0_significance = 0.0 );
    declareProperty( "ClusterDistance", m_cluster_distance = 1000. );
    declareProperty( "ClusterZ0", m_cluster_z0 = 1000.0 );
    declareProperty( "ClusterZ0Significance", m_cluster_z0_significance = 0.0 );
    declareProperty( "ClusterD0Significance", m_cluster_d0_significance = 0.0 );
    declareProperty( "ClusterDR", m_cluster_dr = 100.0 );
    declareProperty( "TrackjetPtThreshold", m_trackjet_pt_threshold = 2000000000.0 );
    declareProperty( "DoJetVeto", m_jetveto = true );
    declareProperty( "DoTrackJetVeto", m_trackjetveto = false );
    declareProperty( "OverlapFraction", m_overlap_frac = 0.7 );
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  SoftBVrtClusterTool::~SoftBVrtClusterTool() {
  
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  StatusCode SoftBVrtClusterTool::initialize() {
    ATH_MSG_INFO ("Initializing " << name() << "..."); 

    ATH_CHECK( initializeTools() );
  
    ATH_MSG_INFO ("Operating point initialised to " << m_operatingPoint << ", all track cuts will be overridden..." );

    if (m_operatingPoint == "loose") {
      m_seed_d0_significance = 0.5;
      m_cluster_d0_significance = 0.5;
      m_cluster_distance = 0.5;
      m_cluster_dr = 0.75;
      m_seed_pt = 2000.;
    }

    if (m_operatingPoint == "medium") {
      m_seed_d0_significance = 0.9;
      m_cluster_d0_significance = 0.9;
      m_cluster_distance = 0.7;
      m_cluster_dr = 0.6;
      m_seed_pt = 2000.;
    }

    if (m_operatingPoint == "tight") {
      m_seed_d0_significance = 2.5;
      m_cluster_d0_significance = 1.5;
      m_cluster_distance = 0.25;
      m_cluster_dr = 0.5;
      m_seed_pt = 1500.;
    }
        
    ATH_MSG_INFO ("SeedPt initialised to " << m_seed_pt );
    ATH_MSG_INFO ("SeedD0Significance initialised to " << m_seed_d0_significance );
    ATH_MSG_INFO ("ClusterDistance initialised to " << m_cluster_distance );
    ATH_MSG_INFO ("ClusterD0Significance initialised to " << m_cluster_d0_significance );
    ATH_MSG_INFO ("ClusterDR initialised to " << m_cluster_dr ); 

    return StatusCode::SUCCESS;
  }


  StatusCode SoftBVrtClusterTool::initializeTools(){

    // used for reconstructing vertices in track clusters
    m_secVertexFinderTool.setTypeAndName("InDet::InDetVKalVxInJetTool/BJetSVFinder");    

    ATH_CHECK( m_secVertexFinderTool.retrieve() );

    // used for determining 3D track-track distance
    m_trkDistanceFinderTool.setTypeAndName("Trk::SeedNewtonTrkDistanceFinder/TrkDistanceFinder"); 

    ATH_CHECK( m_trkDistanceFinderTool.retrieve() );

    return StatusCode::SUCCESS;
  }

  ///////////////////////////////////////////////////////////////////////////////////
  StatusCode SoftBVrtClusterTool::finalize() {
    ATH_MSG_INFO ("Finalizing " << name() << "...");
      
    return StatusCode::SUCCESS;
  }

  ///////////////////////////////////////////////////////////////////////////////////

  namespace {
    typedef std::vector<const xAOD::TrackParticle*> Tracks;
    typedef std::vector<const xAOD::IParticle*> Particles;
  }

  StatusCode SoftBVrtClusterTool::execute() {

    //-------------------------
    // Event information
    //---------------------------
    const xAOD::EventInfo* eventInfo = nullptr;
  
    ATH_MSG_DEBUG(" Retrieving Event Info " );
  
    ATH_CHECK( evtStore()->retrieve(eventInfo) ); 

    auto *RecoVertices = new xAOD::VertexContainer;
    auto *RecoVerticesAux = new xAOD::VertexAuxContainer;
  
    RecoVertices->setStore( RecoVerticesAux);
  
    ATH_CHECK( evtStore()->record( RecoVertices, "SoftBVrtClusterTool_Vertices") );
    ATH_CHECK( evtStore()->record( RecoVerticesAux, "SoftBVrtClusterTool_VerticesAux.") );  
  
    typedef ElementLink<xAOD::TrackParticleContainer> TrackLink;
    typedef std::vector<TrackLink> TrackLinks;
  
    ATH_MSG_DEBUG ("Executing " << name() << "...");

    // to reject tracks associated to calo jets (configurable)
    const xAOD::JetContainer *jets = nullptr;
    ATH_CHECK( evtStore()->retrieve(jets, m_jetCollectionName) );

    // to reject tracks associated to track jets (configurable)
    const xAOD::JetContainer *trackjets = nullptr;
    ATH_CHECK( evtStore()->retrieve(trackjets, m_trackjetCollectionName) );
  
    // primary vertex
    const xAOD::VertexContainer *Primary_vertices = nullptr;
  
    ATH_MSG_DEBUG( " retrieve  PrimaryVertices " );
    ATH_CHECK( evtStore()->retrieve(Primary_vertices, "PrimaryVertices") );
  
    int* npv_p = 0;
  
    // m_VX_COUNT_KEY is the key we use to keep track of how many primary vertices
    // we found if it's missing we didn't run the BTagVertexAugmenter.

    StatusCode vx_count_status = evtStore()->retrieve(npv_p, m_VX_COUNT_KEY);
  
    if (vx_count_status.isFailure()) {
      ATH_MSG_FATAL("could not find " + m_VX_COUNT_KEY + " in file");
      return StatusCode::FAILURE;
    }
  
    int npv = *npv_p;
  
    if (npv < 1) {
      ATH_MSG_WARNING( ".... rejecting the event due to missing PV!!!!");
      return StatusCode::SUCCESS;
    }
  
    ATH_MSG_DEBUG( " get the vertex index (stored in BTaggingVertexAugmenter) " );
    int* indexPV_ptr = 0;
    ATH_CHECK(evtStore()->retrieve(indexPV_ptr, "BTaggingVertexIndex"));
  
    size_t indexPV = *indexPV_ptr;
  
    const xAOD::Vertex *myVertex = Primary_vertices->at(indexPV);
  
    if (myVertex ){
      m_PV_x = myVertex->x();
      m_PV_y = myVertex->y();
      m_PV_z = myVertex->z();  
    
    } else {
      ATH_MSG_DEBUG( " missing primary vertex! " );
      m_PV_x = -999;
      m_PV_y = -999;
      m_PV_z = -999;
    }

    // retrieve tracks from AOD for clustering
    const xAOD::TrackParticleContainer* tracks(nullptr);
  
    ATH_MSG_DEBUG( " retrieve TrackParticles " );
    ATH_CHECK( evtStore()->retrieve( tracks, "InDetTrackParticles") );
  
    // to hold the track clusters
    std::vector<SoftBVrt::TrackCluster> clusterVec;
  
    std::vector<const xAOD::TrackParticle*>* seedtracks = new std::vector<const xAOD::TrackParticle*>;
    std::vector<const xAOD::TrackParticle*>* clustertracks = new std::vector<const xAOD::TrackParticle*>;
  
    // build the vectors of seed / cluster track candidates
    for (const xAOD::TrackParticle *track : *tracks)
      {     

	// recommended track quality cuts (configure to LoosePrimary in job options)
	if (!m_selTool->accept( *track, myVertex) ) continue;       

	double trackz0 = track->z0() + track->vz() - m_PV_z;
	double trackd0sigma = xAOD::TrackingHelpers::d0significance( track , eventInfo->beamPosSigmaX(), eventInfo->beamPosSigmaY(), eventInfo->beamPosSigmaXY() );

	// reject tracks from pileup, any that are used in
	// the fit for primary vertex that is not the 
	// hard-scatter are labelled as pileup
	int ispu = 0;
            
	for( auto pv : *Primary_vertices ) {
	
	  for (unsigned int i = 0; i < pv->nTrackParticles(); i++) {
	  
	    const xAOD::TrackParticle *link = pv->trackParticle(i);

	    if (link == track) {
	      if (myVertex == pv) 
		ispu = 2;
	      else
		ispu = 1;
	    }

	  }
	}
      
	// seed track selection
	if (track->pt() > m_seed_pt && abs(trackz0*TMath::Sin(track->theta())) < m_seed_z0_sintheta && abs(trackz0) < m_seed_z0 && abs(trackz0/sqrt(track->definingParametersCovMatrix()(1,1))) > abs(m_seed_z0_significance) && abs(trackd0sigma) > m_seed_d0_significance && ispu != 1) {

	  int fromjet = 0;
	
	  float seedIso = 10;

	  // find tracks associated to
	  // calo / track jets
	  for (const auto* jet : *jets) {	  
	  
	    //Get ghost associated tracks
	    std::vector<const xAOD::TrackParticle*> jet_tracks;
	  
	    jet->getAssociatedObjects("GhostTrack", jet_tracks);  
	  
	    float dr = track->p4().DeltaR(jet->p4());
	    if (dr < seedIso) seedIso = dr;
	  
	    for (const auto* tjet : *trackjets) {	      	            
	    
	      if (tjet->pt() < m_trackjet_pt_threshold)
		continue;
	    
	      float dr = track->p4().DeltaR(tjet->p4());
	      if (dr < seedIso) seedIso = dr;
	    
	    }
	  
	    for(unsigned int i=0; i<jet_tracks.size(); i++) {
	    
	      if (jet_tracks[i] == track) {
		fromjet = 1;
		break;
	      }    
	    }	      
	  }       
	
	  if (fromjet && m_jetveto)
	    continue;
	
	  if (seedIso < 0.3 && m_trackjetveto)
	    continue;	       

	  seedtracks->push_back(track);

	}
      
	double track2z0 = track->z0() + track->vz() - m_PV_z;
	double track2d0sigma = xAOD::TrackingHelpers::d0significance( track , eventInfo->beamPosSigmaX(), eventInfo->beamPosSigmaY(), eventInfo->beamPosSigmaXY() );     

	// cluster track selection
	if (abs(track2z0) < m_cluster_z0 && abs(track2z0/sqrt(track->definingParametersCovMatrix()(1,1))) > m_cluster_z0_significance && abs(track2d0sigma) > m_cluster_d0_significance && ispu != 1) {
	
	  int fromjet = 0;
	
	  float clusterIso = 10;
	
	  // find tracks associated to
	  // calo / track jets
	  for (const auto* jet : *jets) {
	  
	    float dr = track->p4().DeltaR(jet->p4());
	    if (dr < clusterIso) clusterIso = dr;
	  
	    for (const auto* tjet : *trackjets) {
	    
	      if (tjet->pt() < m_trackjet_pt_threshold)
		continue;
	    
	      float dr = track->p4().DeltaR(tjet->p4());
	      if (dr < clusterIso) clusterIso = dr;	    
	    
	    }
	  
	    //Get ghost associated tracks
	    std::vector<const xAOD::TrackParticle*> jet_tracks;
	  
	    jet->getAssociatedObjects("GhostTrack", jet_tracks);  
	  
	    for(unsigned int i=0; i<jet_tracks.size(); i++) {
	    
	      if (jet_tracks[i] == track) {
		fromjet = 1;
		break;
	      }	    
	    }	    
	  }       
	
	  if (fromjet && m_jetveto)
	    continue;
		
	  if (clusterIso < 0.3 && m_trackjetveto)
	    continue;

	  clustertracks->push_back(track);

	}      
      }

    // sort tracks so that later clusters with higher
    // pt seed are selected in case of overlap
    std::sort (seedtracks->begin(), seedtracks->end(), TrackCluster::trackptsorter);

    if (seedtracks->size() > 0)
      for (size_t j = 0; j < seedtracks->size(); ++j)
	{ 
	
	  SoftBVrt::TrackCluster cluster;

	  const xAOD::TrackParticle* track = (*seedtracks)[j];       
	
	  cluster.addSeed(track);
	  cluster.addTrack(track);     
      
	  if (clustertracks->size() > 0)
	    for (size_t k = 0; k < clustertracks->size(); ++k)  
	      {	  	    	   
	  
		const xAOD::TrackParticle* track2 = (*clustertracks)[k];

		// calculate 3D track-track distance
		bool distance_valid = m_trkDistanceFinderTool->CalculateMinimumDistance(track->perigeeParameters(), track2->perigeeParameters());
	  
		TLorentzVector trackVec = track2->p4();	 	      
		TLorentzVector seed = cluster.m_seed->p4();

		if (distance_valid) {
	    
		  float distance = m_trkDistanceFinderTool->GetDistance();
		  
		  if (distance < m_cluster_distance && seed.DeltaR(trackVec) < m_cluster_dr)		    
		    cluster.addTrack(track2);
	      	      
		}	      
	      }

	  // no hope of reconstructing a secondary
	  // vertex if less than two tracks
	  if (cluster.getNTracks() > 1) {	  
	  
	    bool fillcluster = true;
	    bool addcluster = true;

	    // always add the cluster if 
	    // clusterVec is empty
	    if (clusterVec.size() == 0) {
	    
	      clusterVec.push_back(cluster);
	      // don't add the cluster twice..
	      addcluster = false;	   

	    }
	  	  
	    else
	      for (size_t p = 0; p < clusterVec.size(); p++) {
	      
		double frac = cluster.getSharedTrackFraction(clusterVec[p]);	    	      

		// if new cluster overlaps with an existing cluster (which
		// has higher pt), do not add to clusterVec
		if (frac > m_overlap_frac) {
		  fillcluster = false;
		  addcluster = false;
		  break;
		}

	      }
	
	    if (fillcluster) {	 
	    
	      if (addcluster) {
		clusterVec.push_back(cluster);
	      }	  	  
	    }       
	  }
	}
  
    // loop through all track clusters
    for (size_t p = 0; p < clusterVec.size(); p++) {

      TLorentzVector direction;

      direction = clusterVec[p].m_sumTrack;
    
      // apply vertexing tool
      const Trk::VxSecVertexInfo* myVertexInfo = m_secVertexFinderTool->findSecVertex( *myVertex, direction, clusterVec[p].getTracks() );   
      const std::vector<xAOD::Vertex*> vertices = myVertexInfo->vertices();       
    
      for (auto *vertex : vertices) 
	RecoVertices->push_back( vertex );
      	    
    }    

    return StatusCode::SUCCESS;
  
  }

}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// JetAugmentationTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Teng Jian Khoo (teng.jian.khoo@cern.ch)
//

#include "JetAugmentationTool.h"
#include "xAODCore/ShallowCopy.h"
#include "JetJvtEfficiency/JetJvtEfficiency.h"

#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"

namespace DerivationFramework {

  JetAugmentationTool::JetAugmentationTool(const std::string& t,
    const std::string& n,
    const IInterface* p) :
    AthAlgTool(t,n,p),
    dec_calibpt(0),
    dec_calibeta(0),
    dec_calibphi(0),
    dec_calibm(0),
    m_jetCalibTool(""),
    m_docalib(false),
    dec_jvt(0),
    dec_passJvt(0),
    m_jvtTool(""),
    m_jetJvtEfficiencyTool("CP::JetJvtEfficiency/JetJvtEfficiencyTool"),
    m_dojvt(false),
    m_dobtag(false),
    m_jetTrackSumMomentsTool(""),
    m_decoratetracksum(false),
    m_jetOriginCorrectionTool(""),
    m_decorateorigincorrection(false),
    dec_origincorrection(0),
    dec_originpt(0),
    dec_origineta(0),
    dec_originphi(0),
    dec_originm(0),
    m_jetPtAssociationTool(""),
    m_decorateptassociation(false),
    dec_AssociatedNtracks(0),
    m_decoratentracks(true),
    m_trkSelectionTool("InDet::InDetTrackSelectionTool/TrackSelectionTool", this) //
  {
    declareInterface<DerivationFramework::IAugmentationTool>(this);
    declareProperty("MomentPrefix",   m_momentPrefix = "DFCommonJets_");
    declareProperty("InputJets",      m_containerName = "AntiKt4EMTopoJets");
    // declareProperty("CalibScale",     m_calibScale = "JetGSCScaleMomentum");
    declareProperty("CalibMomentKey", m_calibMomentKey = "Calib");
    declareProperty("JetCalibTool",   m_jetCalibTool);
    declareProperty("JvtMomentKey",   m_jvtMomentKey = "Jvt");
    declareProperty("JetJvtTool",     m_jvtTool);
    declareProperty("JetBtagTools",   m_btagSelTools);
    declareProperty("JetBtagWPs",     m_btagWP);
    declareProperty("JetTrackSumMomentsTool", m_jetTrackSumMomentsTool);
    declareProperty("JetPtAssociationTool", m_jetPtAssociationTool);
    declareProperty("JetOriginCorrectionTool",m_jetOriginCorrectionTool);
    declareProperty("TrackSelectionTool",m_trkSelectionTool);
  }

  StatusCode JetAugmentationTool::initialize()
  {
    ATH_MSG_INFO("Initialising JetAugmentationTool");

    if(!m_jetCalibTool.empty()) {
      CHECK(m_jetCalibTool.retrieve());
      ATH_MSG_INFO("Augmenting jets with calibration \"" << m_momentPrefix+m_calibMomentKey << "\"");
      m_docalib = true;

      dec_calibpt  = new SG::AuxElement::Decorator<float>(m_momentPrefix+m_calibMomentKey+"_pt");
      dec_calibeta = new SG::AuxElement::Decorator<float>(m_momentPrefix+m_calibMomentKey+"_eta");
      dec_calibphi = new SG::AuxElement::Decorator<float>(m_momentPrefix+m_calibMomentKey+"_phi");
      dec_calibm   = new SG::AuxElement::Decorator<float>(m_momentPrefix+m_calibMomentKey+"_m");

      if(!m_jvtTool.empty()) {
        CHECK(m_jvtTool.retrieve());
        ATH_MSG_INFO("Augmenting jets with updated JVT \"" << m_momentPrefix+m_jvtMomentKey << "\"");
        m_dojvt = true;

        dec_jvt  = new SG::AuxElement::Decorator<float>(m_momentPrefix+m_jvtMomentKey);
        dec_passJvt  = new SG::AuxElement::Decorator<char>(m_momentPrefix+"pass"+m_jvtMomentKey);

        if(!m_btagSelTools.empty()) {
          size_t ibtag(0);
          for(const auto& tool : m_btagSelTools) {
            CHECK(tool.retrieve());
            ATH_MSG_INFO("Augmenting jets with B-tag working point \"" << m_momentPrefix+m_btagWP[ibtag] << "\"");
            dec_btag.push_back(new SG::AuxElement::Decorator<float>(m_momentPrefix+m_btagWP[ibtag]));
            m_dobtag = true;
            ++ibtag;
          }
        }
      }
    }

    if(!m_jetJvtEfficiencyTool.empty()) {
      CHECK(m_jetJvtEfficiencyTool.retrieve());
      ATH_MSG_INFO("Jvt efficiency tool initialized \"" << m_momentPrefix+"pass"+m_jvtMomentKey << "\"");
    }

    if(!m_jetTrackSumMomentsTool.empty()) {
      CHECK(m_jetTrackSumMomentsTool.retrieve());
      ATH_MSG_INFO("Augmenting jets with track sum moments \"" << m_momentPrefix << "TrackSumMass,Pt\"");
      m_decoratetracksum = true;
      dec_tracksummass = new SG::AuxElement::Decorator<float>(m_momentPrefix+"TrackSumMass");
      dec_tracksumpt   = new SG::AuxElement::Decorator<float>(m_momentPrefix+"TrackSumPt");
    }

    // This tool creates the GhostTruthAssociation decorations recommended for truth matching //
    if(!m_jetPtAssociationTool.empty()) 
      {
	CHECK(m_jetPtAssociationTool.retrieve());
	ATH_MSG_INFO("Augmenting jets with GhostTruthAssociation moments Link and Fraction");
	m_decorateptassociation = true;
	dec_GhostTruthAssociationFraction = new SG::AuxElement::Decorator<float>("GhostTruthAssociationFraction");
	dec_GhostTruthAssociationLink     = new SG::AuxElement::Decorator< ElementLink<xAOD::JetContainer> >("GhostTruthAssociationLink");
    }
    
    // Here it for ntracks decoration
    dec_AssociatedNtracks = new SG::AuxElement::Decorator<int>("NtracksTest");
    dec_AssociatedNtracks_noCut = new SG::AuxElement::Decorator<int>("NtracksTest_noCut");
    dec_minPtTracks = new SG::AuxElement::Decorator<float>("minPtTracks");
    dec_dTrkPv = new SG::AuxElement::Decorator<vector<float>>("dTrkPv");
    dec_count = new SG::AuxElement::Decorator<vector<int>>("count");
    dec_NTracks_Null = new SG::AuxElement::Decorator<int>("NTracks_Null");
    dec_Track_pt = new SG::AuxElement::Decorator<vector<float>>("Track_pt");
    dec_Track_passCount1 = new SG::AuxElement::Decorator<vector<int>>("Track_passCount1");
    dec_Track_passCount2 = new SG::AuxElement::Decorator<vector<int>>("Track_passCount2");
    dec_Track_passCount3 = new SG::AuxElement::Decorator<vector<int>>("Track_passCount3");

    // set up InDet selection tool
    //if(!m_trkSelectionTool.empty()) {
    assert( m_trkSelectionTool.retrieve() );
    assert( m_trkSelectionTool.setProperty( "CutLevel", "Loose" ) );
    //CHECK( m_trkSelectionTool.retrieve() );
    //CHECK( m_trkSelectionTool.setProperty( "CutLevel", "Loose" ) );
      m_decoratentracks = true; 
      //} non funziona??


    if(!m_jetOriginCorrectionTool.empty()) {
      CHECK(m_jetOriginCorrectionTool.retrieve());
      ATH_MSG_DEBUG("Augmenting jets with origin corrections \"" << m_momentPrefix << "Origin\"");
      m_decorateorigincorrection = true;
      dec_origincorrection = new SG::AuxElement::Decorator<ElementLink<xAOD::VertexContainer>>(m_momentPrefix+"JetOriginConstitScaleMomentum_OriginVertex");
      dec_originpt  = new SG::AuxElement::Decorator<float>(m_momentPrefix+"JetOriginConstitScaleMomentum_pt");
      dec_origineta = new SG::AuxElement::Decorator<float>(m_momentPrefix+"JetOriginConstitScaleMomentum_eta");
      dec_originphi = new SG::AuxElement::Decorator<float>(m_momentPrefix+"JetOriginConstitScaleMomentum_phi");
      dec_originm   = new SG::AuxElement::Decorator<float>(m_momentPrefix+"JetOriginConstitScaleMomentum_m");  
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetAugmentationTool::finalize()
  {

    if(m_docalib) {
      delete dec_calibpt;
      delete dec_calibeta;
      delete dec_calibphi;
      delete dec_calibm;
    }

    if(m_dojvt) {
      delete dec_jvt;
      delete dec_passJvt;
    }

    if(m_dobtag) {
      for(const auto& pdec : dec_btag) delete pdec;
    }

    if(m_decoratetracksum){
      delete dec_tracksummass;
      delete dec_tracksumpt;
    }

    if(m_decorateptassociation)
      {
      delete dec_GhostTruthAssociationFraction;
      delete dec_GhostTruthAssociationLink;
    }
    
    if(m_decoratentracks)
      {
	delete dec_AssociatedNtracks;
      }
    
    if(m_decorateorigincorrection){
      delete dec_origincorrection;
      delete dec_originpt;
      delete dec_originphi;
      delete dec_origineta;
      delete dec_originm; 
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetAugmentationTool::addBranches() const
  {
    // retrieve container
    const xAOD::JetContainer* jets(0);
    if( evtStore()->retrieve( jets, m_containerName ).isFailure() ) {
      ATH_MSG_WARNING ("Couldn't retrieve jets with key: " << m_containerName );
      return StatusCode::FAILURE;
    }

    // make a shallow copy of the jets
    std::pair<xAOD::JetContainer*,xAOD::ShallowAuxContainer*> shallowcopy = xAOD::shallowCopyContainer(*jets);
    std::unique_ptr<xAOD::JetContainer> jets_copy(shallowcopy.first);
    std::unique_ptr<xAOD::ShallowAuxContainer> jets_copyaux(shallowcopy.second);

      // if we have a calibration tool, apply the calibration
    if(m_docalib) {
      if(m_jetCalibTool->modify(*jets_copy) ) {
        ATH_MSG_WARNING("Problem applying jet calibration");
        return StatusCode::FAILURE;
      }
    }

    if(m_decoratetracksum){
      if( m_jetTrackSumMomentsTool->modify(*jets_copy) )
      {
        ATH_MSG_WARNING("Problems calculating TrackSumMass and TrackSumPt");
        return StatusCode::FAILURE;
      }
    }

    if(m_decorateorigincorrection){
      if(m_jetOriginCorrectionTool->modify(*jets_copy))
	{
	  ATH_MSG_WARNING("Problem applying the origin correction tool");
	  return StatusCode::FAILURE;
	}
    }

    // Check if GhostTruthAssociation decorations already exist for first jet, and if so skip them //
    bool isMissingPtAssociation = true;
    if( !m_decorateptassociation || jets_copy->size() == 0 || dec_GhostTruthAssociationFraction->isAvailable(*jets_copy->at(0)) ) {
      isMissingPtAssociation = false;
    }

    if(m_decorateptassociation && isMissingPtAssociation){
      if( m_jetPtAssociationTool->modify(*jets_copy) )
      {
        ATH_MSG_WARNING("Problem running the JetPtAssociationTool");
        return StatusCode::FAILURE;
      }
    }


    // loop over the copies
    for(const auto& jet : *jets_copy) {
      // get the original jet so we can decorate it
      const xAOD::Jet& jet_orig( *(*jets)[jet->index()] );

      if(m_docalib) {
        // generate static decorators to avoid multiple lookups
        (*dec_calibpt)(jet_orig)  = jet->pt();
        (*dec_calibeta)(jet_orig) = jet->eta();
        (*dec_calibphi)(jet_orig) = jet->phi();
        (*dec_calibm)(jet_orig)   = jet->m();

        ATH_MSG_VERBOSE("Calibrated jet pt: " << (*dec_calibpt)(jet_orig) );

        if(m_dojvt) {
          (*dec_jvt)(jet_orig) = m_jvtTool->updateJvt(*jet);
          ATH_MSG_VERBOSE("Calibrated JVT: " << (*dec_jvt)(jet_orig) );
          bool passJVT = m_jetJvtEfficiencyTool->passesJvtCut(jet_orig);
                (*dec_passJvt)(jet_orig) = passJVT;

          if(m_dobtag) {
            size_t ibtag(0);
            for(const auto& tool : m_btagSelTools) {
              (*dec_btag[ibtag])(jet_orig) = jet->pt()>20e3 && fabs(jet->eta())<2.5 && passJVT && tool->accept(*jet);
              ATH_MSG_VERBOSE("Btag working point \"" << m_btagWP[ibtag] << "\" " << ((*dec_btag[ibtag])(jet_orig) ? "passed." : "failed."));
              ++ibtag;
            }
          }
        }
      }

      if(m_decoratetracksum) {
        (*dec_tracksummass)(jet_orig) = jet->getAttribute<float>("TrackSumMass");
        (*dec_tracksumpt)(jet_orig)   = jet->getAttribute<float>("TrackSumPt");
        ATH_MSG_VERBOSE("TrackSumMass: " << (*dec_tracksummass)(jet_orig) );
        ATH_MSG_VERBOSE("TrackSumPt: "   << (*dec_tracksummass)(jet_orig) );
      }

      if(m_decorateorigincorrection) {  
        (*dec_originpt)(jet_orig)  = (*dec_originpt)(*jet);
        (*dec_origineta)(jet_orig) = (*dec_origineta)(*jet);
        (*dec_originphi)(jet_orig) = (*dec_originphi)(*jet);
        (*dec_originm)(jet_orig)   = (*dec_originm)(*jet);
        (*dec_origincorrection)(jet_orig) = jet->getAttribute<ElementLink<xAOD::VertexContainer> >("OriginVertex");
	ATH_MSG_VERBOSE("OriginCorrection: " << (*dec_origincorrection)(jet_orig) );
      }

      if(m_decorateptassociation && isMissingPtAssociation){
        (*dec_GhostTruthAssociationFraction)(jet_orig) = jet->getAttribute<float>("GhostTruthAssociationFraction");
        ATH_MSG_VERBOSE("GhostTruthAssociationFraction: " << (*dec_GhostTruthAssociationFraction)(jet_orig) );
        (*dec_GhostTruthAssociationLink)(jet_orig) = jet->getAttribute< ElementLink<xAOD::JetContainer> >("GhostTruthAssociationLink");
        ATH_MSG_VERBOSE("GhostTruthAssociationLink: " << (*dec_GhostTruthAssociationLink)(jet_orig) );
      }

      if(m_decoratentracks)
	{
	  ATH_MSG_INFO("Test Decorate QG ");
	  std::vector<const xAOD::IParticle*> jettracks;
	  jet->getAssociatedObjects<xAOD::IParticle>(xAOD::JetAttribute::GhostTrack,jettracks);
	  //(*dec_AssociatedNtracks)(jet_orig) = jettracks.size();
	  
	  int nTracksCount = 0;
	  bool invalidJet = false;
	  
	  const xAOD::Vertex *pv = 0;
	  const xAOD::VertexContainer* vxCont = 0;
	  if(evtStore()->retrieve( vxCont, "PrimaryVertices" ).isFailure()){
	    ATH_MSG_WARNING("Unable to retrieve primary vertex container PrimaryVertices");
	    nTracksCount = -1;
	    invalidJet = true;
	  }
	  else if(vxCont->empty()){
	    ATH_MSG_WARNING("Event has no primary vertices!");
	    nTracksCount = -1;
	    invalidJet = true;
	  }
	  else{
	    for(const auto& vx : *vxCont){
	      // take the first vertex in the list that is a primary vertex
	      if(vx->vertexType()==xAOD::VxType::PriVtx){
		pv = vx;
		break;
	      }
	    }
	  }
	  
	  float minPtTracks = 10000000;
	  float dTrkPv_value = -1.;
	  vector<float> dTrkPv;
	  vector<int> count;
	  int ntracks_null = 0;
	  vector<float> track_pt;

	  vector<int> track_passCount1;
	  vector<int> track_passCount2;
	  vector<int> track_passCount3;
	  
	  for (size_t i = 0; i < jettracks.size(); i++) {
	    
	    dTrkPv_value = -1;
	    if(invalidJet) continue;
	    
	    const xAOD::TrackParticle* trk = static_cast<const xAOD::TrackParticle*>(jettracks[i]);

	    if(trk==NULL) ntracks_null++;

	    if(trk->pt()<minPtTracks) minPtTracks = trk->pt();

	    track_pt.push_back( trk->pt() );

	    // only count tracks with selections
	    // 1) pt>500 MeV
	    // 2) accepted track from InDetTrackSelectionTool with CutLevel==Loose
	    // 3) associated to primary vertex OR within 3mm of the primary vertex
	    bool accept = (trk->pt()>500 &&
			   m_trkSelectionTool->accept(*trk) &&
			   (trk->vertex()==pv || (!trk->vertex() && fabs((trk->z0()+trk->vz()-pv->z())*sin(trk->theta()))<3.))
			   );	    
	    
	    if(trk->pt()>500 && m_trkSelectionTool->accept(*trk)){
	      if(!trk->vertex()) dTrkPv_value = fabs((trk->z0()+trk->vz()-pv->z())*sin(trk->theta()));
	      if(trk->vertex()==pv) dTrkPv_value = 0.;
	      dTrkPv.push_back(dTrkPv_value);
	    }


	    // check nTracks counting ---
	    if(   trk->pt()>500  ) track_passCount1.push_back(1);
	    if( !(trk->pt()>500) ) track_passCount1.push_back(0);

	    if(   m_trkSelectionTool->accept(*trk)  ) track_passCount2.push_back(1);
	    if( !(m_trkSelectionTool->accept(*trk)) ) track_passCount2.push_back(0);

	    //TString string = Form("Test Decorate QG: trkSelTool output %d", m_trkSelectionTool->accept(*trk));
	    //ATH_MSG_INFO(string);
	    std::cout << "Test Decorate QG: trkSelTool output " << m_trkSelectionTool->accept(*trk) << std::endl;
	    
	    if(  (trk->vertex()==pv || (!trk->vertex() && fabs((trk->z0()+trk->vz()-pv->z())*sin(trk->theta()))<3.))  ) track_passCount3.push_back(1);
	    if( !(trk->vertex()==pv || (!trk->vertex() && fabs((trk->z0()+trk->vz()-pv->z())*sin(trk->theta()))<3.))  ) track_passCount3.push_back(0);
	    // ---------------------------

	    if (!accept) continue;
	    
	    count.push_back(1);
	    nTracksCount++;
	    
	  }// end loop over jettracks
	  
	  (*dec_AssociatedNtracks)(jet_orig) = nTracksCount;
	  (*dec_AssociatedNtracks_noCut)(jet_orig) = jettracks.size();
	  (*dec_minPtTracks)(jet_orig) = minPtTracks;
	  (*dec_dTrkPv)(jet_orig) = dTrkPv;
	  (*dec_count)(jet_orig) = count;
	  (*dec_NTracks_Null)(jet_orig) = ntracks_null;
	  (*dec_Track_pt)(jet_orig) = track_pt;	  
	  (*dec_Track_passCount1)(jet_orig) = track_passCount1;	  
	  (*dec_Track_passCount2)(jet_orig) = track_passCount2;	  
	  (*dec_Track_passCount3)(jet_orig) = track_passCount3;	  
	  
	}// end if m_decoratentracks

    }//end loop on jets copies

    return StatusCode::SUCCESS;
  }//end addBranches

}

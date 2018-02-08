/*
Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration 
*/

#include "TrigBtagEmulationTool/JetManager.h"

using namespace Trig;

#if !defined( XAOD_STANDALONE ) && !defined( XAOD_ANALYSIS )
ToolHandle< Analysis::IBTagTool > *JetManager::m_bTagTool = nullptr;
ToolHandle< Analysis::IBTagTrackAssociation > *JetManager::m_bTagTrackAssocTool = nullptr;
ToolHandle< Analysis::IBTagSecVertexing > *JetManager::m_bTagSecVtxTool = nullptr;
#endif

//**********************************************************************

JetManager::JetManager(ToolHandle<Trig::TrigDecisionTool>& trigDec,std::string chain, std::string jetContainer, std::string btagContainer) 
  : m_chain(chain), m_jetContainer(jetContainer), m_btagContainer(btagContainer),
    m_jet_key(""), m_primaryVertex_key(""), m_trackParticle_key(""),
    m_trigDec(trigDec) {}
JetManager::JetManager(const JetManager& other)
  : m_chain(other.m_chain), m_jetContainer(other.m_jetContainer),m_btagContainer(other.m_btagContainer),
    m_jet_key(other.m_jet_key), m_primaryVertex_key(other.m_primaryVertex_key), m_trackParticle_key(other.m_trackParticle_key),
    m_jet_Containers(other.m_jet_Containers.begin(),other.m_jet_Containers.end()),
    m_primaryVertex_Containers(other.m_primaryVertex_Containers.begin(),other.m_primaryVertex_Containers.end()),
    m_trackParticle_Containers(other.m_trackParticle_Containers.begin(),other.m_trackParticle_Containers.end()),
    m_btagging_Containers(other.m_btagging_Containers.begin(),other.m_btagging_Containers.end()),
    m_outputJets(other.m_outputJets.begin(),other.m_outputJets.end()),
    m_trigDec(other.m_trigDec) {}
JetManager::~JetManager() {}

void JetManager::setKeys(std::string jet_Key,std::string primaryVertex_key,std::string trackParticle_key) {
  m_jet_key = jet_Key;
  m_primaryVertex_key = primaryVertex_key;
  m_trackParticle_key = trackParticle_key;
}

StatusCode JetManager::retrieveByNavigation() {
  clear();

  Trig::FeatureContainer features = m_trigDec->features(m_chain);
  const std::vector< Trig::Combination >& combinations = features.getCombinations();

  unsigned int nCombinations = combinations.size();
  for (unsigned int combination=0; combination<nCombinations; combination++) 
    {
      getFromCombo( m_jet_Containers          ,combinations.at(combination),m_jet_key           );
      getFromCombo( m_primaryVertex_Containers,combinations.at(combination),m_primaryVertex_key );
      getFromCombo( m_btagging_Containers     ,combinations.at(combination));
      getTPfromCombo( m_trackParticle_Containers,combinations.at(combination),m_trackParticle_key );
    }

  jetCopy();
  return StatusCode::SUCCESS;
}

#ifdef XAOD_STANDALONE
StatusCode JetManager::retrieveByContainer(asg::SgTEvent* evtStore) 
#else
StatusCode JetManager::retrieveByContainer(ServiceHandle<StoreGateSvc>& evtStore)
#endif
{
  if (m_jetContainer.find("GSC")!=std::string::npos) 
    return retrieveByContainerWithMatching(evtStore);

  if (!m_trigDec->isPassed(m_chain)) 
    return StatusCode::SUCCESS;

  const xAOD::JetContainer *sgJetContainer = nullptr;
  if( evtStore->retrieve(sgJetContainer,m_jetContainer).isFailure() || sgJetContainer == nullptr)
    return StatusCode::FAILURE;

  const xAOD::BTaggingContainer *sgBTaggingContainer = nullptr;
  if ( evtStore->retrieve(sgBTaggingContainer,m_btagContainer).isFailure() || sgBTaggingContainer == nullptr )
    return StatusCode::FAILURE;

  // Loop on BTagging objects
  for (auto &bjet : *sgBTaggingContainer)
    {
      // Check if BTagging object is of the required type 
      bool selectOffline = (m_chain.find("off")!=std::string::npos);
      const double mv2c10=bjet->auxdataConst<double>("MV2c10_discriminant");
      const double mv2c20=bjet->auxdataConst<double>("MV2c20_discriminant");
      if ((selectOffline && mv2c20==0 && mv2c10==0) || (!selectOffline && (mv2c20!=0 || mv2c10!=0)) ) continue;
      // Get address of linked Jet and type of link  
      bool goodLink=false;
      const xAOD::Jet* jet; 
      try {
	if (bjet->isAvailable<std::vector<ElementLink<xAOD::JetContainer> > >("BTagBtagToJetAssociator")) 
	  {
	    const auto &jetLink = bjet->auxdataConst<std::vector<ElementLink<xAOD::JetContainer> > >("BTagBtagToJetAssociator");
	    if (jetLink.size()>0) {
	      if (jetLink[0].isValid()) {
		jet = (*(jetLink[0]));
		goodLink=true;
	      }
	    }
	  }
      } catch (...) { }
      try {
	if (bjet->isAvailable<std::vector<ElementLink<xAOD::IParticleContainer> > >("BTagBtagToJetAssociator")) {
	  const auto &jetLink = bjet->auxdataConst<std::vector<ElementLink<xAOD::IParticleContainer> > >("BTagBtagToJetAssociator");
	  if (jetLink.size()>0) {
	    if (jetLink[0].isValid()) {
	      jet = static_cast<const xAOD::Jet*>(*(jetLink[0]));
	      goodLink=true;
	    }
	  }
	}
      } catch (...) { }
      if (goodLink) {
	// Check if the linked Jet is in the requested container
	bool isJetPresent=false;
	for (const auto &sgj : *sgJetContainer) if(jet==sgj) isJetPresent=true;
	// Check if the linked Jet has already been found
	bool isJetUnique=true;
	for (const auto &j : m_jet_Containers)
	  if(jet==j) isJetUnique=false;
	// Save Jet and BTagging objects if jet is found and unique
	if (isJetPresent && isJetUnique) {
	  m_jet_Containers.push_back(jet);
	  m_btagging_Containers.push_back(bjet);
	}
      }
    }

  jetCopy();
  return StatusCode::SUCCESS;
}


#ifdef XAOD_STANDALONE
StatusCode JetManager::retrieveByContainerWithMatching(asg::SgTEvent* evtStore)
#else
StatusCode JetManager::retrieveByContainerWithMatching(ServiceHandle<StoreGateSvc>& evtStore)
#endif
{

  if (!m_trigDec->isPassed(m_chain)) 
    return StatusCode::SUCCESS;

  const xAOD::JetContainer *sgJetContainer = nullptr;
  if ( (evtStore->retrieve(sgJetContainer,m_jetContainer)).isFailure() || sgJetContainer == nullptr )
    return StatusCode::FAILURE;

  const xAOD::BTaggingContainer *sgBTaggingContainer = nullptr;
  if (evtStore->retrieve(sgBTaggingContainer,m_btagContainer).isFailure() || sgBTaggingContainer == nullptr)
    return StatusCode::FAILURE;

  // Loop on BTagging objects
  for (auto &bjet : *sgBTaggingContainer)
    {
      // Check if BTagging object is of the required type 
      bool selectOffline = (m_chain.find("off")!=std::string::npos);
      const double mv2c10=bjet->auxdataConst<double>("MV2c10_discriminant");
      const double mv2c20=bjet->auxdataConst<double>("MV2c20_discriminant");
      if ((selectOffline && mv2c20==0 && mv2c10==0) || (!selectOffline && (mv2c20!=0 || mv2c10!=0)) ) continue;
      // Get address of linked Jet and type of link  
      bool goodLink=false;
      const xAOD::Jet* jet;
      try {
	if (bjet->isAvailable<std::vector<ElementLink<xAOD::JetContainer> > >("BTagBtagToJetAssociator")) 
	  {
	    const auto &jetLink = bjet->auxdataConst<std::vector<ElementLink<xAOD::JetContainer> > >("BTagBtagToJetAssociator");
	    if (jetLink.size()>0) {
	      if (jetLink[0].isValid()) {
		jet = (*(jetLink[0]));
		goodLink=true;
	      }
	    }
	  }
      } catch (...) { }
      try {
	if (bjet->isAvailable<std::vector<ElementLink<xAOD::IParticleContainer> > >("BTagBtagToJetAssociator")) {
	  const auto &jetLink = bjet->auxdataConst<std::vector<ElementLink<xAOD::IParticleContainer> > >("BTagBtagToJetAssociator");
	  if (jetLink.size()>0) {
	    if (jetLink[0].isValid()) {
	      jet = static_cast<const xAOD::Jet*>(*(jetLink[0]));
	      goodLink=true;
	    }
	  }
	}
      } catch (...) { }
      if (goodLink) {
	// Check if the linked Jet is in the requested container
	bool isJetPresent=false;
	const xAOD::Jet *matchedJet = 0;
	//	for (const auto &sgj : *sgJetContainer) if(jet==sgj) isJetPresent=true;
	for (const auto &sgj : *sgJetContainer) 
	  if( isMatched(jet,sgj) ) 
	    {
	      matchedJet = sgj;
	      isJetPresent=true;
	      break;
	    }
	// Check if the linked Jet has already been found
	bool isJetUnique=true;
	for (const auto &j : m_jet_Containers)
	  if(matchedJet==j) isJetUnique=false;
	// Save Jet and BTagging objects if jet is found and unique
	if (isJetPresent && isJetUnique) {
	  m_jet_Containers.push_back(matchedJet);
	  m_btagging_Containers.push_back(bjet);
	}
      }
    }

  jetCopy();
  return StatusCode::SUCCESS;
}

bool JetManager::isMatched(const xAOD::Jet* splitJet,const xAOD::Jet* gscJet)
{
  double dR = sqrt( pow(splitJet->eta() - gscJet->eta(),2) + pow(splitJet->phi() - gscJet->phi(),2) );
  return dR < 0.05;
}

bool JetManager::clear()
{
  m_jet_Containers.clear();
  m_primaryVertex_Containers.clear();
  m_trackParticle_Containers.clear();
  m_btagging_Containers.clear();
  for (unsigned int i(0); i<m_outputJets.size(); i++)
    delete m_outputJets.at(i);
  m_outputJets.clear();
  return true;
}

template<typename T> bool JetManager::getFromCombo(std::vector<const T*> &output,const Trig::Combination& combo,std::string key)
{
  const DataVector<T> *tmpContainer = nullptr;
  const std::vector< Trig::Feature< DataVector<T> > > tmpFeatures = combo.containerFeature< DataVector<T> >(key.c_str());

  if (tmpFeatures.size())
    tmpContainer = tmpFeatures[0].cptr();
  if (tmpContainer == nullptr)
      return false;

  output.push_back( (*tmpContainer)[0] );
  return true;
}

bool JetManager::getTPfromCombo(std::vector<const xAOD::TrackParticleContainer*>& tpContainers,const Trig::Combination& combo,std::string key)
{

  const xAOD::TrackParticleContainer* trackParticleContainer = nullptr;
  const std::vector< Trig::Feature<xAOD::TrackParticleContainer> > trackParticleContainerFeatures = combo.containerFeature<xAOD::TrackParticleContainer>(key.c_str());

  if (trackParticleContainerFeatures.size()) 
    trackParticleContainer = trackParticleContainerFeatures[0].cptr();
  if (trackParticleContainer == nullptr)
    return false;

  tpContainers.push_back( trackParticleContainer );
  return true;
}

void JetManager::jetCopy()
{
  for (unsigned int i(0); i<m_jet_Containers.size(); i++)  
    m_outputJets.push_back( new TrigBtagEmulationJet( m_jet_Containers.at(i) ) );
}

StatusCode JetManager::retagCopy(bool useNavigation,bool tagOffline,bool tagOnline)
{
  auto out = m_outputJets.begin();
  for (auto & btag : m_btagging_Containers) 
    {
      const double mv2c10=btag->auxdataConst<double>("MV2c10_discriminant");
      const double mv2c20=btag->auxdataConst<double>("MV2c20_discriminant");
      double ip3dsv1 = btag->SV1plusIP3D_discriminant();
      double combw=(btag->IP3D_pb()/btag->IP3D_pu()) * (btag->SV1_pb()/btag->SV1_pu());
      double comb=50;
      if(combw/(1+combw)<1) comb=-1.0*TMath::Log10(1-(combw/(1+combw)));

      if (!useNavigation || !tagOffline)
	{
	  (*out)->weights( "MV2c10",mv2c10 );
	  (*out)->weights( "MV2c20",mv2c20 );
	}
      
      if (!useNavigation || !tagOnline)
	{
	  (*out)->weights( "IP3DSV1",ip3dsv1 );
	  (*out)->weights( "COMB"   ,comb );
	}
      out++;
    }
  return StatusCode::SUCCESS;
}

StatusCode JetManager::retagOffline() {
#if !defined( XAOD_STANDALONE ) && !defined( XAOD_ANALYSIS )
  auto pv  = m_primaryVertex_Containers.begin();
  auto tp  = m_trackParticle_Containers.begin();
  auto out = m_outputJets.begin();

  for (auto & jet : m_jet_Containers) {

    // Create container for copied jets
    xAOD::JetContainer* output_jets = new xAOD::JetContainer(SG::OWN_ELEMENTS);
    xAOD::JetAuxContainer* output_jetsAux = new xAOD::JetAuxContainer;
    output_jets->setStore(output_jetsAux);

    // Create container for SV results
    xAOD::VertexContainer* output_svs = new xAOD::VertexContainer(SG::OWN_ELEMENTS);
    xAOD::VertexAuxContainer* output_svsAux = new xAOD::VertexAuxContainer;
    output_svs->setStore(output_svsAux);

    // Create container for JF results
    xAOD::BTagVertexContainer* output_jfs = new xAOD::BTagVertexContainer(SG::OWN_ELEMENTS);
    xAOD::BTagVertexAuxContainer* output_jfsAux = new xAOD::BTagVertexAuxContainer;
    output_jfs->setStore(output_jfsAux);

    // Create container for new BTagging info
    xAOD::BTaggingContainer* output_btags = new xAOD::BTaggingContainer(SG::OWN_ELEMENTS);
    xAOD::BTaggingAuxContainer* output_btagsAux = new xAOD::BTaggingAuxContainer;
    output_btags->setStore(output_btagsAux);

    // Create jet copy
    xAOD::Jet* output_jet = new xAOD::Jet;
    output_jets->push_back(output_jet);
    *output_jet=*jet;

    // Create new BTagging object
    xAOD::BTagging* output_btag = new xAOD::BTagging;
    output_btags->push_back(output_btag);

    // Link the BTagging objects to the corresponding Jet
    ElementLink<xAOD::BTaggingContainer> linkBTagger;
    linkBTagger.toContainedElement(*output_btags, output_btag);
    output_jet->setBTaggingLink(linkBTagger);

    // Get PV
    const xAOD::Vertex* primaryVertex = *pv;

    // Perform track association
    StatusCode sc;
    if (!m_bTagTrackAssocTool->empty()) {
      std::vector<xAOD::Jet*> jetsList;
      jetsList.push_back(output_jet);
      sc = (*m_bTagTrackAssocTool)->BTagTrackAssociation_exec(&jetsList, *tp);
    }
    else {
      printf("### WARNING ::: #BTAG# Empty track association tool\n");
      //      ATH_MSG_WARNING( "#BTAG# Empty track association tool" );
    }
    if (sc.isFailure()) {
      printf("### ERROR ::: #BTAG# Failed to associate tracks to jet\n");
      //      ATH_MSG_ERROR( "#BTAG# Failed to associate tracks to jet" );
      return sc;
    }

    // Execute secondary vertexing
    sc = (*m_bTagSecVtxTool)->BTagSecVtx_exec(*output_jet, output_btag, output_svs, output_jfs, primaryVertex);
    if (sc.isFailure()) {
      printf("### ERROR ::: #BTAG# Failed to reconstruct sec vtx\n");
      //      ATH_MSG_ERROR( "#BTAG# Failed to reconstruct sec vtx" );
      return sc;
    }

    // Tag jet
    sc = (*m_bTagTool)->tagJet(*output_jet, output_btag, primaryVertex);
    if (sc.isFailure()) {
      printf("### ERROR ::: #BTAG# Failed in taggers call\n");
      //      ATH_MSG_ERROR( "#BTAG# Failed in taggers call" );
      return sc;
    }

    // Get weights
    double mv2c10=-1000;
    double mv2c20=-1000;
    output_btag->variable<double>("MV2c10","discriminant",mv2c10);
    output_btag->variable<double>("MV2c20","discriminant",mv2c20);

    // Save weights
    (*out)->weights( "MV2c10",mv2c10 );    
    (*out)->weights( "MV2c20",mv2c20 );

    // Delete new objects
    delete output_jets; delete output_jetsAux;
    delete output_svs; delete output_svsAux;
    delete output_jfs; delete output_jfsAux;
    delete output_btags; delete output_btagsAux;

    // Increment
    pv++; tp++; out++;
  }
#endif
  return StatusCode::SUCCESS;
}
StatusCode JetManager::retagOnline() { return StatusCode::SUCCESS;}

std::vector< TrigBtagEmulationJet* >& JetManager::getJets() { return m_outputJets; }

JetManager& JetManager::operator+=(const JetManager& other) { return merge(other.m_jet_Containers); }
JetManager& JetManager::merge(const std::vector<const xAOD::Jet*>& jets, double minPt, double maxPt) {
  for (auto & jet : jets) {
    std::unique_ptr< TrigBtagEmulationJet > backupJet2( new TrigBtagEmulationJet( jet ) );
    backupJet2->weights( "MV2c10" ,-1 );

    TrigBtagEmulationJet *backupJet = new TrigBtagEmulationJet( jet );
    backupJet->weights( "MV2c10" ,-1 );
    backupJet->weights( "MV2c20" ,-1 );
    backupJet->weights( "IP3DSV1",-1000 );
    backupJet->weights( "COMB"   ,-1000 );

    bool isUnique = true;
    for (unsigned int index(0); index < m_outputJets.size(); index++)
      if ( m_outputJets.at(index)->pt() == backupJet->pt() &&
	   m_outputJets.at(index)->eta() == backupJet->eta() &&
	   m_outputJets.at(index)->phi() == backupJet->phi() ) 
	{isUnique = false; break;}

    if (!isUnique) delete backupJet;
    else if (isUnique && backupJet->pt() >= minPt) {
      if ( maxPt == 0 ) m_outputJets.push_back( backupJet );
      else if ( backupJet->pt() < maxPt ) m_outputJets.push_back( backupJet );
      else delete backupJet;
    }
  }
  
  return *this;
}

//**********************************************************************

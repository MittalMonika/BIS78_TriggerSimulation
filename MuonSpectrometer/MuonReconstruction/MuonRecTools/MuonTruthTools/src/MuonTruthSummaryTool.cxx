/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTruthSummaryTool.h"
#include "MuonSegment/MuonSegment.h"
#include "TrkTrack/Track.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "HepMC/GenParticle.h"
#include <iostream>
#include "TTree.h"
#include "GaudiKernel/ITHistSvc.h"

namespace Muon {

  /** Constructor **/
  MuonTruthSummaryTool::MuonTruthSummaryTool(const std::string& t, const std::string& n, const IInterface* p) : 
    AthAlgTool(t,n,p),
    m_idHelper("Muon::MuonIdHelperTool/MuonIdHelperTool"),
    m_printer("Muon::MuonEDMPrinterTool/MuonEDMPrinterTool"),
    m_incidentSvc("IncidentSvc",n),
    m_wasInit(false),
    m_truthHitsTotal(0),
    m_tree(0),
    m_thistSvc(0),
    m_writeTree(false),
    m_level(0)
  {
    declareInterface<IMuonTruthSummaryTool>(this);
    declareProperty("WriteNtuple",      m_writeTree);
    declareProperty("NtupleTreeName",   m_treeName = "MuonTruthSummaryTree");
    declareProperty("HistStream",       m_histStream = "Summary");
    declareProperty("SelectedPdgId",    m_selectedPdgId = 13, "Should be positive as absolute value is used" );
    declareProperty("UseNSW",           m_useNSW=false);
  }

  StatusCode  MuonTruthSummaryTool::initialize()
  {
    ATH_MSG_VERBOSE("Initializing ...");
    ATH_CHECK( m_idHelper.retrieve() );
    ATH_CHECK( m_printer.retrieve() );
    ATH_CHECK( m_edmHelperSvc.retrieve() );
    ATH_CHECK( m_incidentSvc.retrieve() );      

    m_incidentSvc->addListener( this, std::string("EndEvent"));
    
    if (m_writeTree){
      ATH_CHECK( service("THistSvc", m_thistSvc) );
    
      m_tree = new TTree(m_treeName.c_str(), "Ntuple of MuonTruthSummary");
      static unsigned int numLevels=3;// Hardcoding to 3 levels for the moment.
      
      std::string treePathAndName ("/");
      treePathAndName+=m_histStream;
      treePathAndName+="/";
      treePathAndName+=m_treeName;
      
      ATH_CHECK( m_thistSvc->regTree(treePathAndName.c_str(), m_tree) );
      ATH_MSG_VERBOSE("Registered tree as "<<treePathAndName);
      
      initChamberVariables(numLevels); 
    }
    if( m_selectedPdgId < 0 ) {
      ATH_MSG_WARNING("SelectedPdgId should be positive, taking the absolute value");
      m_selectedPdgId = abs(m_selectedPdgId);
    }

    if(m_useNSW){
      m_TruthNames.emplace_back("MM_TruthMap");
      m_TruthNames.emplace_back("STGC_TruthMap");
    }
    else m_TruthNames.emplace_back("CSC_TruthMap");
    ATH_CHECK(m_TruthNames.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode  MuonTruthSummaryTool::finalize() {
    ATH_MSG_INFO("Of "<<m_truthHitsTotal<<" truth hits in total...");
    for (auto level : m_lossesPerLevel){
      ATH_MSG_INFO(level.second<<" \ttruth hits lost at level :"<<level.first);
    }
    return StatusCode::SUCCESS;
  }

  void MuonTruthSummaryTool::clear() {
    m_wasInit = false;
    m_truthHits.clear();
    m_truthDataPerLevel.clear();
  }

  void MuonTruthSummaryTool::init() const {
    getTruth();
    
    m_wasInit = true;
    ATH_MSG_DEBUG(" Total collected muon truth hits " << m_truthHits.size() ); 
  }

  void MuonTruthSummaryTool::getTruth() const {

    for(SG::ReadHandle<PRD_MultiTruthCollection>& col : m_TruthNames.makeHandles()){
      if(!col.isValid() || !col.isPresent()) continue;
      ATH_MSG_DEBUG(  "PRD_MultiTruthCollection " << col.key() << " found");
      PRD_MultiTruthCollection::const_iterator it = col->begin();
      PRD_MultiTruthCollection::const_iterator it_end = col->end();
      for( ;it!=it_end;++it ){
	const HepMcParticleLink& link = it->second;
	if( link.cptr() && 
	    (abs(link.cptr()->pdg_id()) ==  m_selectedPdgId || abs(link.cptr()->pdg_id()) == 13 ) ) {
	  m_truthHits[it->first] = link.cptr()->barcode();
	  m_pdgIdLookupFromBarcode[link.cptr()->barcode()]=link.cptr()->pdg_id();
	}
      }
    }
  }

  int MuonTruthSummaryTool::getPdgId( int barcode ) const {
    if( !m_wasInit) init();
    auto pos = m_pdgIdLookupFromBarcode.find(barcode);
    if( pos == m_pdgIdLookupFromBarcode.end() ) return 0;
    return pos->second;    
  }

  int MuonTruthSummaryTool::getBarcode( const Identifier& id ) const {
    if( !m_wasInit) init();
    auto pos = m_truthHits.find(id);
    if( pos == m_truthHits.end() ) return -1;
    return pos->second;
  }

  void MuonTruthSummaryTool::add( const Identifier& id, int level ) {
    if( !m_wasInit) init();
    if( m_truthHits.count(id) ) m_truthDataPerLevel[level].insert(id);
  }

  void MuonTruthSummaryTool::add( const MuonSegment& seg, int level ) {
    add(seg.containedMeasurements(),level);
  }

  void MuonTruthSummaryTool::add( const Trk::Track& track, int level ) {
    const DataVector<const Trk::MeasurementBase>* meas = track.measurementsOnTrack();                
    if( meas ) add(meas->stdcont(),level);
  }

  void MuonTruthSummaryTool::add( const std::vector<const Trk::MeasurementBase*>& measurements, int level ) {
    for( std::vector<const Trk::MeasurementBase*>::const_iterator it = measurements.begin();it!=measurements.end();++it ){
      Identifier id = m_edmHelperSvc->getIdentifier(**it);
      if( id.is_valid() && m_idHelper->isMuon(id) ) add(id,level);
    }
  }
  
  std::string MuonTruthSummaryTool::printSummary()  {
    if( m_truthHits.empty() )  return "Event without truth hits";
    if( m_truthDataPerLevel.empty() ) return "No hits added";
    ATH_MSG_DEBUG( "Have " << m_truthHits.size() << " truth hits and "<< m_truthDataPerLevel.size()<<" levels filled."  );

    std::set<Identifier> truthHits;
    for( auto it = m_truthHits.begin();it!=m_truthHits.end();++it ) truthHits.insert(it->first);

    m_truthHitsTotal+=truthHits.size();
    std::ostringstream sout;
    sout << " Summarizing: truth hits " << truthHits.size() << " levels filled " << m_truthDataPerLevel.size()<<std::endl;
    std::map<int,std::set<Identifier> >::const_iterator it = m_truthDataPerLevel.begin();
    std::map<int,std::set<Identifier> >::const_iterator it_end = m_truthDataPerLevel.end();
    for( ;it!=it_end;++it ){
      m_level=it->first-1;
      if (m_writeTree) clearChamberVariables( m_level );
      sout << " Comparing truth to level " << m_level << std::endl << printSummary( truthHits, it->second );
      m_lossesPerLevel[it->first]+=(truthHits.size()-it->second.size());
    }
    if (m_writeTree) {
      std::cout<<"About to try to fill try tree "<<m_tree->GetName()<<std::endl;
      std::cout<<"m_numChambers size =  "<<m_numChambers.size()<<", val [0] = "<<m_numChambers[0]<<std::endl;
      
      m_tree->Fill();  
    }
    return sout.str();
  }

  std::string MuonTruthSummaryTool::printSummary( const std::set<Identifier>& truth, const std::set<Identifier>& found )  {
    std::ostringstream sout;
    if( truth.size() != found.size() ){
      sout << " Some truth hits not found: truth " << truth.size() << " found " << found.size() << std::endl;
      std::vector<Identifier> result(truth.size()-found.size());
      std::vector<Identifier>::iterator pos = std::set_difference(truth.begin(),truth.end(),found.begin(),found.end(),result.begin());
      result.resize(pos-result.begin());
      int nmm = 0;
      std::map<Identifier, unsigned int> chambers; // Store counts per chamber Id
      unsigned int hitNum=0;
      for( std::vector<Identifier>::iterator it=result.begin();it!=result.end();++it ){
        sout << hitNum++<<":\t" << m_idHelper->toString(*it) << std::endl;
        if(m_idHelper->isMM(*it)  ) ++nmm;
        chambers[m_idHelper->chamberId(*it)]++;
      }
      if( nmm > 4 ) sout << " possible missing MM segment : " << nmm << std::endl;

      sout << std::endl<<"++++ Chamber summaries:"<<std::endl;      
      for (auto chamber : chambers ){
        sout << "Chamber "<<m_idHelper->toStringChamber(chamber.first)<<"\t has "<<chamber.second<<" missing truth hits"<<std::endl;
        if (m_writeTree) fillChamberVariables(chamber.first, chamber.second);
      }
    }else{
      sout << " All hits found: truth " << truth.size() << " found " << found.size() << std::endl;
    }
    return sout.str();
  }
  
  void MuonTruthSummaryTool::initChamberVariables(const unsigned int levels){
    if (!m_tree) {
      ATH_MSG_WARNING("Trying to write ntuple, but tree is zero. Setting WriteNtuple to False");
      m_writeTree=false;
    }
    m_numChambers.resize(levels);
    m_numMissedHits.resize(levels);
    m_missedHitTechnologyIndex.resize(levels);
    m_missedHitStationPhi.resize(levels);
    m_missedHitStationSector.resize(levels);
    m_missedHitStationEta.resize(levels);
    // m_missedHitR.resize(levels);
    // m_missedHitZ.resize(levels);
    // m_missedHitStationName.resize(levels);
    m_missedHitStationNameIndex.resize(levels);
    
    for (unsigned int level =0 ; level<levels ; level++) {
      m_numChambers[level]              = 0;
      m_numMissedHits[level]            = new std::vector<uint8_t>;
      m_missedHitTechnologyIndex[level] = new std::vector<uint8_t>;
      m_missedHitStationPhi[level]      = new std::vector<int>;
      m_missedHitStationSector[level]   = new std::vector<int>;
      m_missedHitStationEta[level]      = new std::vector<int>;
      // m_missedHitR[level]               = new std::vector<float>;
      // m_missedHitZ[level]               = new std::vector<float>;
      // m_missedHitStationName[level]     = new std::vector<std::string>;
      m_missedHitStationNameIndex[level]     = new std::vector<int>;
      
      std::string name = "numMissedHitsForLevel"+std::to_string(level);
      std::string leafThing = name + std::string("/i");
      // std::cout<<"m_numChambers["<<level<<"] = "<<m_numChambers[level]<<std::endl;
      m_tree->Branch(name.c_str(), &m_numChambers[level], leafThing.c_str());
      name = "numMissedHitsForLevel"+std::to_string(level);
      m_tree->Branch(name.c_str(), &m_numMissedHits[level]);
      name = "TechnologyIndexForLevel"+std::to_string(level);
      m_tree->Branch(name.c_str(), &m_missedHitTechnologyIndex[level]);
      name = "MissedHitStationPhiForLevel"+std::to_string(level);
      m_tree->Branch(name.c_str(), &m_missedHitStationPhi[level]);
      name = "MissedHitStationSectorForLevel"+std::to_string(level);
      m_tree->Branch(name.c_str(), &m_missedHitStationSector[level]);
      name = "MissedHitStationEtaForLevel"+std::to_string(level);
      m_tree->Branch(name.c_str(), &(m_missedHitStationEta[level]));
      // name = "MissedHitRForLevel"+std::to_string(level);
      // m_tree->Branch(name.c_str(), &(m_missedHitR[level]));
      // name = "MissedHitZForLevel"+std::to_string(level);
      // m_tree->Branch(name.c_str(), &(m_missedHitZ[level]));
      // name = "MissedHitStationName for level "+std::to_string(level);
      // m_tree->Branch(name.c_str(), &m_missedHitStationName[level]);
      name = "MissedHitStationNameIndexForLevel"+std::to_string(level);
      m_tree->Branch(name.c_str(), &m_missedHitStationNameIndex[level]);
    }
  }
  
  void MuonTruthSummaryTool::clearChamberVariables( const unsigned int level){
    ATH_MSG_DEBUG("clearChamberVariables: Level = "<<level+1);
    m_numChambers[level] = 0;
    m_numMissedHits[level]->clear();
    m_missedHitTechnologyIndex[level]->clear();
    m_missedHitStationPhi[level]->clear();
    m_missedHitStationSector[level]->clear();
    m_missedHitStationEta[level]->clear();
    // m_missedHitR[level]->clear();
    // m_missedHitZ[level]->clear();
    // m_missedHitStationName[level]->clear();
    m_missedHitStationNameIndex[level]->clear();
  }
  
  void MuonTruthSummaryTool::fillChamberVariables(const Identifier& chamberId, const unsigned int numMissedHits){
    ATH_MSG_DEBUG("fillChamberVariables: Level = "<<(m_level+1)<<" \t chamber = "<<m_idHelper->toStringChamber(chamberId)<<" numMissedHits="<<numMissedHits );
    
    m_numChambers[m_level]++;
    m_numMissedHits[m_level]->push_back(numMissedHits);
    m_missedHitTechnologyIndex[m_level]->push_back( m_idHelper->technologyIndex(chamberId) );
    m_missedHitStationPhi[m_level]       ->push_back( m_idHelper->stationPhi(chamberId) );
    m_missedHitStationSector[m_level]       ->push_back( m_idHelper->sector(chamberId) );
    m_missedHitStationEta[m_level]       ->push_back( m_idHelper->stationEta(chamberId) );
    // m_missedHitR[m_level]       ->push_back( m_idHelper->stationEta(chamberId) ); // Need to do R/Z elsewhere...
    // m_missedHitZ[m_level]       ->push_back( m_idHelper->stationEta(chamberId) );
    // // m_missedHitStationName[m_level]      ->push_back( m_idHelper->chamberNameString(chamberId) );
    m_missedHitStationNameIndex[m_level]      ->push_back( m_idHelper->stationIndex(chamberId) );
  }

  void MuonTruthSummaryTool::handle(const Incident& inc) {

    // Only clear cache for EndEvent incident
    if (inc.type() == "EndEvent") {
      ATH_MSG_DEBUG( printSummary() );
      clear();
    }     
  }  
}

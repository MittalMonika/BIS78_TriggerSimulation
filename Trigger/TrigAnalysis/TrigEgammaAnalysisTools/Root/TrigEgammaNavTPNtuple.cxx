/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************
 * AsgTool: TrigEgammaNavTPNtuple
 * Authors:
 *      Ryan Mackenzie White <ryan.white@cern.ch>
 *      Denis Damazio <denis.damazio@cern.ch>
 * Contributors:
 *      Joao Victor da Fonseca Pinto <joao.victor.da.fonseca.pinto@cern.ch>
 * Date: Mar 2015
 * Description:
 *      Derived class for dumping the probe information into a ttree.
 *     
 **********************************************************************/

#include "TrigEgammaAnalysisTools/TrigEgammaNavTPNtuple.h"

using namespace std;

TrigEgammaNavTPNtuple::TrigEgammaNavTPNtuple( const std::string& myname ): TrigEgammaNavTPBaseTool(myname) 
{
  m_eventCounter = 0;
  declareProperty("CutLabels",m_cutlabels);
  declareProperty("DirectoryPath",m_dir="NavTPNtuple");
  m_trig_L2_calo_energySample=nullptr;
  m_trig_L2_calo_rings=nullptr;
  m_trig_L2_el_trackAlgID=nullptr;
  m_trig_L2_el_pt=nullptr;
  m_trig_L2_el_eta=nullptr;
  m_trig_L2_el_caloEta=nullptr;
  m_trig_L2_el_phi=nullptr;
  m_trig_L2_el_charge=nullptr;
  m_trig_L2_el_nTRTHits=nullptr;
  m_trig_L2_el_nTRTHiThresholdHits=nullptr;
  m_trig_L2_el_etOverPt=nullptr;
  m_trig_L2_el_trkClusDeta=nullptr;
  m_trig_L2_el_trkClusDphi=nullptr;
  m_mc_hasMC=false     ;
  m_mc_pt=-999        ;
  m_mc_eta=-999       ;
  m_mc_phi=-999       ;
  m_mc_isTop=false     ;
  m_mc_isParton=false  ;
  m_mc_isMeson=false   ;
  m_mc_isQuark=false   ;
  m_mc_isTau=false     ;
  m_mc_isMuon=false    ;
  m_mc_isPhoton=false  ;
  m_mc_isElectron=false;
  m_mc_hasZMother=false;
  m_mc_hasWMother=false;
  m_el_nGoodVtx=0;
  m_el_nPileupPrimaryVtx=0;
  m_calo_et=-999.;
  m_calo_eta=-999.;
  m_calo_phi=-999.;;
  m_trig_EF_el_accept=false;


}

StatusCode TrigEgammaNavTPNtuple::childInitialize(){
  return StatusCode::SUCCESS;
}


StatusCode TrigEgammaNavTPNtuple::childBook(){

  ///Counters
  addDirectory(m_dir+"/Counters");
  addHistogram(new TH1F("ProbeCutCounter", "Number of Probes; Cut ; Count", 12, 0., 12));
  addHistogram(new TH1F("TagCutCounter", "Number of Tags; Cut ; Count", 10, 0., 10));
  addHistogram(new TH1F("Mee", "Offline M(ee); m_ee [GeV] ; Count", 50, m_ZeeMassMin, m_ZeeMassMax));
  addHistogram(new TH1I("CutCounter", "Event Selection; Cut ; Count", 6, 0., 6));
  setLabels(hist1("CutCounter"),m_cutlabels);
  
  ///Trees
  addDirectory(m_dir);
  ATH_MSG_DEBUG("Now configuring chains for analysis");
  std::vector<std::string> selectElectronChains  = tdt()->getListOfTriggers("HLT_e.*");
  for (int j = 0; j < (int) selectElectronChains.size(); j++) {
    ATH_MSG_DEBUG("Electron trigger " << selectElectronChains[j]);
  }
  std::vector<std::string> selectPhotonChains  = tdt()->getListOfTriggers("HLT_g.*");

  for (int i = 0; i < (int) m_trigInputList.size(); i++) {
    std::string trigname = "HLT_"+m_trigInputList[i];
    for (int j = 0; j < (int) selectElectronChains.size(); j++) {
      size_t found = trigname.find(selectElectronChains[j]);
      if(found != std::string::npos) {
        m_trigList.push_back(m_trigInputList[i]);
        break;
      }
    }
    for (int j = 0; j < (int) selectPhotonChains.size(); j++) {
      std::string trigname = "HLT_"+m_trigInputList[i];
      size_t found = trigname.find(selectPhotonChains[j]);
      if(found != std::string::npos) {
        m_trigList.push_back(m_trigInputList[i]);
        break;
      }
    }
  }


  for (int i = 0; i < (int) m_trigList.size(); i++) {
    std::string trigItem = m_trigList[i];
    TTree *t = new TTree( (trigItem).c_str(), "tree of Zee probes");
    bookEventBranches( t );
    bookElectronBranches( t );
    //bookPhotonBranches( t );
    bookTriggerBranches( t );
    bookMonteCarloBranches( t );
    addTree(t, m_dir);
  }

  ///Alloc pointers
  alloc_space();
  return StatusCode::SUCCESS;
}

StatusCode TrigEgammaNavTPNtuple::childExecute(){

  m_eventCounter++;
  cd(m_dir+"/Counters");
  // Event Wise Selection (independent of the required signatures)
  if ( !TrigEgammaNavTPBaseTool::EventWiseSelection() ) return StatusCode::SUCCESS;

  if(!executeProbesDump())  return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}


bool TrigEgammaNavTPNtuple::executeProbesDump(){

  // Select TP Pairs
  executeTandP();

  for(unsigned int ilist = 0; ilist != m_trigList.size(); ++ilist) {
    std::string trigItem = m_trigList.at(ilist); 
    cd(m_dir);
    TTree *t = tree( trigItem, m_dir);
    linkEventBranches(t); 
    linkElectronBranches(t); 
    linkTriggerBranches(t); 
    linkMonteCarloBranches(t); 
    TrigEgammaNavTPBaseTool::matchObjects("HLT_"+trigItem);

    for(unsigned int iprobe = 0; iprobe != m_pairObj.size(); ++iprobe){

      const xAOD::Electron *el = static_cast<const xAOD::Electron *> (m_pairObj[iprobe].first);
      const HLT::TriggerElement *feat = m_pairObj[iprobe].second;
      // Final cuts done here
      //if(!el->auxdecor<bool>(pidword)) continue; 
      ATH_MSG_DEBUG("dumping probe electron information...");
      if ( el->pt() < 24e3 ) continue;

      clear();
      fillEvent();
      fillElectron( el );  

      if(!fillMonteCarlo( el ) ){
        ATH_MSG_WARNING("Cound not found any TruthParticle for this Electron");
      }
 
      if(feat){ 
        ///Start trigger analisys...
        const xAOD::EmTauRoI *emTauRoI = getFeature<xAOD::EmTauRoI>(feat);
        if(emTauRoI){
          fillEmTauRoI( emTauRoI ); 
        }else{ 
          ATH_MSG_WARNING("Cound not found EmTauRoI in this TriggerElement..");
        }
    
        const xAOD::TrigEMCluster *emCluster = getFeature<xAOD::TrigEMCluster>(feat);
        if(emCluster){
    
          if(!fillTrigEMCluster( emCluster )){
            ATH_MSG_WARNING("Cound not attach the trigEMCluster information into the tree.");
          }
    
          if(!fillTrigCaloRings( emCluster )){
            ATH_MSG_WARNING("Cound not attach the trigCaloRinger information into the tree.");
          }

          /*
          const xAOD::TrigRNNOutput *rnnOutput=getFeature<xAOD::TrigRNNOutput>(feat);
          if(rnnOutput){
            m_trig_L2_calo_rnnOutput = rnnOutput->rnnDecision().at(0);
            ATH_MSG_DEBUG("RNN output for ringer is: " << m_trig_L2_calo_rnnOutput);
          }else{
            ATH_MSG_WARNING("No xAOD::TrigRNNOutput was found");
          }*/

        }///cluster protection
    
    
        const xAOD::TrigElectronContainer *trigElCont = getFeature<xAOD::TrigElectronContainer>(feat);
        // Level 2 ID+Calo
        if(trigElCont){
          for(const auto& trigEl : *trigElCont){
            if(!fillTrigElectron(trigEl)) {
              ATH_MSG_WARNING("Cound not attach the trigElectron information into the tree.");
            }    
          }// loop over all trigElectrons for this feat
        }
    
        m_trig_L1_accept       = ancestorPassed<xAOD::EmTauRoI>(feat);
        m_trig_L2_calo_accept  = ancestorPassed<xAOD::TrigEMCluster>(feat);
        m_trig_L2_el_accept    = ancestorPassed<xAOD::TrigElectronContainer>(feat); 
        m_trig_EF_calo_accept  = ancestorPassed<xAOD::CaloClusterContainer>(feat);
        m_trig_EF_el_accept    = ancestorPassed<xAOD::ElectronContainer>(feat);
        
        ATH_MSG_DEBUG("L1Calo: "  << int(m_trig_L1_accept)); 
        ATH_MSG_DEBUG("L2Calo: "  << int(m_trig_L2_calo_accept));
        ATH_MSG_DEBUG("L2ID: "    << int(m_trig_L2_el_accept));
        ATH_MSG_DEBUG("EFCalo: "  << int(m_trig_EF_calo_accept));
        ATH_MSG_DEBUG("EFID: "    << int(m_trig_EF_el_accept));
 
      }else{
        ATH_MSG_DEBUG("No TriggerElement for this probe!");
      }
      ATH_MSG_DEBUG("record probe information into the file.");
      t->Fill();
    }// loop over probes
  } // End loop over trigger list

  return true;
}



StatusCode TrigEgammaNavTPNtuple::childFinalize(){
  //release_space();
  return StatusCode::SUCCESS;
}



bool TrigEgammaNavTPNtuple::fillEmTauRoI( const xAOD::EmTauRoI *emTauRoI ){
  
  m_trig_L1_eta     = emTauRoI->eta();
  m_trig_L1_phi     = emTauRoI->phi();
  m_trig_L1_emClus  = emTauRoI->emClus();
  m_trig_L1_tauClus = emTauRoI->tauClus();
  m_trig_L1_emIsol  = emTauRoI->emIsol();
  m_trig_L1_hadIsol = emTauRoI->hadIsol();
  for(unsigned i=0; i < emTauRoI->thrNames().size();++i)  m_trig_L1_thrNames->push_back(emTauRoI->thrNames().at(i));
  return true;
} 

bool TrigEgammaNavTPNtuple::fillTrigEMCluster( const xAOD::TrigEMCluster *emCluster ){
  
  m_trig_L2_calo_et         = emCluster->et();
  m_trig_L2_calo_eta        = emCluster->eta();
  m_trig_L2_calo_phi        = emCluster->phi();
  m_trig_L2_calo_e237       = emCluster->e237();
  m_trig_L2_calo_e277       = emCluster->e277();
  m_trig_L2_calo_fracs1     = emCluster->fracs1();
  m_trig_L2_calo_weta2      = emCluster->weta2();
  m_trig_L2_calo_ehad1      = emCluster->ehad1();
  m_trig_L2_calo_emaxs1     = emCluster->emaxs1();
  m_trig_L2_calo_e2tsts1    = emCluster->e2tsts1();
  m_trig_L2_calo_wstot      = emCluster->wstot();
  for(unsigned i=0; i<emCluster->energySample().size(); ++i){
    m_trig_L2_calo_energySample->push_back( emCluster->energySample().at(i));
  }

  return true;
}


bool TrigEgammaNavTPNtuple::fillTrigElectron( const xAOD::TrigElectron *trigEl ){

   const xAOD::TrackParticle* trkIter = trigEl->trackParticle();
   if (trkIter==NULL) return false; // disconsider candidates without track
   int algoId = 0;
   if ( trkIter->patternRecoInfo()[xAOD::TrackPatternRecoInfo::FastTrackFinderSeed] ) algoId=9;
   if ( trkIter->patternRecoInfo()[xAOD::TrackPatternRecoInfo::strategyA] )           algoId=5;
   if ( trkIter->patternRecoInfo()[xAOD::TrackPatternRecoInfo::strategyB] )           algoId=6;
   if ( trkIter->patternRecoInfo()[xAOD::TrackPatternRecoInfo::strategyC] )           algoId=7;

   m_trig_L2_el_trackAlgID  ->push_back(algoId);
   m_trig_L2_el_caloEta     ->push_back(trigEl->caloEta()); 
   m_trig_L2_el_trkClusDphi ->push_back(trigEl->trkClusDphi());  
   m_trig_L2_el_trkClusDeta ->push_back(trigEl->trkClusDeta());  
   m_trig_L2_el_pt          ->push_back(trigEl->pt());
   m_trig_L2_el_phi         ->push_back(trigEl->phi());  
   m_trig_L2_el_etOverPt    ->push_back(trigEl->etOverPt());          
   m_trig_L2_el_nTRTHits    ->push_back(trigEl->nTRTHits());        
   m_trig_L2_el_nTRTHiThresholdHits->push_back(trigEl->nTRTHiThresholdHits());
   m_trig_L2_el_charge      ->push_back(trigEl->charge());      
   return true;
}



bool TrigEgammaNavTPNtuple::fillEvent(){
  ///Event information
  m_runNumber               = m_eventInfo->runNumber();
  m_eventNumber             = m_eventInfo->eventNumber();
  if(m_lumiTool)    m_avgmu = m_lumiTool->lbAverageInteractionsPerCrossing();
  return true;
}

bool TrigEgammaNavTPNtuple::fillElectron( const xAOD::Electron *el ){

  ///Cluster information
  m_el_et                   = el->pt();
  m_el_eta                  = el->eta();
  m_el_phi                  = el->phi();
  el->showerShapeValue( m_el_ethad1   , xAOD::EgammaParameters::ShowerShapeType::ethad1   ); 
  el->showerShapeValue( m_el_ehad1    , xAOD::EgammaParameters::ShowerShapeType::ehad1    );
  el->showerShapeValue( m_el_f1       , xAOD::EgammaParameters::ShowerShapeType::f1       );
  el->showerShapeValue( m_el_f3       , xAOD::EgammaParameters::ShowerShapeType::f3       );
  el->showerShapeValue( m_el_f1core   , xAOD::EgammaParameters::ShowerShapeType::f1core   );
  el->showerShapeValue( m_el_f3core   , xAOD::EgammaParameters::ShowerShapeType::f3core   );
  el->showerShapeValue( m_el_weta2    , xAOD::EgammaParameters::ShowerShapeType::weta2    );
  el->showerShapeValue( m_el_wtots1   , xAOD::EgammaParameters::ShowerShapeType::wtots1  );
  el->showerShapeValue( m_el_fracs1   , xAOD::EgammaParameters::ShowerShapeType::fracs1   );
  el->showerShapeValue( m_el_Reta     , xAOD::EgammaParameters::ShowerShapeType::Reta     );
  el->showerShapeValue( m_el_Rphi     , xAOD::EgammaParameters::ShowerShapeType::Rphi     );
  el->showerShapeValue( m_el_Eratio   , xAOD::EgammaParameters::ShowerShapeType::Eratio   );
  el->showerShapeValue( m_el_Rhad     , xAOD::EgammaParameters::ShowerShapeType::Rhad     );
  el->showerShapeValue( m_el_Rhad1    , xAOD::EgammaParameters::ShowerShapeType::Rhad1    );

  ///Combined track/Cluter information
  el->trackCaloMatchValue( m_el_deta1     , xAOD::EgammaParameters::TrackCaloMatchType::deltaEta1   );
  el->trackCaloMatchValue( m_el_deta2     , xAOD::EgammaParameters::TrackCaloMatchType::deltaEta2   );
  el->trackCaloMatchValue( m_el_dphi2     , xAOD::EgammaParameters::TrackCaloMatchType::deltaPhi2   );
  el->trackCaloMatchValue( m_el_dphiresc  , xAOD::EgammaParameters::TrackCaloMatchType::deltaPhiRescaled0 );
  el->trackCaloMatchValue( m_el_dphiresc2 , xAOD::EgammaParameters::TrackCaloMatchType::deltaPhiRescaled2 );


  ///Track particle information
  const xAOD::TrackParticle *track = el->trackParticle();
  if(track){
    m_el_pt                   = track->pt();
    m_el_d0                   = fabsf(track->d0());
    m_el_charge               = el->charge();
    
    track->summaryValue( m_el_eprobht            , xAOD::SummaryType::eProbabilityHT );
    track->summaryValue( m_el_nblayerhits        , xAOD::SummaryType::numberOfBLayerHits );
    track->summaryValue( m_el_nblayerolhits      , xAOD::SummaryType::numberOfBLayerOutliers );
    track->summaryValue( m_el_npixhits           , xAOD::SummaryType::numberOfPixelHits );
    track->summaryValue( m_el_npixolhits         , xAOD::SummaryType::numberOfPixelOutliers );
    track->summaryValue( m_el_nscthits           , xAOD::SummaryType::numberOfSCTHits );
    track->summaryValue( m_el_nsctolhits         , xAOD::SummaryType::numberOfSCTOutliers );
    track->summaryValue( m_el_ntrthightreshits   , xAOD::SummaryType::numberOfTRTHighThresholdHits );
    track->summaryValue( m_el_ntrthits           , xAOD::SummaryType::numberOfTRTHits);
    track->summaryValue( m_el_ntrthighthresolhits, xAOD::SummaryType::numberOfTRTHighThresholdOutliers );
    track->summaryValue( m_el_ntrtolhits         , xAOD::SummaryType::numberOfTRTOutliers );
    track->summaryValue( m_el_ntrtxenonhits      , xAOD::SummaryType::numberOfTRTXenonHits );
    //track->summaryValue( m_el_expectblayerhit    , xAOD::SummaryType::expectBLayerHit );
    track->summaryValue( m_el_npixdeadsensors    , xAOD::SummaryType::numberOfPixelDeadSensors);
    track->summaryValue( m_el_nsctdeadsensors    , xAOD::SummaryType::numberOfSCTDeadSensors);

    m_el_nsideadsensors       = m_el_npixdeadsensors  + m_el_nsctdeadsensors;
    m_el_nsihits              = m_el_npixhits         + m_el_nscthits;
  }

  //m_el_loose              = el->passSelection("Loose");
  //m_el_medium             = el->passSelection("Medium");
  //m_el_tight              = el->passSelection("Tight");
  //m_el_lhLoose            = el->passSelection("LHLoose");
  //m_el_lhMedium           = el->passSelection("LHMedium");
  //m_el_lhTight            = el->passSelection("LHTight"); 
  //m_el_rgLoose            = el->passSelection("RGLoose");
  //m_el_rgMedium           = el->passSelection("RGMedium");
  //m_el_rgTight            = el->passSelection("RGTight");
  //m_el_multiLepton        = el->passSelection("MultiLepton");
  m_el_loose                = ApplyElectronPid(el, "Loose");
  m_el_medium               = ApplyElectronPid(el, "Medium");
  m_el_tight                = ApplyElectronPid(el, "Tight");
  m_el_lhLoose              = ApplyElectronPid(el, "LHLoose");
  m_el_lhMedium             = ApplyElectronPid(el, "LHMedium");
  m_el_lhTight              = ApplyElectronPid(el, "LHTight"); 
  //m_el_rgLoose            = ApplyElectronPid(el, "RGLoose");
  //m_el_rgMedium           = ApplyElectronPid(el, "RGMedium");
  //m_el_rgTight            = ApplyElectronPid(el, "RGTight");
  
  m_calo_et                 = getCluster_et( el );
  m_calo_eta                = getCluster_eta( el );
  m_calo_phi                = getCluster_phi( el );
  ///Extra information about pileup
  m_el_nGoodVtx             = m_nGoodVtx;
  m_el_nPileupPrimaryVtx    = m_nPileupPrimaryVtx;


  if(!fillCaloRings( el )){
    ATH_MSG_WARNING("Could not attach the calorRings information.");
  }

  return true;
}


/*bool TrigEgammaNavTPNtuple::fillPhoton( const xAOD::Photon *ph ){
  return true;
}*/

bool TrigEgammaNavTPNtuple::fillMonteCarlo(const xAOD::Egamma *eg){

  bool Zfound,Wfound = false;
  if(m_truthContainer){
    auto mc = matchTruth(m_truthContainer,eg,Zfound, Wfound);
    if(mc){
      m_mc_hasMC        = true;
      m_mc_pt           = mc->pt();
      m_mc_eta          = mc->eta();
      m_mc_phi          = mc->phi();
      m_mc_isTop        = mc->isTop();
      m_mc_isQuark      = mc->isQuark();
      m_mc_isParton     = mc->isParton();
      m_mc_isMeson      = mc->isMeson();
      m_mc_isTau        = mc->isTau();
      m_mc_isMuon       = mc->isMuon();
      m_mc_isPhoton     = mc->isPhoton();
      m_mc_isElectron   = mc->isElectron();
      m_mc_hasZMother   = Zfound;
      m_mc_hasWMother   = Wfound;
      return true;
    }//has match
  }//has truth container
  return false;
}



bool TrigEgammaNavTPNtuple::fillTrigCaloRings( const xAOD::TrigEMCluster *emCluster ){
  return TrigEgammaAnalysisBaseTool::getTrigCaloRings(emCluster, *m_trig_L2_calo_rings);
}


bool TrigEgammaNavTPNtuple::fillCaloRings( const xAOD::Electron *el ){
  return TrigEgammaAnalysisBaseTool::getCaloRings(el, *m_el_ringsE );
}

/*
 * book, link, clear, alloc and release method divide in:
 * trigger, Egamma and MonteCarlo data base.
 */

template <class T> 
void TrigEgammaNavTPNtuple::InitBranch(TTree* fChain, std::string branch_name, T* param, bool message){

  std::string bname = branch_name;
  if (fChain->GetAlias(bname.c_str()))
     bname = std::string(fChain->GetAlias(bname.c_str()));

  if (!fChain->FindBranch(bname.c_str()) && message) {
    ATH_MSG_WARNING("unknown branch " << bname);
    return;  
  }
  fChain->SetBranchStatus(bname.c_str(), 1.);
  fChain->SetBranchAddress(bname.c_str(), param);
}

void TrigEgammaNavTPNtuple::bookEventBranches(TTree *t){
  
  t->Branch("RunNumber",        &m_runNumber);
  t->Branch("EventNumber",      &m_eventNumber);
  t->Branch("avgmu",            &m_avgmu);
}

void TrigEgammaNavTPNtuple::bookTriggerBranches(TTree *t){

  // Level L1 cluster
  t->Branch( "trig_L1_eta",             &m_trig_L1_eta);
  t->Branch( "trig_L1_phi",             &m_trig_L1_phi);
  t->Branch( "trig_L1_emClus",          &m_trig_L1_emClus);
  t->Branch( "trig_L1_tauClus",         &m_trig_L1_tauClus);
  t->Branch( "trig_L1_emIsol",          &m_trig_L1_emIsol);
  t->Branch( "trig_L1_hadIsol",         &m_trig_L1_hadIsol);
  t->Branch( "trig_L1_thrNames",        &m_trig_L1_thrNames);
  t->Branch( "trig_L1_accept",          &m_trig_L1_accept);
  t->Branch( "trig_L2_calo_et",         &m_trig_L2_calo_et);
  t->Branch( "trig_L2_calo_eta",        &m_trig_L2_calo_eta);
  t->Branch( "trig_L2_calo_phi",        &m_trig_L2_calo_phi);
  t->Branch( "trig_L2_calo_e237",       &m_trig_L2_calo_e237 );
  t->Branch( "trig_L2_calo_e277",       &m_trig_L2_calo_e277 );
  t->Branch( "trig_L2_calo_fracs1",     &m_trig_L2_calo_fracs1);
  t->Branch( "trig_L2_calo_weta2",      &m_trig_L2_calo_weta2);
  t->Branch( "trig_L2_calo_ehad1",      &m_trig_L2_calo_ehad1);
  t->Branch( "trig_L2_calo_emaxs1",     &m_trig_L2_calo_emaxs1);
  t->Branch( "trig_L2_calo_e2tsts1",    &m_trig_L2_calo_e2tsts1);
  t->Branch( "trig_L2_calo_wstot",      &m_trig_L2_calo_wstot);
  t->Branch( "trig_L2_calo_energySample",&m_trig_L2_calo_energySample ); 
  t->Branch( "trig_L2_calo_rnnOutput",  &m_trig_L2_calo_rnnOutput ); 
  t->Branch( "trig_L2_calo_rings",      &m_trig_L2_calo_rings ); 
  t->Branch( "trig_L2_calo_accept",     &m_trig_L2_calo_accept);
  t->Branch( "trig_L2_el_trackAlgID" ,         &m_trig_L2_el_trackAlgID );
  t->Branch( "trig_L2_el_pt" ,                 &m_trig_L2_el_pt );
  t->Branch( "trig_L2_el_eta",                 &m_trig_L2_el_eta);
  t->Branch( "trig_L2_el_phi",                 &m_trig_L2_el_phi );
  t->Branch( "trig_L2_el_caloEta",             &m_trig_L2_el_caloEta );
  t->Branch( "trig_L2_el_charge",              &m_trig_L2_el_charge);
  t->Branch( "trig_L2_el_nTRTHits",            &m_trig_L2_el_nTRTHits);
  t->Branch( "trig_L2_el_nTRTHiThresholdHits", &m_trig_L2_el_nTRTHiThresholdHits);
  t->Branch( "trig_L2_el_etOverPt" ,           &m_trig_L2_el_etOverPt );
  t->Branch( "trig_L2_el_trkClusDeta" ,        &m_trig_L2_el_trkClusDeta );
  t->Branch( "trig_L2_el_trkClusDphi" ,        &m_trig_L2_el_trkClusDphi );
  t->Branch( "trig_L2_el_accept",       &m_trig_L2_el_accept );
  t->Branch( "trig_EF_calo_accept",     &m_trig_EF_calo_accept);
  t->Branch( "trig_EF_el_accept",       &m_trig_EF_el_accept);
}

void TrigEgammaNavTPNtuple::bookElectronBranches(TTree *t){
 
  t->Branch("el_et",                    &m_el_et);
  t->Branch("el_pt",                    &m_el_pt);
  t->Branch("el_eta",                   &m_el_eta);
  t->Branch("el_phi",                   &m_el_phi);
  t->Branch("el_ethad1",                &m_el_ethad1);
  t->Branch("el_ehad1",                 &m_el_ehad1);
  t->Branch("el_f1",                    &m_el_f1);
  t->Branch("el_f3",                    &m_el_f3);
  t->Branch("el_f1core",                &m_el_f1core);
  t->Branch("el_f3core",                &m_el_f3core);
  t->Branch("el_weta2",                 &m_el_weta2);
  t->Branch("el_d0",                    &m_el_d0);
  t->Branch("el_wtots1",                &m_el_wtots1);
  t->Branch("el_fracs1",                &m_el_fracs1);
  t->Branch("el_Reta",                  &m_el_Reta);
  t->Branch("el_Rphi",                  &m_el_Rphi);
  t->Branch("el_Eratio",                &m_el_Eratio);
  t->Branch("el_Rhad",                  &m_el_Rhad);
  t->Branch("el_Rhad1",                 &m_el_Rhad1);
  t->Branch("el_deta1",                 &m_el_deta1);
  t->Branch("el_deta2",                 &m_el_deta2);
  t->Branch("el_dphi2",                 &m_el_dphi2);
  t->Branch("el_dphiresc",              &m_el_dphiresc);
  t->Branch("el_dphiresc2",             &m_el_dphiresc2);
  t->Branch("el_eprobht",               &m_el_eprobht);
  t->Branch("el_charge",                &m_el_charge);
  t->Branch("el_nblayerhits",           &m_el_nblayerhits);
  t->Branch("el_nblayerolhits",         &m_el_nblayerolhits);
  t->Branch("el_npixhits",              &m_el_npixhits);
  t->Branch("el_npixolhits",            &m_el_npixolhits);
  t->Branch("el_nscthits",              &m_el_nscthits);
  t->Branch("el_nsctolhits",            &m_el_nsctolhits);
  t->Branch("el_ntrthightreshits",      &m_el_ntrthightreshits);
  t->Branch("el_ntrthits",              &m_el_ntrthits);
  t->Branch("el_ntrthighthresolhits",   &m_el_ntrthighthresolhits);
  t->Branch("el_ntrtolhits",            &m_el_ntrtolhits);
  t->Branch("el_ntrtxenonhits",         &m_el_ntrtxenonhits);
  t->Branch("el_expectblayerhit",       &m_el_expectblayerhit); 
  t->Branch("el_nsihits",               &m_el_nsihits          );
  t->Branch("el_nsideadsensors",        &m_el_nsideadsensors   );
  t->Branch("el_npixdeadsensors",       &m_el_npixdeadsensors  );
  t->Branch("el_nsctdeadsensors",       &m_el_nsctdeadsensors  );
  t->Branch("el_ringsE",                &m_el_ringsE );
  t->Branch("el_loose",                 &m_el_loose  );
  t->Branch("el_medium",                &m_el_medium );
  t->Branch("el_tight",                 &m_el_tight );
  t->Branch("el_lhLoose",               &m_el_lhLoose );
  t->Branch("el_lhMedium",              &m_el_lhMedium ); 
  t->Branch("el_lhTight",               &m_el_lhTight ); 
  t->Branch("el_rgLoose",               &m_el_rgLoose );
  t->Branch("el_rgMedium",              &m_el_rgMedium ); 
  t->Branch("el_rgTight",               &m_el_rgTight ); 
  t->Branch("el_multiLepton",           &m_el_multiLepton);
  t->Branch("el_nGoodVtx",              &m_el_nGoodVtx);
  t->Branch("el_nPileupPrimaryVtx",     &m_el_nPileupPrimaryVtx);
  t->Branch("calo_et",                  &m_calo_et);
  t->Branch("calo_eta",                 &m_calo_eta);
  t->Branch("calo_phi",                 &m_calo_phi);
 
}


/*void TrigEgammaNavTPNtuple::bookPhotonBranches(TTree *t){
} */ 
  
void TrigEgammaNavTPNtuple::bookMonteCarloBranches(TTree *t){
  // Monte Carlo
  t->Branch("mc_hasMC",       &m_mc_hasMC);
  t->Branch("mc_pt",          &m_mc_pt);
  t->Branch("mc_eta",         &m_mc_eta);
  t->Branch("mc_phi",         &m_mc_phi);
  t->Branch("mc_isTop",       &m_mc_isTop);
  t->Branch("mc_isParton",    &m_mc_isParton);
  t->Branch("mc_isMeson",     &m_mc_isMeson);
  t->Branch("mc_isTau",       &m_mc_isTau);
  t->Branch("mc_isMuon",      &m_mc_isMuon);
  t->Branch("mc_isPhoton",    &m_mc_isPhoton);
  t->Branch("mc_isElectron",  &m_mc_isElectron);
  t->Branch("mc_hasZMother",  &m_mc_hasZMother);
  t->Branch("mc_hasWMother",  &m_mc_hasWMother);
}

void TrigEgammaNavTPNtuple::linkEventBranches(TTree *t){
  
  InitBranch( t, "RunNumber",        &m_runNumber);
  InitBranch( t, "EventNumber",      &m_eventNumber);
  InitBranch( t, "avgmu",            &m_avgmu);
}

void TrigEgammaNavTPNtuple::linkElectronBranches( TTree *t ){
  
  InitBranch( t, "el_et",                    &m_el_et);
  InitBranch( t, "el_pt",                    &m_el_pt);
  InitBranch( t, "el_eta",                   &m_el_eta);
  InitBranch( t, "el_phi",                   &m_el_phi);
  InitBranch( t, "el_ethad1",                &m_el_ethad1);
  InitBranch( t, "el_ehad1",                 &m_el_ehad1);
  InitBranch( t, "el_f1",                    &m_el_f1);
  InitBranch( t, "el_f3",                    &m_el_f3);
  InitBranch( t, "el_f1core",                &m_el_f1core);
  InitBranch( t, "el_f3core",                &m_el_f3core);
  InitBranch( t, "el_weta2",                 &m_el_weta2);
  InitBranch( t, "el_d0",                    &m_el_d0);
  InitBranch( t, "el_wtots1",                &m_el_wtots1);
  InitBranch( t, "el_fracs1",                &m_el_fracs1);
  InitBranch( t, "el_Reta",                  &m_el_Reta);
  InitBranch( t, "el_Rphi",                  &m_el_Rphi);
  InitBranch( t, "el_Eratio",                &m_el_Eratio);
  InitBranch( t, "el_Rhad",                  &m_el_Rhad);
  InitBranch( t, "el_Rhad1",                 &m_el_Rhad1);
  InitBranch( t, "el_deta1",                 &m_el_deta1);
  InitBranch( t, "el_deta2",                 &m_el_deta2);
  InitBranch( t, "el_dphi2",                 &m_el_dphi2);
  InitBranch( t, "el_dphiresc",              &m_el_dphiresc);
  InitBranch( t, "el_dphiresc2",             &m_el_dphiresc2);
  InitBranch( t, "el_eprobht",               &m_el_eprobht);
  InitBranch( t, "el_charge",                &m_el_charge);
  InitBranch( t, "el_nblayerhits",           &m_el_nblayerhits);
  InitBranch( t, "el_nblayerolhits",         &m_el_nblayerolhits);
  InitBranch( t, "el_npixhits",              &m_el_npixhits);
  InitBranch( t, "el_npixolhits",            &m_el_npixolhits);
  InitBranch( t, "el_nscthits",              &m_el_nscthits);
  InitBranch( t, "el_nsctolhits",            &m_el_nsctolhits);
  InitBranch( t, "el_ntrthightreshits",      &m_el_ntrthightreshits);
  InitBranch( t, "el_ntrthits",              &m_el_ntrthits);
  InitBranch( t, "el_ntrthighthresolhits",   &m_el_ntrthighthresolhits);
  InitBranch( t, "el_ntrtolhits",            &m_el_ntrtolhits);
  InitBranch( t, "el_ntrtxenonhits",         &m_el_ntrtxenonhits);
  InitBranch( t, "el_expectblayerhit",       &m_el_expectblayerhit); 
  InitBranch( t, "el_nsihits",               &m_el_nsihits          );
  InitBranch( t, "el_nsideadsensors",        &m_el_nsideadsensors   );
  InitBranch( t, "el_npixdeadsensors",       &m_el_npixdeadsensors  );
  InitBranch( t, "el_nsctdeadsensors",       &m_el_nsctdeadsensors  );
  InitBranch( t, "el_ringsE",                &m_el_ringsE );
  InitBranch( t, "el_loose",                 &m_el_loose  );
  InitBranch( t, "el_medium",                &m_el_medium );
  InitBranch( t, "el_tight",                 &m_el_tight );
  InitBranch( t, "el_lhLoose",               &m_el_lhLoose );
  InitBranch( t, "el_lhMedium",              &m_el_lhMedium ); 
  InitBranch( t, "el_lhTight",               &m_el_lhTight ); 
  InitBranch( t, "el_rgLoose",               &m_el_rgLoose );
  InitBranch( t, "el_rgMedium",              &m_el_rgMedium ); 
  InitBranch( t, "el_rgTight",               &m_el_rgTight ); 
  InitBranch( t, "el_multiLepton",           &m_el_multiLepton);
  InitBranch( t, "el_nGoodVtx",              &m_el_nGoodVtx);
  InitBranch( t, "el_nPileupPrimaryVtx",     &m_el_nPileupPrimaryVtx);
  InitBranch( t, "calo_et",                  &m_calo_et);
  InitBranch( t, "calo_eta",                 &m_calo_eta);
  InitBranch( t, "calo_phi",                 &m_calo_phi);

}


/*void TrigEgammaNavTPNtuple::linkPhotonBranches( TTree *t ){
}*/

void TrigEgammaNavTPNtuple::linkTriggerBranches( TTree *t ){

  InitBranch(t, "trig_L1_eta",             &m_trig_L1_eta);
  InitBranch(t, "trig_L1_phi",             &m_trig_L1_phi);
  InitBranch(t, "trig_L1_emClus",          &m_trig_L1_emClus);
  InitBranch(t, "trig_L1_tauClus",         &m_trig_L1_tauClus);
  InitBranch(t, "trig_L1_emIsol",          &m_trig_L1_emIsol);
  InitBranch(t, "trig_L1_hadIsol",         &m_trig_L1_hadIsol);
  InitBranch(t, "trig_L1_thrNames",        &m_trig_L1_thrNames);
  InitBranch(t, "trig_L1_accept",          &m_trig_L1_accept);
  InitBranch(t, "trig_L2_calo_et",         &m_trig_L2_calo_et);
  InitBranch(t, "trig_L2_calo_eta",        &m_trig_L2_calo_eta);
  InitBranch(t, "trig_L2_calo_phi",        &m_trig_L2_calo_phi);
  InitBranch(t, "trig_L2_calo_e237",       &m_trig_L2_calo_e237 );
  InitBranch(t, "trig_L2_calo_e277",       &m_trig_L2_calo_e277 );
  InitBranch(t, "trig_L2_calo_fracs1",     &m_trig_L2_calo_fracs1);
  InitBranch(t, "trig_L2_calo_weta2",      &m_trig_L2_calo_weta2);
  InitBranch(t, "trig_L2_calo_ehad1",      &m_trig_L2_calo_ehad1);
  InitBranch(t, "trig_L2_calo_emaxs1",     &m_trig_L2_calo_emaxs1);
  InitBranch(t, "trig_L2_calo_e2tsts1",    &m_trig_L2_calo_e2tsts1);
  InitBranch(t, "trig_L2_calo_wstot",      &m_trig_L2_calo_wstot);
  InitBranch(t, "trig_L2_calo_rnnOutput",      &m_trig_L2_calo_rnnOutput ); 
  InitBranch(t, "trig_L2_calo_rings",      &m_trig_L2_calo_rings ); 
  InitBranch(t, "trig_L2_calo_energySample",&m_trig_L2_calo_energySample ); 
  InitBranch(t, "trig_L2_calo_accept",     &m_trig_L2_calo_accept);

  InitBranch(t, "trig_L2_el_trackAlgID" ,         &m_trig_L2_el_trackAlgID );
  InitBranch(t, "trig_L2_el_pt" ,                 &m_trig_L2_el_pt );
  InitBranch(t, "trig_L2_el_eta",                 &m_trig_L2_el_eta);
  InitBranch(t, "trig_L2_el_phi",                 &m_trig_L2_el_phi );
  InitBranch(t, "trig_L2_el_caloEta",             &m_trig_L2_el_caloEta );
  InitBranch(t, "trig_L2_el_charge",              &m_trig_L2_el_charge);
  InitBranch(t, "trig_L2_el_nTRTHits",            &m_trig_L2_el_nTRTHits);
  InitBranch(t, "trig_L2_el_nTRTHiThresholdHits", &m_trig_L2_el_nTRTHiThresholdHits);
  InitBranch(t, "trig_L2_el_etOverPt" ,           &m_trig_L2_el_etOverPt );
  InitBranch(t, "trig_L2_el_trkClusDeta" ,        &m_trig_L2_el_trkClusDeta );
  InitBranch(t, "trig_L2_el_trkClusDphi" ,        &m_trig_L2_el_trkClusDphi );

  InitBranch(t, "trig_L2_el_accept",       &m_trig_L2_el_accept );
  InitBranch(t, "trig_EF_el_accept",       &m_trig_EF_el_accept );
  InitBranch(t, "trig_EF_calo_accept",     &m_trig_EF_calo_accept );
 
}


void TrigEgammaNavTPNtuple::linkMonteCarloBranches(TTree *t){
 
  InitBranch(t, "mc_hasMC",       &m_mc_hasMC);
  InitBranch(t, "mc_pt",          &m_mc_pt);
  InitBranch(t, "mc_eta",         &m_mc_eta);
  InitBranch(t, "mc_phi",         &m_mc_phi);
  InitBranch(t, "mc_isTop",       &m_mc_isTop);
  InitBranch(t, "mc_isParton",    &m_mc_isParton);
  InitBranch(t, "mc_isMeson",     &m_mc_isMeson);
  InitBranch(t, "mc_isTau",       &m_mc_isTau);
  InitBranch(t, "mc_isMuon",      &m_mc_isMuon);
  InitBranch(t, "mc_isPhoton",    &m_mc_isPhoton);
  InitBranch(t, "mc_isElectron",  &m_mc_isElectron);
  InitBranch(t, "mc_hasZMother",  &m_mc_hasZMother);
  InitBranch(t, "mc_hasWMother",  &m_mc_hasWMother); 
}

void TrigEgammaNavTPNtuple::clear(){

  ///EventInfo
  m_runNumber             = 0;
  m_eventNumber           = 0;
  m_avgmu                 = 0;
  ///Egamma
  m_el_et                 = -1;
  m_el_pt                 = -1;
  m_el_eta                = -1;
  m_el_phi                = -1;
  m_el_ethad1             = -1;
  m_el_ehad1              = -1;
  m_el_f1                 = -1;
  m_el_f3                 = -1;
  m_el_f1core             = -1;
  m_el_f3core             = -1;
  m_el_weta2              = -1;
  m_el_d0                 = -1;
  m_el_wtots1             = -1;
  m_el_fracs1             = -1;
  m_el_Reta               = -1;
  m_el_Rphi               = -1;
  m_el_Eratio             = -1;
  m_el_Rhad               = -1;
  m_el_Rhad1              = -1;
  m_el_deta1              = -1;
  m_el_deta2              = -1;
  m_el_dphi2              = -1;
  m_el_dphiresc           = -1;
  m_el_dphiresc2          = -1;
  m_el_eprobht            = -1;
  m_el_charge             =  0;
  m_el_nblayerhits        =  0;
  m_el_nblayerolhits      =  0;
  m_el_npixhits           =  0;
  m_el_npixolhits         =  0;
  m_el_nscthits           =  0;
  m_el_nsctolhits         =  0;
  m_el_ntrthightreshits   =  0;
  m_el_ntrthits           =  0;
  m_el_ntrthighthresolhits=  0;
  m_el_ntrtolhits         =  0;
  m_el_ntrtxenonhits      =  0;
  m_el_expectblayerhit    =  false;
  m_el_nsihits            =  0;
  m_el_nsideadsensors     =  0;
  m_el_npixdeadsensors    =  0;
  m_el_nsctdeadsensors    =  0;
  m_el_loose              = false; 
  m_el_medium             = false; 
  m_el_tight              = false; 
  m_el_lhLoose            = false; 
  m_el_lhMedium           = false; 
  m_el_lhTight            = false;  
  m_el_rgLoose            = false; 
  m_el_rgMedium           = false; 
  m_el_rgTight            = false;  
  m_el_multiLepton        = false;
  m_el_nGoodVtx           = -1;
  m_el_nPileupPrimaryVtx  = -1;
  m_calo_et               = -1; 
  m_calo_eta              = -1; 
  m_calo_phi              = -1; 

  ///Trigger
  m_trig_L1_eta           = -1;
  m_trig_L1_phi           = -1;
  m_trig_L1_emClus        = -1;
  m_trig_L1_tauClus       = -1;
  m_trig_L1_emIsol        = -1;
  m_trig_L1_hadIsol       = -1;
  m_trig_L1_accept        = false;
  m_trig_L2_calo_et       = -1;
  m_trig_L2_calo_eta      = -1;
  m_trig_L2_calo_phi      = -1;
  m_trig_L2_calo_e237     = -1;
  m_trig_L2_calo_e277     = -1;
  m_trig_L2_calo_fracs1   = -1;
  m_trig_L2_calo_weta2    = -1;
  m_trig_L2_calo_ehad1    = -1;
  m_trig_L2_calo_emaxs1   = -1;
  m_trig_L2_calo_e2tsts1  = -1;
  m_trig_L2_calo_wstot    = -1; 
  m_trig_L2_calo_rnnOutput    = 999; 

  m_trig_L1_accept        = false;
  m_trig_L2_calo_accept   = false;
  m_trig_L2_el_accept     = false; 
  m_trig_EF_calo_accept   = false;
  m_trig_EF_el_accept     = false;

  ///Monte Carlo
  m_mc_hasMC              = false;
  m_mc_pt                 = -1;
  m_mc_eta                = -1;
  m_mc_phi                = -1;
  m_mc_isTop              = false;
  m_mc_isParton           = false;
  m_mc_isMeson            = false;
  m_mc_isTau              = false;
  m_mc_isMuon             = false;
  m_mc_isPhoton           = false;
  m_mc_isElectron         = false;
  m_mc_hasZMother         = false;
  m_mc_hasWMother         = false;

  ///Some vectors
  m_trig_L1_thrNames                ->clear(); 
  m_trig_L2_calo_energySample       ->clear();
  m_trig_L2_calo_rings              ->clear();  

  m_trig_L2_el_trackAlgID          ->clear(); 
  m_trig_L2_el_eta                 ->clear(); 
  m_trig_L2_el_phi                 ->clear(); 
  m_trig_L2_el_caloEta             ->clear(); 
  m_trig_L2_el_trkClusDeta         ->clear(); 
  m_trig_L2_el_trkClusDphi         ->clear(); 
  m_trig_L2_el_pt                  ->clear(); 
  m_trig_L2_el_etOverPt            ->clear();         
  m_trig_L2_el_nTRTHits            ->clear();       
  m_trig_L2_el_nTRTHiThresholdHits ->clear();       
  m_trig_L2_el_charge              ->clear();     
 

}

void TrigEgammaNavTPNtuple::alloc_space(){

  m_el_ringsE                       = new std::vector<float>();
  m_trig_L1_thrNames                = new std::vector<std::string>();
  m_trig_L2_calo_energySample       = new std::vector<float>();
  m_trig_L2_calo_rings              = new std::vector<float>();
  m_trig_L2_el_trackAlgID           = new std::vector<int>();
  m_trig_L2_el_eta                  = new std::vector<float>();   
  m_trig_L2_el_phi                  = new std::vector<float>();   
  m_trig_L2_el_caloEta              = new std::vector<float>();   
  m_trig_L2_el_trkClusDeta          = new std::vector<float>();
  m_trig_L2_el_trkClusDphi          = new std::vector<float>();
  m_trig_L2_el_pt                   = new std::vector<float>();   
  m_trig_L2_el_etOverPt             = new std::vector<float>();           
  m_trig_L2_el_nTRTHits             = new std::vector<float>();         
  m_trig_L2_el_nTRTHiThresholdHits  = new std::vector<float>();         
  m_trig_L2_el_charge               = new std::vector<float>();       
  
}


void TrigEgammaNavTPNtuple::release_space(){
  delete m_el_ringsE              ;
  delete m_trig_L1_thrNames       ;
  delete m_trig_L2_calo_energySample;
  delete m_trig_L2_calo_rings     ;
  delete m_trig_L2_el_trackAlgID  ;  
  delete m_trig_L2_el_pt          ;  
  delete m_trig_L2_el_eta         ;   
  delete m_trig_L2_el_caloEta     ;   
  delete m_trig_L2_el_phi         ;   
  delete m_trig_L2_el_charge      ;       
  delete m_trig_L2_el_nTRTHits    ;         
  delete m_trig_L2_el_nTRTHiThresholdHits ;         
  delete m_trig_L2_el_etOverPt    ;           
  delete m_trig_L2_el_trkClusDeta ;
  delete m_trig_L2_el_trkClusDphi ;
}



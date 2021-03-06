#include "./CTPEmulation.h"

#include "TrigConfL1Data/CTPConfig.h"
#include "TrigConfL1Data/L1DataDef.h"
#include "TrigConfL1Data/ClusterThresholdValue.h"
#include "TrigT1Interfaces/TrigT1StoreGateKeys.h"

#include "TrigT1CTP/CTPTriggerThreshold.h"
#include "TrigT1CTP/CTPTriggerItem.h"
#include "TrigT1CTP/BunchGroupTrigger.h"
#include "TrigT1CTP/RandomTrigger.h"
#include "TrigT1CTP/CTPUtil.h"

#include "TrigT1Result/CTP_RDO.h"
#include "TrigT1Interfaces/CTPSLink.h"
#include "TrigSteering/Lvl1ResultAccessTool.h"
#include "TrigT1Result/RoIBResult.h"
#include "TrigT1Result/JetEnergyRoI.h"
#include "TrigT1CaloUtils/CoordToHardware.h"

#include "TrigT1Interfaces/CPRoIDecoder.h"
#include "TrigT1Interfaces/JEPRoIDecoder.h"

#include "TH2.h"

#include <iomanip>
#include <algorithm>
#include <cmath>

using namespace TrigConf;

LVL1CTP::CTPEmulation::CTPEmulation( const std::string& name, ISvcLocator* pSvcLocator ) :
   AthAlgorithm ( name, pSvcLocator ), 
   m_histSvc ("THistSvc", name),
   m_configSvc ("TrigConf::TrigConfigSvc/TrigConfigSvc", name),
   m_rndmSvc("AtRndmGenSvc",name),
   m_lvl1Tool("HLT::Lvl1ResultAccessTool/Lvl1ResultAccessTool",this),
   m_itemCountsSumTBP(512,0),
   m_itemCountsSumTAP(512,0),
   m_itemCountsSumTAV(512,0)      
{
   declareProperty( "HistogramSvc", m_histSvc, "Histogram Service");
   declareProperty( "HistogramStream", m_histStream, "Histogram stream name, bound to file by THistSvc");
   declareProperty( "TrigConfigSvc", m_configSvc, "Trigger configuration service");

   declareProperty( "UseCTPInputFromData", m_useCTPInput, "Set true if using CTP input objects from the various L1 simulations");
   declareProperty( "UseCTPInputFromData", m_useROIBOutput, "Set to true the ROIBResult should be used for Run 2 data (not likely)");

   // input data
   declareProperty( "gFEXMETPufitInput", m_gFEXMETPufitLoc, "StoreGate location of gFEX PUfit MET input" );
   declareProperty( "gFEXMETRhoInput", m_gFEXMETRhoLoc, "StoreGate location of gFEX rho MET input" );
   declareProperty( "gFEXMETJwoJInput", m_gFEXMETJwoJLoc, "StoreGate location of gFEX JwoJ MET input" );
   declareProperty( "jJetInput", m_jJetLoc, "StoreGate location of jFEX Jet inputs" );
   declareProperty( "jLJetInput", m_jLJetLoc, "StoreGate location of jFEX large-R Jet inputs" );
   declareProperty( "gJetInput", m_gJetLoc, "StoreGate location of gFEX Jet inputs" );
   declareProperty( "eFEXClusterInput", m_eFEXClusterLoc, "StoreGate location of eFEX Cluster inputs" );
   
   // muon threshold counts from CTP input recorded in data
   declareProperty( "MuonCTPInput", m_muonCTPLoc, "StoreGate location of Muon inputs" );
   declareProperty( "EmTauCTPLocation", m_emtauCTPLoc, "StoreGate location of EmTau inputs" );
   declareProperty( "JetCTPLocation", m_jetCTPLoc, "StoreGate location of Jet inputs" );
   declareProperty( "EnergyCTPLocation", m_energyCTPLoc, "StoreGate location of Energy inputs" );
   declareProperty( "TopoCTPLocation", m_topoCTPLoc, "StoreGate location of topo inputs" );

   // output locations and control
   declareProperty( "IsData", m_isData, "emulate CTP as part of MC or rerun on data" );  
   declareProperty( "RoIOutputLocation", m_roiOutputLoc, "StoreGate location of CTP RoI (sim)");
   declareProperty( "RoIOutputLocation_Rerun", m_roiOutputLoc_Rerun, "StoreGate location of rerun CTP RoI (data)");
   declareProperty( "RDOOutputLocation", m_rdoOutputLoc, "StoreGate location of CTP RDO (sim)" );
   declareProperty( "RDOOutputRerunLocation", m_rdoOutputLoc_Rerun, "StoreGate location of rerun CTP RDO (data)" );

   m_decoder = new LVL1::CPRoIDecoder();
   m_jetDecoder = new LVL1::JEPRoIDecoder();
}


LVL1CTP::CTPEmulation::~CTPEmulation()
{}


StatusCode
LVL1CTP::CTPEmulation::initialize() {

   CHECK(m_configSvc.retrieve());
   
   if ( m_useROIBOutput ) {
      CHECK(m_lvl1Tool.retrieve());
   }

   CHECK(m_histSvc.retrieve());

   m_thrMap = new ThresholdMap( m_configSvc->ctpConfig()->menu().thresholdVector());

   m_itemMap = new ItemMap( m_configSvc->ctpConfig()->menu().itemVector(), 
                            m_configSvc->ctpConfig()->prescaleSet(),
                            nullptr );

   CHECK( createInternalTriggerMap() );

   // print out configuration
   // printConfiguration().ignore();

   return StatusCode::SUCCESS;
}


StatusCode
LVL1CTP::CTPEmulation::createInternalTriggerMap() {

   unsigned int ctpVersion ( 4 );

   // declare bunch group internal triggers
   const std::vector<TrigConf::BunchGroup> & bunchGroups(m_configSvc->ctpConfig()->bunchGroupSet().bunchGroups());
   ATH_MSG_DEBUG("Defining bunch group internal trigger");
   for (size_t i(0); i < bunchGroups.size(); ++i) {
      InternalTriggerMap::key_type trigtype = std::make_pair(TrigConf::L1DataDef::BGRP,i);
      m_internalTrigger[trigtype] = new LVL1CTP::BunchGroupTrigger(i, bunchGroups[i].bunches(),ctpVersion);
   }


   // get random engine
   CHECK(m_rndmSvc.retrieve());
   CLHEP::HepRandomEngine* rndmEngine = m_rndmSvc->GetEngine("CTPEmulation");
   if ( rndmEngine == nullptr ) {
      ATH_MSG_ERROR("Could not find RndmEngine CTPEmulation");
      return StatusCode::FAILURE;
   }
   const TrigConf::Random random(m_configSvc->ctpConfig()->random());
   ATH_MSG_DEBUG("Random trigger definition: " << random.name()
                 << std::setw(8) << random.cuts(0)
                 << std::setw(8) << random.cuts(1)
                 << std::setw(8) << random.cuts(2)
                 << std::setw(8) << random.cuts(3));

   for(int rndmIdx = 0; rndmIdx<4; rndmIdx++) {
      uint32_t cut = random.cuts(rndmIdx);
      if(cut>=0x1000000) { cut = 0xFFFFFF; }
      if(cut==0) { cut = 0x1; }
      double prescale = double(0xFFFFFF) / (0x1000000-cut);
      ATH_MSG_INFO("REGTEST - Cut for random trigger  RNDM " << rndmIdx << " : " << "0x" << std::hex << cut << std::dec << " (" << cut << ")");
      ATH_MSG_INFO("REGTEST - PS (from 40.08MHz)           " << rndmIdx << " : " << prescale);
      ATH_MSG_INFO("REGTEST - Rate                         " << rndmIdx << " : " << 40080./prescale << " kHz");
      m_internalTrigger[ std::make_pair(TrigConf::L1DataDef::RNDM,rndmIdx)] = new RandomTrigger(rndmIdx, (unsigned int)prescale, ctpVersion, rndmEngine);
   }

   return StatusCode::SUCCESS;
}


StatusCode
LVL1CTP::CTPEmulation::printConfiguration() const {
   ATH_MSG_DEBUG("Mapped decision threshold objects to configuration ones");
   ATH_MSG_DEBUG("          |--------------------------------------------------------|");
   ATH_MSG_DEBUG("          |              TIP layout for the thresholds             |");
   ATH_MSG_DEBUG("          |--------------------------------------------------------|");
   ATH_MSG_DEBUG("          |             Name         |   startbit   |    endbit    |");
   ATH_MSG_DEBUG("          |--------------------------------------------------------|");   
   for( TrigConf::TriggerThreshold * threshold : m_configSvc->ctpConfig()->menu().thresholdVector() ) {
      ATH_MSG_DEBUG( "REGTEST - |   " << std::setw( 20 ) << threshold->name() << "   |   "
                     << std::setw( 8 ) << m_thrMap->decision( threshold )->startBit() << "   |   "
                     << std::setw( 8 ) << m_thrMap->decision( threshold )->endBit() << "   |");
   }
   ATH_MSG_DEBUG("          |--------------------------------------------------------|");
   for (InternalTriggerMap::value_type internalThr : m_internalTrigger) {
      ATH_MSG_DEBUG("REGTEST - |   " << std::setw( 20 ) << L1DataDef::typeAsString(internalThr.first.first) << internalThr.first.second << "   |   " 
                    << std::setw( 8 ) << internalThr.second->pit() << "   |   " << std::setw( 8 ) << internalThr.second->pit() << "   |");
   }
   ATH_MSG_DEBUG("          |--------------------------------------------------------|");   

   ATH_MSG_DEBUG("Mapped decision trigger item objects to configuration ones");
   ATH_MSG_DEBUG("          |---------------------------------------------------------------------------------|");
   ATH_MSG_DEBUG("          |                             Layout of trigger items in result                   |");
   ATH_MSG_DEBUG("          |---------------------------------------------------------------------------------|");
   ATH_MSG_DEBUG("          |                                       Name  | Position | Prescale | TriggerType |");
   ATH_MSG_DEBUG("          |---------------------------------------------------------------------------------|");
	
   for( TrigConf::TriggerItem* item : m_configSvc->ctpConfig()->menu().item_by_name()) {
      ATH_MSG_DEBUG("REGTEST - |   " << std::setw( 40 ) << item->name() 
                    << "  | " << std::setw( 7 ) << m_itemMap->getItem( item )->itemPos() 
                    << "  | " << std::setw( 7 ) << m_itemMap->getItem( item )->prescale()
                    << "  |       0x" << std::hex << std::setfill('0') << std::setw( 2 ) << item->triggerType() << std::dec << std::setfill(' ')
                    << "  |");
   }
   ATH_MSG_DEBUG("          |---------------------------------------------------------------------------------|");

   return StatusCode::SUCCESS;
}



StatusCode
LVL1CTP::CTPEmulation::beginRun() {

   CHECK( bookHists() );

   if ( m_useROIBOutput ) {
      CHECK( m_lvl1Tool->updateConfig() );
   }

   return StatusCode::SUCCESS;
}


StatusCode
LVL1CTP::CTPEmulation::start() {
   return StatusCode::SUCCESS;
}


StatusCode
LVL1CTP::CTPEmulation::execute() {

   retrieveCollections().ignore();

   fillInputHistograms().ignore();

   extractMultiplicities().ignore();

   simulateItems().ignore();

   return StatusCode::SUCCESS;
}



StatusCode
LVL1CTP::CTPEmulation::createMultiplicityHist(const std::string & type, TrigConf::L1DataDef::TriggerType tt, unsigned int maxMult ) {

   const auto & thrV = m_configSvc->ctpConfig()->menu().thresholdConfig().getThresholdVector(tt);

   TH2 * hist = new TH2I(Form("%sMult", type.c_str()), Form("%s threshold multiplicity", type.c_str()), thrV.size(), 0, thrV.size(), maxMult, 0, maxMult);

   for(const TrigConf::TriggerThreshold * thr: thrV) {
      hist->GetXaxis()->SetBinLabel(thr->mapping()+1, thr->name().c_str() );
   }

   StatusCode sc = m_histSvc->regHist( histBasePath() + "/multi/" + type, hist);

   return sc;
}


StatusCode
LVL1CTP::CTPEmulation::bookHists() {

   //Check that we didn't already do this, and just return if yes ...
   std::string histpath {""};
   
   // jets
   histpath = histBasePath() + "/input/jets/";
   CHECK ( m_histSvc->regHist( histpath + "jJetPt",  new TH1I("jJetPt","Jet p_{T} - jJ", 40, 0, 80) ));
   CHECK ( m_histSvc->regHist( histpath + "jJetEta", new TH1I("jJetEta","Jet #eta - jJ", 64, -3.2, 3.2) ));
   CHECK ( m_histSvc->regHist( histpath + "jJetPhi", new TH1I("jJetPhi","Jet #phi - jJ", 64, -3.2, 3.2) ));

   CHECK ( m_histSvc->regHist( histpath + "jLJetPt",  new TH1I("jLJetPt","Jet p_{T} - jLJ", 40, 0, 80) ));
   CHECK ( m_histSvc->regHist( histpath + "jLJetEta", new TH1I("jLJetEta","Jet #eta - jLJ", 64, -3.2, 3.2) ));
   CHECK ( m_histSvc->regHist( histpath + "jLJetPhi", new TH1I("jLJetPhi","Jet #phi - jLJ", 64, -3.2, 3.2) ));

   CHECK ( m_histSvc->regHist( histpath + "gJetPt",  new TH1I("gJetPt","Jet p_{T} - gJ", 40, 0, 80) ));
   CHECK ( m_histSvc->regHist( histpath + "gJetEta", new TH1I("gJetEta","Jet #eta - gJ", 64, -3.2, 3.2) ));
   CHECK ( m_histSvc->regHist( histpath + "gJetPhi", new TH1I("gJetPhi","Jet #phi - gJ", 64, -3.2, 3.2) ));

   // MET
   histpath = histBasePath() + "/input/MET/";
   CHECK ( m_histSvc->regHist( histpath + "Pufit",    new TH1I("MET_Pufit","Missing ET from algorithm pufit", 40, 0, 80)) );
   CHECK ( m_histSvc->regHist( histpath + "PufitPhi", new TH1I("MET_PufitPhi","Missing ET PUfit phi", 64, -3.2, 3.2)) );
   CHECK ( m_histSvc->regHist( histpath + "Rho",      new TH1I("MET_Rho","Missing ET from algorithm rhosub", 40, 0, 80)) );
   CHECK ( m_histSvc->regHist( histpath + "RhoPhi",   new TH1I("MET_RhoPhi","Missing ET rhosub phi", 64, -3.2, 3.2)) );
   CHECK ( m_histSvc->regHist( histpath + "JwoJ",     new TH1I("MET_JwoJ","Missing ET from algorithm jet without jets", 40, 0, 80)) );
   CHECK ( m_histSvc->regHist( histpath + "JwoJPhi",  new TH1I("MET_JwoJPhi","Missing ET jet without jet phi", 64, -3.2, 3.2)) );

   // cluster
   histpath = histBasePath() + "/input/em/";
   CHECK ( m_histSvc->regHist( histpath + "et",  new TH1I("et","Cluster et", 40, 0, 40)) );
   CHECK ( m_histSvc->regHist( histpath + "eta", new TH1I("eta","Cluster eta ", 64, -3.2, 3.2)) );
   CHECK ( m_histSvc->regHist( histpath + "phi", new TH1I("phi","Cluster phi", 64, -3.2, 3.2)) );

   // tau
   histpath = histBasePath() + "/input/tau/";
   CHECK ( m_histSvc->regHist( histpath + "et",  new TH1I("et","Tau et", 40, 0, 40)) );
   CHECK ( m_histSvc->regHist( histpath + "eta", new TH1I("eta","Tau eta ", 64, -3.2, 3.2)) );
   CHECK ( m_histSvc->regHist( histpath + "phi", new TH1I("phi","Tau phi", 64, -3.2, 3.2)) );
   CHECK ( m_histSvc->regHist( histpath + "emIso",  new TH1I("emIso","Tau em isolation", 40, 0, 1)) );
   CHECK ( m_histSvc->regHist( histpath + "hadIso", new TH1I("hadIso","Tau hadronic isolation", 40, 0, 1)) );
   CHECK ( m_histSvc->regHist( histpath + "R3ClusterET",  new TH1I("R3ET","Tau eT", 40, 0, 40)) );
   CHECK ( m_histSvc->regHist( histpath + "R3ClusterIso", new TH1I("R3Iso","Tau isolation", 40, 0, 1)) );

   // ROI
   // jet ROI
   histpath = histBasePath() + "/input/roi/jet/";
   CHECK ( m_histSvc->regHist( histpath + "eta", new TH1I("eta","Jet ROI eta", 64, -32, 32)) );
   CHECK ( m_histSvc->regHist( histpath + "phi", new TH1I("phi","Jet ROI phi", 64, 0, 64)) );
   CHECK ( m_histSvc->regHist( histpath + "etLarge", new TH1I("etLarge","Jet ROI ET Large", 40, 0, 40)) );
   CHECK ( m_histSvc->regHist( histpath + "etSmall", new TH1I("etSmall","Jet ROI ET Small", 40, 0, 40)) );

   // MET ROI
   histpath = histBasePath() + "/input/roi/met/";
   CHECK ( m_histSvc->regHist( histpath + "xe", new TH1I("xe","XE", 40, 0, 80)) );
   CHECK ( m_histSvc->regHist( histpath + "xephi", new TH1I("xephi","XE phi", 64, -3.2, 3.2)) );
   CHECK ( m_histSvc->regHist( histpath + "te", new TH1I("te","TE", 40, 0, 80)) );
   
   // EM ROI
   histpath = histBasePath() + "/input/roi/em/";
   CHECK ( m_histSvc->regHist( histpath + "et",  new TH1I("et", "EM ROI ET",  40,   0, 40)) ); 
   CHECK ( m_histSvc->regHist( histpath + "eta", new TH1I("eta","EM ROI eta", 64, -32, 32)) );
   CHECK ( m_histSvc->regHist( histpath + "phi", new TH1I("phi","EM ROI phi", 64, -32, 32)) );

   // TAU ROI
   histpath = histBasePath() + "/input/roi/tau/";
   CHECK ( m_histSvc->regHist( histpath + "et",  new TH1I("et", "TAU ROI ET ", 40,   0, 40)) );
   CHECK ( m_histSvc->regHist( histpath + "eta", new TH1I("eta","TAU ROI eta", 64, -32, 32)) );
   CHECK ( m_histSvc->regHist( histpath + "phi", new TH1I("phi","TAU ROI phi", 64, -32, 32)) );

   // MUON ROI
   histpath = histBasePath() + "/input/roi/muon/";
   CHECK ( m_histSvc->regHist( histpath + "et",  new TH1I("et", "TAU ROI ET ", 40,   0, 40)) );
   CHECK ( m_histSvc->regHist( histpath + "eta", new TH1I("eta","TAU ROI eta", 64, -32, 32)) );
   CHECK ( m_histSvc->regHist( histpath + "phi", new TH1I("phi","TAU ROI phi", 64, -32, 32)) );

   // input counts
   histpath = histBasePath() + "/input/counts/";
   CHECK ( m_histSvc->regHist( histpath + "jJets", new TH1I("jJets","Number of jets (jJ)", 40, 0, 40)) );
   CHECK ( m_histSvc->regHist( histpath + "jLJets", new TH1I("jLJets","Number of jets (jLJ)", 40, 0, 40)) );
   CHECK ( m_histSvc->regHist( histpath + "gJets", new TH1I("gJets","Number of jets (gJ)", 40, 0, 40)) );
   CHECK ( m_histSvc->regHist( histpath + "muons", new TH1I("muons","Number of muons", 10, 0, 10)) );  
   CHECK ( m_histSvc->regHist( histpath + "emcluster", new TH1I("emcluster","Number of EM clusters", 20, 0, 20)) );
   CHECK ( m_histSvc->regHist( histpath + "taus", new TH1I("taus","Number of TAU candidates", 20, 0, 20)) );
 
   CHECK ( m_histSvc->regHist( histpath + "jetRoIData", new TH1I("nJetRoIs","Number of JET ROIs from data", 20, 0, 20)) );
   CHECK ( m_histSvc->regHist( histpath + "emTauRoIData", new TH1I("nEMTauRoIs","Number of EMTau ROIs from data", 20, 0, 20)) );
   CHECK ( m_histSvc->regHist( histpath + "emRoIData", new TH1I("nEMRoIs","Number of EM ROIs from data", 20, 0, 20)) );
   CHECK ( m_histSvc->regHist( histpath + "tauRoIData", new TH1I("nTauRoIs","Number of Tau ROIs from data", 20, 0, 20)) );
   CHECK ( m_histSvc->regHist( histpath + "muonRoIData", new TH1I("nMuonRoIs","Number of muon ROIs from data", 20, 0, 20)) );

   // threshold multiplicities
   CHECK ( createMultiplicityHist( "muon", L1DataDef::MUON, 5) );
   CHECK ( createMultiplicityHist( "jet", L1DataDef::JET) );
   CHECK ( createMultiplicityHist( "xe", L1DataDef::XE, 2) );
   CHECK ( createMultiplicityHist( "te", L1DataDef::TE, 2) );
   CHECK ( createMultiplicityHist( "em", L1DataDef::EM) );
   CHECK ( createMultiplicityHist( "tau", L1DataDef::TAU) );

   // Topo
   histpath = histBasePath() + "/input/topo/";
   TH1* hTopo0 = new TH1I("L1TopoDecision0","L1Topo Decision Cable 0", 64, 0, 64);
   TH1* hTopo1 = new TH1I("L1TopoDecision1","L1Topo Decision Cable 1", 64, 0, 64);
   for(const TIP * tip : m_configSvc->ctpConfig()->menu().tipVector() ) {
      if ( tip->tipNumber() < 384 )
         continue;
      unsigned int tipNumber = (unsigned int) ( tip->tipNumber() - 384 );
      switch(tipNumber / 64) {
      case 0:
         hTopo0->GetXaxis()->SetBinLabel(1+ tipNumber % 64, tip->thresholdName().c_str() );
         break;
      case 1:
         hTopo1->GetXaxis()->SetBinLabel(1+ tipNumber % 64, tip->thresholdName().c_str() );
         break;
      default:
         break;
      }
   }
   CHECK( m_histSvc->regHist( histpath + "l1Topo0", hTopo0) );
   CHECK( m_histSvc->regHist( histpath + "l1Topo1", hTopo1) );



   // item decision
   histpath = histBasePath() + "/output/";
   TH1 * hbyID = new TH1I("TAVbyId","Items decision", 512, 0, 512);
   TH1 * hbyName = new TH1I("TAVbyName","Items decision", 512, 0, 512);
   std::vector<std::string> orderedItemnames;
   orderedItemnames.reserve(m_configSvc->ctpConfig()->menu().items().size());
   for( const auto & item : m_configSvc->ctpConfig()->menu().items() ) {
      orderedItemnames.emplace_back(item->name());
   }
   std::sort(orderedItemnames.begin(), orderedItemnames.end());
   
   unsigned int bin = 1;
   for ( const std::string & itemname : orderedItemnames ) {
      const TrigConf::TriggerItem * item = m_configSvc->ctpConfig()->menu().item( itemname );
      hbyID->GetXaxis()->SetBinLabel( item->ctpId()+1, item->name().c_str() );
      hbyName->GetXaxis()->SetBinLabel( bin++, item->name().c_str() );
   }

   CHECK ( m_histSvc->regHist( histpath + "TAVbyId", hbyID) );
   CHECK ( m_histSvc->regHist( histpath + "TAVbyName", hbyName) );

   return StatusCode::SUCCESS;
}




StatusCode
LVL1CTP::CTPEmulation::retrieveCollections() {

   // new L1Calo collections

   // met
   CHECK ( evtStore()->retrieve( m_gFEXMETPufit, m_gFEXMETPufitLoc ) );
   ATH_MSG_DEBUG( "Retrieved gFEX MET '" << m_gFEXMETPufitLoc << "'");

   CHECK ( evtStore()->retrieve( m_gFEXMETRho, m_gFEXMETRhoLoc ) );
   ATH_MSG_DEBUG( "Retrieved gFEX MET '" << m_gFEXMETRhoLoc << "'");

   CHECK ( evtStore()->retrieve( m_gFEXMETJwoJ, m_gFEXMETJwoJLoc ) );
   ATH_MSG_DEBUG( "Retrieved gFEX MET '" << m_gFEXMETJwoJLoc << "'");

   // jets
   CHECK ( evtStore()->retrieve( m_jJet, m_jJetLoc ) );
   ATH_MSG_DEBUG( "Retrieved jFEX Jet container '" << m_jJetLoc << "' with size " << m_jJet->size());

   CHECK ( evtStore()->retrieve( m_jLJet, m_jLJetLoc ) );
   ATH_MSG_DEBUG( "Retrieved jFEX large-R Jet container '" << m_jLJetLoc << "' with size " << m_jLJet->size());

   CHECK ( evtStore()->retrieve( m_gJet, m_gJetLoc ) );
   ATH_MSG_DEBUG( "Retrieved gFEX Jet container '" << m_gJetLoc << "' with size " << m_gJet->size());

   // em clusters
   CHECK ( evtStore()->retrieve( m_eFEXCluster, m_eFEXClusterLoc ) );
   ATH_MSG_DEBUG( "Retrieved eFEX em cluster container '" << m_eFEXClusterLoc << "' with size " << m_eFEXCluster->size());
   
   // taus
   CHECK ( evtStore()->retrieve( m_eFEXTau, m_eFEXTauLoc ) );
   ATH_MSG_DEBUG( "Retrieved eFEX em tau container '" << m_eFEXTauLoc << "' with size " << m_eFEXTau->size());


   // Run 2 ROIs 
   if ( m_isData ) {
      ATH_MSG_DEBUG( "Running on data" );

      CHECK ( evtStore()->retrieve( m_muctpiCTP, m_muonCTPLoc ) );
      ATH_MSG_DEBUG( "Retrieved 'LVL1::MuCTPICTP#" << m_muonCTPLoc << "'");

      CHECK ( evtStore()->retrieve( m_emtauCTP, m_emtauCTPLoc ) );
      ATH_MSG_DEBUG( "Retrieved 'LVL1::EmTauCTP#" << m_emtauCTPLoc << "'");

      CHECK ( evtStore()->retrieve( m_jetCTP, m_jetCTPLoc ) );
      ATH_MSG_DEBUG( "Retrieved 'LVL1::JetCTP#" << m_jetCTPLoc << "'");

      CHECK ( evtStore()->retrieve( m_energyCTP, m_energyCTPLoc ) );
      ATH_MSG_DEBUG( "Retrieved 'LVL1::EnergyCTP#" << m_energyCTPLoc << "'");

      CHECK ( evtStore()->retrieve( m_topoCTP, m_topoCTPLoc ) );
      ATH_MSG_DEBUG( "Retrieved 'LVL1::FrontPanelCTP#" << m_topoCTPLoc << "'");

      if ( m_useROIBOutput ) {
         CHECK ( evtStore()->retrieve( m_roibResult ) );
         ATH_MSG_DEBUG( "Retrieved ROIBResult" );

         CHECK ( m_lvl1Tool->updateResult( *m_roibResult, false) );
         ATH_MSG_DEBUG( "Created ROIs from  ROIBResult" );
      }
   } else {
      ATH_MSG_DEBUG( "Running on MC, going to retrieve simulated L1 objects in CTP format" );

      CHECK ( evtStore()->retrieve( m_muctpiCTP, m_muonCTPLoc ) );
      ATH_MSG_DEBUG( "Retrieved 'LVL1::MuCTPICTP#" << m_muonCTPLoc << "'");
      CHECK ( evtStore()->retrieve( m_emtauCTP, m_emtauCTPLoc ) );
      ATH_MSG_DEBUG( "Retrieved 'LVL1::EmTauCTP#" << m_emtauCTPLoc << "'");
      CHECK ( evtStore()->retrieve( m_jetCTP, m_jetCTPLoc ) );
      ATH_MSG_DEBUG( "Retrieved 'LVL1::JetCTP#" << m_jetCTPLoc << "'");
      CHECK ( evtStore()->retrieve( m_energyCTP, m_energyCTPLoc ) );
      ATH_MSG_DEBUG( "Retrieved 'LVL1::EnergyCTP#" << m_energyCTPLoc << "'");
      CHECK ( evtStore()->retrieve( m_topoCTP, m_topoCTPLoc ) );
      ATH_MSG_DEBUG( "Retrieved 'LVL1::FrontPanelCTP#" << m_topoCTPLoc << "'");

   }

   return StatusCode::SUCCESS;
}


StatusCode
LVL1CTP::CTPEmulation::fillInputHistograms() {

   TH1 *h (nullptr ), *h0 ( nullptr ),  *h1 ( nullptr ), *h2 ( nullptr ), *h3 ( nullptr ),
      *h4 ( nullptr ), *h5 ( nullptr ),  *h6 ( nullptr ), *h7 ( nullptr );

   // counts
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/counts/jJets", h) ); h->Fill(m_jJet->size());
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/counts/jLJets", h) ); h->Fill(m_jLJet->size());
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/counts/gJets", h) ); h->Fill(m_gJet->size());
   if(m_muonRoIs) {
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/counts/muons", h) ); h->Fill(m_muonRoIs->size());
   }
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/counts/emcluster", h) ); h->Fill(m_eFEXCluster->size());
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/counts/taus", h) ); h->Fill(m_eFEXTau->size());
   
   // counts of objects from ROIBResult
   if ( m_useROIBOutput ) {
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/counts/jetRoIData", h) ); h->Fill(m_lvl1Tool->getJetEnergyRoIs().size());
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/counts/emTauRoIData", h) ); h->Fill(m_lvl1Tool->getEMTauRoIs().size());
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/counts/muonRoIData", h) ); h->Fill(m_lvl1Tool->getMuonRoIs().size());
   }

   // jets
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/jets/jJetPt",  h) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/jets/jJetEta", h1) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/jets/jJetPhi", h2) );
   for( const auto & jet : *m_jJet ) {
      h->Fill(fabs(jet->et8x8()/1000.));
      h1->Fill(jet->eta());
      h2->Fill(jet->phi());
   }
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/jets/jLJetPt",  h) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/jets/jLJetEta", h1) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/jets/jLJetPhi", h2) );
   for( const auto & jet : *m_jLJet ) {
      h->Fill(fabs(jet->et8x8()/1000.));
      h1->Fill(jet->eta());
      h2->Fill(jet->phi());
   }
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/jets/gJetPt",  h) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/jets/gJetEta", h1) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/jets/gJetPhi", h2) );
   for( const auto & jet : *m_gJet ) {
      h->Fill(fabs(jet->et8x8()/1000.));
      h1->Fill(jet->eta());
      h2->Fill(jet->phi());
   }

   // MET
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/MET/Pufit", h) ); if(h) h->Fill(m_gFEXMETPufit->energyT()/1000.);
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/MET/PufitPhi", h) ); if(h) h->Fill(atan2(m_gFEXMETPufit->energyX(), m_gFEXMETPufit->energyY()));
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/MET/Rho", h) ); if(h) h->Fill(m_gFEXMETRho->energyT()/1000.);
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/MET/RhoPhi", h) ); if(h) h->Fill(atan2(m_gFEXMETRho->energyX(), m_gFEXMETRho->energyY()));
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/MET/JwoJ", h) ); if(h) h->Fill(m_gFEXMETJwoJ->energyT()/1000.);
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/MET/JwoJPhi", h) ); if(h) h->Fill(atan2(m_gFEXMETJwoJ->energyX(), m_gFEXMETJwoJ->energyY()));

   // EM cluster
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/em/et", h1) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/em/eta", h2) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/em/phi", h3) );
   for( const auto & cl : *m_eFEXCluster ) {
      h1->Fill(cl->et());
      h2->Fill(cl->eta());
      h3->Fill(cl->phi());
   }

   // eFEX Tau
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/tau/et", h1) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/tau/eta", h2) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/tau/phi", h3) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/tau/emIso", h4) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/tau/hadIso", h5) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/tau/R3ClusterET", h6) );
   CHECK ( m_histSvc->getHist( histBasePath() + "/input/tau/R3ClusterIso", h7) );
   const static SG::AuxElement::ConstAccessor<float> accR3ClET ("R3ClusterET");
   const static SG::AuxElement::ConstAccessor<float> accR3ClIso ("R3ClusterIso");
   for( const auto & tau : *m_eFEXTau ) {
      h1->Fill(tau->eT());
      h2->Fill(tau->eta());
      h3->Fill(tau->phi());
      h4->Fill(tau->emIsol());
      h5->Fill(tau->hadIsol());
      h6->Fill(accR3ClET(*tau)/1000.);
      h7->Fill(accR3ClIso(*tau));
   }

   // ===
   // ROI
   // ===

   // JET
   if ( m_useROIBOutput ) {
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/jet/eta", h2) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/jet/phi", h3) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/jet/etLarge", h4) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/jet/etSmall", h5) );
      for ( const HLT::JetEnergyRoI & jetROI : m_lvl1Tool->getJetEnergyRoIs() ) {

         ATH_MSG_DEBUG( "HLT::JetEnergyRoI with word " << jetROI.lvl1RoI().roIWord() 
                        << " has type "  << jetROI.lvl1RoI().roIType() 
                        << " and l1roi jet et " << jetROI.lvl1RoI().jetEt() );

         if( jetROI.lvl1RoI().roIType() != LVL1::TrigT1CaloDefs::JetRoIWordType ) 
            continue; // not a jet (likely xe)
      
         const ROIB::JetEnergyRoI & l1JetROI = jetROI.lvl1RoI();

         LVL1::CoordinateRange coordRange = m_jetDecoder->coordinate(l1JetROI.roIWord());
         int ieta = int ( ( coordRange.etaRange().min() + 0.025) / 0.1) +
            ( ( coordRange.etaRange().min() + 0.025 > 0) ? 0 : -1);
         int iphi = int(( coordRange.phiRange().min() + 0.025) * 32 / M_PI);
         h2->Fill( ieta );
         h3->Fill( iphi );
         h4->Fill( l1JetROI.etLarge() );
         h5->Fill( l1JetROI.etSmall() );
      }
   }

   // MET
   if ( m_useROIBOutput ) {
      int energyX(0), energyY(0), energyT(0);
      //unsigned int w0, w1, w2;
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/met/xe",    h1) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/met/xephi", h2) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/met/te",    h3) );
      for( const ROIB::JetEnergyResult & res : m_roibResult->jetEnergyResult() ) {
	 for( const ROIB::JetEnergyRoI & roi : res.roIVec() ) {
            // RoI word
	    uint32_t roIWord = roi.roIWord();
	    // RoI type
	    int roiType = m_jetDecoder->roiType( roIWord );

            // ATH_MSG_DEBUG("MET t=" << roiType << " " << roi.roIType() << " w=" << roIWord << " et=" << roi.jetEt()
            //               << " etLarge=" << roi.etLarge() << " etSmall=" << roi.etSmall() 
            //               << " eX=" << roi.energyX() << " eY=" << roi.energyY() << " eSum=" << roi.energySum()
            //               << " eTsumType=" << roi.etSumType()
            //               );

            if( roi.etSumType() == 1 ) // eta-restricted range XE and TE
               continue;

            if( roiType == LVL1::TrigT1CaloDefs::EnergyRoIWordType0 ) { // eX (XS)
               energyX = m_jetDecoder->energyX( roi.roIWord() );
            } else if( roiType == LVL1::TrigT1CaloDefs::EnergyRoIWordType1 ) { // eY (TE)
               energyY = m_jetDecoder->energyY( roi.roIWord() );
            } else if( roiType == LVL1::TrigT1CaloDefs::EnergyRoIWordType2 ) { //etSum (XE)
               energyT = m_jetDecoder->energyT( roi.roIWord() );
            }

 	    // // Jet ROI
	    // if( roiType == LVL1::TrigT1CaloDefs::JetRoIWordType ) {
	    // 	  // RecRoI
	    // 	  // LVL1::RecJetRoI recRoI( roIWord, &jetThresholds );
	    // }

	 }
      }
      h1->Fill(sqrt(energyX*energyX + energyY*energyY));
      h2->Fill(atan2(energyX, energyY));
      h3->Fill(energyT);
      
   }

   if ( m_useROIBOutput ) {
      // EMTAU ROIs
      unsigned int nEMROI(0), nTAUROI(0);
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/em/et",  h ) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/em/eta", h1) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/em/phi", h2) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/tau/et",  h3) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/tau/eta", h4) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/roi/tau/phi", h5) );
      for( const ROIB::EMTauResult & res : m_roibResult->eMTauResult() ) {
         for ( const ROIB::EMTauRoI & roi : res.roIVec() ) {
            ATH_MSG_DEBUG("EMTAU roi t=" << roi.roIType() << " et=" << roi.et() << " iso=" << roi.isolation());
            if( roi.roIType() == LVL1::TrigT1CaloDefs::EMRoIWordType) {
               nEMROI++;
               h->Fill(roi.et());
            } else if( roi.roIType() == LVL1::TrigT1CaloDefs::TauRoIWordType) {
               nTAUROI++;
               h3->Fill(roi.et());
            }
         }
      }
   }

   // topo
   {
      ATH_MSG_DEBUG("Retrieved input from L1Topo from StoreGate with key " << m_topoCTPLoc);
      ATH_MSG_DEBUG("L1Topo0 word 0 is: 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << m_topoCTP->cableWord1(0));
      ATH_MSG_DEBUG("L1Topo0 word 1 is: 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << m_topoCTP->cableWord1(1));
      ATH_MSG_DEBUG("L1Topo1 word 0 is: 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << m_topoCTP->cableWord2(0));
      ATH_MSG_DEBUG("L1Topo1 word 1 is: 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << m_topoCTP->cableWord2(1));
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/topo/l1Topo0",  h0 ) );
      CHECK ( m_histSvc->getHist( histBasePath() + "/input/topo/l1Topo1",  h1 ) );
      for(unsigned int i=0; i<32; ++i) {
         uint32_t mask = 0x1; mask <<= i;
         if( (m_topoCTP->cableWord1(0) & mask) != 0 ) h0->Fill(i); // cable 0, clock 0
         if( (m_topoCTP->cableWord1(1) & mask) != 0 ) h0->Fill(32 + i); // cable 0, clock 1
         if( (m_topoCTP->cableWord2(0) & mask) != 0 ) h1->Fill(i); // cable 1, clock 0
         if( (m_topoCTP->cableWord2(1) & mask) != 0 ) h1->Fill(32 + i); // cable 1, clock 1
      }
   }

   return StatusCode::SUCCESS;
}


bool
LVL1CTP::CTPEmulation::isNewThreshold(const TrigConf::TriggerThreshold * thr) const {

   for ( const auto & cn : std::vector<std::string>{ "CTPCAL", "TOPO1", "TOPO2", "ALFA" } ) {
      if( thr->cableName() == cn ) {
         return false;
      }
   }
   
   for( const std::string & t : std::vector<std::string>{"MUON", "ZB", "MBTS", "MBTSSI", "NIM", "EM", "TAU", "TE", "XS"} ){
      if( thr->type() == t ) {
         return false;
      }
   }

   return true;
}


StatusCode
LVL1CTP::CTPEmulation::extractMultiplicities() const {

   ATH_MSG_DEBUG("Entered CTPEmmulation::extractMultiplicities()");

   const TrigConf::Menu & l1menu = m_configSvc->ctpConfig()->menu();

   m_thrMap->clear();

   for ( const TrigConf::TriggerThreshold * thr : l1menu.thresholdVector() ) {

      // get the multiplicity for each threshold
      unsigned int multiplicity = 0;
      if( ! isNewThreshold(thr) && m_useCTPInput == true ) {
         multiplicity = extractMultiplicitiesFromCTPInputData( thr );
      } else {
         multiplicity = calculateMultiplicity( thr );
      }

      m_thrMap->decision( thr )->setValue( multiplicity );

   }

   return StatusCode::SUCCESS;
}


unsigned int
LVL1CTP::CTPEmulation::extractMultiplicitiesFromCTPInputData( const TrigConf::TriggerThreshold * thr ) const {
   unsigned int multiplicity = 0;
   ATH_MSG_VERBOSE( "Extracting multiplicity for threshold " << thr->name() << ": " << multiplicity ); 
   return multiplicity;
}

unsigned int
LVL1CTP::CTPEmulation::calculateJetMultiplicity( const TrigConf::TriggerThreshold * confThr ) const {
   unsigned int multiplicity = 0;

   if( confThr->name().find("J") == 0 ) {
      if( m_useROIBOutput ) {
         for ( const HLT::JetEnergyRoI & jetROI : m_lvl1Tool->getJetEnergyRoIs() ) {

            LVL1::TrigT1CaloDefs::RoIType jettype = m_jetDecoder->roiType( jetROI.lvl1RoI().roIWord() );

            ATH_MSG_DEBUG( "JetEnergyRoI has type @hlt "  
                           << jetROI.type() << " @lvl1 "
                           << jetROI.lvl1RoI().roIType() << " @decoder"
                           << m_jetDecoder->roiType( jetROI.lvl1RoI().roIWord() ) );

	 
            if( jettype != LVL1::TrigT1CaloDefs::JetRoIWordType ) continue; // not a jet (likely xe)

            const ROIB::JetEnergyRoI & l1JetROI = jetROI.lvl1RoI();

            LVL1::CoordinateRange coordRange = m_jetDecoder->coordinate(l1JetROI.roIWord());
            int iphi = int(( coordRange.phiRange().min() + 0.025) * 32 / M_PI);
            int ieta = int(( coordRange.eta() + ((coordRange.eta() > 0.01) ? 0.025 : -0.025)) / 0.1) - 1;
            // Adjustment due to irregular geometries
            if (ieta > 24) ieta += 2;

            const TrigConf::TriggerThresholdValue * thrV = confThr->triggerThresholdValue( ieta, iphi );
            bool largeWindow = thrV->windowSize()==JetWindowSize::LARGE;
            unsigned int et = largeWindow ? l1JetROI.etLarge() : l1JetROI.etSmall();

            bool roiPasses = ( et >= thrV->ptcut() ); // need to add cut on isolation and other variables, once available
            ATH_MSG_DEBUG( "For threshold " << confThr->name() << " check ROI ieta " << ieta 
                           << " iphi " << iphi 
                           << " et " << et
                           << "  thrV " << thrV->ptcut()
                           << (roiPasses ? "passes" : "failed")
                           );

            multiplicity +=  roiPasses ? 1 : 0;
         }
      } else {
         ATH_MSG_DEBUG("Threshold " << confThr->name() << " is on cable " << confThr->cableName() << 
                       " jet cable0=" << m_jetCTP->cableWord0() << " and cable1=" << m_jetCTP->cableWord1()  );
         if ( m_jetCTP.isValid() ) {
            if ( confThr->cableName() == "JEP1" || confThr->cableName() == "JET1" ) {
               multiplicity = CTPUtil::getMult( m_jetCTP->cableWord0(), confThr->cableStart(), confThr->cableEnd() );
            } else if ( confThr->cableName() == "JEP2" || confThr->cableName() == "JET2" ) {
               multiplicity = CTPUtil::getMult( m_jetCTP->cableWord1(), confThr->cableStart(), confThr->cableEnd() );
            }
         } 
      }

   } else {
      // Run-3 threshold
      const DataHandle< xAOD::JetRoIContainer > * dh { nullptr };
      if( confThr->name().find("g") == 0 ) {
	 dh = & m_gJet;
      } else if( confThr->name().find("jL") == 0 ) {
	 dh = & m_jLJet;
      } else if( confThr->name().find("j") == 0 ) {
	 dh = & m_jJet;
      } else {
	 ATH_MSG_ERROR( "Unexpected threshold name " << confThr->name() << ". Should start with j, jL, g, or J.");
      }
      if ( dh ) {
	 for ( const auto & jet : **dh ) {

            float eta = jet->eta();
            float phi = jet->phi();
            if ( phi < 0 ) phi += 2*M_PI;
            if ( phi >= 2*M_PI ) phi -= 2*M_PI;

            LVL1::Coordinate coord(phi, eta);
            LVL1::CoordToHardware converter;
            unsigned int jepCoord = converter.jepCoordinateWord(coord);
            uint32_t roiword = jepCoord << 19;

            auto coordRange = m_jetDecoder->coordinate(roiword);

            int ieta =
               int((coordRange.eta() + ((coordRange.eta() > 0.01) ? 0.025 : -0.025)) / 0.1) - 1;
            // Adjustment due to irregular geometries
            if (ieta > 24)
               ieta += 2;
            int iphi = int((coordRange.phiRange().min() + 0.025) * 32 / M_PI);
            
	    // copied from
	    // https://acode-browser.usatlas.bnl.gov/lxr/source/athena/Trigger/TrigT1/TrigT1CaloUtils/src/JetAlgorithm.cxx#0337
	    //int ieta = int((eta + (eta>0 ? 0.005 : -0.005))/0.1);
	    //int iphi = 0; // int((m_refPhi-0.005)*32/M_PI); iphi = 16*(iphi/16) + 8;
            bool pass = ((unsigned int) (jet->et8x8()/1000.)) > confThr->triggerThresholdValue( ieta, iphi )->ptcut();
	          multiplicity += pass ? 1 : 0;
	       }
      }
   }

   TH1 * h { nullptr };
   CHECK( m_histSvc->getHist( histBasePath() + "/multi/jet", h) ); 
   h->Fill(confThr->mapping(), multiplicity);

   ATH_MSG_DEBUG("JET MULT calculated mult for threshold " << confThr->name() << " : " << multiplicity);
   return multiplicity;
}


unsigned int
LVL1CTP::CTPEmulation::calculateEMMultiplicity( const TrigConf::TriggerThreshold * confThr ) const {
   unsigned int multiplicity = 0;

   if ( confThr->name()[0]=='e' ) { 
      // new EM threshold from eFEX
      for ( const auto & cl : * m_eFEXCluster ) {

         float eta = cl->eta();
         int ieta = int((eta + (eta>0 ? 0.005 : -0.005))/0.1);
         int iphi = 0;
         const TrigConf::TriggerThresholdValue * thrV = confThr->triggerThresholdValue( ieta, iphi );
         const ClusterThresholdValue *ctv = dynamic_cast<const ClusterThresholdValue *>(thrV);
         float scale = ctv->caloInfo().globalEmScale();

         bool clusterPasses = ( ((unsigned int) cl->et()) > thrV->ptcut()*scale ); // need to add cut on isolation and other variables, once available
         multiplicity +=  clusterPasses ? 1 : 0;
      }
   } else {
      // old EM threshold from data

      if ( false ) {
         if ( m_roibResult ) {
            for( const ROIB::EMTauResult & res : m_roibResult->eMTauResult() ) {
               for ( const ROIB::EMTauRoI & roi : res.roIVec() ) {
                  if( roi.roIType() != LVL1::TrigT1CaloDefs::EMRoIWordType)
                     continue;
                  LVL1::CoordinateRange coordRange = m_decoder->coordinate(roi.roIWord());
                  // translate eta and phi from float values to tower indices
                  // copied from TrigT1Interfaces/RecEmTauRoI.cxx (l117)
                  int ieta = int ( ( coordRange.etaRange().min() + 0.025) / 0.1) +
                     ( ( coordRange.etaRange().min() + 0.025 > 0) ? 0 : -1);
                  int iphi = int(( coordRange.phiRange().min() + 0.025) * 32 / M_PI);
                  const TrigConf::ClusterThresholdValue * thrV = dynamic_cast<const TrigConf::ClusterThresholdValue*> ( confThr->triggerThresholdValue( ieta, iphi ) );

                  float scale = thrV->caloInfo().globalEmScale();
                  unsigned int etCut = thrV->ptcut();
                  unsigned int threshold = etCut * scale;
                  unsigned int isolMask = thrV->isolationMask();
                  //const TrigConf::TriggerThresholdValue * thrV = confThr->triggerThresholdValue( ieta, iphi );

                  // isolation bit interpretation 
                  bool isolationPassed = true;
                  for (unsigned int bit = 0; bit < LVL1::TrigT1CaloDefs::numOfIsolationBits; ++bit) {
                     if ((isolMask & (1 << bit)) && !(roi.isolation() & (1 << bit)))  // only bits in the mask are checked and 
                        // set isolation to false if isolation bit==1 !!
                        isolationPassed = false;
                  }
                  ATH_MSG_DEBUG("EM " <<  confThr->name() << " mask=" << isolMask 
                                << "iso=" << roi.isolation() 
                                << " isoPassed=" << (isolationPassed?"yes":"no") );
                  bool roiPasses = ( (roi.et() >= threshold) && isolationPassed );
                  multiplicity +=  roiPasses ? 1 : 0;
               }
            }
         }
      } else {
         if ( m_emtauCTP.isValid() ) {
            if ( confThr->cableName() == "CP1" || confThr->cableName() == "EM1" ) {
               multiplicity = CTPUtil::getMult( m_emtauCTP->cableWord0(), confThr->cableStart(), confThr->cableEnd() );
            } else if ( confThr->cableName() == "CP2" || confThr->cableName() == "EM2" ) {
               multiplicity = CTPUtil::getMult( m_emtauCTP->cableWord1(), confThr->cableStart(), confThr->cableEnd() );
            }
         }
      }
   }
   TH1 * h { nullptr };
   CHECK( m_histSvc->getHist( histBasePath() + "/multi/em", h) ); h->Fill(confThr->mapping(), multiplicity);
   
   ATH_MSG_DEBUG("EM MULT calculated mult for threshold " << confThr->name() << " : " << multiplicity);
   return multiplicity;
}

unsigned int
LVL1CTP::CTPEmulation::calculateTauMultiplicity( const TrigConf::TriggerThreshold * confThr ) const {
   unsigned int multiplicity = 0;
   if ( confThr->name()[0]=='e' ) { 
      // new TAU threshold from eFEX
      const static SG::AuxElement::ConstAccessor<float> accR3ClET ("R3ClusterET");
      const static SG::AuxElement::ConstAccessor<float> accR3ClIso ("R3ClusterIso");
      if( m_eFEXTau ) {
         for ( const auto & tau : * m_eFEXTau ) {
            unsigned int eT = (unsigned int) (accR3ClET(*tau)/1000.); // tau eT is in MeV while the cut is in GeV - this is only temporary and needs to be made consistent for all L1Calo
            //float iso = accR3ClIso(*tau);
            float eta = tau->eta();
            int ieta = int((eta + (eta>0 ? 0.005 : -0.005))/0.1);
            int iphi = 0;
            const TrigConf::TriggerThresholdValue * thrV = confThr->triggerThresholdValue( ieta, iphi );
            bool tauPasses = ( eT >= thrV->ptcut() ); // need to add cut on isolation and other variables, once available
            multiplicity +=  tauPasses ? 1 : 0;
         }
      }
   } else {
      // old TAU threshold
      if ( false ) {
         if ( m_roibResult ) {
            for( const ROIB::EMTauResult & res : m_roibResult->eMTauResult() ) {
               for ( const ROIB::EMTauRoI & roi : res.roIVec() ) {
                  if( roi.roIType() != LVL1::TrigT1CaloDefs::TauRoIWordType)
                     continue;
                  LVL1::CoordinateRange coordRange = m_decoder->coordinate(roi.roIWord());
                  int ieta = int ( ( coordRange.etaRange().min() + 0.025) / 0.1) +
                     ( ( coordRange.etaRange().min() + 0.025 > 0) ? 0 : -1);
                  int iphi = int(( coordRange.phiRange().min() + 0.025) * 32 / M_PI);
                  const TrigConf::TriggerThresholdValue * thrV = confThr->triggerThresholdValue( ieta, iphi );
                  bool roiPasses = ( roi.et() >= thrV->ptcut() ); // need to add cut on isolation and other variables, once available
                  multiplicity +=  roiPasses ? 1 : 0;
               }
            }
         }
      } else {
         if ( m_emtauCTP.isValid() ) {
            if ( confThr->cableName() == "TAU1" ) {
               multiplicity = CTPUtil::getMult( m_emtauCTP->cableWord2(), confThr->cableStart(), confThr->cableEnd() );
            } else if ( confThr->cableName() == "TAU2" ) {
               multiplicity = CTPUtil::getMult( m_emtauCTP->cableWord3(), confThr->cableStart(), confThr->cableEnd() );
            }
         }
      }
   }
   TH1 * h { nullptr };
   CHECK( m_histSvc->getHist( histBasePath() + "/multi/tau", h) ); h->Fill(confThr->mapping(), multiplicity);
   
   ATH_MSG_DEBUG("TAU MULT calculated mult for threshold " << confThr->name() << " : " << multiplicity);
   return multiplicity;
}

unsigned int
LVL1CTP::CTPEmulation::calculateMETMultiplicity( const TrigConf::TriggerThreshold * confThr ) const {
   unsigned int multiplicity = 0;

   if ( confThr->name().find("XE")==0 ) {
      // old XE
      if( m_useROIBOutput ) {
         int energyX(0), energyY(0);
         if ( m_roibResult ) {
            for( const ROIB::JetEnergyResult & res : m_roibResult->jetEnergyResult() ) {
               for( const ROIB::JetEnergyRoI & roi : res.roIVec() ) {            
                  if( roi.etSumType() == 1 ) // eta-restricted range XE and TE is ignored for the time being
                     continue;
                  if( roi.roIType() == LVL1::TrigT1CaloDefs::EnergyRoIWordType0 ) { // eX (XS)
                     energyX = m_jetDecoder->energyX( roi.roIWord() );
                  } else if( roi.roIType() == LVL1::TrigT1CaloDefs::EnergyRoIWordType1 ) { // eY (TE)
                     energyY = m_jetDecoder->energyY( roi.roIWord() );
                  }
               }
            }
         }
         double missingET = sqrt(energyX*energyX + energyY*energyY);
         const TrigConf::TriggerThresholdValue * thrV = confThr->triggerThresholdValue( 0, 0 );
         multiplicity = ((unsigned int) missingET) >= thrV->ptcut() ? 1 : 0;
      } else {
         if ( m_energyCTP.isValid() ) {
            if ( confThr->cableName() == "JEP3" || confThr->cableName() == "EN1") {
               multiplicity = CTPUtil::getMult( m_energyCTP->cableWord0(), confThr->cableStart(), confThr->cableEnd() );
            } else if ( confThr->cableName() == "EN2") {
               multiplicity = CTPUtil::getMult( m_energyCTP->cableWord1(), confThr->cableStart(), confThr->cableEnd() );
            }
         }
      }
   } else if ( confThr->name().find("TE")==0 ) {
      // old TE 
      if( m_useROIBOutput ) {
         int energyT(0);
         if ( m_roibResult ) {
            bool isRestrictedRangeTE = confThr->name().find("24ETA49") != std::string::npos;
            for( const ROIB::JetEnergyResult & res : m_roibResult->jetEnergyResult() ) {
               for( const ROIB::JetEnergyRoI & roi : res.roIVec() ) {
                  ATH_MSG_DEBUG("TE " << roi.etSumType() << "  " << roi.roIType());
                  if( (roi.etSumType() == (isRestrictedRangeTE ? 1 : 0) ) && // pick the correct sumtype 0-all, 1-restricted range
                      roi.roIType() == LVL1::TrigT1CaloDefs::EnergyRoIWordType2 ) { // etSum
                     energyT = m_jetDecoder->energyT( roi.roIWord() );
                     ATH_MSG_DEBUG("TE RR " << energyT);
                  }
               }
            }
         }
         const TrigConf::TriggerThresholdValue * thrV = confThr->triggerThresholdValue( 25, 0 ); // eta can be any value 24..49
         multiplicity = (energyT >= thrV->ptcut()) ? 1 : 0;
         ATH_MSG_DEBUG("TE DECISION " << confThr->name() << " TE=" << energyT << " thr=" << thrV->ptcut() 
                       << " mult=" << multiplicity);
      } else {
         if ( m_energyCTP.isValid() ) {
            if ( confThr->cableName() == "JEP3" || confThr->cableName() == "EN1") {
               multiplicity = CTPUtil::getMult( m_energyCTP->cableWord0(), confThr->cableStart(), confThr->cableEnd() );
            } else if ( confThr->cableName() == "EN2") {
               multiplicity = CTPUtil::getMult( m_energyCTP->cableWord1(), confThr->cableStart(), confThr->cableEnd() );
            }
         }
      }
   } else if ( confThr->name().find("XS")==0 ) {
      // old XS
      if ( m_energyCTP.isValid() ) {
         if ( confThr->cableName() == "EN1" ) {
            multiplicity = CTPUtil::getMult( m_energyCTP->cableWord0(), confThr->cableStart(), confThr->cableEnd() );
         }
      }
   } else {
      // new XE 
      // input depends on the name of the threshold
      const DataHandle< xAOD::EnergySumRoI > * dh ( nullptr );
      if ( confThr->name().find("gXEPUFIT")==0 ) {
         dh = & m_gFEXMETPufit;
         ATH_MSG_DEBUG("Using Pufit input for threshold " << confThr->name() );
      } else if ( confThr->name().find("gXERHO")==0 ) {
         dh = & m_gFEXMETRho;
         ATH_MSG_DEBUG("Using Rho input for threshold " << confThr->name() );
      } else if ( confThr->name().find("gXEJWOJ")==0 ) {
         dh = & m_gFEXMETJwoJ;
         ATH_MSG_DEBUG("Using JwoJ input for threshold " << confThr->name() );
      } else {
         dh = & m_gFEXMETJwoJ;
         ATH_MSG_DEBUG("Using default input JwoJ for threshold " << confThr->name() );
      }
      multiplicity = ( (*dh)->energyT()/1000. < confThr->thresholdValueVector()[0]->ptcut() ) ? 0 : 1; // energyT value is in MeV, cut in GeV
   }

   TH1 * h { nullptr };
   if ( confThr->name().find("TE")==0 ) {
      CHECK( m_histSvc->getHist( histBasePath() + "/multi/te", h) ); h->Fill(confThr->mapping(), multiplicity);
   } else {
      CHECK( m_histSvc->getHist( histBasePath() + "/multi/xe", h) ); h->Fill(confThr->mapping(), multiplicity);
   }

   ATH_MSG_DEBUG("XE/TE/XS MULT calculated mult for threshold " << confThr->name() << " : " << multiplicity);
   return multiplicity;

}

unsigned int
LVL1CTP::CTPEmulation::calculateMuonMultiplicity( const TrigConf::TriggerThreshold * confThr ) const {
   unsigned int multiplicity = 0;

   // muon
   if( m_useROIBOutput ) {
      for ( const auto & muon : m_lvl1Tool->getMuonRoIs() ) {
         multiplicity += (muon.lvl1RoI().pt()>= (unsigned int) confThr->mapping()) ? 1 : 0; // TrigT1Result/MuCTPIRoI
      }
   } else {
      if ( m_muctpiCTP.isValid() ) {
         multiplicity = CTPUtil::getMult( m_muctpiCTP->muCTPIWord(), confThr->cableStart(), confThr->cableEnd() );
      }
   }

   TH1 * h { nullptr };
   CHECK( m_histSvc->getHist( histBasePath() + "/multi/muon", h) ); h->Fill(confThr->mapping(), multiplicity);

   ATH_MSG_DEBUG("MU MULT calculated mult for threshold " << confThr->name() << " : " << multiplicity);
   return multiplicity;
}


unsigned int
LVL1CTP::CTPEmulation::calculateTopoMultiplicity( const TrigConf::TriggerThreshold * confThr ) const {
   unsigned int multiplicity = 0;

   // topo
   if ( m_topoCTP.isValid() ) {
      uint64_t cable = 0;
      if ( confThr->cableName() == "TOPO1" ) {
         cable = ( (uint64_t)m_topoCTP->cableWord1( 1 ) << 32) + m_topoCTP->cableWord1( 0 );
      } else {
         cable = ( (uint64_t)m_topoCTP->cableWord2( 1 ) << 32) + m_topoCTP->cableWord2( 0 );
      }

      ATH_MSG_DEBUG( " ---> Topo input " << confThr->name() << " on module " << confThr->cableName() << " with clock " << confThr->clock()
                     << ", cable start " << confThr->cableStart() << " and end " << confThr->cableEnd()
                     << " double word 0x" << std::setw(16) << std::setfill('0') << std::hex << cable << std::dec << std::setfill(' ') );

      multiplicity = CTPUtil::getMultTopo( cable, confThr->cableStart(), confThr->cableEnd(), confThr->clock() );

   }

   TH1 * h { nullptr };
   CHECK( m_histSvc->getHist( histBasePath() + "/multi/muon", h) ); h->Fill(confThr->mapping(), multiplicity);

   ATH_MSG_DEBUG("TOPO MU calculated decision bit for threshold " << confThr->name() << " : " << multiplicity);
   return multiplicity;
}


unsigned int
LVL1CTP::CTPEmulation::calculateMultiplicity( const TrigConf::TriggerThreshold * confThr ) const {
   unsigned int multiplicity = 0;

   if( confThr->cableName() == "CTPCAL" || 
       confThr->cableName() == "ALFA" || 
       confThr->cableName() == "NIM1" || 
       confThr->cableName() == "NIM2" || 
       confThr->type() == "NIM") {
      return 0;
   }

   // here we need to write specific code for the different types of L1 FEX objects
   if ( confThr->ttype() == TrigConf::L1DataDef::EM ) {

      multiplicity = calculateEMMultiplicity( confThr );

   } else if ( confThr->ttype() == TrigConf::L1DataDef::TAU ) {

      multiplicity = calculateTauMultiplicity( confThr );

   } else if ( confThr->ttype() == TrigConf::L1DataDef::XE ||
               confThr->ttype() == TrigConf::L1DataDef::TE ||
               confThr->ttype() == TrigConf::L1DataDef::XS ) {

      multiplicity = calculateMETMultiplicity( confThr );

   } else if ( confThr->ttype() == TrigConf::L1DataDef::JET ) {

      multiplicity = calculateJetMultiplicity( confThr );
      
   } else if ( confThr->ttype() == TrigConf::L1DataDef::MUON ) {

      multiplicity = calculateMuonMultiplicity( confThr );

   } else if ( confThr->ttype() == TrigConf::L1DataDef::TOPO ) {

      multiplicity = calculateTopoMultiplicity( confThr );

   }
   return multiplicity;
}

StatusCode
LVL1CTP::CTPEmulation::simulateItems() {

   m_itemMap->updatePrescaleCounters( m_thrMap, & m_internalTrigger );

   unsigned int ctpVersion = 4;
   unsigned int readoutWindow = 1;
   m_resultBuilder = new ResultBuilder( ctpVersion, m_configSvc->ctpConfig(), m_thrMap, m_itemMap, & m_internalTrigger, readoutWindow);


   // create CTP output format and store in the event
   const CTP_RDO*  rdo_output = m_resultBuilder->constructRDOResult();
   const CTPSLink* roi_output = m_resultBuilder->constructRoIResult();
   if(m_isData) {
      CHECK( evtStore()->record( rdo_output, m_rdoOutputLoc_Rerun ) );
      CHECK( evtStore()->record( roi_output, m_roiOutputLoc_Rerun ) );
   } else {
      CHECK( evtStore()->record( rdo_output, m_rdoOutputLoc ) );
      CHECK( evtStore()->record( roi_output, m_roiOutputLoc ) );
   }


   const auto & tbp = m_resultBuilder->tbp();
   const auto & tap = m_resultBuilder->tap();
   const auto & tav = m_resultBuilder->tav();
   std::vector<uint32_t> lvl1Info;
   lvl1Info.reserve(tbp.size() + tap.size() + tav.size());
   lvl1Info.insert(lvl1Info.end(), tbp.begin(), tbp.end());
   lvl1Info.insert(lvl1Info.end(), tap.begin(), tap.end());
   lvl1Info.insert(lvl1Info.end(), tav.begin(), tav.end());

   unsigned short itemIdx = 0;
   for ( const uint32_t dw : tbp ) { 
      for ( unsigned int bit = 0; bit<32; bit++ ) {
         m_itemCountsSumTBP[itemIdx++] += (dw & (0x1<<bit) ) != 0 ? 1 : 0;
      }
   }
   itemIdx = 0;
   for ( const uint32_t dw : tap ) { 
      for ( unsigned int bit = 0; bit<32; bit++ ) {
         m_itemCountsSumTAP[itemIdx++] += (dw & (0x1<<bit) ) != 0 ? 1 : 0;
      }
   }

   const auto & menu =  m_configSvc->ctpConfig()->menu();

   unsigned short widx (0);
   itemIdx = 0;
   for ( const uint32_t dw : tav ) { 
      ATH_MSG_VERBOSE("TAV " << std::setw(2) << widx++ << " 0x" << std::hex << dw << std::dec );

      for ( unsigned int bit = 0; bit<32; bit++, itemIdx++ ) {
         if( (dw & (0x1<<bit)) == 0 ) continue;
         m_itemCountsSumTAV[itemIdx] += 1;
         ATH_MSG_DEBUG("Item " << menu.item(itemIdx)->name() << " (" << itemIdx << ") passed");
      }
   }


   return StatusCode::SUCCESS;
}


StatusCode
LVL1CTP::CTPEmulation::finalize() {

   const auto & menu =  m_configSvc->ctpConfig()->menu();

   // fill the byName TAV histograms from the byID one
   TH1 * hbyId { nullptr };
   TH1 * hbyName { nullptr };
   CHECK( m_histSvc->getHist( histBasePath() + "/output/TAVbyId", hbyId) ); 
   CHECK( m_histSvc->getHist( histBasePath() + "/output/TAVbyName", hbyName) );
   hbyId->Sumw2(false);
   hbyName->Sumw2(false);

   for( unsigned int idx = 0; idx < 512; ++idx ) {    
      if ( menu.item(idx) == nullptr )
         continue;
      unsigned int binContent = m_itemCountsSumTAV[idx];
      hbyId->Fill( idx, binContent );
      hbyName->Fill( menu.item(idx)->name().c_str(), binContent);
   }

   for(const TriggerItem * item : menu.items()) {
      if(item==nullptr) continue;
      ATH_MSG_DEBUG("REGTEST " << item->name() << " " << item->ctpId() << " TBP " << m_itemCountsSumTBP[item->ctpId()] << " TAP " << m_itemCountsSumTAP[item->ctpId()] << " TAV " << m_itemCountsSumTAV[item->ctpId()]);
   }

   hbyId->Sumw2(false);
   hbyName->Sumw2(false);

   return StatusCode::SUCCESS;
}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// LArNoiseBursts
/// Author: Josu Cantero
/// UAM, November, 2010
/// Modified by Hideki Okawa
/// BNL, June, 2012

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IToolSvc.h"

#include "StoreGate/StoreGateSvc.h"

#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloDM_ID.h"
#include "CaloIdentifier/CaloLVL1_ID.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArMiniFCAL_ID.h"
#include "CaloIdentifier/LArID_Exception.h"


#include "CaloInterface/ICaloNoiseTool.h"
#include "CaloInterface/ICalorimeterNoiseTool.h"

#include "AthenaPoolUtilities/AthenaAttributeList.h"

#include "VxVertex/VxContainer.h"
#include "VxVertex/VxCandidate.h"
#include "VxVertex/VxTrackAtVertex.h"
#include "Particle/TrackParticleContainer.h"

#include "Identifier/Range.h" 
#include "Identifier/IdentifierHash.h"
#include "Identifier/HWIdentifier.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArHVLineID.h"
#include "LArIdentifier/LArElectrodeID.h"

#include "LArRecEvent/LArEventBitInfo.h"
#include "LArRawEvent/LArDigit.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRecEvent/LArNoisyROSummary.h"
#include "LArRecEvent/LArCollisionTime.h"
 
#include "NavFourMom/IParticleContainer.h"
#include "NavFourMom/INavigable4MomentumCollection.h"

// Lar HV
#include "LArElecCalib/ILArHVTool.h"
#include "StoreGate/DataHandle.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloDetDescr/CaloDetectorElements.h"
#include "LArReadoutGeometry/EMBCell.h"
#include "LArHV/EMBHVElectrodeConstLink.h"
#include "LArHV/EMBHVElectrode.h"
#include "LArHV/EMBPresamplerHVModuleConstLink.h"
#include "LArHV/EMBPresamplerHVModule.h"
#include "LArReadoutGeometry/EMECCell.h"
#include "LArHV/EMECHVElectrodeConstLink.h"
#include "LArHV/EMECHVElectrode.h"
#include "LArHV/EMECPresamplerHVModuleConstLink.h"
#include "LArHV/EMECPresamplerHVModule.h"
#include "LArReadoutGeometry/HECCell.h"
#include "LArHV/HECHVSubgapConstLink.h"
#include "LArHV/HECHVSubgap.h"
#include "LArReadoutGeometry/FCALTile.h"
#include "LArHV/FCALHVLineConstLink.h"
#include "LArHV/FCALHVLine.h"
#include "LArHV/LArHVManager.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

#include "LArCondUtils/LArHVToolDB.h"
///////////////////////////////////////////////////////////////////

#include "GaudiKernel/ITHistSvc.h"
#include "TTree.h"
#include "CLHEP/Vector/LorentzVector.h"

/*
#include "EventInfo/EventIncident.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "EventInfo/EventType.h"
#include "EventInfo/TriggerInfo.h"
*/

#include "xAODEventInfo/EventInfo.h"

#include "CaloEvent/CaloCellContainer.h"

// Electrons
#include "egammaEvent/Electron.h"
#include "egammaEvent/EMShower.h"
#include "egammaEvent/EMTrackMatch.h"
#include "egammaEvent/egammaPIDdefs.h"
#include "egammaEvent/egammaParamDefs.h"

#include "LArMonitoring/LArNoiseBursts.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>
#include "PathResolver/PathResolver.h"

using namespace std;
using xAOD::EventInfo;

//////////////////////////////////////////////////////////////////////////////////////
/// Constructor


LArNoiseBursts::LArNoiseBursts(const std::string& name,
			 ISvcLocator* pSvcLocator) 
  : Algorithm(name, pSvcLocator),
    m_detectorStore(0),
    m_thistSvc(0),
    m_storeGate(0),
    m_tree(0),
    m_LArCablingService("LArCablingService"),
    m_LArHVCablingTool("LArHVCablingTool"),
    m_calo_noise_tool("CaloNoiseTool/CaloNoiseToolDefault"),
    m_bc_tool("Trig::TrigConfBunchCrossingTool/BunchCrossingTool"),
    m_trigDec( "Trig::TrigDecisionTool/TrigDecisionTool" ),
    m_LArOnlineIDHelper(0),
    m_LArHVLineIDHelper(0),
    m_LArElectrodeIDHelper(0),
    m_LArEM_IDHelper(0),
    m_LArFCAL_IDHelper(0),
    m_LArHEC_IDHelper(0),
    m_caloIdMgr(0),
    m_calodetdescrmgr(0),
    m_CosmicCaloStream(false),
    m_nb_sat(0),
    m_lowqfactor(0),
    m_medqfactor(0),
    m_hiqfactor(0),
    n_noisycell(0),
    m_nt_larcellsize(0),
    m_nt_cellsize(0),
    m_nt_npv(0),
    m_nt_npvtracks(0),
    //m_nt_cellpartition(0),
    //m_nt_runnumber(0),
    m_nt_evtId(0),
    m_nt_evtTime(0),
    m_nt_evtTime_ns(0),
    m_nt_lb(0),
    m_nt_bcid(0),
    m_nt_ntracks(0),
    m_nt_isbcidFilled(0),
    m_nt_isbcidInTrain(0),
    m_nt_isBunchesInFront(0),
    m_nt_bunchtype(0),
    m_nt_bunchtime(0),
    m_nt_atlasready(0),
    m_nt_stablebeams(0),
    m_nt_streamTagName(0),
    m_nt_streamTagType(0),
    m_nt_larnoisyro(0),
    m_nt_larnoisyro_satOne(0),
    m_nt_larnoisyro_satTwo(0),
    m_nt_veto_mbts(0),
    //m_nt_veto_indet(0),
    m_nt_veto_bcm(0),
    m_nt_veto_lucid(0),
    m_nt_veto_pixel(0),
    m_nt_veto_sct(0),
    m_nt_veto_mbtstdHalo(0),
    m_nt_veto_mbtstdCol(0),
    m_nt_veto_lartdHalo(0),
    m_nt_veto_lartdCol(0),
    m_nt_veto_csctdHalo(0),
    m_nt_veto_csctdCol(0),
    m_nt_veto_bcmtHalo(0),
    m_nt_veto_bcmtCol(0),
    m_nt_veto_muontCol(0),
    m_nt_veto_muontCosmic(0),
    m_nt_ECTimeDiff(0),
    m_nt_ECTimeAvg(0),
    m_nt_nCellA(0),
    m_nt_nCellC(0),
    m_nt_mbtstimediff(0),
    m_nt_nmbtscellA(0),
    m_nt_nmbtscellC(0),
    m_nt_mbtstimeavrg(0),
    m_nt_energycell(0),
    m_nt_qfactorcell(0),
    m_nt_phicell(0),
    m_nt_etacell(0),
    m_nt_signifcell(0),
    m_nt_noisycellpercent(0),
    m_nt_ft_noisy(0),
    m_nt_slot_noisy(0),
    m_nt_channel_noisy(0),
    m_nt_cellpartlayerindex(0),
    m_nt_cellIdentifier(0),
    m_nt_noisycellpart(0),
    m_nt_nominalhv(0),
    m_nt_maximalhv(0),
    m_nt_noisycellHVphi(0),
    m_nt_noisycellHVeta(0),
    m_nt_samples(0),
    m_nt_gain(0),
    m_nt_isbadcell(0),
    m_nt_partition(0),
    m_nt_layer(0),
    m_nt_isbadcell_sat(0),
    m_nt_barrelec_sat(0),
    m_nt_posneg_sat(0),
    m_nt_ft_sat(0),
    m_nt_slot_sat(0),
    m_nt_channel_sat(0),
    m_nt_partition_sat(0),
    m_nt_energy_sat(0),
    m_nt_phicell_sat(0),
    m_nt_etacell_sat(0),
    m_nt_layer_sat(0),
    m_nt_cellIdentifier_sat(0),
    m_nt_el_E(0),
    m_nt_el_Et(0),
    m_nt_el_eta(0),
    m_nt_el_phi(0),
    m_nt_el_cl_E(0),
    m_nt_el_cl_Et(0),
    m_nt_el_cl_eta(0),
    m_nt_el_cl_phi(0),
    m_nt_el_author(0),
    m_nt_el_loosePP(0),
    m_nt_el_mediumPP(0),
    m_nt_el_tightPP(0)  
 {

   // Trigger
   declareProperty( "TrigDecisionTool", m_trigDec );
   
   /** switches to control the analysis through job options */
   declareProperty("LArCablingService", m_LArCablingService);
   declareProperty("LArHVCablingTool",  m_LArHVCablingTool);
   
   // NEW
   declareProperty( "ICaloNoiseTool", m_calo_noise_tool );
   declareProperty( "BCTool", m_bc_tool );
   //   declareProperty( "SCTClusteringTool",m_sctclustering_tool);
   
   //event cuts
   declareProperty("SigmaCut", m_sigmacut = 3.0);
   declareProperty("MBTSCellChargeThreshold", m_MBTSThreshold= 40./222);
   declareProperty("NumberOfBunchesInFront",m_frontbunches = 36);
   
   // Keep cell properties
   declareProperty("KeepOnlyCellID",          m_keepOnlyCellID = false);

   // Physics objects
   declareProperty("ElectronContainer", m_elecContainerName = "ElectronCollection");

 }

/////////////////////////////////////////////////////////////////////////////////////
/// Destructor - check up memory allocation
/// delete any memory allocation on the heap

LArNoiseBursts::~LArNoiseBursts() {}

////////////////////////////////////////////////////////////////////////////////////
/// Initialize
/// initialize StoreGate
/// get a handle on the analysis tools
/// book histograms
/*
StatusCode LArNoiseBursts::initializeBeforeEventLoop() {
  MsgStream mLog( messageService(), name() );

  mLog << MSG::DEBUG << "Initializing LArNoiseBursts (before eventloop)" << endreq;
  
  // NEW
  

  // retrieve trigger decision tool
  // needs to be done before the first run/event since a number of
  // BeginRun/BeginEvents are registered by dependent services
  StatusCode sc = StatusCode::SUCCESS;

  return sc;
} */
//////////////////////////////////////////////////////////////////////////////////////
///////////////////          INITIALIZE        ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

StatusCode LArNoiseBursts::initialize() {

  MsgStream mLog( messageService(), name() );

  mLog << MSG::DEBUG << "Initializing LArNoiseBursts" << endreq;
 

  /** get a handle of StoreGate for access to the Event Store */
  StatusCode sc = service("StoreGateSvc", m_storeGate);
  if (sc.isFailure()) {
     mLog << MSG::ERROR
          << "Unable to retrieve pointer to StoreGateSvc"
          << endreq;
     return sc;
  }
 
  // Retrieve detector service
  sc = service("DetectorStore", m_detectorStore);
  if (sc.isFailure()) {
    mLog << MSG::FATAL << "Unable to locate Service DetectorStore" << endreq;
    return sc;
  }

  // Trigger Decision Tool
  if(m_trigDec.retrieve().isFailure()){
    mLog << MSG::WARNING << "Failed to retrieve trigger decision tool " << m_trigDec << endreq; 
  }else{
    mLog << MSG::INFO << "Retrieved tool " << m_trigDec << endreq; 
  }
  
  // Retrieve LArCabling Service
  sc=m_LArCablingService.retrieve();
  if (sc.isFailure()) {
    mLog << MSG::ERROR << "Could not retrieve LArCabling Service " << m_LArCablingService << endreq;
    return sc;
  }


  // Retrieve online ID helper
  const DataHandle<LArOnlineID> LArOnlineIDHelper;
  sc = m_detectorStore->retrieve(LArOnlineIDHelper, "LArOnlineID");
  if (sc.isFailure()) {
    mLog<< MSG::FATAL << "Could not get LArOnlineIDHelper" << endreq;
    return sc;
  }else{
    m_LArOnlineIDHelper = LArOnlineIDHelper;
    mLog<<MSG::DEBUG<< " Found LArOnline Helper"<<endreq;
  }

  // Retrieve HV line ID helper
  const DataHandle<LArHVLineID> LArHVLineIDHelper;
  sc = m_detectorStore->retrieve(LArHVLineIDHelper, "LArHVLineID");
  if (sc.isFailure()) {
    mLog<< MSG::FATAL << "Could not get LArOnlineIDHelper" << endreq;
    return sc;
  }else{
    m_LArHVLineIDHelper = LArHVLineIDHelper;
    mLog<<MSG::DEBUG<< " Found LArOnlineIDHelper Helper"<<endreq;
  }
  // Retrieve HV electrode ID helper
  const DataHandle<LArElectrodeID> LArElectrodeIDHelper;
  sc = m_detectorStore->retrieve(LArElectrodeIDHelper, "LArElectrodeID");
  if (sc.isFailure()) {
    mLog<< MSG::FATAL << "Could not get LArElectrodeIDHelper" << endreq;
    return sc;
  }else{
    m_LArElectrodeIDHelper = LArElectrodeIDHelper;
    mLog<<MSG::DEBUG<< " Found LArElectrodeIDHelper Helper"<<endreq;
  }

  // Retrieve ID helpers
  sc =  m_detectorStore->retrieve(m_caloIdMgr);
  if (sc.isFailure()) {
    mLog << MSG::FATAL << "Could not get CaloIdMgr" << endreq;
    return sc;
  }
  m_LArEM_IDHelper   = m_caloIdMgr->getEM_ID();
  m_LArHEC_IDHelper  = m_caloIdMgr->getHEC_ID();
  m_LArFCAL_IDHelper = m_caloIdMgr->getFCAL_ID();
  
  if ( m_calo_noise_tool.retrieve().isFailure() ) {
      mLog << MSG::WARNING << "Failed to retrieve tool " << m_calo_noise_tool << endreq; 
  }else{
       mLog << MSG::INFO << "Retrieved tool " << m_calo_noise_tool << endreq;
  }
 
  if(m_bc_tool.retrieve().isFailure()){
     mLog << MSG::WARNING << "Failed to retrieve tool " << m_bc_tool << endreq; 
  }else{
     mLog << MSG::INFO << "Retrieved tool " << m_bc_tool << endreq;
  }

  /** get a handle on the NTuple and histogramming service */
  sc = service("THistSvc", m_thistSvc);
  if (sc.isFailure()) {
     mLog << MSG::ERROR
          << "Unable to retrieve pointer to THistSvc"
          << endreq;
     return sc;
  }
 
  /*const AthenaAttributeList* fillparams(0);
  sc =  m_storeGate->retrieve(fillparams, "/TDAQ/OLC/LHC/FILLPARAMS");
  if (sc.isFailure()) {
     mLog << MSG::WARNING <<"Unable to retrieve fillparams information; falling back to" << endreq;
     return StatusCode::SUCCESS;
   }
 
  if (fillparams != 0) {
     mLog << MSG::DEBUG <<"beam 1 #bunches are: " << (*fillparams)["Beam1Bunches"].data<uint32_t>()
		   <<endreq;
     mLog << MSG::DEBUG <<"beam 2 #bunches are: " << (*fillparams)["Beam2Bunches"].data<uint32_t>()
		   <<endreq;
  }
*/

  /** Prepare TTree **/
  m_tree = new TTree( "CollectionTree", "CollectionTree" );
  std::string treeName =  "/TTREE/CollectionTree" ;
  sc = m_thistSvc->regTree(treeName, m_tree);
  if(sc.isFailure()){
    mLog << MSG::ERROR << "Unable to register TTree : " << treeName << endreq;
    return sc;
  }

  /**number of events*/
// QUESTION by BT : I do not understand this variable : is it really alls cells partition?
// answer by Josu: yes, i added this variable to calculate #noisy cell/cell per partition (offline).it's Better doing that in this code to minimize the size of the ntuple (i comment it).
  //addBranch("CellPartition",m_nt_cellpartition); 

  // General properties of events
  m_tree->Branch("EventId",&m_nt_evtId,"EventId/I");// Event ID
  m_tree->Branch("EventTime",&m_nt_evtTime,"EventTime/I");// Event time
  m_tree->Branch("EventTime_ns",&m_nt_evtTime_ns,"EventTime_ns/I");// Event time in nanosecond
  m_tree->Branch("LumiBlock",&m_nt_lb,"LumiBlock/I"); // LB
  m_tree->Branch("BCID",&m_nt_bcid,"BCID/I"); // BCID
  m_tree->Branch("StreamTagName",&m_nt_streamTagName);// stream tag name
  m_tree->Branch("StreamTagType",&m_nt_streamTagType); // stream tag type 
  m_tree->Branch("IsBCIDFilled", &m_nt_isbcidFilled,"IsBCIDFilled/I"); // check if bunch is filled
  m_tree->Branch("IsBCIDInTrain",&m_nt_isbcidInTrain,"ISBCIDInTrain/I"); // check if bunch belong to a train
  m_tree->Branch("BunchesInFront",&m_nt_isBunchesInFront); // check front bunches
  m_tree->Branch("BunchType",&m_nt_bunchtype,"BunchType/I");// Empty = 0, FirstEmpty=1,&middleEmpty=2, Single=100,Front=200,&middle=201,Tail=202 
  m_tree->Branch("TimeAfterBunch",&m_nt_bunchtime,"TimeAfterBunch/F"); //time "distance" between the colliding bunch and the nearest one.
  m_tree->Branch("ATLASIsReady",&m_nt_atlasready,"AtlasIsReady/I"); //check if atlas is ready for physics 
  m_tree->Branch("StableBeams",&m_nt_stablebeams,"StableBeams/I");//check stablebeams


  m_tree->Branch("nPV", &m_nt_npv,"nPV/I"); // Number of primary vertex
  m_tree->Branch("nTracksAtPV",&m_nt_npvtracks,"nTracksAtPV/I"); // Number of tracks at primary vertex
  m_tree->Branch("nTracks",&m_nt_ntracks,"nTracks/I"); //Number of reconstructed tracks

  // Background bits in EventInfo
  m_tree->Branch("vetoMBTS",&m_nt_veto_mbts,"vetoMBST/S"); //Beam/collision veto based on mbts
  m_tree->Branch("vetoPixel",&m_nt_veto_pixel,"vetoPixel/S"); //Beam/collision veto based on indet
  m_tree->Branch("vetoSCT",&m_nt_veto_sct,"vetoSCT/S"); //Beam/collision veto based on indet
  m_tree->Branch("vetoBcm",&m_nt_veto_bcm,"vetoBcm/S"); //Beam/collision veto based on bcm
  m_tree->Branch("vetoLucid",&m_nt_veto_lucid,"vetoLucid/S"); //Beam/collision veto based on lucid
  m_tree->Branch("vetoMBTSDtHalo",&m_nt_veto_mbtstdHalo,"vetoMBTSDtHalo/S");
  m_tree->Branch("vetoMBTSDtCol",&m_nt_veto_mbtstdCol,"vetoMBTSDtCol/S");
  m_tree->Branch("vetoLArDtHalo",&m_nt_veto_lartdHalo,"vetoLArDtHalo/S");
  m_tree->Branch("vetoLArDtCol",&m_nt_veto_lartdCol,"vetoLArDtCol/S");
  m_tree->Branch("vetoCSCDtHalo",&m_nt_veto_csctdHalo,"vetoCSCDtHalo/S");
  m_tree->Branch("vetoCSCDtCol",&m_nt_veto_csctdCol,"vetoCSCDtCol/S");
  m_tree->Branch("vetoBCMDtHalo",&m_nt_veto_bcmtHalo,"vetoBCMDtHalo/S");
  m_tree->Branch("vetoBCMDtCol",&m_nt_veto_bcmtCol,"vetoBCMDtCol/S");
  m_tree->Branch("vetoMuonTimmingCol", &m_nt_veto_muontCol,"vetoMuonTimmingCol/S");
  m_tree->Branch("vetoMuonTimmingCosmic",&m_nt_veto_muontCosmic,"vetoMuonTimmingCosmic/S");

  // LAr event bit info
  m_tree->Branch("larflag_badFEBs",&m_nt_larflag_badFEBs,"larflag_badFEBs/O");
  m_tree->Branch("larflag_mediumSaturatedDQ",&m_nt_larflag_mediumSaturatedDQ,"larflag_mediumSaturatedDQ/O");
  m_tree->Branch("larflag_tightSaturatedDQ",&m_nt_larflag_tightSaturatedDQ,"larflag_tightSaturatedDQ/O");
  m_tree->Branch("larflag_noiseBurstVeto",&m_nt_larflag_noiseBurstVeto,"larflag_noiseBurstVeto/O");
  m_tree->Branch("larflag_dataCorrupted",&m_nt_larflag_dataCorrupted,"larflag_dataCorrupted/O");
  m_tree->Branch("larflag_dataCorruptedVeto",&m_nt_larflag_dataCorruptedVeto,"larflag_dataCorruptedVeto/O");

  // trigger flags
  m_tree->Branch("L1_J75",&m_nt_L1_J75,"L1_J75/O");
  m_tree->Branch("L1_J10_EMPTY",&m_nt_L1_J10_EMPTY,"L1_J10_EMPTY/O");
  m_tree->Branch("L1_J30_FIRSTEMPTY",&m_nt_L1_J30_FIRSTEMPTY,"L1_J30_FIRSTEMPTY/O");
  m_tree->Branch("L1_J30_EMPTY",&m_nt_L1_J30_EMPTY,"L1_J30_EMPTY/O");
  m_tree->Branch("L1_XE40",&m_nt_L1_XE40,"L1_XE40/O");
  m_tree->Branch("L1_XE50",&m_nt_L1_XE50,"L1_XE50/O");
  m_tree->Branch("L1_XE50_BGRP7",&m_nt_L1_XE50_BGRP7,"L1_XE50_BGRP7/O");
  m_tree->Branch("L1_XE70",&m_nt_L1_XE70,"L1_XE70/O");
  
  m_tree->Branch("EF_j165_u0uchad_LArNoiseBurst",&m_nt_EF_j165_u0uchad_LArNoiseBurst,"EF_j165_u0uchad_LArNoiseBurst/O");
  m_tree->Branch("EF_j30_u0uchad_empty_LArNoiseBurst",&m_nt_EF_j30_u0uchad_empty_LArNoiseBurst,"EF_j30_u0uchad_empty_LArNoiseBurst/O");
  m_tree->Branch("EF_j55_u0uchad_firstempty_LArNoiseBurst",&m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurst,"EF_j55_u0uchad_firstempty_LArNoiseBurst/O");
  m_tree->Branch("EF_j55_u0uchad_empty_LArNoiseBurst",&m_nt_EF_j55_u0uchad_empty_LArNoiseBurst,"EF_j55_u0uchad_empty_LArNoiseBurst/O");
  m_tree->Branch("EF_xe45_LArNoiseBurst",&m_nt_EF_xe45_LArNoiseBurst,"EF_xe45_LArNoiseBurst/O");
  m_tree->Branch("EF_xe55_LArNoiseBurst",&m_nt_EF_xe55_LArNoiseBurst,"EF_xe55_LArNoiseBurst/O");
  m_tree->Branch("EF_xe60_LArNoiseBurst",&m_nt_EF_xe60_LArNoiseBurst,"EF_xe60_LArNoiseBurst/O");
  m_tree->Branch("EF_j55_u0uchad_firstempty_LArNoiseBurstT",&m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurstT,"EF_j55_u0uchad_firstempty_LArNoiseBurstT/O");
  m_tree->Branch("EF_j100_u0uchad_LArNoiseBurstT",&m_nt_EF_j100_u0uchad_LArNoiseBurstT,"EF_j100_u0uchad_LArNoiseBurstT/O");
  m_tree->Branch("EF_j165_u0uchad_LArNoiseBurstT",&m_nt_EF_j165_u0uchad_LArNoiseBurstT,"EF_j165_u0uchad_LArNoiseBurstT/O");
  m_tree->Branch("EF_j130_u0uchad_LArNoiseBurstT",&m_nt_EF_j130_u0uchad_LArNoiseBurstT,"EF_j130_u0uchad_LArNoiseBurstT/O");
  m_tree->Branch("EF_j35_u0uchad_empty_LArNoiseBurst",&m_nt_EF_j35_u0uchad_empty_LArNoiseBurst,"EF_j35_u0uchad_empty_LArNoiseBurst/O");
  m_tree->Branch("EF_j35_u0uchad_firstempty_LArNoiseBurst",&m_nt_EF_j35_u0uchad_firstempty_LArNoiseBurst,"EF_j35_u0uchad_firstempty_LArNoiseBurst/O");
  m_tree->Branch("EF_j80_u0uchad_LArNoiseBurstT",&m_nt_EF_j80_u0uchad_LArNoiseBurstT,"EF_j80_u0uchad_LArNoiseBurstT/O");
  
  // 
  m_tree->Branch("LArCellSize", &m_nt_larcellsize,"LArCellSize/I"); // NEW number of online conected LAr cells
  m_tree->Branch("CaloCellSize",&m_nt_cellsize,"CaloCellSize/I");// NEW number of total cells.
  // LAr time difference as computed by LArCollisionTime info
  m_tree->Branch("LArTime_Diff",&m_nt_ECTimeDiff,"LArTime_Diff/F"); // time diff between 2 endcaps
  m_tree->Branch("LArTime_Avg",&m_nt_ECTimeAvg,"LArTime_Avg/F"); // time average of 2 endcaps
  m_tree->Branch("LArTime_nCellA",&m_nt_nCellA,"LArTime_nCellA/I"); // nb of cells used to compute endcap A time
  m_tree->Branch("LArTime_nCellC",&m_nt_nCellC,"LArTime_nCellC/I"); // nb of cells used to compute endcap C time

  // Event properties related to yield of channels in 3 sigma tails 
  m_tree->Branch("PerCentNoisyCell",&m_nt_noisycellpercent,"PerCentNoisyCell/F"); // Yield of channels in 3sigma tails in whole LAr
  m_tree->Branch("PerCentNoisyCellPartition",&m_nt_noisycellpart); // Yield in each partition:0:embc 1:emba 2:emecc 3:emeca 4:fcalc 5:fcala 6:hecc 7:heca

  // LArNoisyRO output
  m_tree->Branch("LArNoisyRO_Std", &m_nt_larnoisyro,"LArNoisyRO_Std/S"); // standard flag (>5 FEB with more than 30 cells with q factor > 4000)
  m_tree->Branch("LArNoisyRO_SatMedium",&m_nt_larnoisyro_satOne,"LArNoisyRO_SatMedium/S"); //  medium flag (> 9 cells with E>500MeV and saturated q factor) 
  m_tree->Branch("LArNoisyRO_SatTight",&m_nt_larnoisyro_satTwo,"LArNoisyRO_SatTight/S"); // tight flag (> 20 cells with E>1000MeV and saturated q factor) 

  // Properties of cells with fabs(energy/noise)>3
  m_tree->Branch("NoisyCellPartitionLayerIndex",&m_nt_cellpartlayerindex); /// NEW Identifier of the cell
  m_tree->Branch("NoisyCellOnlineIdentifier",&m_nt_cellIdentifier); // Identifier of the noisy cell
  m_tree->Branch("NoisyCellPartition",&m_nt_partition); // Partition in 1 integer: 0:embc 1:emba 2:emecc 3:emeca 4:fcalc 5:fcala 6:hecc 7:heca
  m_tree->Branch("NoisyCellFT",&m_nt_ft_noisy);                        // FT 
  m_tree->Branch("NoisyCellSlot",&m_nt_slot_noisy);                    // Slot
  m_tree->Branch("NoisyCellChannel",&m_nt_channel_noisy);              // Channel
  m_tree->Branch("NoisyCellADCvalues", &m_nt_samples);                 // ADC values
  m_tree->Branch("NoisyCellGain",&m_nt_gain);                          // Gain
  m_tree->Branch("NoisyCellPhi",&m_nt_phicell);                        // Phi
  m_tree->Branch("NoisyCellEta",&m_nt_etacell);                        // Eta
  m_tree->Branch("NoisyCellLayer", &m_nt_layer);                       // layer
  m_tree->Branch("NoisyCellHVphi", &m_nt_noisycellHVphi);              // Phi of HV
  m_tree->Branch("NoisyCellHVeta", &m_nt_noisycellHVeta);              // Eta of HV
  m_tree->Branch("NoisyCellEnergy",&m_nt_energycell);                  // Energy
  m_tree->Branch("NoisyCellSignificance",&m_nt_signifcell);            // Significance (energy/noise)
  m_tree->Branch("NoisyCellQFactor",&m_nt_qfactorcell);                // Q factor
  m_tree->Branch("NoisyCellIsBad",&m_nt_isbadcell);                    // Bad channel status
  // Event properties related to q factor
  //  m_tree->Branch("nNoisyCell",n_noisycell,"nNoisyCell/i"); // Nb of cells in 3 sigma tails - redundant with PerCentNoisyCell - Removed by BT 5/1/11
  m_tree->Branch("nbLowQFactor", &m_lowqfactor,"m_lowqfactor/i"); // Nb of cells per event with q factor<1000
  m_tree->Branch("nbMedQFactor", &m_medqfactor,"m_medqfactor/i"); // Nb of cells per event with 1000<q factor<10000
  m_tree->Branch("nbHighQFactor", &m_hiqfactor,"m_hiqfactor/i");  // Nb of cells per event with q 10000<factor<65535
  m_tree->Branch("nbSatQFactor",&m_nb_sat,"nbSat/i"); // Nb of cells per event with saturated q factor (65535) 
  // Properties of cells with q factor saturated
  m_tree->Branch("SatCellPartition",&m_nt_partition_sat);
  m_tree->Branch("SatCellBarrelEc",&m_nt_barrelec_sat);
  m_tree->Branch("SatCellPosNeg",&m_nt_posneg_sat);
  m_tree->Branch("SatCellFT",&m_nt_ft_sat);
  m_tree->Branch("SatCellSlot",&m_nt_slot_sat);
  m_tree->Branch("SatCellChannel",&m_nt_channel_sat);
  m_tree->Branch("SatCellEnergy",&m_nt_energy_sat);
  m_tree->Branch("SatCellPhi", &m_nt_phicell_sat);
  m_tree->Branch("SatCellEta",&m_nt_etacell_sat);
  m_tree->Branch("SatCellLayer",&m_nt_layer_sat);
  m_tree->Branch("SatCellIsBad", &m_nt_isbadcell_sat);
  m_tree->Branch("SatCellOnlineIdentifier",&m_nt_cellIdentifier_sat);

  //HV information
  m_tree->Branch("HV_Nominal", &m_nt_nominalhv); //Not yet filled
  m_tree->Branch("HV_Maximal",&m_nt_maximalhv);  // Not yet filled

  // Electrons
  m_tree->Branch("el_n",&m_nt_el_n,"el_n/I");
  m_tree->Branch("el_cl_E",&m_nt_el_cl_E);
  m_tree->Branch("el_cl_Et",&m_nt_el_cl_Et);
  m_tree->Branch("el_cl_eta",&m_nt_el_cl_eta);
  m_tree->Branch("el_cl_phi",&m_nt_el_cl_phi);
  m_tree->Branch("el_author",&m_nt_el_author);
  m_tree->Branch("el_loosePP",&m_nt_el_loosePP);
  m_tree->Branch("el_mediumPP",&m_nt_el_mediumPP);
  m_tree->Branch("el_tightPP",&m_nt_el_tightPP);

  if (sc.isFailure()) { 
     mLog << MSG::ERROR << "ROOT Hist registration failed" << endreq; 
     return sc; 
  }

  mLog << MSG::DEBUG << "End of Initializing LArNoiseBursts" << endreq;
 
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////
/// Finalize - delete any memory allocation from the heap

StatusCode LArNoiseBursts::finalize() {
  MsgStream mLog( messageService(), name() );

  mLog << MSG::DEBUG << "in finalize()" << endreq;
  return StatusCode::SUCCESS;

}

///////////////////////////////////////////////////////////////////////////////////
/// Clear - clear CBNT members
StatusCode LArNoiseBursts::clear() {
  /// For Athena-Aware NTuple

  MsgStream mLog( messageService(), name() );

  mLog << MSG::DEBUG << "start clearing variables " << endreq;
  
  m_nb_sat     = 0;
  n_noisycell  = 0;
  m_lowqfactor = 0;
  m_medqfactor = 0;
  m_hiqfactor  = 0;

  m_nt_evtId      = 0;
  m_nt_evtTime    = 0;
  m_nt_evtTime_ns = 0;
  m_nt_lb         = 0;
  m_nt_bcid       = -1;
 
  m_nt_npv       = -1;
  m_nt_npvtracks = -1;
  m_nt_ntracks   = -1;

  m_nt_streamTagName.clear();
  m_nt_streamTagType.clear();
  m_nt_isbcidFilled     = -1;
  m_nt_isbcidInTrain    = -1;
  m_nt_isBunchesInFront.clear();
  m_nt_bunchtype        = -1;
  m_nt_bunchtime        = -1.0;
  m_nt_atlasready       = -1;
  m_nt_stablebeams      = -1;
  m_nt_larnoisyro       = -1;
  m_nt_larnoisyro_satOne= -1;
  m_nt_larnoisyro_satTwo= -1;

  mLog << MSG::DEBUG << "clearing event info veto variables " << endreq;

  //clearing event info veto variables
  m_nt_veto_mbts        = -1;
  m_nt_veto_pixel       = -1;
  m_nt_veto_sct         = -1;
  m_nt_veto_bcm         = -1;
  m_nt_veto_lucid       = -1;
  m_nt_veto_mbtstdHalo  = -1;
  m_nt_veto_mbtstdCol   = -1;
  m_nt_veto_lartdHalo   = -1;
  m_nt_veto_lartdCol    = -1;
  m_nt_veto_csctdHalo   = -1;
  m_nt_veto_csctdCol    = -1;
  m_nt_veto_bcmtHalo    = -1;
  m_nt_veto_bcmtCol     = -1;
  m_nt_veto_muontCol    = -1;
  m_nt_veto_muontCosmic = -1;

  mLog << MSG::DEBUG << "clearing LAr event flags " << endreq;

  // lar bit event info
  m_nt_larflag_badFEBs = false;
  m_nt_larflag_mediumSaturatedDQ = false;
  m_nt_larflag_tightSaturatedDQ = false;
  m_nt_larflag_noiseBurstVeto = false;
  m_nt_larflag_dataCorrupted = false;
  m_nt_larflag_dataCorruptedVeto = false;

  mLog << MSG::DEBUG << "clearing Pixel variables " << endreq;

  // Trigger flags
  m_nt_L1_J75 = true; // turned on for tests
  m_nt_L1_J10_EMPTY = false;
  m_nt_L1_J30_FIRSTEMPTY = false;
  m_nt_L1_J30_EMPTY = false;
  m_nt_L1_XE40 = false;
  m_nt_L1_XE50 = false;
  m_nt_L1_XE50_BGRP7 = false;
  m_nt_L1_XE70 = false;
  
  m_nt_EF_j165_u0uchad_LArNoiseBurst = false;
  m_nt_EF_j30_u0uchad_empty_LArNoiseBurst = false;
  m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurst = false;
  m_nt_EF_j55_u0uchad_empty_LArNoiseBurst = false;
  m_nt_EF_xe45_LArNoiseBurst = false;
  m_nt_EF_xe55_LArNoiseBurst = false;
  m_nt_EF_xe60_LArNoiseBurst = false;
  m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurstT = false;
  m_nt_EF_j100_u0uchad_LArNoiseBurstT = false;
  m_nt_EF_j165_u0uchad_LArNoiseBurstT = false;
  m_nt_EF_j130_u0uchad_LArNoiseBurstT = false;
  m_nt_EF_j35_u0uchad_empty_LArNoiseBurst = false;
  m_nt_EF_j35_u0uchad_firstempty_LArNoiseBurst = false;
  m_nt_EF_j80_u0uchad_LArNoiseBurstT = false;
   
  mLog << MSG::DEBUG << "clearing trigger flags " << endreq;

  mLog << MSG::DEBUG << "clearing noisy cells variables " << endreq;

  //Quantities for noisy cells
  m_nt_energycell.clear();
  m_nt_qfactorcell.clear();
  m_nt_phicell.clear();
  m_nt_etacell.clear();
  m_nt_signifcell.clear();
  m_nt_isbadcell.clear();
  m_nt_partition.clear();
  m_nt_layer.clear();
  m_nt_noisycellpercent = -1;
  m_nt_ft_noisy.clear();
  m_nt_slot_noisy.clear();
  m_nt_channel_noisy.clear();
  m_nt_larcellsize = -1;
  m_nt_cellsize    = -1;
  m_nt_cellpartlayerindex.clear();
  m_nt_cellIdentifier.clear();
  m_nt_noisycellpart.clear();
  m_nt_samples.clear();
  m_nt_gain.clear();

  ///HV branches
  m_nt_nominalhv.clear();
  m_nt_maximalhv.clear();
  m_nt_noisycellHVphi.clear();
  m_nt_noisycellHVeta.clear();

  mLog << MSG::DEBUG << "clearing saturated cell variables " << endreq;

  //clearing quantities for sat cells
  m_nt_barrelec_sat.clear();
  m_nt_posneg_sat.clear();
  m_nt_ft_sat.clear();
  m_nt_slot_sat.clear();
  m_nt_channel_sat.clear();
  m_nt_partition_sat.clear();
  m_nt_energy_sat.clear();
  m_nt_phicell_sat.clear();
  m_nt_etacell_sat.clear();
  m_nt_layer_sat.clear();
  m_nt_cellIdentifier_sat.clear();
  m_nt_isbadcell_sat.clear();

  mLog << MSG::DEBUG << "clearing LArTimeDiff variables " << endreq;
  
  //DiffTime computed with LAR
  m_nt_ECTimeDiff  = 9999;
  m_nt_ECTimeAvg   = 9999;
  m_nt_nCellA      = -1;
  m_nt_nCellC      = -1;

  mLog << MSG::DEBUG << "clearing electron variables " << endreq;
  
  // Electrons
  m_nt_el_n = 0;
  m_nt_el_cl_E.clear();
  m_nt_el_cl_Et.clear();
  m_nt_el_cl_eta.clear();
  m_nt_el_cl_phi.clear(); 
  m_nt_el_author.clear();
  m_nt_el_loosePP.clear();
  m_nt_el_mediumPP.clear();
  m_nt_el_tightPP.clear(); 
  
  mLog << MSG::DEBUG << "end of clearing" << endreq;

  return StatusCode::SUCCESS;


 
}

//////////////////////////////////////////////////////////////////////////////////
/// Execute - on event by event

StatusCode LArNoiseBursts::execute() {
  MsgStream mLog( messageService(), name() );
  
  mLog << MSG::DEBUG << "in execute()" << endreq;
  StatusCode sc;

  sc = clear();
  if(sc.isFailure()) {
    mLog << MSG::WARNING << "The method clear() failed" << endreq;
    return StatusCode::SUCCESS;
  }
  
  sc = doTrigger();
  if(sc.isFailure()) {
     mLog << MSG::WARNING << "The method doTrigger() failed" << endreq;
    return StatusCode::SUCCESS;
  }

  sc = doEventProperties();
  if(sc.isFailure()) {
     mLog << MSG::WARNING << "The method doEventProperties() failed" << endreq;
    return StatusCode::SUCCESS;
  }

  sc = doLArNoiseBursts();
  if (sc.isFailure()) {
      mLog << MSG::WARNING << "The method doLArNoiseBursts() failed" << endreq;
      return StatusCode::SUCCESS;
  }

  sc = doPhysicsObjects();
  if (sc.isFailure()) {
      mLog << MSG::WARNING << "The method doPhysicsObjects() failed" << endreq;
      return StatusCode::SUCCESS;
  }
  
  m_tree->Fill();

  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
///////////////////          doTrigger        ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
StatusCode LArNoiseBursts::doTrigger(){
  MsgStream mLog(messageService(), name()); 
  mLog<< MSG::DEBUG <<"in doTrigger "<<endreq;
  
  std::string mychain( "L1_J75" );
  if( ! m_trigDec->getListOfTriggers(mychain).empty() ){
    m_nt_L1_J75 = m_trigDec->isPassed( mychain );
  }
  mychain = "L1_J10_EMPTY";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_J10_EMPTY =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_J30_FIRSTEMPTY";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_J30_FIRSTEMPTY =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_J30_EMPTY";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_J30_EMPTY =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_XE40";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_XE40 =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_XE50";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_XE50 =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_XE50_BGRP7";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_XE50_BGRP7 =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_XE70";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_XE70 =  m_trigDec->isPassed( mychain );
  }

  // EF
  mychain = "EF_j165_u0uchad_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j165_u0uchad_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j30_u0uchad_empty_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j30_u0uchad_empty_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j55_u0uchad_firstempty_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j55_u0uchad_empty_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j55_u0uchad_empty_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_xe45_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_xe45_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_xe55_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_xe55_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_xe60_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_xe60_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j55_u0uchad_firstempty_LArNoiseBurstT";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurstT =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j100_u0uchad_LArNoiseBurstT";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j100_u0uchad_LArNoiseBurstT =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j165_u0uchad_LArNoiseBurstT";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j165_u0uchad_LArNoiseBurstT =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j130_u0uchad_LArNoiseBurstT";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j130_u0uchad_LArNoiseBurstT =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j35_u0uchad_empty_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j35_u0uchad_empty_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j35_u0uchad_firstempty_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j35_u0uchad_firstempty_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j80_u0uchad_LArNoiseBurstT";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j80_u0uchad_LArNoiseBurstT =  m_trigDec->isPassed( mychain );
  }
 
  return StatusCode::SUCCESS;

}

//////////////////////////////////////////////////////////////////////////////////////
///////////////////          doEventProperties        //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
StatusCode LArNoiseBursts::doEventProperties(){
  MsgStream mLog(messageService(), name()); 
  mLog<< MSG::DEBUG <<"in doEventProperties "<<endreq;


  //////////////////////////////// EventInfo variables /////////////////////////////////////////////////
  // Retrieve event info
  const EventInfo* eventInfo;
  StatusCode sc = m_storeGate->retrieve(eventInfo);
  if (sc.isFailure()) {
    mLog << MSG::WARNING << "Event info not found !" << endreq;
    return StatusCode::SUCCESS;
  }

  int run = eventInfo->runNumber();
  m_nt_evtId      = eventInfo->eventNumber();
  m_nt_evtTime    = eventInfo->timeStamp();
  m_nt_evtTime_ns = eventInfo->timeStampNSOffset();
  m_nt_lb         = eventInfo->lumiBlock();
  m_nt_bcid       = eventInfo->bcid();

  mLog<<MSG::DEBUG<<"Run Number: "<<run<<", event Id "<<m_nt_evtId<<", bcid = "<<m_nt_bcid<<endreq;
  

  m_nt_isbcidFilled = m_bc_tool->isFilled(m_nt_bcid);
  m_nt_isbcidInTrain = m_bc_tool->isInTrain(m_nt_bcid);
  m_nt_bunchtype = m_bc_tool->bcType(m_nt_bcid);
  mLog<<MSG::DEBUG<<"BCID is Filled: "<<m_nt_isbcidFilled<<endreq;
  mLog<<MSG::DEBUG<<"BCID is in Train: "<<m_nt_isbcidInTrain<<endreq;
  mLog<<MSG::DEBUG<<"bunch type "<<m_nt_bunchtype<<endreq;

  std::vector<bool> isBunchesInFront = m_bc_tool->bunchesInFront(m_nt_bcid,m_frontbunches);
  bool checkfirstbunch = true;
  for(unsigned int i=0;i<isBunchesInFront.size();i++){
     mLog<<MSG::DEBUG<<"bunch "<<i<<" is Filled "<<isBunchesInFront[i]<<endreq;
     m_nt_isBunchesInFront.push_back(isBunchesInFront[i]);
       if(isBunchesInFront[i]==1){
         if(i!=0){
           if(checkfirstbunch){
             float time = 25.0*i;
             m_nt_bunchtime = time;
             mLog<<MSG::DEBUG<<"next bunch time: "<<time<<" ns "<<endreq;
             checkfirstbunch = false;
	   }
	 }
     }
  }

  m_CosmicCaloStream = false;
  //std::vector<TriggerInfo::StreamTag>::const_iterator streamInfoIt=myTriggerInfo->streamTags().begin();
  //std::vector<TriggerInfo::StreamTag>::const_iterator streamInfoIt_e=myTriggerInfo->streamTags().end();
  //for (;streamInfoIt!=streamInfoIt_e;streamInfoIt++) { 
  for (const auto& streamInfo : eventInfo->streamTags()) {
    const std::string& stream_name = streamInfo.name();
    const std::string& stream_type = streamInfo.type();
    m_nt_streamTagName.push_back(stream_name);
    m_nt_streamTagType.push_back(stream_type);
    mLog<<MSG::DEBUG<<"event stream tag name "<<streamInfo.name()<<endreq;
    mLog<<MSG::DEBUG<<"event stream tag type "<<streamInfo.type()<<endreq;
    if(streamInfo.name()=="CosmicCalo" && streamInfo.type()=="physics"){
      m_CosmicCaloStream = true;
    }
  }
  

  mLog<<MSG::DEBUG<<"CosmicCalo stream value: "<<m_CosmicCaloStream<<endreq;

  // Retrieve output of LArNoisyRO
  bool larnoisyro = eventInfo->isEventFlagBitSet(EventInfo::LAr,0);
  bool larnoisyro_satOne = eventInfo->isEventFlagBitSet(EventInfo::LAr,1);
  bool larnoisyro_satTwo = eventInfo->isEventFlagBitSet(EventInfo::LAr,2);
  m_nt_larnoisyro        = larnoisyro ? 1 : 0;
  m_nt_larnoisyro_satOne = larnoisyro_satOne ? 1 : 0;
  m_nt_larnoisyro_satTwo = larnoisyro_satTwo ? 1 : 0;
  
 // Retrieve output of EventInfo veto
  mLog << MSG::DEBUG <<"Background: MBTSBeamVeto "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::MBTSBeamVeto)<<endreq;
  mLog << MSG::DEBUG <<"Background: PixSPNonEmpty "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::PixSPNonEmpty)<<endreq;
  mLog << MSG::DEBUG <<"Background: SCTSPNonEmpty "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::SCTSPNonEmpty)<<endreq;
  mLog << MSG::DEBUG <<"Background: BCMBeamVeto "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::BCMBeamVeto)<<endreq;
  mLog << MSG::DEBUG <<"Background: LUCIDBeamVeto "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::LUCIDBeamVeto)<<endreq;
  mLog << MSG::DEBUG <<"Background: MBTSTimeDiffHalo "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::MBTSTimeDiffHalo)<<endreq; 
  mLog << MSG::DEBUG <<"Background: MBTSTimeDiffCol "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::MBTSTimeDiffCol)<<endreq;
  mLog << MSG::DEBUG <<"Background: LArECTimeDiffHalo "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::LArECTimeDiffHalo)<<endreq;
  mLog << MSG::DEBUG <<"Background: LArECTimeDiffCol "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::LArECTimeDiffCol)<<endreq;  
  mLog << MSG::DEBUG <<"Background: CSCTimeDiffHalo "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::CSCTimeDiffHalo)<<endreq;
  mLog << MSG::DEBUG <<"Background: CSCTimeDiffCol "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::CSCTimeDiffCol)<<endreq;
  mLog << MSG::DEBUG <<"Background: BCMTimeDiffHalo "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::BCMTimeDiffHalo)<<endreq;
  mLog << MSG::DEBUG <<"Background: BCMTimeDiffCol "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::BCMTimeDiffCol)<<endreq;
  mLog << MSG::DEBUG <<"Background: MuonTimmingCol "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::MuonTimingCol)<<endreq;
  mLog << MSG::DEBUG <<"Background: MuonTimmingCosmic "<<eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::MuonTimingCosmic)<<endreq;

  m_nt_veto_mbts      = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::MBTSBeamVeto);
  m_nt_veto_pixel     = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::PixSPNonEmpty);
  m_nt_veto_sct       = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::SCTSPNonEmpty);
  m_nt_veto_bcm       = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::BCMBeamVeto);
  m_nt_veto_lucid     = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::LUCIDBeamVeto);

  //more variables
  m_nt_veto_mbtstdHalo = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::MBTSTimeDiffHalo); 
  m_nt_veto_mbtstdCol  = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::MBTSTimeDiffCol);
  m_nt_veto_lartdHalo  = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::LArECTimeDiffHalo);
  m_nt_veto_lartdCol   = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::LArECTimeDiffCol);  
  m_nt_veto_csctdHalo  = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::CSCTimeDiffHalo);
  m_nt_veto_csctdCol   = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::CSCTimeDiffCol);
  m_nt_veto_bcmtHalo   = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::BCMTimeDiffHalo);
  m_nt_veto_bcmtCol    = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::BCMTimeDiffCol);
  m_nt_veto_muontCol   = eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::MuonTimingCol);
  m_nt_veto_muontCosmic= eventInfo->isEventFlagBitSet(EventInfo::Background,EventInfo::MuonTimingCosmic);
 
   // LArEventInfo

  mLog << MSG::DEBUG <<"NOISEBURSTVETO bit " << eventInfo->isEventFlagBitSet(EventInfo::LAr,LArEventBitInfo::NOISEBURSTVETO) << endreq;
  mLog << MSG::DEBUG <<"BADFEBS bit " << eventInfo->isEventFlagBitSet(EventInfo::LAr,LArEventBitInfo::BADFEBS)<< endreq;
  mLog << MSG::DEBUG <<"TIGHTSATURATEDQ bit " << eventInfo->isEventFlagBitSet(EventInfo::LAr,LArEventBitInfo::TIGHTSATURATEDQ)<< endreq;

  m_nt_larflag_badFEBs = eventInfo->isEventFlagBitSet(EventInfo::LAr,LArEventBitInfo::BADFEBS);
  m_nt_larflag_mediumSaturatedDQ = eventInfo->isEventFlagBitSet(EventInfo::LAr,LArEventBitInfo::MEDIUMSATURATEDQ);
  m_nt_larflag_tightSaturatedDQ = eventInfo->isEventFlagBitSet(EventInfo::LAr,LArEventBitInfo::TIGHTSATURATEDQ);
  m_nt_larflag_noiseBurstVeto = eventInfo->isEventFlagBitSet(EventInfo::LAr,LArEventBitInfo::NOISEBURSTVETO);
  m_nt_larflag_dataCorrupted = eventInfo->isEventFlagBitSet(EventInfo::LAr,LArEventBitInfo::DATACORRUPTED);
  m_nt_larflag_dataCorruptedVeto = eventInfo->isEventFlagBitSet(EventInfo::LAr,LArEventBitInfo::DATACORRUPTEDVETO);

  ///////////////////////////////////////end EventInfo variables/////////////////////////////////////////////////////////////////////////

  const AthenaAttributeList* attrList(0);
  sc =  m_storeGate->retrieve(attrList, "/TDAQ/RunCtrl/DataTakingMode");
  if (sc.isFailure()) {
     mLog << MSG::WARNING <<"Unable to retrieve DataTakingMode information; falling back to" << endreq;
     return StatusCode::SUCCESS;
   }
   if (attrList != 0) {
     mLog << MSG::DEBUG <<"ReadyForPhysics is: " << (*attrList)["ReadyForPhysics"].data<uint32_t>()<<endreq;
     //m_valueCache = ((*attrList)["ReadyForPhysics"].data<uint32_t>() != 0);
     m_nt_atlasready = (*attrList)["ReadyForPhysics"].data<uint32_t>();
   }

   /*const AthenaAttributeList* fillstate(0);
  sc =  m_storeGate->retrieve(fillstate, "/LHC/DCS/FILLSTATE");
  if (sc.isFailure()) {
     mLog << MSG::WARNING <<"Unable to retrieve fillstate information; falling back to" << endreq;
     return StatusCode::SUCCESS;
   }
   if (fillstate != 0) {
     mLog << MSG::DEBUG <<"Stable beams is: " << (*fillstate)["StableBeams"].data<uint32_t>()<<endreq;
     //m_valueCache = ((*attrList)["ReadyForPhysics"].data<uint32_t>() != 0);
     m_nt_stablebeams.push_back((*fillstate)["StableBeams"].data<uint32_t>());
     }*/

   //retrieve primary vertex information
   const VxContainer* vxContainer = NULL;
  sc = m_storeGate->retrieve(vxContainer,"VxPrimaryCandidate" );
  if (sc.isFailure()) {
     mLog << MSG::WARNING << "Could not retrieve primary vertex info: "<<endreq;
     //return StatusCode::SUCCESS;
  }

   mLog << MSG::DEBUG << "Found primary vertex info: " << endreq;


  if(vxContainer) {
    m_nt_npv = vxContainer->size();
    if(m_nt_npv>0){
    mLog<< MSG::DEBUG<< "vxContainer size = "<<m_nt_npv<<endreq;
    VxContainer::const_iterator fz = vxContainer->begin();
    const Trk::VxCandidate* vxcand = (*fz);
    const std::vector<Trk::VxTrackAtVertex*>* trklist = vxcand->vxTrackAtVertex();
    mLog<< MSG::DEBUG<< "--- Number of tracks asociated to P.V: "<<trklist->size()<<endreq;
    m_nt_npvtracks = trklist->size();
    }
  }

  //retrieve track particle container information
   const Rec::TrackParticleContainer* trackTES=0;
   sc=m_storeGate->retrieve( trackTES, "TrackParticleCandidate");
   if( sc.isFailure()) {
     mLog << MSG::WARNING<< "No InDet container found in TDS  "<<endreq;
   }
   if(trackTES){
     mLog << MSG::DEBUG <<"number of reconstructed tracks = "<<trackTES->size()<<endreq;
     m_nt_ntracks = trackTES->size();
   }
  
  // 29/11/10 : Debug messages removed by BT 
  //   mLog << MSG::INFO << "Event LAr flags " << std::hex
  //      << eventInfo->errorState(EventInfo::LAr) << " "
  //      << std::hex << eventInfo->eventFlags(EventInfo::LAr)
  //      << ", bit 0: " << eventInfo->isEventFlagBitSet(EventInfo::LAr,0)
  //      << ", bit 1: " << eventInfo->isEventFlagBitSet(EventInfo::LAr,1)
  //      << ", bit 2: " << eventInfo->isEventFlagBitSet(EventInfo::LAr,2)
  //      << endreq;

  // Retrieve LArCollision Timing information
  const LArCollisionTime *  larTime;
  sc =  m_storeGate->retrieve(larTime,"LArCollisionTime");
  if( sc.isFailure()){
    mLog << MSG::WARNING  << "Unable to retrieve LArCollisionTime event store" << endreq;
    //return StatusCode::SUCCESS; // Check if failure shd be returned. VB
  }else {
    mLog << MSG::DEBUG  << "LArCollisionTime successfully retrieved from event store" << endreq;
  }
  
  if (larTime) {
      m_nt_nCellA = larTime->ncellA();
      m_nt_nCellC = larTime->ncellC();
    if(m_nt_nCellA>0 && m_nt_nCellC>0){
      // Calculate the time diff between ECC and ECA
      m_nt_ECTimeDiff = larTime->timeC() - larTime->timeA();
      m_nt_ECTimeAvg  = (larTime->timeC() + larTime->timeA()) / 2.0;
      mLog << MSG::DEBUG  << "LAr: Calculated time difference of " << m_nt_ECTimeDiff << " ns" << endreq;
    }
  }

  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
///////////////////          doLArNoiseBursts        ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
StatusCode LArNoiseBursts::doLArNoiseBursts(){
 MsgStream mLog( messageService(), name() );
 mLog << MSG::DEBUG << "in doLarCellInfo " << endreq;


 // Retrieve LAr calocells container
  const CaloCellContainer* caloTES;
  StatusCode sc = m_storeGate->retrieve( caloTES, "AllCalo");
  if (sc.isFailure()) {
    mLog << MSG::WARNING << "CaloCell Container not found!" << endreq;
    return StatusCode::SUCCESS;
  }

  m_nb_sat = 0;
  n_noisycell =0;
  int nlarcell = 0;
  CaloCellContainer::const_iterator caloItr;
  int n_noisy_cell_part[8] = {0,0,0,0,0,0,0,0};
  int n_cell_part[8] = {0,0,0,0,0,0,0,0};
  std::vector<short> m_ft_noisy, m_slot_noisy, m_channel_noisy;
  std::vector<bool> m_isbarrel, m_isendcap, m_isfcal, m_ishec;
  std::vector<short> m_layer; 
  std::vector<int> m_partition,m_noisycellHVphi,m_noisycellHVeta;
  std::vector<float> m_energycell, m_qfactorcell, m_signifcell;
  std::vector<float> m_phicell, m_etacell;
  std::vector<bool> m_isbadcell;
  std::vector<IdentifierHash>  m_IdHash;
  std::vector<int> m_cellpartlayerindex;
  std::vector<Identifier> m_cellIdentifier;

  m_ft_noisy.clear();m_slot_noisy.clear();m_channel_noisy.clear();
  m_isbarrel.clear();m_isendcap.clear();m_isfcal.clear();m_ishec.clear();
  m_layer.clear();m_partition.clear();m_energycell.clear();m_qfactorcell.clear(); 
  m_phicell.clear();m_etacell.clear();m_signifcell.clear();m_isbadcell.clear();
  m_IdHash.clear();m_noisycellHVeta.clear();m_noisycellHVphi.clear();
  m_cellpartlayerindex.clear();m_cellIdentifier.clear();

  for(caloItr=caloTES->begin();caloItr!=caloTES->end();caloItr++){
    const CaloDetDescrElement* caloDDE = (*caloItr)->caloDDE();
    if (caloDDE->is_tile())continue;
    HWIdentifier onlID;
    try {onlID = m_LArCablingService->createSignalChannelID((*caloItr)->ID());}
    catch(LArID_Exception& except) {
	    mLog << MSG::ERROR  << "LArID_Exception " << m_LArEM_IDHelper->show_to_string((*caloItr)->ID()) << " " << (std::string) except << endreq ;
	    mLog << MSG::ERROR  << "LArID_Exception " << m_LArHEC_IDHelper->show_to_string((*caloItr)->ID()) << endreq;
	    mLog << MSG::ERROR  << "LArID_Exception " << m_LArFCAL_IDHelper->show_to_string((*caloItr)->ID()) << endreq;
            continue;
    }
    bool connected = m_LArCablingService->isOnlineConnected(onlID);
    if(!connected) continue;
    const Identifier idd = m_LArCablingService->cnvToIdentifier(onlID);
    nlarcell++;
    IdentifierHash channelHash = m_LArOnlineIDHelper->channel_Hash(onlID);
    int layer = caloDDE->getLayer();
    //    CaloCell_ID::CaloSample sampling = (*caloItr)->caloDDE()->getSampling();
    float eCalo = (*caloItr)->energy();
    float qfactor = (*caloItr)->quality();
    float phi = (*caloItr)->phi();
    //float noise  = m_calo_noise_tool->getNoise( (*caloItr), ICaloNoiseTool::TOTALNOISE );
    float noise  = m_calo_noise_tool->totalNoiseRMS( (*caloItr), (*caloItr)->gain());
    float significance = eCalo / noise ;
    float eta = (*caloItr)->eta();
    bool badcell = (*caloItr)->badcell();
    unsigned int partition = 0;
    bool is_lar_em_barrel = caloDDE->is_lar_em_barrel();
    if(is_lar_em_barrel){
       if(eta<0){
          partition = 0;
        }else{
          partition = 1;
        }
    }//barrel
    bool is_lar_em_endcap = caloDDE->is_lar_em_endcap();
    if(is_lar_em_endcap){
        if(eta<0){
          partition = 2;
        }else{
          partition = 3;
	}
    }//endcap
    bool is_lar_fcal   = caloDDE->is_lar_fcal();
    if(is_lar_fcal){
        if(eta<0){
          partition = 4;
        }else{
          partition = 5;
	}
    }//fcal
    bool is_lar_hec    = caloDDE->is_lar_hec();
    if(is_lar_hec){
      if(eta<0){
        partition = 6;
      }else{
        partition = 7;
      }
    }//hec
    for(unsigned int k=0;k<8;k++){
       if(partition==k){
          n_cell_part[k]++;
       }
    }
    if(qfactor <1000)                  {m_lowqfactor++;}
    if(qfactor>=1000 && qfactor<10000) {m_medqfactor++;}
    if(qfactor>=10000 && qfactor<65535){m_hiqfactor++;}
    if(qfactor==65535){
      mLog << MSG::DEBUG <<"Satured cell at eta: "<<eta<<", phi: "<<phi<<", partition: "<<partition<<endreq;
       m_nb_sat = m_nb_sat +1;
       m_nt_partition_sat.push_back(partition);
       m_nt_energy_sat.push_back(eCalo);
       m_nt_cellIdentifier_sat.push_back((m_LArCablingService->cnvToIdentifier(onlID)).get_identifier32().get_compact());
       if(!m_keepOnlyCellID){
         m_nt_barrelec_sat.push_back(m_LArOnlineIDHelper->barrel_ec(onlID));
         m_nt_posneg_sat.push_back(m_LArOnlineIDHelper->pos_neg(onlID));
         m_nt_ft_sat.push_back(m_LArOnlineIDHelper->feedthrough(onlID));
         m_nt_slot_sat.push_back(m_LArOnlineIDHelper->slot(onlID));
         m_nt_channel_sat.push_back(m_LArOnlineIDHelper->channel(onlID));
         m_nt_phicell_sat.push_back(phi);
         m_nt_etacell_sat.push_back(eta);
         m_nt_layer_sat.push_back(layer);
         m_nt_isbadcell_sat.push_back(badcell);
       }
    }
    // Store all cells in positive and negative 3 sigma tails...
    if(significance > m_sigmacut || qfactor > 4000){
      m_ft_noisy.push_back(m_LArOnlineIDHelper->feedthrough(onlID));
      m_slot_noisy.push_back(m_LArOnlineIDHelper->slot(onlID));
      m_channel_noisy.push_back(m_LArOnlineIDHelper->channel(onlID));

      /*
      m_isbarrel.push_back(is_lar_em_barrel);
      m_isendcap.push_back(is_lar_em_endcap);
      m_isfcal.push_back(is_lar_fcal);
      m_ishec.push_back(is_lar_hec);
      */

      m_layer.push_back(layer);
      m_energycell.push_back(eCalo);
      m_qfactorcell.push_back(qfactor);
      m_phicell.push_back(phi);
      m_etacell.push_back(eta);
      m_signifcell.push_back(significance);
      m_isbadcell.push_back(badcell);
      m_partition.push_back(partition);
      m_IdHash.push_back(channelHash);
      m_cellIdentifier.push_back(m_LArCablingService->cnvToIdentifier(onlID));
    // ...but count only cells in positive 3 sigma tails!
      if (significance > m_sigmacut){
	for(unsigned int k=0;k<8;k++){
	  if(partition==k){
	    n_noisy_cell_part[k]++;
	  }
	}
      }
      int caloindex = GetPartitionLayerIndex(idd);
      m_cellpartlayerindex.push_back(caloindex);
      //store HV information
      /*std::vector<int> * hvid = new std::vector<int>;
      hvid.clear();
      //hvid = GetHVLines(idd);
      std::vector<std::pair<LArHVCellID, int> > hv_vector;
      hv_vector.clear();
      m_LArHVCablingTool->getLArHVCellID(idd,hv_vector);
      if(hv_vector.size()>0){
        for(unsigned int i=0;i<hv_vector.size();i++){
	  int nominal_HV =  m_LArHVCablingTool->getNominalHV(hv_vector[i].first);//fill the branch but produces lots of warnings. 
	  int maximal_HV =  m_LArHVCablingTool->getMaximalHV(hv_vector[i].first);//fill the branch but produces lots of warnings.
	    m_nt_nominalhv.push_back(nominal_HV);
            m_nt_maximalhv.push_back(maximal_HV);
	}
	}*/
      m_noisycellHVphi.push_back(m_LArElectrodeIDHelper->hv_phi(onlID));
      m_noisycellHVeta.push_back(m_LArElectrodeIDHelper->hv_eta(onlID));
      n_noisycell++;
    }   

  }//loop over cells
  
  m_nt_larcellsize = nlarcell;
  m_nt_cellsize    = caloTES->size();
  mLog << MSG::DEBUG <<"lar cell size = "<<int(nlarcell)<<endreq;
  mLog << MSG::DEBUG <<"all cell size = "<<int(caloTES->size())<<endreq;

  m_nt_noisycellpercent = 100.0*double(n_noisycell)/double(nlarcell);
 
  bool checknoise = false;
  //ratio of noisy cells per partition
  for(unsigned int i=0;i<8;i++){
    float noise  =  100.0*(double(n_noisy_cell_part[i])/double(n_cell_part[i]));
    m_nt_noisycellpart.push_back(noise);
    if(noise> 1.0){
      checknoise = true;
      mLog << MSG::DEBUG <<"noise burst in this  event "<<endreq;
    }   
  }

  const LArDigitContainer* LArDigitCont;
  sc = m_storeGate->retrieve(LArDigitCont, "LArDigitContainer_Thinned");
  if (sc.isFailure()) {
    mLog << MSG::WARNING << "LArDigitContainer Container not found!" << endreq;
    return StatusCode::SUCCESS;
  }    

  LArDigitContainer::const_iterator it=LArDigitCont->begin();
  LArDigitContainer::const_iterator it_end=LArDigitCont->end();
  const LArDigit* pLArDigit;
  
  bool store_condition = false;
  // CosmicCalo stream : Store detailed infos of cells only if Y3Sigma>1% or burst found by LArNoisyRO
  if(m_CosmicCaloStream){
    if(checknoise==true || m_nt_larnoisyro==1 || m_nt_larnoisyro_satOne==1 || m_nt_larnoisyro_satTwo==1){
      store_condition = true;
    }
  }
  // Not cosmicCalo stream : Store detailed infos of cells only if burst found by LArNoisyRO
  if(!m_CosmicCaloStream){
    if(m_nt_larnoisyro==1 || m_nt_larnoisyro_satOne==1 || m_nt_larnoisyro_satTwo==1){
      store_condition = true;
    }
  }

  //store the information of the noisy cell only when %noisycells > 1%.
  if(store_condition){
    std::vector<short> samples;
    samples.clear();
    for(unsigned int k=0;k<m_etacell.size();k++){
      m_nt_samples.push_back(samples);
      m_nt_gain.push_back(0);
    }
    for (;it!=it_end;it++) {
         pLArDigit = *it;  
         HWIdentifier id2 = pLArDigit->hardwareID();
         IdentifierHash hashid2 = m_LArOnlineIDHelper->channel_Hash(id2);
          for(unsigned int j=0;j<m_IdHash.size();j++){
            if (hashid2 == m_IdHash[j] ){
              mLog<<MSG::DEBUG << "find a IdentifierHash of the noisy cell in LArDigit container " << endreq;
              samples = pLArDigit->samples();
              int gain=-1;
              if (pLArDigit->gain() == CaloGain::LARHIGHGAIN)   gain = 0;
              if (pLArDigit->gain() == CaloGain::LARMEDIUMGAIN) gain = 1;
              if (pLArDigit->gain() == CaloGain::LARLOWGAIN)    gain = 2;
              m_nt_gain.at(j)= gain;
              m_nt_samples.at(j) = samples;
              mLog << MSG::DEBUG << "I got the samples!" << endreq;
              break;
	    }  
	 }
     }
    for(unsigned int i=0;i<m_etacell.size();i++){
      m_nt_energycell.push_back( m_energycell[i]);
      m_nt_qfactorcell.push_back( m_qfactorcell[i]);
      m_nt_signifcell.push_back( m_signifcell[i]);
      m_nt_partition.push_back( m_partition[i]);   
      m_nt_cellIdentifier.push_back(m_cellIdentifier[i].get_identifier32().get_compact());
      if(!m_keepOnlyCellID){
        m_nt_ft_noisy.push_back( m_ft_noisy[i]);
        m_nt_slot_noisy.push_back( m_slot_noisy[i]);
        m_nt_channel_noisy.push_back( m_channel_noisy[i]);
   
        /*
        m_nt_isbarrel.push_back( m_isbarrel[i]);
        m_nt_isendcap.push_back( m_isendcap[i]);
        m_nt_isfcal.push_back( m_isfcal[i]);
        m_nt_ishec.push_back( m_ishec[i]);
        */
   
        m_nt_layer.push_back( m_layer[i]);
        m_nt_phicell.push_back( m_phicell[i]);
        m_nt_etacell.push_back( m_etacell[i]);
        m_nt_isbadcell.push_back( m_isbadcell[i]);
        m_nt_noisycellHVphi.push_back(m_noisycellHVphi[i]);     
        m_nt_noisycellHVeta.push_back(m_noisycellHVeta[i]);
        m_nt_cellpartlayerindex.push_back(m_cellpartlayerindex[i]);
      }
    }		  
    mLog << MSG::DEBUG <<"leaving if checknoise and larnoisyro"<<endreq;

  }//if(checknoisy==true ..)

  return StatusCode::SUCCESS;
  
}

//////////////////////////////////////////////////////////////////////////////////////
///////////////////          doPhysicsObjects        ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
StatusCode LArNoiseBursts::doPhysicsObjects(){
  MsgStream mLog(messageService(), name()); 
  mLog<< MSG::DEBUG <<"in doPhysicsObjects "<<endreq;

  const ElectronContainer* elecTES = 0;
  StatusCode sc=m_storeGate->retrieve( elecTES, m_elecContainerName);
  if( sc.isFailure()  ||  !elecTES ) {
    mLog << MSG::WARNING << "No ESD electron container found in StoreGate" << endreq;
    return StatusCode::SUCCESS;
  }
  mLog << MSG::DEBUG << "ElectronContainer successfully retrieved. Size = " << elecTES->size();

  ElectronContainer::const_iterator elecItr  = elecTES->begin();
  ElectronContainer::const_iterator elecItrE = elecTES->end();
  
  /// Electron
  m_nt_el_n = 0;

  for (; elecItr != elecItrE; ++elecItr) {
    m_nt_el_n++;
    
    if( (*elecItr)->cluster() ){
      m_nt_el_cl_E.push_back(float((*elecItr)->cluster()->e()));
      m_nt_el_cl_Et.push_back(float((*elecItr)->cluster()->et()));
      m_nt_el_cl_eta.push_back(float((*elecItr)->cluster()->eta()));
      m_nt_el_cl_phi.push_back(float((*elecItr)->cluster()->phi()));
    }

    m_nt_el_author.push_back((*elecItr)->author());

    (*elecItr)->isem(egammaPID::ElectronLoosePP) == 0 ? m_nt_el_loosePP.push_back(1) : m_nt_el_loosePP.push_back(0);
    (*elecItr)->isem(egammaPID::ElectronMediumPP) == 0 ? m_nt_el_mediumPP.push_back(1) : m_nt_el_mediumPP.push_back(0);
    (*elecItr)->isem(egammaPID::ElectronTightPP) == 0 ? m_nt_el_tightPP.push_back(1) : m_nt_el_tightPP.push_back(0);
  }

  return StatusCode::SUCCESS;
}


//

int LArNoiseBursts::GetPartitionLayerIndex(const Identifier& id)
{
  // O.Simard -- GetPartitionLayer index [0,32]:
  // Returns a single integer code that corresponds to the
  // mapping of CaloCells in CaloMonitoring histogram frames.

  int caloindex = -1;
  int bc = 0;
  int sampling = 0;

  if(m_LArEM_IDHelper->is_lar_em(id)){ // EMB
    bc = m_LArEM_IDHelper->barrel_ec(id);
    sampling = m_LArEM_IDHelper->sampling(id);
    if(abs(bc)==1){
      if(bc<0) caloindex = sampling+4;
      else caloindex = sampling;
    } else { // EMEC
      if(bc<0) caloindex = sampling+12;
      else caloindex = sampling+8;
    }
  } else if(m_LArHEC_IDHelper->is_lar_hec(id)) { // LAr HEC
    bc = m_LArHEC_IDHelper->pos_neg(id);
    sampling = m_LArHEC_IDHelper->sampling(id);
    if(bc<0) caloindex = sampling+20;
    else caloindex = sampling+16;
  } else if(m_LArFCAL_IDHelper->is_lar_fcal(id)) { // LAr FCAL
    bc = m_LArFCAL_IDHelper->pos_neg(id);
    sampling = (int)m_LArFCAL_IDHelper->module(id); // module instead of sampling
    if(bc<0) caloindex = sampling+26;
    else caloindex = sampling+23;
  }

  return caloindex;
}

std::vector<int>* LArNoiseBursts::GetHVLines(const Identifier& id)
{

  MsgStream mLog( messageService(), name() );
  mLog << MSG::DEBUG << "in GetHVLines function " << endreq;

  std::vector<int>  tmplines;
  unsigned int nelec = 0;
  //unsigned int ngap = 0;
  unsigned int nsubgaps = 0;
  unsigned int nlines = 0;
  unsigned int i=0,j=0,igap=0;

  // LAr EMB
  if(m_LArEM_IDHelper->is_lar_em(id) && m_LArEM_IDHelper->sampling(id)>0){
  mLog << MSG::DEBUG << "LAr EMB"<< endreq;
    if(abs(m_LArEM_IDHelper->barrel_ec(id))==1) {
      const EMBDetectorElement* embElement = dynamic_cast<const EMBDetectorElement*>(m_calodetdescrmgr->get_element(id));
      if (!embElement)
        return 0;
      const EMBCellConstLink cell = embElement->getEMBCell();
      nelec = cell->getNumElectrodes();
      //ngap = 2*nelec;
      for(i=0;i<nelec;i++) {
	      const EMBHVElectrodeConstLink electrode = cell->getElectrode(i);
	      for(igap=0;igap<2;igap++) tmplines.push_back(electrode->hvLineNo(igap));
      }        
    } else { // LAr EMEC
      mLog << MSG::DEBUG << "LAr EMEC"<< endreq;
      const EMECDetectorElement* emecElement = dynamic_cast<const EMECDetectorElement*>(m_calodetdescrmgr->get_element(id));
      if (!emecElement)
        return 0;
      const EMECCellConstLink cell = emecElement->getEMECCell();
      nelec = cell->getNumElectrodes();
      //ngap = 2*nelec;
      for(i=0;i<nelec;i++) {
	      const EMECHVElectrodeConstLink electrode = cell->getElectrode(i);
	      for(igap=0;igap<2;igap++) tmplines.push_back(electrode->hvLineNo(igap));
      }      
    }
  } else if(m_LArHEC_IDHelper->is_lar_hec(id)) { // LAr HEC
    mLog << MSG::DEBUG << "LAr HEC"<< endreq;
    const HECDetectorElement* hecElement = dynamic_cast<const HECDetectorElement*>(m_calodetdescrmgr->get_element(id));
    if (!hecElement)
      return 0;
    const HECCellConstLink cell = hecElement->getHECCell();
    nsubgaps = cell->getNumSubgaps();
    for(i=0;i<nsubgaps;i++) {
      const HECHVSubgapConstLink subgap = cell->getSubgap(i);
      tmplines.push_back(subgap->hvLineNo());
    }
  } else if(m_LArFCAL_IDHelper->is_lar_fcal(id)) { // LAr FCAL
    mLog << MSG::DEBUG << "LAr HEC"<< endreq;
    const FCALDetectorElement* fcalElement = dynamic_cast<const FCALDetectorElement*>(m_calodetdescrmgr->get_element(id));
    if (!fcalElement)
      return 0;
    const FCALTile* tile = fcalElement->getFCALTile();
    nlines = tile->getNumHVLines();
    for(i=0;i<nlines;i++) {
      const FCALHVLineConstLink line = tile->getHVLine(i);
      if(line) tmplines.push_back(line->hvLineNo());
    }   
  } else if(m_LArEM_IDHelper->is_lar_em(id) && m_LArEM_IDHelper->sampling(id)==0) { // Presamplers
    mLog << MSG::DEBUG << "LAr PRESAMPLES"<< endreq;
    
    if(abs(m_LArEM_IDHelper->barrel_ec(id))==1){
      
      const EMBDetectorElement* embElement = dynamic_cast<const EMBDetectorElement*>(m_calodetdescrmgr->get_element(id));
      if (!embElement) {
        return 0;
      }
      const EMBCellConstLink cell = embElement->getEMBCell();
      const EMBPresamplerHVModuleConstLink hvmodule =  cell->getPresamplerHVModule();
      for(igap=0;igap<2;igap++) tmplines.push_back(hvmodule->hvLineNo(igap));

    } else {
      
      const EMECDetectorElement* emecElement = dynamic_cast<const EMECDetectorElement*>(m_calodetdescrmgr->get_element(id));
      if (!emecElement)
        return 0;
      const EMECCellConstLink cell = emecElement->getEMECCell();
      const EMECPresamplerHVModuleConstLink hvmodule = cell->getPresamplerHVModule();
      for(igap=0;igap<2;igap++) tmplines.push_back(hvmodule->hvLineNo(igap));

    }
  } else {
    mLog << MSG::WARNING << " cell neither in EM nor HEC nor FCAL !!!!!  return empty HV " << endreq;
    return (vector<int>*)0;
  }

  // optimization of storage: replace by std:set
  std::vector<int>* hvlines = new std::vector<int>;
  bool found = 0;
  for(i=0;i<tmplines.size();i++){
    found = 0;
    for(j=0;j<hvlines->size();j++){
      if(hvlines->at(j)==tmplines.at(i)){
        found=1;
        break;
      }
    }
    if(!found) hvlines->push_back(tmplines.at(i));
  }

  return hvlines;
}


 

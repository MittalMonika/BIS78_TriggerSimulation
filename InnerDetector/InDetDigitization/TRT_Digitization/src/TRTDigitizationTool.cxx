/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRTDigitizationTool.cxx
//
//   Implementation file for class TRTDigitizationTool
//
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TRTDigitizationTool.h"
#include "HitManagement/TimedHitCollection.h"
#include "InDetSimEvent/TRTUncompressedHitCollection.h"//hide?
#include "TRTProcessingOfStraw.h"
#include "TRTElectronicsProcessing.h"

#include "TRTDigCondBase.h"
#include "TRTDigCondFakeMap.h"

#include "TRTNoise.h"

#include "TRTDigSimpleTimer.h"

#include "TRTElectronicsNoise.h"

#include "TRTFastORAlg.h"

#include "Identifier/Identifier.h"
#include "InDetSimData/InDetSimData.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "TRT_PAI_Process/ITRT_PAITool.h"
#include "TRT_Digitization/ITRT_SimDriftTimeTool.h"
#include "InDetSimEvent/TRTHitIdHelper.h"

#include "GeneratorObjects/HepMcParticleLink.h"

// Det descr includes:
#include "InDetIdentifier/TRT_ID.h"
#include "InDetReadoutGeometry/TRT_DetectorManager.h"
#include "TRTDigSettings.h"

#include "InDetRawData/TRT_LoLumRawData.h"
#include "InDetRawData/TRT_RDO_Collection.h"

// Other includes
#include "PileUpTools/PileUpMergeSvc.h"
#include "AthenaKernel/IAtRndmGenSvc.h"

// particle table
#include "HepPDT/ParticleDataTable.hh"
#include "GaudiKernel/IPartPropSvc.h"

// for the tool
#include "GaudiKernel/IToolSvc.h"

// Gaudi includes
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/SmartDataPtr.h"

#include <set>
#include <sstream>
#include "TRT_ConditionsServices/ITRT_StrawNeighbourSvc.h"

//CondDB
#include "AthenaPoolUtilities/AthenaAttributeList.h"

//AJB Temporary for debugging and development
//#include "driftCircle.h"

//_____________________________________________________________________________
TRTDigitizationTool::TRTDigitizationTool(const std::string& type,
					 const std::string& name,
					 const IInterface* parent)
  : PileUpToolBase(type, name, parent),
    m_TRTpaiTool("TRT_PAI_Process"),
    m_TRTpaiTool_optional("TRT_PAI_Process_optional"),
    m_TRTsimdrifttimetool("TRT_SimDriftTimeTool"),
    m_mergeSvc("PileUpMergeSvc",name),
    m_pElectronicsProcessing(NULL),
    m_pProcessingOfStraw(NULL),
    m_pDigConditions(NULL),
    m_pNoise(NULL),
    m_container(NULL),
    m_timer_eventcount(0),
    m_atRndmGenSvc ("AtDSFMTGenSvc", name),
    m_TRTStrawNeighbourSvc("TRT_StrawNeighbourSvc",name),
    m_manager(NULL),
    m_trt_id(NULL),
    m_thpctrt(NULL),
    m_deadstraws(false),
    m_alreadyPrintedPDGcodeWarning(false),
    m_minCrossingTimeSDO(0.0),
    m_maxCrossingTimeSDO(0.0),
    m_minpileuptruthEkin(0.0),
    m_override_deadStrawFraction(0.0),
    m_ComTime(NULL),
    m_particleTable(NULL),
    m_dig_vers_from_condDB(-1),
    m_digverscontainerkey("/TRT/Cond/DigVers"),
    m_first_event(true),
    m_condDBdigverfoldersexists(false),
    m_HardScatterSplittingMode(0),
    m_HardScatterSplittingSkipper(false),
    m_UseArgonStraws(false), // added by Sasha for Argon
    m_useConditionsHTStatus(false), // added by Sasha for Argon
    m_particleFlag(0),
    m_sumSvc("TRT_StrawStatusSummarySvc","TRT_StrawStatusSummarySvc") // added by Sasha for Argon

{

  declareInterface<TRTDigitizationTool>(this);
  declareProperty("PAI_Tool", m_TRTpaiTool, "The PAI model for ionisation in the TRT gas" );
  declareProperty("PAI_Tool_optional", m_TRTpaiTool_optional, "The optional PAI model for ionisation in the TRT gas" );
  declareProperty("SimDriftTimeTool", m_TRTsimdrifttimetool, "Drift time versus distance (r-t-relation) for TRT straws" );
  declareProperty("MergeSvc", m_mergeSvc, "Merge service" );
  declareProperty("DataObjectName", m_dataObjectName="TRTUncompressedHits", "Data Object Name" );
  declareProperty("OutputObjectName", m_outputRDOCollName="TRT_RDOs",            "Output Object name" );
  declareProperty("OutputSDOName", m_outputSDOCollName="TRT_SDO_Map",            "Output SDO container name");

  declareProperty("PrintOverrideableSettings", m_printOverrideableSettings = true, "Print overrideable settings" );
  declareProperty("PrintDigSettings", m_printUsedDigSettings = true, "Print ditigization settings" );

  m_settings = new TRTDigSettings();
  m_settings->addPropertiesForOverrideableParameters(static_cast<AlgTool*>(this));
  declareProperty("RndmSvc",                       m_atRndmGenSvc, "Random Number Service used in TRT digitization" );
  declareProperty("TRT_StrawNeighbourSvc",         m_TRTStrawNeighbourSvc);
  declareProperty("InDetTRTStrawStatusSummarySvc", m_sumSvc);  // need for Argon
  declareProperty("UseArgonStraws",                m_UseArgonStraws); // need for Argon
  declareProperty("UseConditionsHTStatus",         m_useConditionsHTStatus); // need for Argon
  declareProperty("HardScatterSplittingMode",      m_HardScatterSplittingMode);

  m_fastOR = new TRTFastORAlg();

}

//_____________________________________________________________________________
StatusCode TRTDigitizationTool::initialize()
{

  TRTDigSimpleTimer timer;//fixme - remove timer stuff (?)
  TRTDigSimpleTimer timer_getpaiservice;
  TRTDigSimpleTimer timer_getdrifttimetoolservice;
  TRTDigSimpleTimer timer_getservices;

  timer.start();
  timer_getservices.start();

  ATH_MSG_DEBUG ( "TRTDigitization::initialize() begin" );

  // Get the TRT Detector Manager
  if (StatusCode::SUCCESS != detStore()->retrieve(m_manager,"TRT") ) {
    ATH_MSG_ERROR ( "Can't get TRT_DetectorManager " );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG ( "Retrieved TRT_DetectorManager with version "  << m_manager->getVersion().majorNum() );
  }

  if (detStore()->retrieve(m_trt_id, "TRT_ID").isFailure()) {
    ATH_MSG_ERROR ( "Could not get TRT ID helper" );
    return StatusCode::FAILURE;
  }

  // Fill setting defaults and process joboption overrides:
  m_settings->initialize(m_manager);

  if (m_printOverrideableSettings) {
    m_settings->printFlagsForOverrideableParameters("TRTDigSettings Overrideables : ");
  }

  /// Get the PAI Tool:

  timer_getpaiservice.start();
  if ( StatusCode::SUCCESS != m_TRTpaiTool.retrieve() ) {
    ATH_MSG_ERROR ( "Can't get the PAI Tool" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG ( "Retrieved the PAI Tool" );
  }

  if ( StatusCode::SUCCESS != m_TRTpaiTool_optional.retrieve() ) {
    /// FIXME Sasha: this warning will appear every time when someone will run default setting for pure Xenon condition
    ATH_MSG_WARNING ( "Can't get the optional PAI Tool --> default PAI tool will be used for all straws (okay, if you don't use Argon straws)" );
  } else {
    ATH_MSG_DEBUG ( "Retrieved the optional PAI Tool" );
  }
  timer_getpaiservice.stop();

  /// Get the Sim-DriftTime Tool:
  timer_getdrifttimetoolservice.start();
  if ( StatusCode::SUCCESS != m_TRTsimdrifttimetool.retrieve() ) {
    ATH_MSG_ERROR ( "Can't get the Sim. Drifttime Tool" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG ( "Retrieved the Sim. Drifttime Tool" );
  }
  timer_getdrifttimetoolservice.stop();

  // Get Random Service
  if (!m_atRndmGenSvc.retrieve().isSuccess()) {
    ATH_MSG_FATAL ( "Could not initialize Random Number Service." );
    return StatusCode::FAILURE;
  }

  // Get the Particle Properties Service
  IPartPropSvc* p_PartPropSvc = 0;
  static const bool CREATEIFNOTTHERE(true);
  StatusCode PartPropStatus = service("PartPropSvc", p_PartPropSvc, CREATEIFNOTTHERE);
  if ( !PartPropStatus.isSuccess() || 0 == p_PartPropSvc ) {
    ATH_MSG_ERROR ( "Could not initialize Particle Properties Service" );
    return PartPropStatus;
  }
  m_particleTable = p_PartPropSvc->PDT();

  //locate the PileUpMergeSvc and initialize our local ptr
  if (!m_mergeSvc.retrieve().isSuccess()) {
    ATH_MSG_ERROR ( "Could not find PileUpMergeSvc" );
    return StatusCode::FAILURE;
  }

  //Retrieve TRT_StrawNeighbourService.
  if (!m_TRTStrawNeighbourSvc.retrieve().isSuccess()) {
    ATH_MSG_FATAL ( "Could not get StrawNeighbourSvc!" );
    return StatusCode::FAILURE;
  }

  // init the FastOR class
  if(StatusCode::SUCCESS != m_fastOR->initialize() ){
    ATH_MSG_ERROR ( "Could not get TRT FASTOR" );
    return StatusCode::FAILURE;
  }

  timer_getservices.stop();

  //   // Create new  TRT_RDO_Container
  //   m_container = new TRT_RDO_Container(m_trt_id->straw_layer_hash_max());
  //   ATH_MSG_DEBUG ( " TRT_RDO_Container created " );
  //   // Prevent SG from deleting object
  //   m_container->addRef();
  //   ATH_MSG_DEBUG ( " TRT_RDO_Container created " );

  // Check data object name
  if (m_dataObjectName == "")  {
    ATH_MSG_FATAL ( "Property DataObjectName not set!" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG ( "Input hits: " << m_dataObjectName );
  }

  timer.stop();
  if (msgLvl(MSG::DEBUG)) {
    msg(MSG::DEBUG) << "Time spent in TRTDigitization::initialize was " << timer.check() << " seconds " << endreq;
    msg(MSG::DEBUG) << "Time spent getting pai tool was " << timer_getpaiservice.check() <<" seconds " << endreq;
    msg(MSG::DEBUG) << "Time spent getting sim. drifttime tool was " << timer_getdrifttimetoolservice.check() <<" seconds " << endreq;
    msg(MSG::DEBUG) << "Time spent getting other services was " << timer_getservices.check()-timer_getpaiservice.check() << " seconds " << endreq;
    msg(MSG::DEBUG) << "TRTDigitization::initialize() done" << endreq;
  }

  m_minpileuptruthEkin = m_settings->pileUpSDOsMinEkin();

  // Set SDO readout range
  const double intervalBetweenCrossings(m_settings->timeInterval() / 3.);

  switch ( (m_settings->storeSDO() )) {
  case 0:  m_minCrossingTimeSDO =  -1.*CLHEP::ns;  m_maxCrossingTimeSDO = -1.*CLHEP::ns; break;
  case 1:  m_minCrossingTimeSDO =  -1.*CLHEP::ns;  m_maxCrossingTimeSDO = (intervalBetweenCrossings * 2. + 1.*CLHEP::ns); break;
  case 2:  m_minCrossingTimeSDO = -(intervalBetweenCrossings * 2. + 1.*CLHEP::ns);  m_maxCrossingTimeSDO = (intervalBetweenCrossings * 2. + 1.*CLHEP::ns); break;
  default:
    ATH_MSG_ERROR ( "storeSDO is out of range:"<<m_settings->storeSDO()<<"allowed values are: 0,1,2. Setting storeSDO = 2 " );
    m_minCrossingTimeSDO = -51.*CLHEP::ns;  m_maxCrossingTimeSDO = 51.*CLHEP::ns;
    break;
  }

  //CondDB access
  m_condDBdigverfoldersexists = detStore()->StoreGateSvc::contains<AthenaAttributeList>(m_digverscontainerkey) ;
  // Register callback function for cache updates:
  if (m_condDBdigverfoldersexists) {
    const DataHandle<AthenaAttributeList> aptr; //CondAttrListCollection
    if (StatusCode::SUCCESS == detStore()->regFcn(&TRTDigitizationTool::update,this, aptr, m_digverscontainerkey )) {
      ATH_MSG_DEBUG ("Registered callback for TRT_Digitization.");
    } else {
      ATH_MSG_ERROR ("Callback registration failed for TRT_Digitization! ");
    }
  }

  m_fastOR_HTvector.clear();
  m_fastOR_chipId.clear();
  m_fastOR_phiId.clear();
  m_fastOR_barrelEC.clear();
  m_fastOR_particleFlag.clear();

  return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
StatusCode TRTDigitizationTool::prepareEvent(unsigned int)
{

  m_vDigits.clear();
  m_trtHitCollList.clear();
  m_thpctrt = new TimedHitCollection<TRTUncompressedHit>();
  m_HardScatterSplittingSkipper = false;

  return StatusCode::SUCCESS;

}

//_____________________________________________________________________________
StatusCode TRTDigitizationTool::processBunchXing(int bunchXing,
						 PileUpEventInfo::SubEvent::const_iterator bSubEvents,
						 PileUpEventInfo::SubEvent::const_iterator eSubEvents) {

  m_seen.push_back(std::make_pair(std::distance(bSubEvents,eSubEvents), bunchXing));
  //decide if this event will be processed depending on HardScatterSplittingMode & bunchXing
  if (m_HardScatterSplittingMode == 2 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; return StatusCode::SUCCESS; }
  if (m_HardScatterSplittingMode == 1 && m_HardScatterSplittingSkipper )  { return StatusCode::SUCCESS; }
  if (m_HardScatterSplittingMode == 1 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; }

  PileUpEventInfo::SubEvent::const_iterator iEvt(bSubEvents);

  while (iEvt != eSubEvents) {
    StoreGateSvc& seStore(*iEvt->pSubEvtSG);
    PileUpTimeEventIndex thisEventIndex(PileUpTimeEventIndex(static_cast<int>(iEvt->time()),iEvt->index()));
    const TRTUncompressedHitCollection* seHitColl(NULL);
    if (!seStore.retrieve(seHitColl,m_dataObjectName).isSuccess()) {
      ATH_MSG_ERROR ( "SubEvent TRTUncompressedHitCollection not found in StoreGate " << seStore.name() );
      return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE ( "TRTUncompressedHitCollection found with " << seHitColl->size() << " hits" );
    //Copy Hit Collection
    TRTUncompressedHitCollection* trtHitColl(new TRTUncompressedHitCollection("TRTUncompressedHits"));
    TRTUncompressedHitCollection::const_iterator i(seHitColl->begin());
    TRTUncompressedHitCollection::const_iterator e(seHitColl->end());
    // Read hits from this collection
    for (; i!=e; ++i) {
      const TRTUncompressedHit trthit(*i);
      trtHitColl->Insert(trthit);
    }
    m_thpctrt->insert(thisEventIndex, trtHitColl);
    //store these for deletion at the end of mergeEvent
    m_trtHitCollList.push_back(trtHitColl);

    ++iEvt;
  }

  return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
StatusCode TRTDigitizationTool::lateInitialize() {

  m_first_event=false;

  if (m_condDBdigverfoldersexists) {

    if ( ConditionsDependingInitialization().isFailure() ) {
      ATH_MSG_ERROR ( "Folder holder TRT digitization version exists in condDB, but tag is faulty" );
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_DEBUG ( "Using Digitization version as defined in conditions tag" );
    }

  } else {
    ATH_MSG_DEBUG ( "No folder containing TRT digitization version found in CondDB. Using default from Det Desc tag: " << m_settings->digVers() );
  }

  //Resuming initialiazation. Section below had to be moved into event loop due to dependence on conditions data

  TRTElectronicsNoise *electronicsNoise(NULL);
  if ( m_settings->noiseInUnhitStraws() || m_settings->noiseInSimhits() ) {
    electronicsNoise = new TRTElectronicsNoise(m_settings, m_atRndmGenSvc);
  }
  // ElectronicsProcessing is needed for the regular straw processing,
  // but also for the noise (it assumes ownership of electronicsnoise )
  m_pElectronicsProcessing = new TRTElectronicsProcessing( m_settings, m_atRndmGenSvc, electronicsNoise );

  m_pDigConditions = new TRTDigCondFakeMap(m_settings,
					   m_manager,
					   m_atRndmGenSvc,
					   m_trt_id,
					   m_UseArgonStraws,
					   m_useConditionsHTStatus,
					   m_sumSvc);

  m_pDigConditions->initialize();

  m_deadstraws = ( m_pDigConditions->totalNumberOfDeadStraws() > 0 );

  if ( m_settings->noiseInUnhitStraws() || m_settings->noiseInSimhits() ) {

    // In short this next constructor does 3 things;
    //     i)  tunes the amplitude of the electronics noise,
    //    ii) creates a pool of noise digits,
    //   iii) figures out exact low thresholds needed to reproduce actual
    //        straw noise-frequencies:
    m_pNoise = new TRTNoise( m_settings,
			     m_manager,
			     m_atRndmGenSvc,
			     m_pDigConditions,
			     m_pElectronicsProcessing,
			     electronicsNoise,
			     m_trt_id,
			     m_UseArgonStraws,
			     m_useConditionsHTStatus,
			     m_sumSvc);

    ATH_MSG_DEBUG ( "Average straw noise level is " << m_pDigConditions->strawAverageNoiseLevel() );

  } else {
    m_pNoise = NULL;
  }

  ITRT_PAITool *TRTpaiTool = &(* m_TRTpaiTool);
  // optional PAI tool for Argon straws simulation
  ITRT_PAITool *TRTpaiTool_optional = &(* m_TRTpaiTool_optional);
  ITRT_SimDriftTimeTool *pTRTsimdrifttimetool = &(*m_TRTsimdrifttimetool);

  m_pProcessingOfStraw =
    new TRTProcessingOfStraw( m_settings,
			      m_manager,
			      TRTpaiTool,
			      pTRTsimdrifttimetool,
			      m_atRndmGenSvc,
			      m_pElectronicsProcessing,
			      m_pNoise,
			      m_pDigConditions,
			      m_particleTable,
			      m_trt_id,
			      m_UseArgonStraws,
			      TRTpaiTool_optional);
  return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
StatusCode TRTDigitizationTool::processStraws(std::set<int>& sim_hitids, std::set<Identifier>& simhitsIdentifiers) {

  // Create a map for the SDO
  InDetSimDataCollection *simDataMap(new InDetSimDataCollection);

  // Register the map into StoreGate
  if ( evtStore()->record(simDataMap, m_outputSDOCollName).isFailure() ) {
    ATH_MSG_FATAL ( "InDetSimData map " << m_outputSDOCollName << " could not be registered in StoreGate !" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG ( "InDetSimData map " << m_outputSDOCollName << " registered in StoreGate" );
  }

  // loop over all straws
  TimedHitCollection<TRTUncompressedHit>::const_iterator i, e;
  while (m_thpctrt->nextDetectorElement(i, e)) {

    int hitID((*i)->GetHitID()); // Get hitID

    // evtIndex should be 0 for main event and 1,2,3,... for pileup events:
    // (event Id is a property of the TimedHitPtr)
    HepMcParticleLink::index_type evtIndex(i->eventId());

    if ( m_settings->noiseInUnhitStraws() ) {
      sim_hitids.insert(hitID);
    }
    //Safeguard against a rare case of hitID corruption found by Davide:
    if ( hitID & 0xc0000000 ) {
      ATH_MSG_ERROR ( "Hit ID not Valid (" << MSG::hex << hitID << ")" << MSG::dec );
      continue;
    }

    if ( m_deadstraws && m_pDigConditions->strawIsDead(hitID) ) {
      ATH_MSG_VERBOSE ( "Skipping simhit in dead straw" );
      continue;
    }

    // Convert hitID to Identifier
    IdentifierHash IdHash;
    Identifier idLayer;
    bool identifierOK;
    Identifier idStraw(getIdentifier(hitID, IdHash, idLayer, identifierOK));
    if ( !identifierOK ) {
      ATH_MSG_ERROR ( "Ignoring simhits with suspicious identifier (1)" );
      continue;
    }

    //For crosstalk sim
    simhitsIdentifiers.insert(idStraw);

    ///// START OF SDO CREATION
    // Create and fill a vector of deposits
    std::vector<InDetSimData::Deposit> depositVector;
    depositVector.reserve(std::distance(i,e));
    for (TimedHitCollection<TRTUncompressedHit>::const_iterator hit_iter(i); hit_iter != e; ++hit_iter ) {

      // create a new deposit
      InDetSimData::Deposit deposit( HepMcParticleLink((*hit_iter)->GetTrackID(), hit_iter->eventId()), (*hit_iter)->GetEnergyDeposit() );
      if(deposit.first.barcode()==0 || deposit.first.barcode() == 10001){
          continue;
      }
      ATH_MSG_VERBOSE ( "Deposit: trackID " << deposit.first << " energyDeposit " << deposit.second );
      depositVector.push_back(deposit);
      // evtIndex != 0 ? hit_iter->eventId() : 0 ;   commented for now. Once mc09 campaign is over, consider to implement + implement cuts on hitTime in addition/in stead bunchcrossing cuts.
    }

    const TimedHitPtr<TRTUncompressedHit>& theHit(*i);
    //double globalHitTime(hitTime(theHit));
    //double globalTime(static_cast<double>(theHit->GetGlobalTime()));
    //double bunchCrossingTime(globalHitTime - globalTime);
    const double bunchCrossingTime(hitTime(theHit) - static_cast<double>(theHit->GetGlobalTime()));

    // Add the simdata object to the map.
    if ( depositVector.size() &&
	 (evtIndex == 0 || ((*i)->GetKineticEnergy()>m_minpileuptruthEkin))  &&
	 (bunchCrossingTime < m_maxCrossingTimeSDO) && (bunchCrossingTime > m_minCrossingTimeSDO) ) {
      simDataMap->insert(std::make_pair(idStraw, InDetSimData(depositVector)));
    }
    ///// END OF SDO CREATION

    // Digitization for the given straw
    TRTDigit digit_straw;

    std::vector<int> fastOrHtDiscriminator;

    // Get the ComTime tool
    if (m_settings->doCosmicTimingPit()) {
      if ( StatusCode::SUCCESS == evtStore()->retrieve(m_ComTime,"ComTime")) {
	ATH_MSG_VERBOSE ( "Found tool for cosmic timing: ComTime" );
      } else {
        ATH_MSG_ERROR ( "Did not find tool needed for cosmic timing: ComTime" );
      }
    }

    // This will be set in TRTProcessingOfStraw::ProcessStraw
    // according to what types of particles hit the straw.
    m_particleFlag=0;

    // fixme: do we need to pass the straw ID?
    // Sasha: yes, we need to pass strawID (in such way we save some CPU time) otherwise we need to perform again the same
    //        steps as in function TRTDigitization::getIdentifier(...) to extract strawID from hitID -> more CPU time
    m_pProcessingOfStraw->ProcessStraw(i, e, digit_straw,
                                       fastOrHtDiscriminator,
                                       m_alreadyPrintedPDGcodeWarning,
                                       m_ComTime,
                                       IsArgonStraw(idStraw),
                                       m_particleFlag);

    // query m_particleFlag bits 0 to 15 (left-to-right)
    //std::cout << "AJB "; for (unsigned i=0;i<16;i++) std::cout << particleFlagQueryBit(i,m_particleFlag); std::cout << std::endl;

    //AJB, Print out the digits etc (for debugging)
    //unsigned int mword = digit_straw.GetDigit();
    //print_mword_properties(mword); // AJB
    //std::cout << "AJB "; bits24(mword);
    //std::cout << "AJB "; bits27(mword);

/*
    // AJB get Barrel or Endcap.
    if ( digit_straw.GetDigit() ) {
      int ichip = 0;
      m_TRTStrawNeighbourSvc->getChip(idStraw, ichip);
      int detid = m_trt_id->barrel_ec(idLayer);
      if (detid==1)  std::cout << "AJBBA " << highLevel(mword) << std::endl;
      if (detid==-1) std::cout << "AJBBC " << highLevel(mword) << std::endl;
      if (detid==-2) std::cout << "AJBEC " << highLevel(mword) << std::endl;
      if (detid==2)  std::cout << "AJBEA " << highLevel(mword) << std::endl;
    }
*/

    // finally (before the FastOr part) push back the output digit.
    if ( digit_straw.GetDigit() ) {
      m_vDigits.push_back(digit_straw);
    }

    ///////////////////////
    // FastOR processing //
    ///////////////////////
    // If this straw has non-empty detailed highThreshold imformation (fastOrHtDiscriminator) then push it to
    // the set of m_fastOR vectors. By default the doFastOR bool is false and so this will normally be empty.

    if (fastOrHtDiscriminator.size() > 0) {

      //std::cout << "AJB ";
      //for (unsigned int i=0; i<fastOrHtDiscriminator.size(); ++i) std::cout << fastOrHtDiscriminator.at(i);
      //std::cout << std::endl;

      // get the chip ID using the TRTStrawNeighbourSvc class
      int i_chip = 0;
      m_TRTStrawNeighbourSvc->getChip(idStraw, i_chip);

      int side = m_trt_id->barrel_ec(idLayer);
      int idLayerWheel = m_trt_id->layer_or_wheel(idLayer);
      // int Layer     = m_trt_id->straw_layer(idLayer); // not required
      int phi_id       = m_trt_id->phi_module(idLayer);

      // get the chip ID for the barrel, shift the chip ID per layer
      // in order to have continuous numbering from 0 - 103
      if( abs( side ) == 1 ) //Barrel
        {
          if( idLayerWheel == 0 ) i_chip -= 1;
          if( idLayerWheel == 1 ) i_chip += 20;
          if( idLayerWheel == 2 ) i_chip += 53;
        }

      // the standard side of the detector is 1, -1 for the barrel sides A and C and 2, -2 for the ECs
      // in order to reduce the amount of information since we only need to determine if the HIP comes
      // from the side A or C, we use zero for side A and 1 for side C.
      side = (side<0) ? 0 : 1;

      // Store the relevant information for the FastOR per straw
      m_fastOR_HTvector.push_back(fastOrHtDiscriminator);
      m_fastOR_chipId.push_back(i_chip);
      m_fastOR_phiId.push_back(phi_id);
      m_fastOR_barrelEC.push_back(side);
      m_fastOR_particleFlag.push_back(m_particleFlag);
      fastOrHtDiscriminator.clear();
    } // end of FastOR

  } // end of straw loop

  // Get the FastOR output for this event, and clear the vectors.
  // fixme: Currently the fastOR_output goes to std::cout!
  if (m_fastOR_HTvector.size() > 0) {

    // fixme XP: using zero as default for q and preproc
    int q=0;
    int preproc=0;
    int fastOR_output =
      m_fastOR->TRTFastORAlg_getStrawId(m_fastOR_HTvector, m_fastOR_chipId,
					m_fastOR_phiId, m_fastOR_barrelEC,
					m_fastOR_particleFlag,
					q, preproc);

    // dummy usage to silence "unused" fastOR_output
    // Fixme: make void?
    if (0) std::cout << "FastOR output -> prepro="<< preproc << " q="<< q << ": " << std::hex << fastOR_output << std::dec << std::endl;

    m_fastOR_HTvector.clear();
    m_fastOR_chipId.clear();
    m_fastOR_phiId.clear();
    m_fastOR_barrelEC.clear();
    m_fastOR_particleFlag.clear();

  }

  return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
StatusCode TRTDigitizationTool::processAllSubEvents() {

  ++m_timer_eventcount;
  m_timer_execute.start();

  if (m_first_event) {
    if(this->lateInitialize().isFailure()) {
      ATH_MSG_FATAL ( "lateInitialize method failed!" );
      return StatusCode::FAILURE;
    }
  }

  m_alreadyPrintedPDGcodeWarning = false;

  ATH_MSG_DEBUG ( "TRTDigitization::execute()" );

  //make new RDO container at each event
  try {
    m_container = new TRT_RDO_Container(m_trt_id->straw_layer_hash_max());
  } catch (std::bad_alloc) {
    ATH_MSG_FATAL ( "Could not create a new TRT_RDO_Container!" );
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG ( " TRT_RDO_Container created " );

  //  Register it to StoreGate
  if ( evtStore()->record(m_container, m_outputRDOCollName).isFailure() ) {
    ATH_MSG_FATAL ( "Container " << m_outputRDOCollName << " could not be registered in StoreGate!" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG ( "Container " << m_outputRDOCollName << " registered in StoreGate" );
  }

  m_timer_simhits.start();

  m_vDigits.clear();

  //Set of all hitid's with simhits (used for noise simulation).
  std::set<int> sim_hitids;
  std::set<Identifier> simhitsIdentifiers;

  //  get the container(s)
  typedef PileUpMergeSvc::TimedList<TRTUncompressedHitCollection>::type TimedHitCollList;

  //this is a list<pair<time_t, DataLink<TRTUncompressedHitCollection> > >
  TimedHitCollList hitCollList;
  unsigned int numberOfSimHits(0);
  if ( !(m_mergeSvc->retrieveSubEvtsData(m_dataObjectName, hitCollList, numberOfSimHits).isSuccess()) && hitCollList.size()==0 ) {
    ATH_MSG_ERROR ( "Could not fill TimedHitCollList" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG ( hitCollList.size() << " TRTUncompressedHitCollections with key " << m_dataObjectName << " found" );
  }

  // Define Hit Collection
  TimedHitCollection<TRTUncompressedHit> thpctrt(numberOfSimHits);

  //now merge all collections into one
  TimedHitCollList::iterator   iColl(hitCollList.begin());
  TimedHitCollList::iterator endColl(hitCollList.end()  );

  // loop on the hit collections
  while ( iColl != endColl ) {
    //decide if this event will be processed depending on HardScatterSplittingMode & bunchXing
    if (m_HardScatterSplittingMode == 2 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; ++iColl; continue; }
    if (m_HardScatterSplittingMode == 1 && m_HardScatterSplittingSkipper )  { ++iColl; continue; }
    if (m_HardScatterSplittingMode == 1 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; }
    const TRTUncompressedHitCollection* p_collection(iColl->second);
    thpctrt.insert(iColl->first, p_collection);
    ATH_MSG_DEBUG ( "TRTUncompressedHitCollection found with " << p_collection->size() << " hits" );
    ++iColl;
  }
  m_thpctrt = &thpctrt;

  // Process the Hits straw by straw: get the iterator pairs for given straw
  if(this->processStraws(sim_hitids, simhitsIdentifiers).isFailure()) {
    ATH_MSG_FATAL ( "processStraws method failed!" );
    return StatusCode::FAILURE;
  }

  // no more hits
  m_timer_simhits.stop();

  //Noise in straws without simhits:
  if (m_settings->noiseInUnhitStraws()) {
    const int numberOfDigitsBeforeNoise(m_vDigits.size());
    m_timer_purenoise.start();

    m_pNoise->appendPureNoiseToProperDigits(m_vDigits, sim_hitids);
    if (m_settings->doCrosstalk()) {
      m_pNoise->appendCrossTalkNoiseToProperDigits(m_vDigits, simhitsIdentifiers,m_TRTStrawNeighbourSvc);
    }

    m_timer_purenoise.stop();
    ATH_MSG_DEBUG ( " Number of digits " << m_vDigits.size() << " (" << m_vDigits.size()-numberOfDigitsBeforeNoise << " of those are pure noise)" );

    //Temporary resorting since code below assumes sorted digits. (fixme)
    m_timer_stupidsort.start();
    //Alas, we need them sorted...
    m_pNoise->sortDigits(m_vDigits);
    m_timer_stupidsort.stop();
    //End of temporary fix
  } else {
    ATH_MSG_DEBUG ( " Number of digits " << m_vDigits.size() );
  }

  // All digits are ready.
  // We just need to convert to relevant identifiers and output to storegate.

  if (createAndStoreRDOs().isFailure()) {
    ATH_MSG_FATAL ( "createAndStoreRDOs() failed!" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( "createAndStoreRDOs() succeeded" );
  }
  m_timer_execute.stop();

  return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
StatusCode TRTDigitizationTool::mergeEvent() {
  std::vector<std::pair<unsigned int, int> >::iterator ii(m_seen.begin());
  std::vector<std::pair<unsigned int, int> >::iterator ee(m_seen.end());
  while (ii != ee) {
    ATH_MSG_DEBUG( "mergeEvent: there are " << ii->first << " events in bunch xing " << ii->second );
    ++ii;
  }

  ++m_timer_eventcount;
  m_timer_execute.start();

  if (m_first_event) {
    if(this->lateInitialize().isFailure()) {
      ATH_MSG_FATAL ( "lateInitialize method failed!" );
      return StatusCode::FAILURE;
    }
  }

  m_alreadyPrintedPDGcodeWarning = false;

  ATH_MSG_DEBUG ( "TRTDigitization::execute()"  );

  //make new RDO container at each event
  try {
    m_container = new TRT_RDO_Container(m_trt_id->straw_layer_hash_max());
  } catch (std::bad_alloc) {
    ATH_MSG_FATAL ( "Could not create a new TRT_RDO_Container !" );
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG ( " TRT_RDO_Container created " );

  //  Register it to StoreGate
  if ( evtStore()->record(m_container, m_outputRDOCollName ).isFailure() ) {
    ATH_MSG_FATAL ( "Container " << m_outputRDOCollName << " could not be registered in StoreGate !" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG ( "Container " << m_outputRDOCollName << " registered in StoreGate" );
  }

  m_timer_simhits.start();

  //Set of all hitid's with simhits (used for noise simulation).
  std::set<int> sim_hitids;
  std::set<Identifier> simhitsIdentifiers;

  // Process the Hits straw by straw:
  //   get the iterator pairs for given straw
  if(this->processStraws(sim_hitids, simhitsIdentifiers).isFailure()) {
    ATH_MSG_FATAL ( "processStraws method failed!" );
    return StatusCode::FAILURE;
  }

  delete m_thpctrt;
  std::list<TRTUncompressedHitCollection*>::iterator trtHitColl(m_trtHitCollList.begin());
  std::list<TRTUncompressedHitCollection*>::iterator trtHitCollEnd(m_trtHitCollList.end());
  while(trtHitColl!=trtHitCollEnd)
    {
      delete (*trtHitColl);
      ++trtHitColl;
    }
  m_trtHitCollList.clear();
  // no more hits
  m_timer_simhits.stop();

  //Noise in straws without simhits:
  if (m_settings->noiseInUnhitStraws()) {
    const unsigned int numberOfDigitsBeforeNoise(m_vDigits.size());
    m_timer_purenoise.start();

    m_pNoise->appendPureNoiseToProperDigits(m_vDigits, sim_hitids);
    if (m_settings->doCrosstalk()) {
      m_pNoise->appendCrossTalkNoiseToProperDigits(m_vDigits, simhitsIdentifiers,m_TRTStrawNeighbourSvc);
    }

    m_timer_purenoise.stop();
    ATH_MSG_DEBUG ( " Number of digits " << m_vDigits.size() << " (" << m_vDigits.size()-numberOfDigitsBeforeNoise << " of those are pure noise)" );
    //Temporary resorting since code below assumes sorted digits. (fixme)
    m_timer_stupidsort.start();
    //Alas, we need them sorted...
    m_pNoise->sortDigits(m_vDigits);
    m_timer_stupidsort.stop();
    //End of temporary fix
  } else {
    ATH_MSG_DEBUG ( " Number of digits " << m_vDigits.size() );
  };

  // All digits are ready.
  // We just need to convert to relevant identifiers and output to storegate.

  if (createAndStoreRDOs().isFailure()) {
    ATH_MSG_FATAL ( "createAndStoreRDOs() failed!" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( "createAndStoreRDOs() succeeded" );
  }
  m_timer_execute.stop();

  return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
StatusCode TRTDigitizationTool::createAndStoreRDOs()
{
  std::vector<TRTDigit>::const_iterator TRTDigitIter(m_vDigits.begin());
  std::vector<TRTDigit>::const_iterator endOfTRTDigits(m_vDigits.end());

  // for testing
  IdentifierHash IdHash; // default value is 0xFFFFFFFF
  IdentifierHash IdHashOld; // default value is 0xFFFFFFFF
  TRT_RDO_Collection *RDOColl(NULL);

  Identifier idStraw;

  while (TRTDigitIter != endOfTRTDigits) {
    ATH_MSG_DEBUG ( "Digit ID " << TRTDigitIter->GetStrawID() << " Digit " << TRTDigitIter->GetDigit() );

    Identifier layer_id;
    bool identifierOK(false);
    idStraw = getIdentifier(TRTDigitIter->GetStrawID(), IdHash, layer_id, identifierOK);
    if (!identifierOK) {
      ATH_MSG_ERROR ( "Ignoring simhits with suspicious identifier (2)" );
      ++TRTDigitIter;
      continue;
    };

    // Create new TRT RDO Collection
    ATH_MSG_DEBUG ( " RDO ID " << m_trt_id->print_to_string(idStraw) );

    //TK: wauv - we are really betting the farm on the fact that the
    //ordering of digits will result in a similar ordering of the
    //idhash'es here... (this is not immediately the case when noise
    //hits are appended afterwards).

    if (IdHash != IdHashOld) {
      RDOColl = new TRT_RDO_Collection(IdHash);

      ATH_MSG_DEBUG ( "New TRT RDO Collection created with IdHash " << static_cast<int>(IdHash) );

      IdHashOld = IdHash;
      RDOColl->setIdentifier(layer_id);

      // Add to the container
      if (m_container->addCollection(RDOColl, RDOColl->identifyHash()).isFailure()) {
	ATH_MSG_FATAL ( "Container " << m_outputRDOCollName << " could not be registered in StoreGate !" );
	return StatusCode::FAILURE;
      } else {
	ATH_MSG_DEBUG ( "Container " << m_outputRDOCollName << " registered in StoreGate" );
      }
    }

    // Put RDO into Collection
    TRT_LoLumRawData *p_rdo(new TRT_LoLumRawData(idStraw, TRTDigitIter->GetDigit()));
    if (RDOColl!=NULL) { RDOColl->push_back(p_rdo); }
    else {
      ATH_MSG_FATAL ( "Failed to create the TRT_RDO_Collection before trying to add an RDO to it! IdHash = " << static_cast<int>(IdHash) );
      return StatusCode::FAILURE;
    }
    ++TRTDigitIter;
  }

  m_vDigits.clear();
  return StatusCode::SUCCESS;
}


//_____________________________________________________________________________
Identifier TRTDigitizationTool::getIdentifier ( int hitID,
						IdentifierHash& hashId,
						Identifier& IdLayer,
						bool & statusok ) const
{
  statusok = true;

  Identifier IdStraw;

  const int mask(0x0000001F);
  const int word_shift(5);
  int trtID, ringID, moduleID, layerID, strawID;
  int wheelID, planeID, sectorID;

  const InDetDD::TRT_BarrelElement *barrelElement;
  const InDetDD::TRT_EndcapElement *endcapElement;

  if ( !(hitID & 0x00200000) ) {      // barrel
    strawID   = hitID & mask;
    hitID   >>= word_shift;
    layerID   = hitID & mask;
    hitID   >>= word_shift;
    moduleID  = hitID & mask;
    hitID   >>= word_shift;
    ringID    = hitID & mask;
    trtID     = hitID >> word_shift;

    barrelElement = m_manager->getBarrelElement(trtID, ringID, moduleID, layerID);
    if ( barrelElement ) {
      hashId  = barrelElement->identifyHash();
      IdLayer = barrelElement->identify();
      IdStraw = m_trt_id->straw_id(IdLayer, strawID);
    } else {
      ATH_MSG_ERROR ( "Could not find detector element for barrel identifier with "
		      << "(ipos,iring,imod,ilayer,istraw) = ("
		      << trtID << ", " << ringID << ", " << moduleID << ", "
		      << layerID << ", " << strawID << ")" );
      statusok = false;
    }
  } else {                           // endcap
    strawID   = hitID & mask;
    hitID   >>= word_shift;
    planeID   = hitID & mask;
    hitID   >>= word_shift;
    sectorID  = hitID & mask;
    hitID   >>= word_shift;
    wheelID   = hitID & mask;
    trtID     = hitID >> word_shift;

    // change trtID (which is 2/3 for endcaps) to use 0/1 in getEndcapElement
    if (trtID == 3) { trtID = 0; }
    else            { trtID = 1; }

    endcapElement = m_manager->getEndcapElement(trtID, wheelID, planeID, sectorID);

    if ( endcapElement ) {
      hashId  = endcapElement->identifyHash();
      IdLayer = endcapElement->identify();
      IdStraw = m_trt_id->straw_id(IdLayer, strawID);
    } else {
      ATH_MSG_ERROR ( "Could not find detector element for endcap identifier with "
		      << "(ipos,iwheel,isector,iplane,istraw) = ("
		      << trtID << ", " << wheelID << ", " << sectorID << ", "
		      << planeID << ", " << strawID << ")" );
      ATH_MSG_ERROR ( "If this happens very rarely, don't be alarmed (it is a Geant4 'feature')" );
      ATH_MSG_ERROR ( "If it happens a lot, you probably have misconfigured geometry in the sim. job." );
      statusok = false;
    }

  }

  return IdStraw;
}

//_____________________________________________________________________________
StatusCode TRTDigitizationTool::finalize() {

  if (m_printUsedDigSettings) {
    m_settings->print("TRTDigSettings Settings : ");
  }

  std::cout << "TRTDigitizationTool: Total time spent in execute() per event : " << m_timer_execute.check() / m_timer_eventcount << " seconds" << std::endl;

  ATH_MSG_INFO ( "TRTDigitizationTool::finalize()" );
  if (msgLvl(MSG::DEBUG)) {
    msg(MSG::DEBUG) << "Total time spent in execute() per event : " << m_timer_execute.check() / m_timer_eventcount << " seconds" << endreq;
    msg(MSG::DEBUG) << "Total time spent processing simhits per event : " << m_timer_simhits.check() / m_timer_eventcount<< " seconds" << endreq;
    msg(MSG::DEBUG) << "Total time spent adding noise in unhit straws per event : " << m_timer_purenoise.check() / m_timer_eventcount << " seconds" << endreq;
    msg(MSG::DEBUG) << "Total time spent doing a temporary sort per event (to be fixed): " << m_timer_stupidsort.check() / m_timer_eventcount << " seconds" << endreq;
  }
  delete m_pElectronicsProcessing;
  delete m_pProcessingOfStraw;
  delete m_pDigConditions;
  delete m_pNoise;
  delete m_settings;

  return StatusCode::SUCCESS;
}

/* ----------------------------------------------------------------------------------- */
// Callback function to update constants from database:
/* ----------------------------------------------------------------------------------- */

StatusCode TRTDigitizationTool::update( IOVSVC_CALLBACK_ARGS_P(I,keys) ) {

  ATH_MSG_INFO ("Updating condition settings TRT_Digitization! ");

  const AthenaAttributeList* atrlist(NULL);

  if (StatusCode::SUCCESS == detStore()->retrieve(atrlist, m_digverscontainerkey ) && atrlist != 0) {
    std::list<std::string>::const_iterator itr;
    if (msgLvl(MSG::INFO)) {
      for( itr=keys.begin(); itr !=keys.end(); ++itr) {
	msg(MSG::INFO)<< "IOVCALLBACK for key "<< *itr << " number " << I << endreq;
      }
    }
    m_dig_vers_from_condDB =(*atrlist)["TRT_Dig_Vers"].data<int>();
  } else {
    ATH_MSG_ERROR ( "Problem reading condDB object." );
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
bool TRTDigitizationTool::IsArgonStraw(Identifier& TRT_Identifier) const {
  bool isArgonStraw = false;
  if(m_UseArgonStraws){
    if (m_useConditionsHTStatus) {
      if (m_sumSvc->getStatusHT(TRT_Identifier) != TRTCond::StrawStatus::Good) {
        isArgonStraw = true;
      }
    } else {
      isArgonStraw = true;
    }
  }
  return isArgonStraw;
}

//_____________________________________________________________________________

StatusCode TRTDigitizationTool::ConditionsDependingInitialization() {

  if (m_dig_vers_from_condDB!=0) {

    if (StatusCode::SUCCESS == m_settings->DigSettingsFromCondDB(m_dig_vers_from_condDB)) {
      ATH_MSG_INFO ( "Retrieved TRT_Settings from CondDB with TRT digitization version: digversion = " <<
                      m_dig_vers_from_condDB );
    } else {
      ATH_MSG_WARNING ( "Unknown TRT digitization version: digversion = " << m_dig_vers_from_condDB <<
                        " read from CondDB. Overriding to use default from Det Desc tag: " <<
                        m_settings->digVers() );
    }

  } else {
    ATH_MSG_WARNING ( "TRT digitization version: digversion = " << m_dig_vers_from_condDB <<
                      " read from CondDB. Overriding to use default from Det Desc tag: " <<
                      m_settings->digVers() );
  }

  return StatusCode::SUCCESS;

}

//_____________________________________________________________________________
//The meaning of these 16 bits is given in TRTProcessingOfStraw::particleFlagSetBit()
bool TRTDigitizationTool::particleFlagQueryBit(int bitposition, unsigned short particleFlag) const {
  if ( bitposition >= 0 && bitposition < 16) {
    return (1 << bitposition) & particleFlag;
  } else {
    ATH_MSG_WARNING ( "Trying to read bit position " << bitposition << " in unsigned short m_particleFlag !");
    return 0; // returning a false neagtive is preferred over a false positive.
  }
}

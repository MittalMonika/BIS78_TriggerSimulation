/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "TransportTool.h"

//package includes
#include "ISF_Geant4Tools/G4AtlasRunManager.h"

// ISF classes
#include "ISF_Event/ISFParticle.h"
#include "ISF_Event/ISFParticleVector.h"

// Athena classes
#include "GeneratorObjects/McEventCollection.h"

#include "MCTruth/PrimaryParticleInformation.h"
#include "MCTruth/EventInformation.h"

// HepMC classes
#include "HepMC/GenParticle.h"

// Geant4 classes
#include "G4LorentzVector.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4Trajectory.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"
#include "G4ParticleTable.hh"
#include "G4StateManager.hh"
#include "G4TransportationManager.hh"
#include "G4UImanager.hh"
#include "G4ScoringManager.hh"

// call_once mutexes
#include <mutex>
static std::once_flag initializeOnceFlag;
static std::once_flag finalizeOnceFlag;

//________________________________________________________________________
iGeant4::G4TransportTool::G4TransportTool(const std::string& t,
                                          const std::string& n,
                                          const IInterface*  p )
  : base_class(t,n,p)
{
  declareProperty("Dll",                   m_libList);
  declareProperty("Physics",               m_physList);
  declareProperty("FieldMap",              m_fieldMap);
  declareProperty("RandomGenerator",       m_rndmGen);
  declareProperty("ReleaseGeoModel",       m_releaseGeoModel);
  declareProperty("RecordFlux",            m_recordFlux);
  declareProperty("McEventCollection",     m_mcEventCollectionName);
  declareProperty("G4Commands",            m_g4commands, "Commands to send to the G4UI");
  declareProperty("MultiThreading",        m_useMT, "Multi-threading specific settings");
  //declareProperty("KillAllNeutrinos",      m_KillAllNeutrinos=true);
  //declareProperty("KillLowEPhotons",       m_KillLowEPhotons=-1.);

}

//________________________________________________________________________
iGeant4::G4TransportTool::~G4TransportTool()
{}

//________________________________________________________________________
StatusCode iGeant4::G4TransportTool::initialize()
{
  ATH_MSG_VERBOSE("initialize");

  ATH_CHECK(m_inputConverter.retrieve());

 // One-time initialization
  try {
    std::call_once(initializeOnceFlag, &iGeant4::G4TransportTool::initializeOnce, this);
  }
  catch(const std::exception& e) {
    ATH_MSG_ERROR("Failure in iGeant4::G4TransportTool::initializeOnce: " << e.what());
    return StatusCode::FAILURE;
  }
  ATH_CHECK( m_userActionSvc.retrieve() );

  ATH_CHECK(m_g4atlasSvc.retrieve());

  if (m_recordFlux) G4ScoringManager::GetScoringManager();

  return StatusCode::SUCCESS;
}

//________________________________________________________________________
void iGeant4::G4TransportTool::initializeOnce()
{
  // get G4AtlasRunManager
  ATH_MSG_DEBUG("initialize G4AtlasRunManager");

  if (m_g4RunManagerHelper.retrieve().isFailure()) {
    throw std::runtime_error("Could not initialize G4RunManagerHelper!");
  }
  ATH_MSG_DEBUG("retrieved "<<m_g4RunManagerHelper);
  m_pRunMgr = m_g4RunManagerHelper ? m_g4RunManagerHelper->g4RunManager() : nullptr;
  if (!m_pRunMgr) {
    throw std::runtime_error("G4RunManagerHelper::g4RunManager() returned nullptr.");
  }

  if(m_physListTool.retrieve().isFailure()) {
    throw std::runtime_error("Could not initialize ATLAS PhysicsListTool!");
  }
  m_physListTool->SetPhysicsList();

  m_pRunMgr->SetRecordFlux( m_recordFlux );
  m_pRunMgr->SetLogLevel( int(msg().level()) ); // Synch log levels
  m_pRunMgr->SetUserActionSvc( m_userActionSvc.typeAndName() );
  m_pRunMgr->SetDetGeoSvc( m_detGeoSvc.typeAndName() );
  m_pRunMgr->SetSDMasterTool(m_senDetTool.typeAndName() );
  m_pRunMgr->SetFastSimMasterTool(m_fastSimTool.typeAndName() );
  m_pRunMgr->SetPhysListTool(m_physListTool.typeAndName() );

  G4UImanager *ui = G4UImanager::GetUIpointer();

  if (!m_libList.empty()) {
    ATH_MSG_INFO("G4AtlasAlg specific libraries requested ") ;
    std::string temp="/load "+m_libList;
    ui->ApplyCommand(temp);
  }

  if (!m_physList.empty()) {
    ATH_MSG_INFO("requesting a specific physics list "<< m_physList) ;
    std::string temp="/Physics/GetPhysicsList "+m_physList;
    ui->ApplyCommand(temp);
  }

  if (!m_fieldMap.empty()) {
    ATH_MSG_INFO("requesting a specific field map "<< m_fieldMap) ;
    ATH_MSG_INFO("the field is initialized straight away") ;
    std::string temp="/MagneticField/Select "+m_fieldMap;
    ui->ApplyCommand(temp);
    ui->ApplyCommand("/MagneticField/Initialize");
  }

  if (m_rndmGen=="athena" || m_rndmGen=="ranecu")     {
    // Set the random number generator to AtRndmGen
    if (m_rndmGenSvc.retrieve().isFailure()) {
      throw std::runtime_error("Could not initialize ATLAS Random Generator Service");
    }
    CLHEP::HepRandomEngine* engine = m_rndmGenSvc->GetEngine("AtlasG4");
    CLHEP::HepRandom::setTheEngine(engine);
    ATH_MSG_DEBUG("Random nr. generator is set to Athena");
  }
  else if (m_rndmGen=="geant4" || m_rndmGen.empty()) {
    ATH_MSG_INFO("Random nr. generator is set to Geant4");
  }

  // Send UI commands
  ATH_MSG_DEBUG("G4 Command: Trying at the end of initializeOnce()");
  for (auto g4command : m_g4commands) {
    int returnCode = ui->ApplyCommand( g4command );
    commandLog(returnCode, g4command);
  }

  return;
}

//________________________________________________________________________
StatusCode iGeant4::G4TransportTool::finalize()
{
  ATH_MSG_VERBOSE("++++++++++++  ISF G4 G4TransportTool finalized  ++++++++++++");

  // One time finalization
  try {
    std::call_once(finalizeOnceFlag, &iGeant4::G4TransportTool::finalizeOnce, this);
  }
  catch(const std::exception& e) {
    ATH_MSG_ERROR("Failure in iGeant4::G4TransportTool::finalizeOnce: " << e.what());
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

//________________________________________________________________________
void iGeant4::G4TransportTool::finalizeOnce()
{
  ATH_MSG_DEBUG("\t terminating the current G4 run");

  m_pRunMgr->RunTermination();

  return;
}

//________________________________________________________________________
StatusCode iGeant4::G4TransportTool::process(const ISF::ISFParticle& isp)
{
  ATH_MSG_VERBOSE("process(...)");

  // wrap the given ISFParticle into a STL vector of ISFParticles with length 1
  // (minimizing code duplication)
  const ISF::ConstISFParticleVector ispVector(1, &isp);
  return this->processVector(ispVector);
}

//________________________________________________________________________
StatusCode iGeant4::G4TransportTool::processVector(const ISF::ConstISFParticleVector& ispVector)
{
  ATH_MSG_VERBOSE("processVector(...)");
  ATH_MSG_DEBUG("processing vector of "<<ispVector.size()<<" particles");

  G4Event* inputEvent = m_inputConverter->ISF_to_G4Event(ispVector, genEvent());
  if (!inputEvent) {
    ATH_MSG_ERROR("ISF Event conversion failed ");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Calling ISF_Geant4 ProcessEvent");
  bool abort = m_pRunMgr->ProcessEvent(inputEvent);

  if (abort) {
    ATH_MSG_WARNING("Event was aborted !! ");
    //ATH_MSG_WARNING("Simulation will now go on to the next event ");
    //ATH_MSG_WARNING("setFilterPassed is now False");
    //setFilterPassed(false);
    return StatusCode::FAILURE;
  }


  // const DataHandle <TrackRecordCollection> tracks;

  // StatusCode sc = evtStore()->retrieve(tracks,m_trackCollName);

  // if (sc.isFailure()) {
  //   ATH_MSG_WARNING(" Cannot retrieve TrackRecordCollection " << m_trackCollName);
  // }

  // not implemented yet... need to get particle stack from Geant4 and convert to ISFParticle
  return StatusCode::SUCCESS;
}

//________________________________________________________________________
HepMC::GenEvent* iGeant4::G4TransportTool::genEvent() const
{

  // collect last GenEvent from McEventCollection

  McEventCollection* mcEventCollection(nullptr);
  // retrieve McEventCollection from storegate
  if (evtStore()->contains<McEventCollection>(m_mcEventCollectionName)) {
    if (evtStore()->retrieve( mcEventCollection, m_mcEventCollectionName).isFailure()) {
      ATH_MSG_WARNING( "Unable to retrieve McEventCollection with name=" << m_mcEventCollectionName
                       << ".");
    }
    else {
      ATH_MSG_DEBUG( "Sucessfully retrieved McEventCollection with name=" << m_mcEventCollectionName);

      return mcEventCollection->back();
    }
  }
  return nullptr;

}

//________________________________________________________________________
void iGeant4::G4TransportTool::commandLog(int returnCode, const std::string& commandString) const
{
  switch(returnCode) {
  case 0: { ATH_MSG_DEBUG("G4 Command: " << commandString << " - Command Succeeded"); } break;
  case 100: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Command Not Found!"); } break;
  case 200: {
    auto* stateManager = G4StateManager::GetStateManager();
    ATH_MSG_DEBUG("G4 Command: " << commandString << " - Illegal Application State (" <<
                    stateManager->GetStateString(stateManager->GetCurrentState()) << ")!");
  } break;
  case 300: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Parameter Out of Range!"); } break;
  case 400: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Parameter Unreadable!"); } break;
  case 500: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Parameter Out of Candidates!"); } break;
  case 600: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Alias Not Found!"); } break;
  default: { ATH_MSG_ERROR("G4 Command: " << commandString << " - Unknown Status!"); } break;
  }

}


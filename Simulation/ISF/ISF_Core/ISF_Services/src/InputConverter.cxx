/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// InputConverter.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// class header include
#include "InputConverter.h"

// framework
#include "GaudiKernel/IPartPropSvc.h"
#include "GaudiKernel/PhysicalConstants.h"

// ISF_HepMC include
#include "ISF_Event/TruthBinding.h"
#include "ISF_HepMC_Interfaces/IGenParticleFilter.h"
// barcode event and interfaces for bc creation
#include "BarcodeEvent/Barcode.h"
#include "BarcodeInterfaces/IBarcodeSvc.h"
// ISF_Event include
#include "ISF_Event/ISFParticle.h"
#include "ISF_Event/ISFParticleContainer.h"
#include "ISF_Event/ISFParticleVector.h"
#include "ISF_Event/ParticleUserInformation.h"
// MCTruth includes
#include "MCTruth/PrimaryParticleInformation.h"
#include "MCTruth/EventInformation.h"
// McEventCollection
#include "GeneratorObjects/McEventCollection.h"
// Geant4 includes
#include "G4PrimaryParticle.hh"
#include "G4Event.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"
#include "G4ParticleTable.hh"
#include "G4LorentzVector.hh"
#include "G4TransportationManager.hh"
// HepMC includes
#include "HepMC/GenParticle.h"
#include "HepMC/GenEvent.h"
// CLHEP includes
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"
// HepPDT
#include "HepPDT/ParticleID.hh"
#include "HepPDT/DecayData.hh"
#include "HepPDT/ParticleDataTable.hh"


/** Constructor **/
ISF::InputConverter::InputConverter(const std::string& name, ISvcLocator* svc)
    : AthService(name,svc)
    , m_particlePropSvc("PartPropSvc",name)
    , m_particleDataTable(nullptr)
    , m_useGeneratedParticleMass(false)
    , m_genParticleFilters()
    , m_worldSolid(nullptr)
    , m_quasiStableParticlesIncluded(false)
    , m_barcodeSvc("",name)
    , m_barcodeGenerationIncrement(Barcode::fUndefinedBarcode)
{
  // particle mass from particle data table?
  declareProperty("UseGeneratedParticleMass",
                  m_useGeneratedParticleMass,
                  "Use particle mass assigned to GenParticle.");
  // particle filters
  declareProperty("GenParticleFilters",
                  m_genParticleFilters,
                  "Tools for filtering out GenParticles.");
  // the particle property service
  declareProperty("ParticlePropertyService",
                  m_particlePropSvc,
                  "ParticlePropertyService to retrieve the PDT.");
  // the barcode service (used to compute Vertex Barcodes)
  declareProperty("BarcodeSvc",                 m_barcodeSvc           );
  declareProperty("QuasiStableParticlesIncluded", m_quasiStableParticlesIncluded);
}


/** Destructor **/
ISF::InputConverter::~InputConverter()
{
}


// Athena algtool's Hooks
StatusCode
ISF::InputConverter::initialize()
{
  ATH_MSG_VERBOSE("initialize() begin");

  // retrieve BarcodeSvc
  ATH_CHECK(m_barcodeSvc.retrieve());
  m_barcodeGenerationIncrement = m_barcodeSvc->particleGenerationIncrement();
  if (m_barcodeGenerationIncrement == Barcode::fUndefinedBarcode) {
    ATH_MSG_FATAL( "'Barcode::fUndefinedBarcode' returned as 'BarcodeGenerationIncrement' by BarcodeService. Abort." );
    return StatusCode::FAILURE;
  }

  // setup PDT if requested (to get particle masses later on)
  if (!m_useGeneratedParticleMass) {
    ATH_CHECK( m_particlePropSvc.retrieve() );
    m_particleDataTable = m_particlePropSvc->PDT();
    if (!m_particleDataTable) {
      ATH_MSG_FATAL( "Could not get ParticleDataTable from " << m_particlePropSvc << ". Abort" );
      return StatusCode::FAILURE;
    }
  }

  if (!m_genParticleFilters.empty()){
    ATH_CHECK(m_genParticleFilters.retrieve());
  }

  ATH_MSG_VERBOSE("initialize() successful");
  return StatusCode::SUCCESS;
}


/** Athena algtool Hook */
StatusCode
ISF::InputConverter::finalize()
{
  ATH_MSG_DEBUG("Finalizing ...");
  return StatusCode::SUCCESS;
}


/** Convert selected particles from the given McEventCollection into ISFParticles
    and push them into the given ISFParticleContainer */
StatusCode
ISF::InputConverter::convert(const McEventCollection& inputGenEvents,
                             ISF::ISFParticleContainer& simParticles,
                             bool isPileup) const
{
  for ( const auto& eventPtr : inputGenEvents ) {
    // skip empty events
    if (eventPtr == nullptr) continue;

    // @FIXME: set the bunch-crossing identifier for pileup dynamically
    // rather than a constant '1' (e.g. could use GenEvent index for that?)
    int bcid = isPileup ? 1 : 0;

    ATH_MSG_DEBUG("Starting conversion of GenEvent with"
                  " signal_process_id=" << eventPtr->signal_process_id() <<
                  " and event_number=" << eventPtr->event_number() );

    // new collection containing all gen particles that passed filters
    bool legacyOrdering = true; // FIXME: this is only to keep the same order of particles
                                //        as the prior 'StackFiller' implementation
                                // TODO:  remove this functionality from the
                                //        getSelectedParticles function once
                                //        happy with the results
    const auto passedGenParticles = getSelectedParticles(*eventPtr, legacyOrdering);

    for ( auto& genPartPtr : passedGenParticles ) {
      ATH_MSG_VERBOSE("Picking up following GenParticle for conversion to ISFParticle: " <<  *genPartPtr);
      auto simParticlePtr = convertParticle(genPartPtr, bcid);
      if (!simParticlePtr) {
        ATH_MSG_ERROR("Error while trying to convert input generator particles. Aborting.");
        return StatusCode::FAILURE;
      }
      // add to collection that will be returned
      simParticles.push_back(simParticlePtr);

    } // loop over passed gen particles

  } // loop over gen events

  ATH_MSG_DEBUG( "Created initial simulation particle collection with size " << simParticles.size() );

  return StatusCode::SUCCESS;
}

StatusCode ISF::InputConverter::convertHepMCToG4Event(McEventCollection& inputGenEvents,
                                                      G4Event*& outputG4Event,
                                                      bool isPileup) const
{
  ISF::ISFParticleContainer simParticleList{}; // particles for ISF simulation
  ATH_CHECK(this->convert(inputGenEvents, simParticleList, isPileup));
  //Convert from ISFParticleContainer to ConstISFParticleVector
  ISF::ConstISFParticleVector simParticleVector{std::make_move_iterator(std::begin(simParticleList)),
                                                std::make_move_iterator(std::end(simParticleList))};
  outputG4Event = this->ISF_to_G4Event(simParticleVector, inputGenEvents.back());
  return StatusCode::SUCCESS;
}


/** get all generator particles which pass filters */
std::vector<HepMC::GenParticle*>
ISF::InputConverter::getSelectedParticles(const HepMC::GenEvent& evnt, bool legacyOrdering) const {
  auto allGenPartBegin = evnt.particles_begin();
  auto allGenPartEnd = evnt.particles_end();

  // reserve destination container with maximum size, i.e. number of particles in input event
  std::vector<HepMC::GenParticle*> passedGenParticles{};
  size_t maxParticles = std::distance(allGenPartBegin, allGenPartEnd);
  passedGenParticles.reserve(maxParticles);

  if (legacyOrdering) {
    // FIXME: remove this block and the 'legacyOrdering' flag
    //        once we don't need the legacy order any longer
    auto vtxIt = evnt.vertices_begin();
    auto vtxItEnd = evnt.vertices_end();
    for ( ; vtxIt != vtxItEnd; ++vtxIt ) {
      const auto vtxPtr = *vtxIt;
      std::copy_if(vtxPtr->particles_begin(HepMC::children),
                   vtxPtr->particles_end(HepMC::children),
                   std::back_inserter(passedGenParticles),
                   [this](HepMC::GenParticle* p){return this->passesFilters(*p);});
    }

  } else {
    std::copy_if(allGenPartBegin,
                 allGenPartEnd,
                 std::back_inserter(passedGenParticles),
                 [this](HepMC::GenParticle* p){return this->passesFilters(*p);});
  }

  passedGenParticles.shrink_to_fit();

  return passedGenParticles;
}


/** get all generator particles which pass filters */
ISF::ISFParticle*
ISF::InputConverter::convertParticle(HepMC::GenParticle* genPartPtr, int bcid) const {

  if (!genPartPtr)
      return nullptr;

  auto& genPart = *genPartPtr;

  // -> particle origin (TODO: add proper GeoID, collision/cosmics)
  DetRegionSvcIDPair origin(AtlasDetDescr::fUndefinedAtlasRegion, ISF::fEventGeneratorSimID);

  // -> truth binding
  ISF::TruthBinding* tBinding = new ISF::TruthBinding(genPartPtr);

  // 4-vectors: position, momentum
  HepMC::GenVertex* pVertex = genPart.production_vertex();
  if (!pVertex) {
    ATH_MSG_ERROR("Unable to convert following generator particle due to missing "
                  << "production vertex: " << genPart);
    return nullptr;
  }
  const auto& pVertexPos(pVertex->point3d());
  const Amg::Vector3D pos(pVertexPos.x(), pVertexPos.y(), pVertexPos.z());
  const auto& pMomentum(genPart.momentum());
  const Amg::Vector3D mom(pMomentum.px(), pMomentum.py(), pMomentum.pz());

  // get the pdg_id, mass & time, barcode
  int pPdgId = genPart.pdg_id();
  double pMass = getParticleMass(genPart);
  double pTime = pVertex->position().t() / Gaudi::Units::c_light;
  double charge = HepPDT::ParticleID(pPdgId).charge();
  auto pBarcode = genPart.barcode();

  auto sParticle = new ISF::ISFParticle( std::move(pos),
                                         std::move(mom),
                                         pMass,
                                         charge,
                                         pPdgId,
                                         pTime,
                                         origin,
                                         bcid,
                                         pBarcode,
                                         tBinding );
  return sParticle;
}


/** get right GenParticle mass */
double
ISF::InputConverter::getParticleMass(const HepMC::GenParticle &part) const {
  // default value: generated particle mass
  double mass = part.generated_mass();
  ATH_MSG_VERBOSE("part.generated_mass, mass="<<mass);

  // 1. use PDT mass?
  if ( !m_useGeneratedParticleMass) {
    int absPDG = abs(part.pdg_id());
    HepPDT::ParticleData const *pData = (m_particleDataTable)
      ? m_particleDataTable->particle(absPDG)
      : nullptr ;
    if (pData) {
      mass = pData->mass();
      ATH_MSG_VERBOSE("using pData mass, mass="<<mass);
    }
    else
      ATH_MSG_WARNING( "Unable to find mass of particle with PDG ID '" << absPDG << "' in ParticleDataTable. Will set mass to generated_mass: " << mass);
  }
  return mass;
}


/** check if the given particle passes all filters */
bool
ISF::InputConverter::passesFilters(const HepMC::GenParticle& part) const {
  // TODO: implement this as a std::find_if with a lambda function
  for ( const auto& filter : m_genParticleFilters ) {
    // determine if the particle passes current filter
    bool passFilter = filter->pass(part);
    ATH_MSG_VERBOSE("GenParticleFilter '" << filter.typeAndName() << "' returned: "
                    << (passFilter ? "true, will keep particle."
                        : "false, will remove particle."));
    const auto& momentum = part.momentum();
    ATH_MSG_VERBOSE("Particle: ("
                    <<momentum.px()<<", "
                    <<momentum.py()<<", "
                    <<momentum.pz()<<"), pdgCode: "
                    <<part.pdg_id() );

    if (!passFilter) {
      return false;
    }
  }

  return true;
}


/** Query the interfaces. */
StatusCode ISF::InputConverter::queryInterface(const InterfaceID& riid, void** ppvInterface) {

  if ( IID_IInputConverter == riid )
    *ppvInterface = (IInputConverter*)this;
  else  {
    // Interface is not directly available: try out a base class
    return AthService::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}

//________________________________________________________________________
G4Event* ISF::InputConverter::ISF_to_G4Event(const ISF::ConstISFParticleVector& ispVector, HepMC::GenEvent *genEvent) const
{

  G4Event * g4evt = new G4Event(1);

  int n_pp=0;
  for ( const ISF::ISFParticle *ispPtr: ispVector ) {

    const ISF::ISFParticle &isp = *ispPtr;

    if ( !isInsideG4WorldVolume(isp) ) {
        ATH_MSG_WARNING("Unable to convert ISFParticle to G4PrimaryParticle!");
        ATH_MSG_WARNING(" ISFParticle outside Geant4 world volume: " << isp );
        continue;
    }

    addPrimaryVertex(g4evt,isp);

    n_pp++;
  }

  EventInformation *eventInfo=new EventInformation();
  eventInfo->SetNrOfPrimaryParticles(n_pp);
  eventInfo->SetNrOfPrimaryVertices(n_pp); // special case for ISF batches of particles
  eventInfo->SetHepMCEvent(genEvent);
  g4evt->SetUserInformation(eventInfo);

  return g4evt;
}

//________________________________________________________________________
G4PrimaryParticle* ISF::InputConverter::getPrimaryParticle(const HepMC::GenParticle& genpart) const
{
  ATH_MSG_VERBOSE("Creating G4PrimaryParticle from GenParticle.");
  G4ParticleTable * ptable = G4ParticleTable::GetParticleTable();
  const G4ParticleDefinition* particle_definition = 0;
  if (genpart.pdg_id()==998){
    particle_definition = G4ChargedGeantino::Definition();
  }
  else if (genpart.pdg_id()==999){
    particle_definition = G4Geantino::GeantinoDefinition();
  }
  else {
    particle_definition = ptable->FindParticle(genpart.pdg_id());
  }

  if(particle_definition==0) {
    ATH_MSG_ERROR("ISF_to_G4Event particle conversion failed. ISF_Particle PDG code = " << genpart.pdg_id() <<
                  "\n This usually indicates a problem with the evgen step.\n" <<
                  "Please report this to the Generators group, mentioning the release and generator used for evgen and the PDG code above." );
    return 0;
  }

  // create new primaries and set them to the vertex
  //  G4double mass =  particle_definition->GetPDGMass();
  auto &genpartMomentum = genpart.momentum();
  G4double px = genpartMomentum.x();
  G4double py = genpartMomentum.y();
  G4double pz = genpartMomentum.z();

  G4PrimaryParticle* particle =
    new G4PrimaryParticle(particle_definition,px,py,pz);

  if (genpart.end_vertex()){
    // Add all necessary daughter particles
    for (HepMC::GenVertex::particles_out_const_iterator iter=genpart.end_vertex()->particles_out_const_begin();
         iter!=genpart.end_vertex()->particles_out_const_end(); ++iter){
      if (! (*iter) ) continue;
      G4PrimaryParticle * daught = getPrimaryParticle( **iter );
      particle->SetDaughter( daught );
    }
    // Set the lifetime appropriately - this is slow but rigorous, and we
    //  don't want to end up with something like vertex time that we have
    //  to validate for every generator on earth...
    G4LorentzVector lv0 ( genpart.production_vertex()->position().x(),
                          genpart.production_vertex()->position().y(),
                          genpart.production_vertex()->position().z(),
                          genpart.production_vertex()->position().t() );
    G4LorentzVector lv1 ( genpart.end_vertex()->position().x(),
                          genpart.end_vertex()->position().y(),
                          genpart.end_vertex()->position().z(),
                          genpart.end_vertex()->position().t() );
    particle->SetProperTime( (lv1-lv0).mag()/Gaudi::Units::c_light );
  }

  // Set the user information for this primary to point to the HepMcParticleLink...
  PrimaryParticleInformation* ppi = new PrimaryParticleInformation(&genpart);
  ppi->SetParticle(&genpart);
  ppi->SetRegenerationNr(0);
  particle->SetUserInformation(ppi);
  std::cout << "ZLM making primary down the line with " << ppi->GetParticleBarcode() << std::endl;

  return particle;
}


//________________________________________________________________________
G4PrimaryParticle* ISF::InputConverter::getPrimaryParticle(const ISF::ISFParticle& isp) const
{
  ATH_MSG_VERBOSE("Creating G4PrimaryParticle from ISFParticle.");
  /*
    see conversion from PrimaryParticleInformation to TrackInformation in
    http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Simulation/G4Atlas/G4AtlasAlg/src/AthenaStackingAction.cxx#0044

    need to check with
    http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Simulation/G4Atlas/G4AtlasAlg/src/TruthHepMCEventConverter.cxx#0151

    that we don't miss something
  */

  G4ParticleTable * ptable = G4ParticleTable::GetParticleTable();
  const G4ParticleDefinition* particle_definition = 0;
  if (isp.pdgCode()==998){
    particle_definition = G4ChargedGeantino::Definition();
  }
  else if (isp.pdgCode()==999){
    particle_definition = G4Geantino::GeantinoDefinition();
  }
  else {
    particle_definition = ptable->FindParticle(isp.pdgCode());
  }

  // ptable->DumpTable();

  if(particle_definition==0) {
    ATH_MSG_ERROR("ISF_to_G4Event particle conversion failed. ISF_Particle PDG code = " << isp.pdgCode() <<
                  "\n This usually indicates a problem with the evgen step.\n" <<
                  "Please report this to the Generators group, mentioning the release and generator used for evgen and the PDG code above." );
    return 0;
  }

  // create new primaries and set them to the vertex
  //  G4double mass =  particle_definition->GetPDGMass();
  auto &ispMomentum = isp.momentum();
  G4double px = ispMomentum.x();
  G4double py = ispMomentum.y();
  G4double pz = ispMomentum.z();

  G4PrimaryParticle* particle =
    new G4PrimaryParticle(particle_definition,px,py,pz);

  auto truthBinding = isp.getTruthBinding();
  if (!truthBinding) {
      G4ExceptionDescription description;
      description << G4String("getPrimaryParticle: ") + "No ISF::TruthBinding associated with ISParticle (" << isp <<")";
      G4Exception("iGeant4::TransportTool", "NoISFTruthBinding", FatalException, description);
      return nullptr; //The G4Exception call above should abort the job, but Coverity does not seem to pick this up.
  }
  HepMC::GenParticle*        genpart = truthBinding->getTruthParticle();
  HepMC::GenParticle* primaryGenpart = truthBinding->getPrimaryTruthParticle();

  // UserInformation
  PrimaryParticleInformation* ppi = new PrimaryParticleInformation(primaryGenpart,&isp);

  Barcode::ParticleBarcode barcode = isp.barcode();
  int regenerationNr = (barcode - barcode%m_barcodeGenerationIncrement)/m_barcodeGenerationIncrement;
  ppi->SetRegenerationNr(regenerationNr);

  // if truth binding exists -> try to retrieve the HepMC::GenParticle from it
  if ( genpart ) {
    if (genpart->end_vertex()){
      if(m_quasiStableParticlesIncluded) {
        ATH_MSG_VERBOSE( "Detected primary particle with end vertex." );
        ATH_MSG_VERBOSE( "Will add the primary particle set on." );
        ATH_MSG_VERBOSE( "ISF Particle: " << isp );
        ATH_MSG_VERBOSE( "Primary Particle: " << *genpart );
        ATH_MSG_VERBOSE("Number of daughters of "<<genpart->barcode()<<": " << genpart->end_vertex()->particles_out_size() );
      }
      else {
        ATH_MSG_WARNING( "Detected primary particle with end vertex. This should only be the case if" );
        ATH_MSG_WARNING( "you are running with quasi-stable particle simulation enabled.  This is not" );
        ATH_MSG_WARNING( "yet validated - you'd better know what you're doing.  Will add the primary" );
        ATH_MSG_WARNING( "particle set on." );
        ATH_MSG_WARNING( "ISF Particle: " << isp );
        ATH_MSG_WARNING( "Primary Particle: " << *genpart );
        ATH_MSG_WARNING("Number of daughters of "<<genpart->barcode()<<": " << genpart->end_vertex()->particles_out_size() );
      }
      // Add all necessary daughter particles
      for (HepMC::GenVertex::particles_out_const_iterator iter=genpart->end_vertex()->particles_out_const_begin();
           iter!=genpart->end_vertex()->particles_out_const_end(); ++iter){
        G4PrimaryParticle * daught = getPrimaryParticle( **iter );
        if(m_quasiStableParticlesIncluded) {
          ATH_MSG_VERBOSE ( "Daughter Particle of "<<genpart->barcode()<<": " << **iter );
        }
        else {
          ATH_MSG_WARNING ( "Daughter Particle of "<<genpart->barcode()<<": " << **iter );
        }
        if(nullptr==(*iter)->end_vertex()) {
          if(m_quasiStableParticlesIncluded) {
            ATH_MSG_VERBOSE ( "Number of daughters of "<<(*iter)->barcode()<<": 0 (NULL)." );
          }
          else {
            ATH_MSG_WARNING ( "Number of daughters of "<<(*iter)->barcode()<<": 0 (NULL)." );
          }
        }
        else {
          if(m_quasiStableParticlesIncluded) {
            ATH_MSG_VERBOSE ( "Number of daughters of "<<(*iter)->barcode()<<": " << (*iter)->end_vertex()->particles_out_size() );
          }
          else {
            ATH_MSG_WARNING ( "Number of daughters of "<<(*iter)->barcode()<<": " << (*iter)->end_vertex()->particles_out_size() );
          }
        }
        particle->SetDaughter( daught );
      }
      // Set the lifetime appropriately - this is slow but rigorous, and we
      //  don't want to end up with something like vertex time that we have
      //  to validate for every generator on earth...
      const auto& prodVtx = genpart->production_vertex()->position();
      const auto& endVtx = genpart->end_vertex()->position();
      const G4LorentzVector lv0( prodVtx.x(), prodVtx.y(), prodVtx.z(), prodVtx.t() );
      const G4LorentzVector lv1( endVtx.x(), endVtx.y(), endVtx.z(), endVtx.t() );
      particle->SetProperTime( (lv1-lv0).mag()/Gaudi::Units::c_light );
    } // particle had an end vertex
  } // Truth was detected
  
  ATH_MSG_VERBOSE("PrimaryParticleInformation:");
  ATH_MSG_VERBOSE("     GetParticleBarcode = " << ppi->GetParticleBarcode());
  ATH_MSG_VERBOSE("     GetRegenerationNr = " << ppi->GetRegenerationNr());
  ATH_MSG_VERBOSE("     GetHepMCParticle = " << ppi->GetHepMCParticle());
  ATH_MSG_VERBOSE("     GetISFParticle = " << ppi->GetISFParticle());
  particle->SetUserInformation(ppi);

  return particle;
}

//________________________________________________________________________
void ISF::InputConverter::addPrimaryVertex(G4Event* g4evt, const ISF::ISFParticle& isp) const
{
  /*
    see conversion from PrimaryParticleInformation to TrackInformation in
    http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Simulation/G4Atlas/G4AtlasAlg/src/AthenaStackingAction.cxx#0044

    need to check with
    http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Simulation/G4Atlas/G4AtlasAlg/src/TruthHepMCEventConverter.cxx#0151

    that we don't miss something
  */

  G4PrimaryParticle * particle = getPrimaryParticle( isp );
  if (!particle) return; // Already printed a warning

  // create a new vertex
  G4PrimaryVertex* vertex =
    new G4PrimaryVertex(isp.position().x(),isp.position().y(),isp.position().z(),isp.timeStamp());

  vertex->SetPrimary( particle );

  g4evt->AddPrimaryVertex( vertex );

}

//________________________________________________________________________
bool ISF::InputConverter::isInsideG4WorldVolume(const ISF::ISFParticle& isp) const {

  // retrieve world solid (volume)
  if (!m_worldSolid) {
    // NB: assuming that the pointers are all valid
    // (simulation is really sick otherwise)
    m_worldSolid = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume()->GetLogicalVolume()->GetSolid();
  }

  const Amg::Vector3D &pos = isp.position();
  const G4ThreeVector g4Pos( pos.x(), pos.y(), pos.z() );
  EInside insideStatus     = m_worldSolid->Inside( g4Pos );

  bool insideWorld = insideStatus != kOutside;
  return insideWorld;
}

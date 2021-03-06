/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// FastCaloSimSvcV2.cxx, (c) ATLAS Detector software             //
///////////////////////////////////////////////////////////////////

// class header include
#include "FastCaloSimSvcV2.h"


// FastCaloSim includes
#include "ISF_FastCaloSimEvent/TFCSParametrizationBase.h"
#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"
#include "ISF_FastCaloSimEvent/TFCSTruthState.h"
#include "ISF_FastCaloSimEvent/TFCSExtrapolationState.h"
#include "ISF_FastCaloSimParametrization/CaloGeometryFromCaloDDM.h"

//!
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "IdDictParser/IdDictParser.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/TileID.h"
//!

// StoreGate
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/StoreGate.h"
#include "ISF_Interfaces/IParticleBroker.h"


#include "CaloEvent/CaloCellContainer.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArReadoutGeometry/FCALDetectorManager.h"

#include "PathResolver/PathResolver.h"


#include "TFile.h"
#include <fstream>

using std::abs;
using std::atan2;

/** Constructor **/
ISF::FastCaloSimSvcV2::FastCaloSimSvcV2(const std::string& name, ISvcLocator* svc)
  : BaseSimulationSvc(name, svc)
  , m_paramSvc("ISF_FastCaloSimV2ParamSvc", name)
  , m_rndGenSvc("AtRndmGenSvc", name)
  , m_doPunchThrough(false)
  , m_punchThroughTool("")
  , m_particleBroker ("ISF_ParticleBroker",name)

{
  declareProperty("ParamSvc"                       ,       m_paramSvc);
  declareProperty("CaloCellsOutputName"            ,       m_caloCellsOutputName) ;
  declareProperty("CaloCellMakerTools_setup"       ,       m_caloCellMakerToolsSetup) ;
  declareProperty("CaloCellMakerTools_release"     ,       m_caloCellMakerToolsRelease) ;
  declareProperty("PunchThroughTool"               ,       m_punchThroughTool);
  declareProperty("DoPunchThroughSimulation"       ,       m_doPunchThrough) ;
  declareProperty("RandomSvc"                      ,       m_rndGenSvc                );
  declareProperty("RandomStream"                   ,       m_randomEngineName         );
  declareProperty("FastCaloSimCaloExtrapolation"   ,       m_FastCaloSimCaloExtrapolation );
  declareProperty("ParticleBroker"                 ,       m_particleBroker, "ISF ParticleBroker Svc" );
}

/** framework methods */
StatusCode ISF::FastCaloSimSvcV2::initialize()
{
  ATH_MSG_INFO(m_screenOutputPrefix << "Initializing ...");

  ATH_CHECK(m_rndGenSvc.retrieve());
  m_randomEngine = m_rndGenSvc->GetEngine( m_randomEngineName);

  if (!m_randomEngine) {
    ATH_MSG_ERROR("Could not get random number engine from RandomNumberService. Abort.");
    return StatusCode::FAILURE;
  }

  if (m_doPunchThrough && m_punchThroughTool.retrieve().isFailure() ) 
  {
    ATH_MSG_ERROR (m_punchThroughTool.propertyName() << ": Failed to retrieve tool " << m_punchThroughTool.type());
    return StatusCode::FAILURE;
  } 

  ATH_CHECK(m_paramSvc.retrieve());

  // Get FastCaloSimCaloExtrapolation
  ATH_CHECK(m_FastCaloSimCaloExtrapolation.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode ISF::FastCaloSimSvcV2::setupEvent()
{
  ATH_MSG_VERBOSE(m_screenOutputPrefix << "setupEvent NEW EVENT!");

  m_theContainer = new CaloCellContainer(SG::VIEW_ELEMENTS);

  StatusCode sc = evtStore()->record(m_theContainer, m_caloCellsOutputName);
  if (sc.isFailure())
  {
    ATH_MSG_FATAL( m_screenOutputPrefix << "cannot record CaloCellContainer " << m_caloCellsOutputName );
    return StatusCode::FAILURE;
  }

  CHECK( m_caloCellMakerToolsSetup.retrieve() );
  ATH_MSG_DEBUG( "Successfully retrieve CaloCellMakerTools: " << m_caloCellMakerToolsSetup );
  ToolHandleArray<ICaloCellMakerTool>::iterator itrTool = m_caloCellMakerToolsSetup.begin();
  ToolHandleArray<ICaloCellMakerTool>::iterator endTool = m_caloCellMakerToolsSetup.end();
  for (; itrTool != endTool; ++itrTool)
  {
    std::string chronoName=this->name()+"_"+ itrTool->name();
    if (m_chrono) m_chrono->chronoStart(chronoName);
    StatusCode sc = (*itrTool)->process(m_theContainer);
    if (m_chrono) {
      m_chrono->chronoStop(chronoName);
      ATH_MSG_DEBUG( m_screenOutputPrefix << "Chrono stop : delta " << m_chrono->chronoDelta (chronoName,IChronoStatSvc::USER) * CLHEP::microsecond / CLHEP::second << " second " );
    }

    if (sc.isFailure())
    {
      ATH_MSG_ERROR( m_screenOutputPrefix << "Error executing tool " << itrTool->name() );
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode ISF::FastCaloSimSvcV2::releaseEvent()
{

 ATH_MSG_VERBOSE(m_screenOutputPrefix << "release Event");

 CHECK( m_caloCellMakerToolsRelease.retrieve() );

 //run release tools in a loop
 ToolHandleArray<ICaloCellMakerTool>::iterator itrTool = m_caloCellMakerToolsRelease.begin();
 ToolHandleArray<ICaloCellMakerTool>::iterator endTool = m_caloCellMakerToolsRelease.end();
 for (; itrTool != endTool; ++itrTool)
 {
  ATH_MSG_VERBOSE( m_screenOutputPrefix << "Calling tool " << itrTool->name() );

  StatusCode sc = (*itrTool)->process(m_theContainer);

  if (sc.isFailure())
  {
   ATH_MSG_ERROR( m_screenOutputPrefix << "Error executing tool " << itrTool->name() );
  }
 }

 return StatusCode::SUCCESS;

}

/** Simulation Call */
StatusCode ISF::FastCaloSimSvcV2::simulate(const ISF::ISFParticle& isfp)
{

  ATH_MSG_VERBOSE("NEW PARTICLE! FastCaloSimSvcV2 called with ISFParticle: " << isfp);

  Amg::Vector3D particle_position =  isfp.position();
  Amg::Vector3D particle_direction(isfp.momentum().x(),isfp.momentum().y(),isfp.momentum().z());

  if (m_doPunchThrough) {
     // call punch-through simulation
     const ISF::ISFParticleContainer* isfpVec = m_punchThroughTool->computePunchThroughParticles(isfp);

     // add punch-through particles to the ISF particle broker
     if (isfpVec) {
       for (ISF::ISFParticle *particle : *isfpVec) {
         m_particleBroker->push( particle, &isfp);
       }
     }
   }

   //int barcode=isfp.barcode(); // isfp barcode, eta and phi: in case we need them
  // float eta_isfp = particle_position.eta();
  // float phi_isfp = particle_position.phi();

  //Don't simulate particles with total energy below 10 MeV
  if(isfp.ekin() < 10) {
    ATH_MSG_VERBOSE("Skipping particle with Ekin: " << isfp.ekin() <<" MeV. Below the 10 MeV threshold.");
    return StatusCode::SUCCESS;
  }

  TFCSTruthState truth;
  truth.SetPtEtaPhiM(particle_direction.perp(), particle_direction.eta(), particle_direction.phi(), isfp.mass());
  truth.set_pdgid(isfp.pdgCode());
  truth.set_vertex(particle_position[Amg::x], particle_position[Amg::y], particle_position[Amg::z]);

  /// for anti protons and anti neutrons the kinetic energy should be
  /// calculated as Ekin = E() + M() instead of E() - M()
  /// this is achieved by setting an Ekin offset of 2*M() to the truth state
  if(isfp.pdgCode() == -2212 || isfp.pdgCode() == -2112) {
    truth.set_Ekin_off(2*isfp.mass());
    ATH_MSG_VERBOSE("Found anti-proton/neutron, setting Ekin offset in TFCSTruthState.");
  }

  TFCSExtrapolationState extrapol;
  m_FastCaloSimCaloExtrapolation->extrapolate(extrapol,&truth);
  TFCSSimulationState simulstate(m_randomEngine);

  ATH_MSG_DEBUG(" particle: " << isfp.pdgCode() << " Ekin: " << isfp.ekin() << " position eta: " << particle_position.eta() << " direction eta: " << particle_direction.eta() << " position phi: " << particle_position.phi() << " direction phi: " << particle_direction.phi());

  ATH_CHECK(m_paramSvc->simulate(simulstate, &truth, &extrapol));

  ATH_MSG_DEBUG("Energy returned: " << simulstate.E());
  ATH_MSG_VERBOSE("Energy fraction for layer: ");
  for (int s = 0; s < CaloCell_ID_FCS::MaxSample; s++)
  ATH_MSG_VERBOSE(" Sampling " << s << " energy " << simulstate.E(s));

  //Now deposit all cell energies into the CaloCellContainer
  for(const auto& iter : simulstate.cells()) {
    CaloCell* theCell = (CaloCell*)m_theContainer->findCell(iter.first->calo_hash());
    theCell->addEnergy(iter.second);
  }

  return StatusCode::SUCCESS;
}

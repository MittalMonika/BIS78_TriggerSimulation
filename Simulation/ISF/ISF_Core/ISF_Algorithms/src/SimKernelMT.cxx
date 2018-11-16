/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author Elmar Ritsch <Elmar.Ritsch@cern.ch>
 * @date October 2016
 * @brief Core Athena algorithm for the Integrated Simulation Framework
 */

#include "SimKernelMT.h"

// ISF includes
#include "ISF_Event/ISFParticle.h"


#include "AtlasDetDescr/AtlasRegionHelper.h"

// STL
#include <queue>


ISF::SimKernelMT::SimKernelMT( const std::string& name, ISvcLocator* pSvcLocator ) :
    ::AthAlgorithm( name, pSvcLocator ),
    m_inputEvgenKey(),
    m_outputTruthKey(),
    m_inputConverter("", name),
    m_simSelectors(),
    m_particleKillerSimulationSvc("", name)
{
    declareProperty("InputEvgenCollection",
                    m_inputEvgenKey,
                    "Input EVGEN collection.");
    declareProperty("OutputTruthCollection",
                    m_outputTruthKey,
                    "Output Truth collection.");
    declareProperty("InputConverter",
                    m_inputConverter,
                    "Input McEventCollection->ISFParticleContainer conversion service.");

    // routing tools
    declareProperty("ForwardBeamPipeSimulationSelectors", m_simSelectors[AtlasDetDescr::fAtlasForward] );
    declareProperty("InDetSimulationSelectors", m_simSelectors[AtlasDetDescr::fAtlasID] );
    declareProperty("CaloSimulationSelectors", m_simSelectors[AtlasDetDescr::fAtlasCalo] );
    declareProperty("MSSimulationSelectors", m_simSelectors[AtlasDetDescr::fAtlasMS] );
    declareProperty("CavernSimulationSelectors", m_simSelectors[AtlasDetDescr::fAtlasCavern] );

    declareProperty("ParticleKillerSimulationSvc",
                    m_particleKillerSimulationSvc,
                    "Simulation Service to use when particle is not selected by any SimulationSelector in the routing chain.");
}


ISF::SimKernelMT::~SimKernelMT() {
}


StatusCode ISF::SimKernelMT::initialize() {

  // retrieve simulation selectors (i.e. the "routing chain")
  for ( auto& selectorsToolHandleArray: m_simSelectors ) {
    ATH_CHECK( selectorsToolHandleArray.retrieve() );
  }

  // info screen output
  ATH_MSG_INFO("The following routing chains are defined:");
  for ( short geoID=AtlasDetDescr::fFirstAtlasRegion; geoID<AtlasDetDescr::fNumAtlasRegions ; ++geoID) {
    auto& localSelectors = m_simSelectors[geoID];
    ATH_MSG_INFO( AtlasDetDescr::AtlasRegionHelper::getName(geoID)
                  << " (GeoID=" << geoID << "): \t" << localSelectors);
    for (auto& selector: localSelectors)
      {
        auto flavor = selector->simFlavor();
        auto itr = m_simSvcMap.find(flavor);
        if (itr == m_simSvcMap.end() )
          {
            // New flavour add it to the map.
            m_simSvcMap[flavor] = &**(selector->simulator());
            continue;
          }
        if ( itr->second == &**(selector->simulator()) )
          {
            continue; // OK - multiple selectors can point to the same simulator
          }
        ATH_MSG_FATAL("Two ISimulationSvc instances with the same flavor in this job!\n Check your configuration!");
        return StatusCode::FAILURE;
    }
  }

  ATH_CHECK( m_inputEvgenKey.initialize() );
  ATH_CHECK( m_outputTruthKey.initialize() );

  ATH_CHECK( m_particleKillerSimulationSvc.retrieve() );
  if ( m_simSvcMap.find(ISF::ParticleKiller) == m_simSvcMap.end() )
    {
      m_simSvcMap[ISF::ParticleKiller] = &*m_particleKillerSimulationSvc;
    }
  ATH_CHECK( m_inputConverter.retrieve() );

  return StatusCode::SUCCESS;
}


StatusCode ISF::SimKernelMT::execute() {

  SG::ReadHandle<McEventCollection> inputEvgen(m_inputEvgenKey);
  if (!inputEvgen.isValid()) {
    ATH_MSG_FATAL("Unable to read input GenEvent collection '" << inputEvgen.key() << "'");
    return StatusCode::FAILURE;
  }

  // copy input Evgen collection to output Truth collection
  SG::WriteHandle<McEventCollection> outputTruth(m_outputTruthKey);
  outputTruth = CxxUtils::make_unique<McEventCollection>(*inputEvgen);

  // read and convert input
  ISFParticleContainer simParticles; // particles for ISF simulation
  bool isPileup = false;
  ATH_CHECK( m_inputConverter->convert(*outputTruth, simParticles, isPileup) );
  // loop until there are no more particles to simulate
  while ( simParticles.size() ) {
    ISFParticle* curParticle = simParticles.back();
    simParticles.pop_back();
    auto& simSvc = identifySimulator(*curParticle);

    ATH_CHECK( simSvc.simulate( std::move(*curParticle) ).isSuccess() );
    // TODO: this is work in progress and newSecondaries should actually be
    // returned by the simulate() call above
    ISFParticleContainer newSecondaries;

    simParticles.splice( end(simParticles), std::move(newSecondaries) );
  }
  return StatusCode::SUCCESS;
}


StatusCode ISF::SimKernelMT::finalize() {
  return StatusCode::SUCCESS;
}


/// Returns the simulator to use for the given particle
ISF::ISimulationSvc& ISF::SimKernelMT::identifySimulator(const ISF::ISFParticle& particle) {
  AtlasDetDescr::AtlasRegion geoID = particle.nextGeoID();

  auto& localSelectors = m_simSelectors[geoID];
  for (auto& selector: localSelectors) {
    bool selected = selector->selfSelect(particle);
    if (selected) {
      return *m_simSvcMap.at(selector->simFlavor());
    }
  }

  ATH_MSG_WARNING("No simulator found for particle (" << particle << ")."
      << " Will send it to " << m_particleKillerSimulationSvc);
  return *m_simSvcMap.at(ISF::ParticleKiller);
}

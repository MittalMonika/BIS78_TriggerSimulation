/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// JetTrackParticleThinning.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: James Catmore (James.Catmore@cern.ch)
// This is a trivial example of an implementation of a thinning tool
// which removes all ID tracks which do not pass a user-defined cut

#include "DerivationFrameworkInDet/JetTrackParticleThinning.h"
#include "ExpressionEvaluation/ExpressionParser.h"
#include "ExpressionEvaluation/SGxAODProxyLoader.h"
#include "ExpressionEvaluation/SGNTUPProxyLoader.h"
#include "ExpressionEvaluation/MultipleProxyLoader.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ThinningHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include <vector>
#include <string>

// Constructor
DerivationFramework::JetTrackParticleThinning::JetTrackParticleThinning(const std::string& t,
                                                                        const std::string& n,
                                                                        const IInterface* p ) :
base_class(t,n,p),
m_ntot(0),
m_npass(0),
m_jetSGKey("")
{
    declareProperty("JetKey", m_jetSGKey);
}

// Destructor
DerivationFramework::JetTrackParticleThinning::~JetTrackParticleThinning() {
}

// Athena initialize and finalize
StatusCode DerivationFramework::JetTrackParticleThinning::initialize()
{
    // Decide which collections need to be checked for ID TrackParticles
    ATH_MSG_VERBOSE("initialize() ...");
    ATH_CHECK( m_inDetSGKey.initialize (m_streamName) );
    ATH_MSG_INFO("Using " << m_inDetSGKey << "as the source collection for inner detector track particles");
    if (m_jetSGKey=="") {
        ATH_MSG_FATAL("No jet collection provided for thinning.");
        return StatusCode::FAILURE;
    } else { ATH_MSG_INFO("Inner detector track particles associated with objects in " << m_jetSGKey << " will be retained in this format with the rest being thinned away");}
    
    // Set up the text-parsing machinery for selectiong the jet directly according to user cuts
    if (m_selectionString!="") {
	    ExpressionParsing::MultipleProxyLoader *proxyLoaders = new ExpressionParsing::MultipleProxyLoader();
	    proxyLoaders->push_back(new ExpressionParsing::SGxAODProxyLoader(evtStore()));
	    proxyLoaders->push_back(new ExpressionParsing::SGNTUPProxyLoader(evtStore()));
	    m_parser = std::make_unique<ExpressionParsing::ExpressionParser>(proxyLoaders);
	    m_parser->loadExpression(m_selectionString);
    }

    if (m_trackSelectionString!="") {
	    ExpressionParsing::MultipleProxyLoader *proxyLoaders = new ExpressionParsing::MultipleProxyLoader();
	    proxyLoaders->push_back(new ExpressionParsing::SGxAODProxyLoader(evtStore()));
	    proxyLoaders->push_back(new ExpressionParsing::SGNTUPProxyLoader(evtStore()));
	    m_trackParser = std::make_unique<ExpressionParsing::ExpressionParser>(proxyLoaders);
	    m_trackParser->loadExpression(m_trackSelectionString);
    }
    return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::JetTrackParticleThinning::finalize()
{
    ATH_MSG_VERBOSE("finalize() ...");
    ATH_MSG_INFO("Processed "<< m_ntot <<" tracks, "<< m_npass<< " were retained ");
    return StatusCode::SUCCESS;
}

// The thinning itself
StatusCode DerivationFramework::JetTrackParticleThinning::doThinning() const
{
    const EventContext& ctx = Gaudi::Hive::currentContext();
    
    // Retrieve main TrackParticle collection
    SG::ThinningHandle<xAOD::TrackParticleContainer> importedTrackParticles
      (m_inDetSGKey, ctx);
    
    // Check the event contains tracks
    unsigned int nTracks = importedTrackParticles->size();
    if (nTracks==0) return StatusCode::SUCCESS;
    
    // Set up a mask with the same entries as the full TrackParticle collection
    std::vector<bool> mask;
    mask.assign(nTracks,false); // default: don't keep any tracks
    m_ntot += nTracks;
    
    // Retrieve containers
    // ... jets
    const xAOD::JetContainer* importedJets(0);
    if (evtStore()->retrieve(importedJets,m_jetSGKey).isFailure()) {
        ATH_MSG_ERROR("No jet collection with name " << m_jetSGKey << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    unsigned int nJets(importedJets->size());
    std::vector<const xAOD::Jet*> jetToCheck; jetToCheck.clear();
    
    // Execute the text parser if requested
    if (m_selectionString!="") {
        std::vector<int> entries =  m_parser->evaluateAsVector();
        unsigned int nEntries = entries.size();
        // check the sizes are compatible
        if (nJets != nEntries ) {
        	ATH_MSG_ERROR("Sizes incompatible! Are you sure your selection string used jets??");
            return StatusCode::FAILURE;
        } else {
        	// identify which jets to keep for the thinning check
        	for (unsigned int i=0; i<nJets; ++i) if (entries[i]==1) jetToCheck.push_back((*importedJets)[i]);
        }
    }
    
    // Set elements in the mask to true if there is a corresponding ElementLink from a reconstructed object
    // ... jets
    if (m_selectionString=="") { // check all jets as user didn't provide a selection string
        for (xAOD::JetContainer::const_iterator jetIt=importedJets->begin(); jetIt!=importedJets->end(); ++jetIt) {
            std::vector<const xAOD::TrackParticle*> jetTracks;
            bool haveJetTracks = (*jetIt)->getAssociatedObjects(xAOD::JetAttribute::GhostTrack, jetTracks);
            if ( !haveJetTracks ) {ATH_MSG_WARNING("Associated tracks not found");}
            else {
                for (std::vector<const xAOD::TrackParticle*>::iterator jetTrIt=jetTracks.begin(); jetTrIt!=jetTracks.end(); ++jetTrIt) {
                    int index = (*jetTrIt)->index();
                    mask[index] = true;
                }
            }
        }
    } else { // check only jets passing user selection string
    	for (std::vector<const xAOD::Jet*>::iterator jetIt = jetToCheck.begin(); jetIt!=jetToCheck.end(); ++jetIt) {
            std::vector<const xAOD::TrackParticle*> jetTracks;
            bool haveJetTracks = (*jetIt)->getAssociatedObjects(xAOD::JetAttribute::GhostTrack, jetTracks);
            if ( !haveJetTracks ) {ATH_MSG_WARNING("Associated tracks not found");}
            else {
                for (std::vector<const xAOD::TrackParticle*>::iterator jetTrIt=jetTracks.begin(); jetTrIt!=jetTracks.end(); ++jetTrIt) {
                    int index = (*jetTrIt)->index();
                    mask[index] = true;
                }
            }
        }
    }

    // Apply a track selection string.
    if (m_trackParser) {
      std::vector<int> entries =  m_trackParser->evaluateAsVector();
      unsigned int nEntries = entries.size();
      // check the sizes are compatible
      if (nTracks != nEntries ) {
        ATH_MSG_ERROR("Sizes incompatible! Are you sure your track selection string used tracks??");
        return StatusCode::FAILURE;
      } else {
        // identify which jets to keep for the thinning check
        for (unsigned int i=0; i<nEntries; ++i) {
          if (!entries[i]) mask[i] = false;
        }
      }
    }
    
    // Count up the mask contents
    for (unsigned int i=0; i<nTracks; ++i) {
        if (mask[i]) ++m_npass;
    }
    
    // Execute the thinning service based on the mask. Finish.
    importedTrackParticles.keep (mask);

    return StatusCode::SUCCESS;
}


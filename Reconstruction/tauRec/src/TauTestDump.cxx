/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
// file:        TauTestDump.cxx
// package:     Reconstruction/tauRec
// authors:     Felix Friedrich
// date:        2012-11-05
// 
//-----------------------------------------------------------------------------

#include <GaudiKernel/IToolSvc.h>
#include <GaudiKernel/ListItem.h>

#include "FourMomUtils/P4Helpers.h"
#include "FourMom/P4EEtaPhiM.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "Particle/TrackParticle.h"


#include "tauRec/TauCandidateData.h"
//#include "tauEvent/TauCommonDetails.h"
//#include "tauEvent/TauPi0Details.h"
//#include "tauEvent/TauPi0Cluster.h"
#include "xAODTau/TauJet.h"

//#include "tauEvent/TauJetParameters.h"        

#include "tauRec/TauTestDump.h"

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

TauTestDump::TauTestDump(const std::string &type,
    const std::string &name,
    const IInterface *parent) :
TauToolBase(type, name, parent) {
    declareInterface<TauToolBase > (this);
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------

TauTestDump::~TauTestDump() {
}


//-----------------------------------------------------------------------------
// Initializer
//-----------------------------------------------------------------------------

StatusCode TauTestDump::initialize() {
    return StatusCode::SUCCESS;
}


//-----------------------------------------------------------------------------
// Finalizer
//-----------------------------------------------------------------------------

StatusCode TauTestDump::finalize() {
    return StatusCode::SUCCESS;
}


//-----------------------------------------------------------------------------
// Execution
//-----------------------------------------------------------------------------
StatusCode TauTestDump::execute(TauCandidateData *data) {

    ATH_MSG_INFO("=== TAU TEST DUMP BEGIN ==================== ");
  
    xAOD::TauJet *pTau = data->xAODTau;
    
    if (pTau == NULL) {
        ATH_MSG_ERROR("no candidate given");
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Tau recorded with: pt="<< pTau->pt() <<", eta=" <<pTau->eta()<<", phi="<<pTau->phi());
    float tfFlightPathSig;
    float ipSigLeadTrk;
    pTau->detail(xAOD::TauJetParameters::ipSigLeadTrk, ipSigLeadTrk);
    pTau->detail(xAOD::TauJetParameters::trFlightPathSig,tfFlightPathSig );
    ATH_MSG_INFO(" numTrack="<<pTau->nTracks() << ", tfFlightPathSig=" << tfFlightPathSig <<", ipSigLeadTrk=" << ipSigLeadTrk );
    
    //stop here
    return StatusCode::SUCCESS;
    
    if (pTau->nTracks() != 1) {
      // Bonn Pi0 calculated only for 1p taus --> leave test case
      ATH_MSG_INFO("Bonn Pi0 calculated only for 1p taus --> leave test case");
      return StatusCode::SUCCESS;
    }

    // Default PFO pi0
    ATH_MSG_INFO("dumping pi0 standard");
    if (pTau->nPi0_PFOs()>0) {
	    for (unsigned int i=0; i<pTau->nPi0_PFOs();++i) ATH_MSG_INFO(pTau->pi0_PFO(i)->e()<< " ");
    }
    else ATH_MSG_INFO("no pi0 cand");

    // Cell-based PFO pi0
    ATH_MSG_INFO("dumping pi0 cell-based");
    if (pTau->nCellBased_Pi0_PFOs()>0) {
	    for (unsigned int i=0; i<pTau->nCellBased_Pi0_PFOs();++i) ATH_MSG_INFO(pTau->cellBased_Pi0_PFO(i)->e()<< " ");
    }
    else ATH_MSG_INFO("no pi0 cand");

    // EFlow PF0 pi0
    ATH_MSG_INFO("dumping pi0 eflow");
    if (pTau->nEflowRec_Pi0_PFOs()>0) {
	    for (unsigned int i=0; i<pTau->nEflowRec_Pi0_PFOs();++i) ATH_MSG_INFO(pTau->eflowRec_Pi0_PFO(i)->e()<< " ");
    }
    else ATH_MSG_INFO("no pi0 cand");

    ATH_MSG_INFO("end dumping pi0");
    
    ATH_MSG_INFO("=== TAU TEST DUMP END ==================== ");

    return StatusCode::SUCCESS;
}



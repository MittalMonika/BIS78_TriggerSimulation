/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TFile.h"
#include "TF1.h"
#include "GaudiKernel/Property.h"
#include <PathResolver/PathResolver.h>
#include "CLHEP/Units/SystemOfUnits.h"
#include "EventKernel/SignalStateHelper.h"
#include "JetEvent/Jet.h"

#include "xAODTau/TauJet.h"
#include "tauRec/TauCandidateData.h"
#include "tauRec/TauToolBase.h"
#include "tauRec/TauCalibrateEM.h"

using CLHEP::GeV;

//-------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------

TauCalibrateEM::TauCalibrateEM(const std::string& type,
    const std::string& name,
    const IInterface* parent) :
TauToolBase(type, name, parent) {
    declareInterface<TauToolBase > (this);
    declareProperty("response_functions_file", m_response_functions_file = "EMTES_Fits_Oct2010.root");
}

//-------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------

TauCalibrateEM::~TauCalibrateEM() {
}

//-------------------------------------------------------------------------
// initialize
//-------------------------------------------------------------------------

StatusCode TauCalibrateEM::initialize() {

    std::string response_functions_path = PathResolver::find_file(m_response_functions_file, "DATAPATH");
    TFile* f = TFile::Open(response_functions_path.c_str(), "READ");

    m_f1_1p_lem = new TF1(*((TF1*) f->Get("OneP_LowEMF_Eta_All")));
    m_f1_1p_hem_barrel = new TF1(*((TF1*) f->Get("OneP_HighEMF_Eta_0")));
    m_f1_1p_hem_crack = new TF1(*((TF1*) f->Get("OneP_HighEMF_Eta_1")));
    m_f1_1p_hem_endcap = new TF1(*((TF1*) f->Get("OneP_HighEMF_Eta_2")));
    m_f1_mp_barrel = new TF1(*((TF1*) f->Get("MultiP_Eta_0")));
    m_f1_mp_crack = new TF1(*((TF1*) f->Get("MultiP_Eta_1")));
    m_f1_mp_endcap = new TF1(*((TF1*) f->Get("MultiP_Eta_2")));

    m_min_1p_lem.first = m_f1_1p_lem->GetMinimumX(1.5, 3.0);
    m_min_1p_lem.second = m_f1_1p_lem->Eval(m_min_1p_lem.first);
    m_min_1p_hem_barrel.first = m_f1_1p_hem_barrel->GetMinimumX(1.5, 3.0);
    m_min_1p_hem_barrel.second = m_f1_1p_hem_barrel->Eval(m_min_1p_hem_barrel.first);
    m_min_1p_hem_crack.first = m_f1_1p_hem_crack->GetMinimumX(1.5, 3.0);
    m_min_1p_hem_crack.second = m_f1_1p_hem_crack->Eval(m_min_1p_hem_crack.first);
    m_min_1p_hem_endcap.first = m_f1_1p_hem_endcap->GetMinimumX(1.5, 3.0);
    m_min_1p_hem_endcap.second = m_f1_1p_hem_endcap->Eval(m_min_1p_hem_endcap.first);
    m_min_mp_barrel.first = m_f1_mp_barrel->GetMinimumX(2.0, 3.0);
    m_min_mp_barrel.second = m_f1_mp_barrel->Eval(m_min_mp_barrel.first);
    m_min_mp_crack.first = m_f1_mp_crack->GetMinimumX(2.0, 3.0);
    m_min_mp_crack.second = m_f1_mp_crack->Eval(m_min_mp_crack.first);
    m_min_mp_endcap.first = m_f1_mp_endcap->GetMinimumX(2.5, 3.5);
    m_min_mp_endcap.second = m_f1_mp_endcap->Eval(m_min_mp_endcap.first);

    f->Close();
    return StatusCode::SUCCESS;
}

//-------------------------------------------------------------------------
// execute
//-------------------------------------------------------------------------

StatusCode TauCalibrateEM::execute(TauCandidateData *data) {
    xAOD::TauJet *pTau = data->xAODTau;

    if (pTau == NULL) {
        ATH_MSG_ERROR("no candidate given");
        return StatusCode::FAILURE;
    }

    const xAOD::Jet* pJetSeed = (*pTau->jetLink());

    // XXX still need to migrate to signalstate handling of xAOD::Jet
    // SignalStateHelper sigstateH(P4SignalState::JETEMSCALE);
    // sigstateH.controlObject(pJetSeed);

    float emscale_ptEM = 0;
    float emscale_ptHad = 0;
    
    if ( !pTau->detail( xAOD::TauJetParameters::etEMAtEMScale, emscale_ptEM ) ) 
      {
	ATH_MSG_DEBUG("retrieval of tau detail failed. not calculating new em scale pt");
	return StatusCode::SUCCESS;
      }

    if ( !pTau->detail( xAOD::TauJetParameters::etHadAtEMScale, emscale_ptHad ) )
      {
	ATH_MSG_DEBUG("retrieval of tau detail failed. not calculating new em scale pt");
	return StatusCode::SUCCESS;
      }

    double emscale_pt = emscale_ptEM + emscale_ptHad;
    double emscale_eta = pJetSeed->eta();
    double emfrac = (emscale_pt != 0) ? emscale_ptEM / emscale_pt : 0.;


    ATH_MSG_DEBUG("input variables: em_pt " << emscale_pt << " eta " << emscale_eta << " ntrack " << pTau->nTracks() << " emfrac " << emfrac);

    double new_pt = evaluate_new_pt(emscale_pt / GeV, fabs(emscale_eta), pTau->nTracks(), emfrac);


    // do NOT set TauJet energy, as this will be done in tauCalibrateLC
    //tau->setE( new_pt * GeV * cosh( tau->eta() ) );

    // instead fill place holder in TauCommonDetails
    //pDetails->setSeedCalo_etEMCalib(new_pt * GeV);
    
    pTau->setDetail( xAOD::TauJetParameters::EM_TES_scale, static_cast<float>( new_pt * GeV ) );

    return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Finalize
//-----------------------------------------------------------------------------

StatusCode TauCalibrateEM::finalize() {
    delete m_f1_1p_lem;
    delete m_f1_1p_hem_barrel;
    delete m_f1_1p_hem_crack;
    delete m_f1_1p_hem_endcap;
    delete m_f1_mp_barrel;
    delete m_f1_mp_crack;
    delete m_f1_mp_endcap;

    return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// calculate new tau pt at EM scale
//-----------------------------------------------------------------------------

double TauCalibrateEM::evaluate_new_pt(double pt, double eta, int ntrack, double emfrac) {

    if (pt <= 0)return 0;
    double ln_pt = log(pt);
    if (ln_pt > 8.5) return pt;

    if (ntrack <= 1) {
        // corrections for single prong taus

        if (emfrac < 0.15) {
            // corrections for low EMF      
            if (ln_pt < m_min_1p_lem.first) return pt / m_min_1p_lem.second;
            else return pt / m_f1_1p_lem->Eval(ln_pt);
        } else {
            // corrections for high EMF

            if (eta < 1.3) {
                if (ln_pt < m_min_1p_hem_barrel.first) return pt / m_min_1p_hem_barrel.second;
                else return pt / m_f1_1p_hem_barrel->Eval(ln_pt);
            } else if (eta < 1.6) {
                if (ln_pt < m_min_1p_hem_crack.first) return pt / m_min_1p_hem_crack.second;
                else return pt / m_f1_1p_hem_crack->Eval(ln_pt);
            } else {
                if (ln_pt < m_min_1p_hem_endcap.first) return pt / m_min_1p_hem_endcap.second;
                else return pt / m_f1_1p_hem_endcap->Eval(ln_pt);
            }
        }

    } else {
        // corrections for multi-prong taus

        if (eta < 1.3) {
            if (ln_pt < m_min_mp_barrel.first) return pt / m_min_mp_barrel.second;
            else return pt / m_f1_mp_barrel->Eval(ln_pt);
        } else if (eta < 1.6) {
            if (ln_pt < m_min_mp_crack.first) return pt / m_min_mp_crack.second;
            else return pt / m_f1_mp_crack->Eval(ln_pt);
        } else {
            if (ln_pt < m_min_mp_endcap.first) return pt / m_min_mp_endcap.second;
            else return pt / m_f1_mp_endcap->Eval(ln_pt);
        }
    }

    return pt;
}

// EOF



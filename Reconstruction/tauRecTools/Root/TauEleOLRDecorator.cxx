/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Tau Electron Overlap Removal Decorator Tool
 *
 * Author: Dirk Duschinger
 * Modified: Lorenz Hauswald
 */

#include "tauRecTools/TauEleOLRDecorator.h"
#include "ElectronPhotonSelectorTools/AsgElectronLikelihoodTool.h"
#include "TFile.h"

TauEleOLRDecorator::TauEleOLRDecorator(const std::string& name):
  TauRecToolBase(name),
  m_tEMLHTool(nullptr),
  m_xElectronContainer(nullptr),
  m_bElectonsAvailable(true),
  m_sEleOLRFilePath("eveto_cutvals.root"),
  m_hCutValues(nullptr)
{
  declareProperty("EleOLRFile", m_sEleOLRFilePath);
}

TauEleOLRDecorator::~TauEleOLRDecorator(){

}

StatusCode TauEleOLRDecorator::initialize()
{
  ATH_MSG_INFO( "Initializing TauEleOLRDecorator" );
  ATH_CHECK( m_electronInputContainer.initialize() );

  std::string confDir = "ElectronPhotonSelectorTools/offline/mc15_20150712/";
  m_tEMLHTool = std::make_unique<AsgElectronLikelihoodTool>(name()+"_ELHTool");
  m_tEMLHTool->msg().setLevel( msg().level() );
  ATH_CHECK (m_tEMLHTool->setProperty("primaryVertexContainer","PrimaryVertices"));
  ATH_CHECK (m_tEMLHTool->setProperty("ConfigFile",confDir+"ElectronLikelihoodLooseOfflineConfig2015.conf"));
  ATH_CHECK (m_tEMLHTool->initialize());

  m_sEleOLRFilePath = find_file(m_sEleOLRFilePath);
  TFile tmpFile(m_sEleOLRFilePath.c_str());
  m_hCutValues = std::unique_ptr<TH2F>(static_cast<TH2F*>(tmpFile.Get("eveto_cutvals")));
  m_hCutValues->SetDirectory(0);
  tmpFile.Close();
  
  return StatusCode::SUCCESS;
}

StatusCode TauEleOLRDecorator::execute(xAOD::TauJet& tau)
{
  // get electron container                                                                                                                               
  SG::ReadHandle<xAOD::ElectronContainer> electronInHandle( m_electronInputContainer );
  if (!electronInHandle.isValid()) {
    ATH_MSG_FATAL("Electron container with name " << electronInHandle.key() << " was not found in event store, but is needed for electron OLR. Ensure that it is there with the correct name");
    return StatusCode::FAILURE;
  }
  m_xElectronContainer = electronInHandle.cptr();
  ATH_MSG_DEBUG("  read: " << electronInHandle.key() << " = " << "..." );                                                                                     
  
  const xAOD::Electron * xEleMatch = 0;
  float fLHScore = -4.; // default if no match was found

  if (m_bElectonsAvailable)
    if (!m_xElectronContainer)
      m_bElectonsAvailable = false;
  float fEleMatchPt = -1.;
  // find electron with pt>5GeV within 0.4 cone with largest pt
  for( auto xElectron : *(m_xElectronContainer) )
    {
      if(xElectron->pt() < 5000.) continue;
      if(xElectron->p4().DeltaR( tau.p4() ) > 0.4 ) continue;
      if(xElectron->pt() > fEleMatchPt )
	{
	  fEleMatchPt=xElectron->pt();
	  xEleMatch=xElectron;
	}
    }

  // compute the LH score if there is a match
  if(xEleMatch!=0)
    fLHScore = m_tEMLHTool->calculate(xEleMatch);

  // create link to the matched electron
  if (xEleMatch) {
    tau.setDetail( xAOD::TauJetParameters::electronLink, xEleMatch, (m_xElectronContainer) );
  }
  else {
    tau.setDetail( xAOD::TauJetParameters::electronLink, (const xAOD::IParticle* ) 0 );
  }

  // decorate tau with score
  tau.setDiscriminant( xAOD::TauJetParameters::EleMatchLikelihoodScore, fLHScore );

  bool bPass = false;
  if (tau.nTracks() == 1) {
    bPass = (fLHScore <= getCutVal(tau.track(0)->eta(), tau.pt()/1000.));
  }
  else {
    bPass = true;
  }

  tau.setIsTau(xAOD::TauJetParameters::PassEleOLR, bPass);
  
  return StatusCode::SUCCESS;
}

StatusCode TauEleOLRDecorator::finalize()
{
  return StatusCode::SUCCESS;
}

float TauEleOLRDecorator::getCutVal(float fEta, float fPt)
{
  if(fPt>1900) fPt=1900;
  if(fabs(fEta)>2.465) fEta=2.465;
  int iBin= m_hCutValues->FindBin(fPt, fabs(fEta));
  return m_hCutValues->GetBinContent(iBin);
}

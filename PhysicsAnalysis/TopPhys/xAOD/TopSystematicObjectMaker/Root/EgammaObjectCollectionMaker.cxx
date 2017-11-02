/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: EgammaObjectCollectionMaker.cxx 811374 2017-10-24 13:04:52Z iconnell $
#include "TopSystematicObjectMaker/EgammaObjectCollectionMaker.h"
#include "TopConfiguration/TopConfig.h"
#include "TopEvent/EventTools.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/PhotonAuxContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "PATInterfaces/SystematicsUtil.h"

#include <list>

namespace top{
  
  EgammaObjectCollectionMaker::EgammaObjectCollectionMaker( const std::string& name ) :
    asg::AsgTool( name ),
    m_config(nullptr),
    
    m_specifiedSystematicsPhotons(),
    m_specifiedSystematicsElectrons(),
    m_recommendedSystematicsPhotons(),
    m_recommendedSystematicsElectrons(),

    m_calibrationTool("CP::EgammaCalibrationAndSmearingTool"),
    m_photonFudgeTool("PhotonFudgeTool"),
    
    m_isolationTool_LooseTrackOnly("CP::IsolationTool_LooseTrackOnly"),
    m_isolationTool_Loose("CP::IsolationTool_Loose"),
    m_isolationTool_Gradient("CP::IsolationTool_Gradient"),
    m_isolationTool_GradientLoose("CP::IsolationTool_GradientLoose"),
    m_isolationTool_FixedCutTight("CP::IsolationTool_FixedCutTight"),
    m_isolationTool_FixedCutTightTrackOnly("CP::IsolationTool_FixedCutTightTrackOnly"),
    m_isolationTool_FixedCutTightCaloOnly("CP::IsolationTool_FixedCutTightCaloOnly"),
    m_isolationTool_FixedCutLoose("CP::IsolationTool_FixedCutLoose"),
    m_isolationTool_FixedCutHighPtCaloOnly("CP::IsolationTool_FixedCutHighPtCaloOnly"),
    m_isolationCorr("CP::IsolationCorrectionTool")
  {
    declareProperty( "config" , m_config ); 
    
    declareProperty( "EgammaCalibrationAndSmearingTool" , m_calibrationTool );

    declareProperty( "IsolationTool_LooseTrackOnly" , m_isolationTool_LooseTrackOnly );
    declareProperty( "IsolationTool_Loose" , m_isolationTool_Loose );
    declareProperty( "IsolationTool_Gradient" , m_isolationTool_Gradient );
    declareProperty( "IsolationTool_GradientLoose" , m_isolationTool_GradientLoose );     
    declareProperty( "IsolationTool_FixedCutTight" , m_isolationTool_FixedCutTight );
    declareProperty( "IsolationTool_FixedCutTightTrackOnly" , m_isolationTool_FixedCutTightTrackOnly );
    declareProperty( "IsolationTool_FixedCutTightCaloOnly" , m_isolationTool_FixedCutTightCaloOnly );
    declareProperty( "IsolationTool_FixedCutLoose" , m_isolationTool_FixedCutLoose );
    declareProperty( "IsolationTool_FixedCutHighPtCaloOnly", m_isolationTool_FixedCutHighPtCaloOnly );
    declareProperty( "IsolationCorrectionTool", m_isolationCorr );
  } 
  
  StatusCode EgammaObjectCollectionMaker::initialize()
  {
    ATH_MSG_INFO(" top::EgammaObjectCollectionMaker initialize" );
    
    top::check( m_calibrationTool.retrieve() , "Failed to retrieve egamma calibration tool" );
    
    // Release 21 scrutiny effort currently requires that no egamma calibrations are applied
    // We will use the tools with R20.7 settings, but not store the calibrated objects
    // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/AtlasRelease21Scrutiny#Temporary_Recommendations_for_Co
    if(m_config->getReleaseSeries() == 25){
      ATH_MSG_INFO( "Configuring EgammaObjectCollectionMaker for Release 21 scrutiny" );
      ATH_MSG_INFO( "... We should not currently apply any calibrations ..." );
      ATH_MSG_INFO( "... The tools will be setup, but the calibrated objects will not be saved ..." );
      calibrateElectrons = false;
      calibratePhotons   = false;
    }
    else{
      calibrateElectrons = true;
      calibratePhotons   = true;
    }


    if (m_config->usePhotons()) {
      top::check(m_isolationTool_FixedCutTight.retrieve(),
                 "Failed to retrieve Isolation Tool" );
      top::check(m_isolationTool_FixedCutTightCaloOnly.retrieve(),
                 "Failed to retrieve Isolation Tool");
      top::check(m_isolationTool_FixedCutLoose.retrieve(),
                 "Failed to retrieve Isolation Tool");
      top::check(m_photonFudgeTool.retrieve(),
                 "Failed to retrieve photon shower shape fudge tool");
    }

    if (m_config->useElectrons()) {
      top::check( m_isolationTool_LooseTrackOnly.retrieve() , "Failed to retrieve Isolation Tool" );
      top::check( m_isolationTool_Loose.retrieve() , "Failed to retrieve Isolation Tool" );
      top::check( m_isolationTool_Gradient.retrieve() , "Failed to retrieve Isolation Tool" );
      top::check( m_isolationTool_GradientLoose.retrieve() , "Failed to retrieve Isolation Tool" );
      top::check( m_isolationTool_FixedCutTight.retrieve() , "Failed to retrieve Isolation Tool" );
      top::check( m_isolationTool_FixedCutTightTrackOnly.retrieve() , "Failed to retrieve Isolation Tool" );
      top::check( m_isolationTool_FixedCutLoose.retrieve() , "Failed to retrieve Isolation Tool" );
    }
    
    top::check( m_isolationCorr.retrieve() , "Failed to retrieve Isolation Correction Tool" );

    std::set<std::string> systPhoton;
    std::set<std::string> systElectron;    

    const std:: string& syststr = m_config->systematics();
    std::set<std::string> syst;  

    if( !m_config->isSystNominal(syststr) && !m_config->isSystAll(syststr) ){
      bool ok = m_config->getSystematicsList(syststr, syst);
      if(!ok){
	ATH_MSG_ERROR(" top::EgammaObjectCollectionMaker could not determine systematic list" );
	return StatusCode::FAILURE;
      }
      if(!m_config->contains(syst, "AllElectrons")){
	systElectron=syst;
      }
      if(!m_config->contains(syst, "AllPhotons")){
	systPhoton=syst;
      }
    }

    specifiedSystematicsPhotons( systPhoton );
    specifiedSystematicsElectrons( systElectron );
    
    if (m_config->usePhotons()) {
      m_config->systematicsPhotons( specifiedSystematicsPhotons() );
    }
    if (m_config->useElectrons()) {
      m_config->systematicsElectrons( specifiedSystematicsElectrons() );
    }

    return StatusCode::SUCCESS;
  }
  
  StatusCode EgammaObjectCollectionMaker::executePhotons()
  {
    ///-- Get base photons from xAOD --///
    const xAOD::PhotonContainer* xaod(nullptr);
    top::check( evtStore()->retrieve( xaod , m_config->sgKeyPhotons() ) , "Failed to retrieve Photons" );

    ///-- Loop over all systematics --///
    for( auto systematic : m_specifiedSystematicsPhotons ){
      
      ///-- Tell tool which systematic to use --///
      top::check( m_calibrationTool->applySystematicVariation( systematic ) , "Failed to applySystematicVariation" ); 
      
      ///-- Shallow copy of the xAOD --///
      std::pair< xAOD::PhotonContainer*, xAOD::ShallowAuxContainer* > shallow_xaod_copy = xAOD::shallowCopyContainer( *xaod );
      
      ///-- Loop over the xAOD Container and apply corrections--///
      for( auto photon : *(shallow_xaod_copy.first) ){      
        
        ///-- Apply correction to object --///
	// Needs a calo cluster so carry on if no cluster
	if (!photon->caloCluster()) continue;
	
	if(calibratePhotons){

	  top::check(m_calibrationTool->applyCorrection( *photon ) , 
		     "Failed to applyCorrection" );        
	  top::check(m_isolationCorr->applyCorrection(*photon),
		     "Failed to apply photon isolation leakage correction");

	  // Only apply shower shape fudging on full simulation MC
	  if (m_config->isMC() && !m_config->isAFII()) {
	    top::check(m_photonFudgeTool->applyCorrection(*photon),
		       "Failed to apply photon shower shape fudge tool");
	  }

	}
        
        ///-- Isolation selection --///
        char passIsol_FixedCutTight(0);
        char passIsol_FixedCutTightCaloOnly(0);
        char passIsol_FixedCutLoose(0);
        if (m_isolationTool_FixedCutTight->accept(*photon)) {
          passIsol_FixedCutTight = 1;
        }
        if (m_isolationTool_FixedCutTightCaloOnly->accept(*photon)) {
          passIsol_FixedCutTightCaloOnly = 1;
        }
        if (m_isolationTool_FixedCutLoose->accept(*photon)) {
          passIsol_FixedCutLoose = 1;
        }
        photon->auxdecor<char>("AnalysisTop_Isol_FixedCutTight") = passIsol_FixedCutTight;
        photon->auxdecor<char>("AnalysisTop_Isol_FixedCutTightCaloOnly") = passIsol_FixedCutTightCaloOnly;
        photon->auxdecor<char>("AnalysisTop_Isol_FixedCutLoose") = passIsol_FixedCutLoose;
      }

      ///-- set links to original objects- needed for MET calculation --///
      bool setLinks = xAOD::setOriginalObjectLink( *xaod, *shallow_xaod_copy.first );
      if (!setLinks)
	ATH_MSG_ERROR(" Cannot set original object links for photons, MET recalculation may struggle" );
      
      ///-- Save corrected xAOD Container to StoreGate / TStore --///
      std::string outputSGKey = m_config->sgKeyPhotons( systematic.hash() );
      std::string outputSGKeyAux = outputSGKey + "Aux.";      

      xAOD::TReturnCode save = evtStore()->tds()->record( shallow_xaod_copy.first , outputSGKey );
      xAOD::TReturnCode saveAux = evtStore()->tds()->record( shallow_xaod_copy.second , outputSGKeyAux );      

      if( !save || !saveAux ){
	return StatusCode::FAILURE;
      }

    }  // Loop over all systematics      
    
    return StatusCode::SUCCESS;
  }   
    
  StatusCode EgammaObjectCollectionMaker::executeElectrons()
  {

    const xAOD::EventInfo* eventInfo(nullptr);
    top::check( evtStore()->retrieve( eventInfo, m_config->sgKeyEventInfo() ), "Failed to retrieve EventInfo");
    float beam_pos_sigma_x = eventInfo->beamPosSigmaX();
    float beam_pos_sigma_y = eventInfo->beamPosSigmaY();
    float beam_pos_sigma_xy = eventInfo->beamPosSigmaXY();
    
    ///-- Get base electrons from xAOD --///
    const xAOD::ElectronContainer* xaod(nullptr);
    top::check( evtStore()->retrieve( xaod , m_config->sgKeyElectrons() ) , "Failed to retrieve Electrons" );
    
    ///-- Loop over all systematics --///
    for( auto systematic : m_specifiedSystematicsElectrons ){
      
      ///-- Tell tool which systematic to use -///
      top::check( m_calibrationTool->applySystematicVariation( systematic ) , "Failed to applySystematicVariation" );
      
      ///-- Shallow copy of the xAOD --///
      std::pair< xAOD::ElectronContainer*, xAOD::ShallowAuxContainer* > shallow_xaod_copy = xAOD::shallowCopyContainer( *xaod );
      
      ///-- Loop over the xAOD Container --///
      for( auto electron : *(shallow_xaod_copy.first) ){
        
        // Apply correction to object 
        // should not affect derivations if there is no CC or track thinning
        if (electron->caloCluster() != nullptr && electron->trackParticle() != nullptr) { // derivations might remove CC and tracks for low pt electrons

	  if(calibrateElectrons){

	    top::check( m_calibrationTool->applyCorrection( *electron ) , "Failed to applyCorrection" );	    
	    top::check( m_isolationCorr->applyCorrection( *electron ), "Failed to apply leakage correction" );

	  }

	  double d0sig = xAOD::TrackingHelpers::d0significance( electron->trackParticle(),
								beam_pos_sigma_x,
								beam_pos_sigma_y,
								beam_pos_sigma_xy );
	  electron->auxdecor<float>("d0sig") = d0sig;
	  
	  if( eventInfo->isAvailable<float>("AnalysisTop_PRIVTX_z_position") ){
	    float vtx_z = eventInfo->auxdata<float>("AnalysisTop_PRIVTX_z_position");
	    float delta_z0 = electron->trackParticle()->z0() + electron->trackParticle()->vz() - vtx_z;
	    electron->auxdecor<float>("delta_z0") = delta_z0;
	    electron->auxdecor<float>("delta_z0_sintheta") = delta_z0*std::sin( electron->trackParticle()->theta() );
	    
	  }
	}
        
        ///-- Isolation selection --///
        char passIsol_LooseTrackOnly(0),passIsol_Loose(0),passIsol_FixedCutTight(0),passIsol_FixedCutTightTrackOnly(0),passIsol_FixedCutLoose(0);
        char passIsol_Gradient(0),passIsol_GradientLoose(0);
	char passIsol_FixedCutHighPtCaloOnly(0);
        if (m_isolationTool_LooseTrackOnly->accept( *electron )) {passIsol_LooseTrackOnly = 1;}
        if (m_isolationTool_Loose->accept( *electron )) {passIsol_Loose = 1;}
        if (m_isolationTool_Gradient->accept( *electron )) {passIsol_Gradient = 1;}
        if (m_isolationTool_GradientLoose->accept( *electron )) {passIsol_GradientLoose = 1;}
	if (m_isolationTool_FixedCutTight->accept( *electron )) {passIsol_FixedCutTight = 1;}
	if (m_isolationTool_FixedCutTightTrackOnly->accept( *electron )) {passIsol_FixedCutTightTrackOnly = 1;}	
	if (m_isolationTool_FixedCutLoose->accept( *electron )) {passIsol_FixedCutLoose = 1;}	
	if (m_isolationTool_FixedCutHighPtCaloOnly->accept( *electron )) {passIsol_FixedCutHighPtCaloOnly = 1;}

        electron->auxdecor<char>("AnalysisTop_Isol_LooseTrackOnly")         = passIsol_LooseTrackOnly;
        electron->auxdecor<char>("AnalysisTop_Isol_Loose")                  = passIsol_Loose;
        electron->auxdecor<char>("AnalysisTop_Isol_Gradient")               = passIsol_Gradient;
        electron->auxdecor<char>("AnalysisTop_Isol_GradientLoose")          = passIsol_GradientLoose;        
	electron->auxdecor<char>("AnalysisTop_Isol_FixedCutTight")          = passIsol_FixedCutTight;
	electron->auxdecor<char>("AnalysisTop_Isol_FixedCutTightTrackOnly") = passIsol_FixedCutTightTrackOnly;
	electron->auxdecor<char>("AnalysisTop_Isol_FixedCutLoose")          = passIsol_FixedCutLoose;
	electron->auxdecor<char>("AnalysisTop_Isol_FixedCutHighPtCaloOnly") = passIsol_FixedCutHighPtCaloOnly;

	// For electron prompt isolation tagger, check a cut on the tagger and also loose isolation
	electron->auxdecor<char>("AnalysisTop_Isol_PromptLepton") = (electron->auxdata<float>("PromptLeptonIso_TagWeight") < -0.5) ? 1 : 0;

      }
      
      ///-- set links to original objects- needed for MET calculation --///
      bool setLinks = xAOD::setOriginalObjectLink( *xaod, *shallow_xaod_copy.first );
      if (!setLinks)
	ATH_MSG_ERROR(" Cannot set original object links for electrons, MET recalculation may struggle" );
      
      // Save corrected xAOD Container to StoreGate / TStore
      std::string outputSGKey = m_config->sgKeyElectronsStandAlone( systematic.hash() );
      std::string outputSGKeyAux = outputSGKey + "Aux.";

      xAOD::TReturnCode save = evtStore()->tds()->record( shallow_xaod_copy.first , outputSGKey );
      xAOD::TReturnCode saveAux = evtStore()->tds()->record( shallow_xaod_copy.second , outputSGKeyAux );
      
      if( !save || !saveAux ){
	return StatusCode::FAILURE;
      }

    }  // Loop over all systematics     
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode EgammaObjectCollectionMaker::printoutPhotons()
  {
    // Loop over all systematics 
    for( auto s : m_specifiedSystematicsPhotons ){
      const xAOD::PhotonContainer* xaod(nullptr);
      top::check( evtStore()->retrieve( xaod , m_config->sgKeyPhotons( s.hash() ) ) , "Failed to retrieve Photons" );   
      
      ATH_MSG_INFO(" Photons with sgKey = "<<m_config->sgKeyPhotons( s.hash() ) );
      for( auto x : *xaod ){
        float ptcone30(0.);
        x->isolationValue( ptcone30 , xAOD::Iso::ptcone30 );
        ATH_MSG_INFO("   ph pT , eta , ptcone30 = "<<x->pt()<<" , "<<x->eta()<<" , "<<ptcone30 );
      }
    }
    
    return StatusCode::SUCCESS;    
  }  
   
  
  StatusCode EgammaObjectCollectionMaker::printoutElectrons()
  {
    // Loop over all systematics 
    for( auto s : m_specifiedSystematicsElectrons ){
      const xAOD::ElectronContainer* xaod(nullptr);
      top::check( evtStore()->retrieve( xaod , m_config->sgKeyElectronsStandAlone( s.hash() ) ), "Failed to retrieve Electrons" );   
      
      ATH_MSG_INFO(" Electrons with sgKey = "<<m_config->sgKeyElectronsStandAlone( s.hash() ) );
      for( auto x : *xaod ){
        float ptcone30(0.);
        x->isolationValue( ptcone30 , xAOD::Iso::ptcone30 );
        ATH_MSG_INFO("   El pT , eta , ptcone30 = "<<x->pt()<<" , "<<x->eta()<<" , "<<ptcone30 );
      }
    }
    
    return StatusCode::SUCCESS;    
  }
    
  void EgammaObjectCollectionMaker::specifiedSystematicsPhotons  ( const std::set<std::string>& specifiedSystematics )
  {
    ///-- Get the recommended systematics from the tool, in std::vector format --///
    const std::vector<CP::SystematicSet> systList = CP::make_systematics_vector( m_calibrationTool->recommendedSystematics() );
    
    for (auto s : systList) {
      m_recommendedSystematicsPhotons.push_back(s);
      if (s.name() == "") {
        m_specifiedSystematicsPhotons.push_back(s);
      }
      
      ///-- MC only --///
      if (m_config->isMC()) {
        ///-- Are we only doing Nominal? Did the user specify specific systematics to use? --///
        if (!m_config->isSystNominal( m_config->systematics() )) {
          if (specifiedSystematics.size() == 0) {
            m_specifiedSystematicsPhotons.push_back(s);
          }
          if (specifiedSystematics.size()  > 0) {
            for (auto i : specifiedSystematics) {
              if ( i == s.name() ) {
                m_specifiedSystematicsPhotons.push_back(s);              
              }
            }
          }
        }
      }
    }  
    m_recommendedSystematicsPhotons.sort();    
    m_recommendedSystematicsPhotons.unique();
    m_specifiedSystematicsPhotons.sort();
    m_specifiedSystematicsPhotons.unique();    
  }
  
  void EgammaObjectCollectionMaker::specifiedSystematicsElectrons( const std::set<std::string>& specifiedSystematics )
  {
    ///-- Get the recommended systematics from the tool, in std::vector format --///
    const std::vector<CP::SystematicSet> systList = CP::make_systematics_vector( m_calibrationTool->recommendedSystematics() );
    
    for (auto s : systList) {
      m_recommendedSystematicsElectrons.push_back(s);
      if (s.name() == "") {
        m_specifiedSystematicsElectrons.push_back(s);
      }
      
      ///-- MC only --///
      if (m_config->isMC()) {
        ///-- Are we only doing Nominal? Did the user specify specific systematics to use? --///
        if (!m_config->isSystNominal( m_config->systematics() )) {
          if (specifiedSystematics.size() == 0) {
            m_specifiedSystematicsElectrons.push_back(s);
          }
          if (specifiedSystematics.size()  > 0) {
            for (auto i : specifiedSystematics) {
              if ( i == s.name() ) {
                m_specifiedSystematicsElectrons.push_back(s);              
              }
            }
          }
        }
      }
    } 
    m_recommendedSystematicsElectrons.sort();    
    m_recommendedSystematicsElectrons.unique();
    m_specifiedSystematicsElectrons.sort();
    m_specifiedSystematicsElectrons.unique();
  }
  
}

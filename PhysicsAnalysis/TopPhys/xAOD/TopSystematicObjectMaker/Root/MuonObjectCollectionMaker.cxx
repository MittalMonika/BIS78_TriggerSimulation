/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: MuonObjectCollectionMaker.cxx 810751 2017-09-29 14:41:39Z iconnell $
#include "TopSystematicObjectMaker/MuonObjectCollectionMaker.h"
#include "TopConfiguration/TopConfig.h"
#include "TopEvent/EventTools.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "PATInterfaces/SystematicsUtil.h"

namespace top{

  MuonObjectCollectionMaker::MuonObjectCollectionMaker( const std::string& name ) :
    asg::AsgTool( name ),
    m_config(nullptr),

    m_specifiedSystematics(),
    m_recommendedSystematics(),

    m_calibrationTool("CP::MuonCalibrationAndSmearingTool"),
    m_calibrationTool2017("CP::MuonCalibrationAndSmearingTool2017"),
    m_isolationTool_LooseTrackOnly("CP::IsolationTool_LooseTrackOnly"),
    m_isolationTool_Loose("CP::IsolationTool_Loose"),
    m_isolationTool_Gradient("CP::IsolationTool_Gradient"),
    m_isolationTool_GradientLoose("CP::IsolationTool_GradientLoose"),    
    m_isolationTool_FixedCutTightTrackOnly("CP::IsolationTool_FixedCutTightTrackOnly"),
    m_isolationTool_FixedCutLoose("CP::IsolationTool_FixedCutLoose"),
    m_muonSelectionToolVeryLooseVeto("CP::MuonSelectionToolVeryLooseVeto")
  {
    declareProperty( "config" , m_config );  
    
    declareProperty( "MuonCalibrationAndSmearingTool" ,       m_calibrationTool );
    declareProperty( "MuonCalibrationAndSmearingTool2017" ,   m_calibrationTool2017 );
    declareProperty( "IsolationTool_LooseTrackOnly" ,         m_isolationTool_LooseTrackOnly );
    declareProperty( "IsolationTool_Loose" ,                  m_isolationTool_Loose );
    declareProperty( "IsolationTool_Gradient" ,               m_isolationTool_Gradient );
    declareProperty( "IsolationTool_GradientLoose" ,          m_isolationTool_GradientLoose );
    declareProperty( "IsolationTool_FixedCutTightTrackOnly" , m_isolationTool_FixedCutTightTrackOnly );
    declareProperty( "IsolationTool_FixedCutLoose" ,          m_isolationTool_FixedCutLoose );
    declareProperty( "MuonSelectionToolVeryLooseVeto" ,       m_muonSelectionToolVeryLooseVeto );
  }

  StatusCode MuonObjectCollectionMaker::initialize()
  {
    ATH_MSG_INFO(" top::MuonObjectCollectionMaker initialize" );  
    
    top::check( m_calibrationTool.retrieve()     , "Failed to retrieve muon calibration tool" );
    top::check( m_calibrationTool2017.retrieve() , "Failed to retrieve muon 2017 calibration tool" );

    top::check( m_isolationTool_LooseTrackOnly.retrieve() , "Failed to retrieve Isolation Tool" );
    top::check( m_isolationTool_Loose.retrieve() , "Failed to retrieve Isolation Tool" );
    top::check( m_isolationTool_Gradient.retrieve() , "Failed to retrieve Isolation Tool" );
    top::check( m_isolationTool_GradientLoose.retrieve() , "Failed to retrieve Isolation Tool" );    
    top::check( m_isolationTool_FixedCutTightTrackOnly.retrieve() , "Failed to retrieve Isolation Tool" );
    top::check( m_isolationTool_FixedCutLoose.retrieve() , "Failed to retrieve Isolation Tool" );
    top::check( m_muonSelectionToolVeryLooseVeto.retrieve() , "Failed to retrieve Selection Tool" );

    ///-- Set Systematics Information --///
    const std:: string& syststr = m_config->systematics();
    std::set<std::string> syst;  

    if( !m_config->isSystNominal(syststr) && !m_config->isSystAll(syststr)){
      bool ok = m_config->getSystematicsList(syststr, syst);
      if (!ok) {
	ATH_MSG_ERROR(" top::MuonObjectCollectionMaker could not determine systematic list" );
	return StatusCode::FAILURE;
      }
      if (m_config->contains(syst, "AllMuons")) {
	syst.clear();
      }
    }
 
    specifiedSystematics( syst );
    
    m_config->systematicsMuons( specifiedSystematics() );
    
    ATH_MSG_INFO(" top::MuonObjectCollectionMaker completed initialize" );
    return StatusCode::SUCCESS;
  }
    
  StatusCode MuonObjectCollectionMaker::execute(bool executeNominal)
  {

    const xAOD::EventInfo* eventInfo(nullptr);
    top::check( evtStore()->retrieve( eventInfo, m_config->sgKeyEventInfo() ), "Failed to retrieve EventInfo");
    float beam_pos_sigma_x = eventInfo->beamPosSigmaX();
    float beam_pos_sigma_y = eventInfo->beamPosSigmaY();
    float beam_pos_sigma_xy = eventInfo->beamPosSigmaXY();

    ///--- Need to know the year for each event until MCP have consistent recommendations for all years ---///
    unsigned int runnumber = -999999;
    if(m_config->isMC()){
      top::check(eventInfo->isAvailable<unsigned int>("RandomRunNumber"), "Require that RandomRunNumber decoration is available.");
      runnumber = eventInfo->auxdataConst<unsigned int>("RandomRunNumber");
    }
    else{
      runnumber = eventInfo->runNumber();
    }
    const std::string thisYear = m_config->getYear(runnumber);

    ///-- Get base muons and tracks from xAOD --///
    const xAOD::MuonContainer* xaod(nullptr);
    top::check( evtStore()->retrieve( xaod , m_config->sgKeyMuons() ) , "Failed to retrieve Muons" );
    
    ///-- Loop over all systematics --///
    for( auto systematic : m_specifiedSystematics ){

      ///-- if executeNominal, skip other systematics (and vice-versa) --///
      if(executeNominal && !m_config->isSystNominal(systematic)) continue;
      if(!executeNominal && m_config->isSystNominal(systematic)) continue;
        
      ///-- Tell tool which systematic to use --///
      if(thisYear == "2015" || thisYear == "2016")	 
	top::check( m_calibrationTool->applySystematicVariation( systematic ) , "Failed to applySystematicVariation" );
      else if(thisYear == "2017")
	top::check( m_calibrationTool2017->applySystematicVariation( systematic ) , "Failed to applySystematicVariation 2017" );
      else
	ATH_MSG_ERROR("Unknown year found from (Random)RunNumber - Do not know which MCP calibration to apply");
      
      ///-- Shallow copy of the xAOD --///
      std::pair< xAOD::MuonContainer*, xAOD::ShallowAuxContainer* > shallow_xaod_copy = xAOD::shallowCopyContainer( *xaod );
      
      ///-- Loop over the xAOD Container and apply corrections--///
      for( auto muon : *(shallow_xaod_copy.first) ){
          
	///-- Apply momentum correction --///
        if (muon->primaryTrackParticle()) {
	  if(thisYear == "2015" || thisYear == "2016")
	    top::check( m_calibrationTool->applyCorrection( *muon ) , "Failed to applyCorrection" );
	  else if(thisYear == "2017")
	    top::check( m_calibrationTool2017->applyCorrection( *muon ) , "Failed to applyCorrection 2017" );
	  else
	    ATH_MSG_ERROR("Unknown year found from (Random)RunNumber - Do not know which MCP calibration to apply");

          // don't do the decorations unless the muons are at least Loose
          // this is because it may fail if the muons are at just VeryLoose
          if (m_muonSelectionToolVeryLooseVeto->accept(*muon)) {
            double d0sig = xAOD::TrackingHelpers::d0significance( muon->primaryTrackParticle(),
                                                                  beam_pos_sigma_x,
                                                                  beam_pos_sigma_y,
                                                                  beam_pos_sigma_xy );
            muon->auxdecor<float>("d0sig") = d0sig;

            if( eventInfo->isAvailable<float>("AnalysisTop_PRIVTX_z_position") ){
              float vtx_z = eventInfo->auxdata<float>("AnalysisTop_PRIVTX_z_position");
              float delta_z0 = muon->primaryTrackParticle()->z0() + muon->primaryTrackParticle()->vz() - vtx_z;
              muon->auxdecor<float>("delta_z0") = delta_z0;
              muon->auxdecor<float>("delta_z0_sintheta") = delta_z0*std::sin( muon->primaryTrackParticle()->theta() );
            }
          }
	}
	
        ///-- Isolation selection --///
        char passIsol_LooseTrackOnly(0),passIsol_Loose(0),passIsol_FixedCutTightTrackOnly(0);
        char passIsol_Gradient(0),passIsol_GradientLoose(0),passIsol_FixedCutLoose(0);
        if (m_isolationTool_LooseTrackOnly->accept( *muon )) {passIsol_LooseTrackOnly = 1;}
        if (m_isolationTool_Loose->accept( *muon )) {passIsol_Loose = 1;}
        if (m_isolationTool_Gradient->accept( *muon )) {passIsol_Gradient = 1;}
        if (m_isolationTool_GradientLoose->accept( *muon )) {passIsol_GradientLoose = 1;}
	if (m_isolationTool_FixedCutTightTrackOnly->accept( *muon )) {passIsol_FixedCutTightTrackOnly = 1;}
	if (m_isolationTool_FixedCutLoose->accept( *muon )) {passIsol_FixedCutLoose = 1;}

        muon->auxdecor<char>("AnalysisTop_Isol_LooseTrackOnly") = passIsol_LooseTrackOnly;
        muon->auxdecor<char>("AnalysisTop_Isol_Loose") = passIsol_Loose;
        muon->auxdecor<char>("AnalysisTop_Isol_Gradient") = passIsol_Gradient;
        muon->auxdecor<char>("AnalysisTop_Isol_GradientLoose") = passIsol_GradientLoose; 	
	muon->auxdecor<char>("AnalysisTop_Isol_FixedCutTightTrackOnly") = passIsol_FixedCutTightTrackOnly;
	muon->auxdecor<char>("AnalysisTop_Isol_FixedCutLoose") = passIsol_FixedCutLoose;
	// PromptLeptonIsolation - Some protection incase things change in R21
	if(muon->isAvailable<float>("PromptLeptonIso_TagWeight")){
	  muon->auxdecor<char>("AnalysisTop_Isol_PromptLepton") = (muon->auxdata<float>("PromptLeptonIso_TagWeight") < -0.5) ? 1 : 0;
	}
	else{
	  muon->auxdecor<char>("AnalysisTop_Isol_PromptLepton") = 0;
	}

      }

      ///-- set links to original objects- needed for MET calculation --///
      bool setLinks = xAOD::setOriginalObjectLink( *xaod, *shallow_xaod_copy.first );
      if (!setLinks)
	ATH_MSG_ERROR(" Cannot set original object links for muons, MET recalculation may struggle" );
      
      ///-- Save corrected xAOD Container to StoreGate / TStore --///
      std::string outputSGKey = m_config->sgKeyMuons( systematic.hash() );
      std::string outputSGKeyAux = outputSGKey + "Aux.";

      xAOD::TReturnCode save = evtStore()->tds()->record( shallow_xaod_copy.first , outputSGKey );
      xAOD::TReturnCode saveAux = evtStore()->tds()->record( shallow_xaod_copy.second , outputSGKeyAux );
      if( !save || !saveAux ){
        return StatusCode::FAILURE;
      }      
    }  // Loop over all systematics    
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode MuonObjectCollectionMaker::printout()
  {
    ///-- Loop over all systematics --///
    for( auto systematic : m_specifiedSystematics ){
      const xAOD::MuonContainer* xaod(nullptr);
      top::check( evtStore()->retrieve( xaod , m_config->sgKeyMuons( systematic.hash() ) ) , "Failed to retrieve Muons" );   
      
      ATH_MSG_INFO(" Muons with sgKey = "<< m_config->sgKeyMuons( systematic.hash() ) );
      for( auto muon : *xaod ){
        ATH_MSG_INFO("   MU pT , eta  = "<< muon->pt() <<" , "<< muon->eta() );
      }
    }
    
    return StatusCode::SUCCESS;    
  }
    
  
  void MuonObjectCollectionMaker::specifiedSystematics( const std::set<std::string>& specifiedSystematics )
  {
    ///-- Get the recommended systematics from the tool, in std::vector format --///
    const std::vector<CP::SystematicSet> systList = CP::make_systematics_vector( m_calibrationTool->recommendedSystematics() );
    
    for (auto s : systList) {
      m_recommendedSystematics.push_back(s);
      if (s.name() == "") {
        m_specifiedSystematics.push_back(s);
      }
      
      ///-- MC only --///
      if (m_config->isMC()) {
        ///-- Are we only doing Nominal? Did the user specify specific systematics to use? --///
        if (!m_config->isSystNominal( m_config->systematics() )) {
          if (specifiedSystematics.size() == 0) {
            m_specifiedSystematics.push_back(s);
          }
          if (specifiedSystematics.size()  > 0) {
            for (auto i : specifiedSystematics) {
              if ( i == s.name() ) {
                m_specifiedSystematics.push_back(s);              
              }
            }
          }
        }
      }
    }
    m_recommendedSystematics.sort();    
    m_recommendedSystematics.unique();
    m_specifiedSystematics.sort();
    m_specifiedSystematics.unique();      
  }
  
}

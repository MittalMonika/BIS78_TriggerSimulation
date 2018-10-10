/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/


#include "DerivationFrameworkHiggs/MergedElectronDetailsDecorator.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

#include "egammaInterfaces/IEMExtrapolationTools.h"
#include <vector>

namespace DerivationFramework {
  
  MergedElectronDetailsDecorator::MergedElectronDetailsDecorator(const std::string& t, const std::string& n, const IInterface* p):
    AthAlgTool(t,n,p),
    m_emExtrapolationTool("EMExtrapolationTools"),
    m_minET(5000)
  {
    declareInterface<DerivationFramework::IAugmentationTool>(this);
    declareProperty("EMExtrapolationTool",m_emExtrapolationTool);
    declareProperty("MinET",m_minET);
  }
  
  MergedElectronDetailsDecorator::~MergedElectronDetailsDecorator() {}
  
  
  StatusCode MergedElectronDetailsDecorator::initialize() {
    
    ATH_MSG_INFO("Initialize " );

    if (m_emExtrapolationTool.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool: " << m_emExtrapolationTool); 
      return StatusCode::FAILURE; 
    } 
    ATH_MSG_INFO("Retrieved tool: " << m_emExtrapolationTool);

    return StatusCode::SUCCESS;
  }
  
  StatusCode MergedElectronDetailsDecorator::finalize(){
    return StatusCode::SUCCESS;
  }
  
  StatusCode MergedElectronDetailsDecorator::addBranches() const{    

    // Retrieve the xAOD event info
    const xAOD::ElectronContainer *electrons = nullptr;
    ATH_CHECK( evtStore()->retrieve(electrons,"Electrons") );
    if( !electrons )
    {
      return StatusCode::FAILURE;
    }

    for( const auto& el : *electrons ){
      
      std::vector<float> trkMatchTrkP_dEta1(el->nTrackParticles(),-999);
      std::vector<float> trkMatchTrkP_dEta2(el->nTrackParticles(),-999);
      std::vector<float> trkMatchTrkP_dPhi1(el->nTrackParticles(),-999);
      std::vector<float> trkMatchTrkP_dPhi2(el->nTrackParticles(),-999);
      std::vector<float> trkMatchTrkLM_dEta1(el->nTrackParticles(),-999);
      std::vector<float> trkMatchTrkLM_dEta2(el->nTrackParticles(),-999);
      std::vector<float> trkMatchTrkLM_dPhi1(el->nTrackParticles(),-999);
      std::vector<float> trkMatchTrkLM_dPhi2(el->nTrackParticles(),-999);

      auto caloCluster =  el->caloCluster();

      if( caloCluster && caloCluster->pt() > m_minET ){ 
        std::vector<float> trkMatch(8,-999);
        for( unsigned int i(0); i < el->nTrackParticles(); ++i ){
          auto trackParticle = el->trackParticle( i );
          if(trackParticle){        
            fillMatchDetails( trkMatch, trackParticle, caloCluster);
            trkMatchTrkP_dEta1[i]  = trkMatch[0];
            trkMatchTrkP_dEta2[i]  = trkMatch[1];
            trkMatchTrkP_dPhi1[i]  = trkMatch[2];
            trkMatchTrkP_dPhi2[i]  = trkMatch[3];
            trkMatchTrkLM_dEta1[i] = trkMatch[4];
            trkMatchTrkLM_dEta2[i] = trkMatch[5];
            trkMatchTrkLM_dPhi1[i] = trkMatch[6];
            trkMatchTrkLM_dPhi2[i] = trkMatch[7];
          }
        }  
      }

      el->auxdecor<std::vector<float>>("TrackMatchingP_dEta1") = trkMatchTrkP_dEta1;
      el->auxdecor<std::vector<float>>("TrackMatchingP_dEta2") = trkMatchTrkP_dEta2;
      el->auxdecor<std::vector<float>>("TrackMatchingP_dPhi1") = trkMatchTrkP_dPhi1;
      el->auxdecor<std::vector<float>>("TrackMatchingP_dPhi2") = trkMatchTrkP_dPhi2;
      el->auxdecor<std::vector<float>>("TrackMatchingLM_dEta1") = trkMatchTrkLM_dEta1;
      el->auxdecor<std::vector<float>>("TrackMatchingLM_dEta2") = trkMatchTrkLM_dEta2;
      el->auxdecor<std::vector<float>>("TrackMatchingLM_dPhi1") = trkMatchTrkLM_dPhi1;
      el->auxdecor<std::vector<float>>("TrackMatchingLM_dPhi2") = trkMatchTrkLM_dPhi2;
   

      std::vector<float> subCluster_E;
      std::vector<float> subCluster_dEta;
      std::vector<float> subCluster_dPhi;

      static SG::AuxElement::Accessor<std::vector<ElementLink<xAOD::CaloClusterContainer> > > clusterLinksAcc("constituentClusterLinks");
      if(caloCluster && clusterLinksAcc.isAvailable(*caloCluster) ){
        std::vector<ElementLink<xAOD::CaloClusterContainer> >  clusterLinks = clusterLinksAcc(*caloCluster);
        for( auto link : clusterLinks){
          if( link.isValid() ){
            subCluster_E.push_back( (*link)->e() );
            subCluster_dEta.push_back( caloCluster->eta() - (*link)->eta() );
            float dphi =  caloCluster->phi() - (*link)->phi();
            while( dphi > TMath::Pi() )
              dphi -= TMath::Pi() * 2; 
            while( dphi < -TMath::Pi() )
              dphi += TMath::Pi() * 2;
            subCluster_dPhi.push_back(dphi);
          }
        }
      }
      el->auxdecor<std::vector<float>>("SubCluster_E") = subCluster_E;
      el->auxdecor<std::vector<float>>("SubCluster_dEta") = subCluster_dEta;
      el->auxdecor<std::vector<float>>("SubCluster_dPhi") = subCluster_dPhi;
    }

    return StatusCode::SUCCESS;

  } // addBranches
  
 
  void DerivationFramework::MergedElectronDetailsDecorator::fillMatchDetails( std::vector<float>& trkMatchTrk, 
                                                                              const xAOD::TrackParticle* tp, 
                                                                              const xAOD::CaloCluster* cluster) const {

    std::vector<double>  eta(4, -999.0);
    std::vector<double>  phi(4, -999.0);   
    std::vector<double>  deltaEta(4, -999.0);
    std::vector<double>  deltaPhi(4, -999.0);   
  
    uint8_t dummy(-1);
    int nPix = tp->summaryValue( dummy, xAOD::numberOfPixelHits )? dummy :-1;
    int nPix_DS = tp->summaryValue( dummy, xAOD::numberOfPixelDeadSensors )? dummy :-1;
    int nSCT = tp->summaryValue( dummy, xAOD::numberOfSCTHits )? dummy :-1; 
    int nSCT_DS = tp->summaryValue( dummy, xAOD::numberOfSCTDeadSensors )? dummy :-1;

    bool isTRT = nPix + nPix_DS + nSCT + nSCT_DS < 7 ? true : false;
   
    if(isTRT) 
      return; 

    if(m_emExtrapolationTool->matchesAtCalo (cluster, 
                                           tp, 
                                           isTRT,
                                           Trk::alongMomentum, 
                                           eta,
                                           phi,
                                           deltaEta, 
                                           deltaPhi, 
                                           IEMExtrapolationTools::fromPerigee)) // Perigee
    {
      trkMatchTrk[0] = deltaEta[1];   
      trkMatchTrk[1] = deltaEta[2];   
      trkMatchTrk[2] = deltaPhi[1];   
      trkMatchTrk[3] = deltaPhi[2];   
    }
    
    if(m_emExtrapolationTool->matchesAtCalo (cluster, 
                                           tp, 
                                           isTRT,
                                           Trk::alongMomentum, 
                                           eta,
                                           phi,
                                           deltaEta, 
                                           deltaPhi, 
                                           IEMExtrapolationTools::fromLastMeasurement)) //Last Measurement
    {
      trkMatchTrk[4] = deltaEta[1];   
      trkMatchTrk[5] = deltaEta[2];   
      trkMatchTrk[6] = deltaPhi[1];   
      trkMatchTrk[7] = deltaPhi[2];   
    }

  }

} // namespace

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


/** 
 * NAME : 	EFexEratioRateAlgo.cxx
 * PACKAGE : 	Trigger/TrigL1Upgrade/TrigL1CaloUpgradeAnalysis/EFexEratioRateAlgo
 *
 * AUTHOR : 	Denis Oliveira Damazio
 *
 *
 * **/

#include "EFexEratioRateAlgo.h"
#include <math.h>
#include <string>
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "xAODTrigCalo/TrigEMClusterContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTrigger/EmTauRoIContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "TrigL1CaloUpgrade/BCID.h"

EFexEratioRateAlgo::EFexEratioRateAlgo( const std::string& name, ISvcLocator* pSvcLocator ) : AthAlgorithm (name, pSvcLocator){
       declareProperty("EnableMonitoring", m_enableMon=true);
       declareProperty("DoTruth", m_doTruth=true);
       declareProperty("DoOffline", m_doOffline=false);
       declareProperty("InputClusterName", m_inputClusterName = "SCluster" );
       declareProperty("InputEratioName", m_inputEratioName = "LArLayer1Vars" );
       declareProperty("InputLvl1Name", m_inputLvl1Name = "LVL1EmTauRoIs" );
       declareProperty("eFexEtThreshold",m_EEtThr = 20e3 );
       declareProperty("rEtaThreshold",m_rEtaThr = 0.745 );
       declareProperty("f3Threshold",m_f3Thr = 1.0 );
       declareProperty("rEratioThreshold",m_EratioThr = 0.41 );
}

EFexEratioRateAlgo::~EFexEratioRateAlgo(){}

StatusCode EFexEratioRateAlgo::initialize(){
	
        MsgStream msg(msgSvc(), name());
	msg << MSG::DEBUG << "initializing EFexEratioRateAlgo" << endreq;
        if ( m_enableMon ){
		std::string histoName(name());
		histoName+="Algo.root";
		m_histFile = new TFile(histoName.c_str(),"RECREATE");
		m_EtSElectron = new TH1F("EtSElectron","Et of Super Cell based Electron",60,0,60);
		m_numpt1 = new TH1F("numpt1","numpt1",100,0,100);
		m_numpt2 = new TH1F("numpt2","numpt2",100,0,100);
		m_numpt2f3 = new TH1F("numpt2f3","numpt2f3",100,0,100);
		m_numpt3 = new TH1F("numpt3","numpt3",100,0,100);
		m_numpt4 = new TH1F("numpt4","numpt4",100,0,100);
		m_denpt = new TH1F("denpt","denpt",100,0,100);
		m_numeta1 = new TH1F("numeta1","numeta1",100,-2.5,2.5);
		m_numeta2 = new TH1F("numeta2","numeta2",100,-2.5,2.5);
		m_numeta2f3 = new TH1F("numeta2f3","numeta2f3",100,-2.5,2.5);
		m_numeta3 = new TH1F("numeta3","numeta3",100,-2.5,2.5);
		m_numeta4 = new TH1F("numeta4","numeta4",100,-2.5,2.5);
		m_deneta = new TH1F("deneta","deneta"   ,100,-2.5,2.5);
		m_numphi1 = new TH1F("numphi1","numphi1",128,-M_PI,M_PI);
		m_numphi2 = new TH1F("numphi2","numphi2",128,-M_PI,M_PI);
		m_numphi2f3 = new TH1F("numphi2if3","numphi2f3",128,-M_PI,M_PI);
		m_numphi3 = new TH1F("numphi3","numphi3",128,-M_PI,M_PI);
		m_numphi4 = new TH1F("numphi4","numphi4",128,-M_PI,M_PI);
		m_denphi = new TH1F("denphi","denphi"    ,128,-M_PI,M_PI);
		m_trackPt = new TH1F("trackPt","trackPt",160,0,80);
		m_trackEta = new TH1F("trackEta","trackEta",100,-2.5,2.5);
		m_trackBPt = new TH1F("trackBPt","trackBPt",160,0,80);
		m_trackBEta = new TH1F("trackBEta","trackBEta",100,-2.5,2.5);

	}
	return StatusCode::SUCCESS;
}

StatusCode EFexEratioRateAlgo::finalize(){
        MsgStream msg(msgSvc(), name());
	msg << MSG::DEBUG << "finalizing EFexEratio" << endreq;
        if ( m_enableMon ){
		m_numpt1->Write();
		m_numpt2->Write();
		m_numpt3->Write();
		m_numpt4->Write();
		m_denpt->Write();
		m_numeta1->Write();
		m_numeta2->Write();
		m_numeta2f3->Write();
		m_numeta3->Write();
		m_numeta4->Write();
		m_deneta->Write();
		m_numphi1->Write();
		m_numphi2->Write();
		m_numphi2f3->Write();
		m_numphi3->Write();
		m_numphi4->Write();
		m_denphi->Write();
		m_trackPt->Write();
		m_trackEta->Write();
		m_trackBPt->Write();
		m_trackBEta->Write();
		m_histFile->Write();
		m_histFile->Close();
	}
	return StatusCode::SUCCESS;
}

StatusCode EFexEratioRateAlgo::execute(){
	
        MsgStream msg(msgSvc(), name());
	msg << MSG::DEBUG << "execute TrigT1CaloEFex" << endreq;
        const xAOD::EventInfo* evt(0);
        if ( evtStore()->retrieve(evt,"EventInfo").isFailure() ){
                msg << MSG::WARNING << "did not find EventInfo container" << endreq;
        }
/*      Future usage to come soon
        long bunch_crossing(-1);
        long bunch_crossingNor(-1); // future usage
        if ( evt ) {
           bunch_crossing = evt->bcid();
           bunch_crossingNor = bcids_from_start ( bunch_crossing );
        }
*/
        const xAOD::TrigEMClusterContainer* scluster(nullptr);
	if ( evtStore()->retrieve(scluster,m_inputClusterName).isFailure() ){
		msg << MSG::WARNING << "did not find super cluster container" << endreq;
		return StatusCode::SUCCESS;
	}
        const xAOD::TrigEMClusterContainer* lASP(nullptr);
	if ( evtStore()->retrieve(lASP,m_inputEratioName).isFailure() ){
		msg << MSG::WARNING << "did not find Eratio cluster container" << endreq;
		return StatusCode::SUCCESS;
	}
        const xAOD::EmTauRoIContainer* lvl1(nullptr);
	if ( evtStore()->retrieve(lvl1,m_inputLvl1Name).isFailure() ){
		msg << MSG::WARNING << "did not find old l1 container" << endreq;
		//return StatusCode::SUCCESS;
	}

/*      Future usage to come soon
	const xAOD::VertexContainer* nvtx(NULL);
        if ( evtStore()->retrieve(nvtx,"PrimaryVertices").isFailure() ) {
                msg << MSG::WARNING << "did not find Vectices container" << endreq;
                return StatusCode::SUCCESS;
        }
*/

	const xAOD::TruthParticleContainer* truth;
	if ( evtStore()->retrieve(truth,"TruthParticles").isFailure() ) {
		msg << MSG::WARNING << "did not find truth particle container" << endreq;
		return StatusCode::SUCCESS;
	}

        const xAOD::TrackParticleContainer* tracks;
        if ( evtStore()->retrieve(tracks,"InDetTrackParticles").isFailure() ) {
                msg << MSG::WARNING << "did not find track particle container" << endreq;
                return StatusCode::SUCCESS;
        }

	for( auto cl : *scluster ){
		float pt = cl->et()/1e3;
		float eta = cl->eta();
		float phi = cl->phi();
		m_denpt->Fill( pt );
		if ( cl->et() < m_EEtThr ) continue;
		m_numpt1->Fill( pt );
		if ( pt > 20 ) {
		    m_numeta1->Fill( eta );
		    m_numphi1->Fill( phi );
		}
		float rEta=-999.0;
		float eRatio=-999.0;
		float f3=999.0;
		if ( cl->e277() > 0.1 ) 
		    rEta= cl->e237() / cl->e277();
		if ( rEta < m_rEtaThr ) continue;
		m_numpt2->Fill( pt );
		if ( pt > 20 ) {
		    m_numeta2->Fill( eta );
		    m_numphi2->Fill( phi );
		}
                if ( cl->et() > 1 )
                   f3 = (cl->energy(CaloSampling::EMB3) + cl->energy(CaloSampling::EME3) ) / cl->et() ;
                if ( f3 > m_f3Thr ) continue;
                m_numpt2f3->Fill( pt );
                if ( pt > 20 ) {
                    m_numeta2f3->Fill( eta );
                    m_numphi2f3->Fill( phi );
                }

		for( auto er : *lASP ) {
		    if ( fabsf( er->eta() - cl->eta() ) > 0.035 ) continue;
		    if ( deltaPhi( er->phi(), cl->phi() ) > 0.1 ) continue;
		    if ( er->emaxs1() + er->e2tsts1() > 0.1 ) 
			eRatio = ( er->emaxs1() - er->e2tsts1() ) / (er->emaxs1() + er->e2tsts1() );
		}
		for(auto track : *tracks ){
                    if ( track->pt() < 2e3 ) continue;
                    if ( fabsf( track->eta() - cl->eta() ) > 0.02 ) continue;
                    if ( deltaPhi( track->phi(), cl->phi() ) > 0.03 ) continue;
                    m_trackBPt->Fill( track->pt() / 1e3 );
                    m_trackBEta->Fill(track->eta() );
                }
		if ( eRatio < m_EratioThr ) continue;
		m_numpt3->Fill( pt );
		if ( pt > 20 ) {
		    m_numeta3->Fill( eta );
		    m_numphi3->Fill( phi );
		}
		for(auto track : *tracks ){
                    if ( track->pt() < 2e3 ) continue;
                    if ( fabsf( track->eta() - cl->eta() ) > 0.02 ) continue;
                    if ( deltaPhi( track->phi(), cl->phi() ) > 0.03 ) continue;
                    m_trackPt->Fill( track->pt() / 1e3 );
                    m_trackEta->Fill(track->eta() );
                }
		
		//float resolution = 100.0*(tt->pt() - cl->et() ) / tt->pt();
	}
		

	return StatusCode::SUCCESS;
}

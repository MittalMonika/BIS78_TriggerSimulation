/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


/** 
 * NAME : 	DumpAll.cxx
 * PACKAGE : 	Trigger/TrigL1Upgrade/TrigL1CaloUpgradeAnalysis/DumpAll
 *
 * AUTHOR : 	Denis Oliveira Damazio
 *
 *
 * **/

#include "DumpAll.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include <xAODCaloEvent/CaloClusterAuxContainer.h>
#include "xAODEgamma/ElectronAuxContainer.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArCabling/LArSuperCellCablingTool.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTrigCalo/TrigEMClusterContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTrigger/EmTauRoIContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "CaloInterface/ICalorimeterNoiseTool.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH1I.h"
#include "TTree.h"
#include "TNtuple.h"
#include <math.h>
#include "TrigL1CaloUpgrade/BCID.h"
#include "RecoToolInterfaces/ICaloTopoClusterIsolationTool.h"

DumpAll::DumpAll( const std::string& name, ISvcLocator* pSvcLocator ) : AthAlgorithm (name, pSvcLocator), m_cabling("LArSuperCellCablingTool")  {
       declareProperty("InputClusterName", m_inputClusterName = "SCluster" );
       declareProperty("InputLArFexName", m_inputLArName = "LArLayer1Vars" );
       declareProperty("InputLvl1Name", m_inputLvl1Name = "LVL1EmTauRoIs" );
       declareProperty("SaveLayer1Cells", m_saveLa1Cells = false );
       declareProperty("CaloNoiseTool",m_noiseTool,"Tool Handle for noise tool");
       //declareProperty("CaloTopoIsolTool", m_isolTopoTool,"Tool Handle for topoIsol tool");
       declareProperty("CaloTopoIsolationTool",           m_topoIsolationTool,    "Handle of the calo topo IsolationTool");
       declareProperty("EtInSigma", m_etInSigma = -1.0, "sigma cut");
       declareProperty("Et", m_et = 100.0, "et cut");
}

DumpAll::~DumpAll(){}

StatusCode DumpAll::initialize(){
	
        MsgStream msg(msgSvc(), name());
	msg << MSG::DEBUG << "initializing DumpAll" << endreq;
	if ( (m_etInSigma > 0 ) && ( m_et > 0 ) )
	msg << MSG::WARNING << "Configuration issue" << endreq;
	if ( m_etInSigma > 0 ) m_noiseTool.retrieve().ignore();
        //m_isolTopoTool.retrieve().ignore();
        m_counter = 0;
	std::string filename=name();
	filename+=".DumpAll.root";
	m_file = new TFile (filename.c_str(),"RECREATE");
	m_file->mkdir("DumpAll");
	m_file->cd("DumpAll");
	m_evt = (TTree*)new TNtuple("Evt","Evt","count:event_number:bunch_crossing:bunch_crossingNor:nVtx");
	//m_truth = (TTree*)new TNtuple("Truth","Truth","count:truth_pt:truth_eta:truth_phi:truth_pdg:truth_barcode");
	m_truth = (TTree*)new TTree("Truth","Truth");
	m_truth->Branch("truth_pt",    &m_truth_pt);
	m_truth->Branch("truth_eta",    &m_truth_eta);
	m_truth->Branch("truth_phi",    &m_truth_phi);
	m_truth->Branch("truth_pdg",    &m_truth_pdg);
	m_truth->Branch("truth_pdgM",    &m_truth_pdgM);
	m_truth->Branch("truth_barcode",    &m_truth_barcode);
	m_truth->Branch("truth_charge",    &m_truth_charge);
	m_truth->Branch("truth_z",    &m_truth_z);

	m_l1 = (TTree*)new TTree("L1","L1"); //,"count:l1_et:l1_eta:l1_phi:l1_emclus:l1_hadcore:l1_emisol:l1_hadisol");
	m_l1->Branch("l1_et",&m_l1_et);
	m_l1->Branch("l1_eta",&m_l1_eta);
	m_l1->Branch("l1_phi",&m_l1_phi);
	m_l1->Branch("l1_emclus",&m_l1_emclus);
	m_l1->Branch("l1_hadcore",&m_l1_hadcore);
	m_l1->Branch("l1_hadisol",&m_l1_hadisol);
	m_l1->Branch("l1_emisol",&m_l1_emisol);


	m_selectron = new TTree("SCluster","SCluster");
	m_selectron->Branch("selectron_et"    ,&m_selectron_et);
	m_selectron->Branch("selectron_eta"    ,&m_selectron_eta);
	m_selectron->Branch("selectron_phi"    ,&m_selectron_phi);
	m_selectron->Branch("selectron_energy"    ,&m_selectron_energy);
	m_selectron->Branch("selectron_e237"    ,&m_selectron_e237);
	m_selectron->Branch("selectron_e277"    ,&m_selectron_e277);
	m_selectron->Branch("selectron_ehad1"    ,&m_selectron_ehad1);
	m_selectron->Branch("selectron_e0"    ,&m_selectron_e0);
	m_selectron->Branch("selectron_e1"    ,&m_selectron_e1);
	m_selectron->Branch("selectron_e2"    ,&m_selectron_e2);
	m_selectron->Branch("selectron_e3"    ,&m_selectron_e3);
	m_selectron->Branch("selectron_wstot"    ,&m_selectron_wstot);
	m_selectron->Branch("selectron_time"    ,&m_selectron_time);
	m_selectron->Branch("selectron_isolcell"   ,&m_selectron_isolcell);
	m_selectron->Branch("selectron_isolscell"  ,&m_selectron_isolscell);
	m_selectron->Branch("selectron_e233"   ,&m_selectron_e233);
	m_selectron->Branch("selectron_emaxs1"  ,&m_selectron_emaxs1);
	m_selectron->Branch("selectron_isoltopo20"  ,&m_selectron_isoltopo20);
	m_selectron->Branch("selectron_isoltopo30"  ,&m_selectron_isoltopo30);
	m_selectron->Branch("selectron_isoltopo40"  ,&m_selectron_isoltopo40);

        m_selectronLAr = new TTree("SClusterLAr","SClusterLAr");
        m_selectronLAr->Branch("selectronLAr_eta"    ,&m_selectronLAr_eta);
        m_selectronLAr->Branch("selectronLAr_phi"    ,&m_selectronLAr_phi);
        m_selectronLAr->Branch("selectronLAr_Eratio"    ,&m_selectronLAr_Eratio);
        m_selectronLAr->Branch("selectronLAr_fracs1"    ,&m_selectronLAr_fracs1);
        m_selectronLAr->Branch("selectronLAr_wstot"    ,&m_selectronLAr_wstot);

	m_offelectron = new TTree("OffEl","OffEl");
	m_offelectron->Branch("offel_et", &m_offel_et);
	m_offelectron->Branch("offel_eta", &m_offel_eta);
	m_offelectron->Branch("offel_phi", &m_offel_phi);
	m_offelectron->Branch("offel_cleta", &m_offel_cleta);
	m_offelectron->Branch("offel_clphi", &m_offel_clphi);
	m_offelectron->Branch("offel_reta", &m_offel_reta);
	m_offelectron->Branch("offel_f1", &m_offel_f1);
	m_offelectron->Branch("offel_f3", &m_offel_f3);
	m_offelectron->Branch("offel_had", &m_offel_had);
	m_offelectron->Branch("offel_istight", &m_offel_istight);
	m_offelectron->Branch("offel_ismedium", &m_offel_ismedium);
	m_offelectron->Branch("offel_isloose", &m_offel_isloose);

	m_cell_infront = new TTree("CellInFront","CellInFront");
	m_cell_infront->Branch("e",&m_cell_infront_e);
	m_cell_infront->Branch("eta",&m_cell_infront_eta);
	m_cell_infront->Branch("phi",&m_cell_infront_phi);
	m_cell_infront->Branch("layer",&m_cell_infront_layer);
	m_cell_infront->Branch("time",&m_cell_infront_time);
	m_cell_infront->Branch("quality",&m_cell_infront_quality);

	if ( m_saveLa1Cells ) {
	m_allcell_infront = new TTree("AllCellInFrontLayer","AllCellInFrontLayer");
	m_allcell_infront->Branch("idx",&m_allcell_infront_idx);
	m_allcell_infront->Branch("e",&m_allcell_infront_e);
	m_allcell_infront->Branch("eta",&m_allcell_infront_eta);
	m_allcell_infront->Branch("phi",&m_allcell_infront_phi);
	m_allcell_infront->Branch("layer",&m_allcell_infront_layer);
	m_allcell_infront->Branch("time",&m_allcell_infront_time);
	m_allcell_infront->Branch("quality",&m_allcell_infront_quality);
	}

	m_file->cd("/");

	// for cell <-> SCell comparison
	if ( m_cabling.retrieve().isFailure() ){
		msg << MSG::ERROR << "cannot perform comparisons between SuperCells and digits" << endreq;
	}
	if ( m_topoIsolationTool.retrieve().isFailure() ){
		msg << MSG::ERROR << "no topIsolation possible" << endreq;
	}

	return StatusCode::SUCCESS;
}

StatusCode DumpAll::finalize(){
        MsgStream msg(msgSvc(), name());
	msg << MSG::DEBUG << "finalizing SimpleLArDigitsChecks" << endreq;
	m_file->Write();
	m_file->Close();
	return StatusCode::SUCCESS;
}

StatusCode DumpAll::execute(){
	
	struct timeval t1,t2;
	gettimeofday(&t1,NULL);
        MsgStream msg(msgSvc(), name());
	//msg << MSG::DEBUG << "execute DumpAll" << endreq;
        const xAOD::EventInfo* evt(0);
        if ( evtStore()->retrieve(evt,"EventInfo").isFailure() ){
                msg << MSG::WARNING << "did not find EventInfo container" << endreq;
        }
        long bunch_crossing(-1);
        long bunch_crossingNor(-1);
	long event_number(-1);
        if ( evt ) {
	   event_number = evt->eventNumber();
           bunch_crossing = evt->bcid();
           bunch_crossingNor = bcids_from_start ( bunch_crossing );
        }
        const CaloCellContainer* scells;
	if ( evtStore()->retrieve(scells,"SCell").isFailure() ){
		msg << MSG::WARNING << "did not find cell container" << endreq;
		return StatusCode::SUCCESS;
	}
/*
	if ( evtStore()->retrieve(allcalo,"LArDigitSCL1").isFailure() ){
		msg << MSG::WARNING << "did not find lardigit container for regular cells" << endreq;
		return StatusCode::SUCCESS;
	}
*/
	const xAOD::VertexContainer* nvtx(NULL);
	if ( evtStore()->retrieve(nvtx,"PrimaryVertices").isFailure() ) {
		msg << MSG::WARNING << "did not find Vectices container" << endreq;
		return StatusCode::SUCCESS;
	}
        const xAOD::TrigEMClusterContainer* scluster(nullptr);
        if ( evtStore()->retrieve(scluster,m_inputClusterName).isFailure() ){
                msg << MSG::WARNING << "did not find super cluster container" << endreq;
                return StatusCode::SUCCESS;
        }
        const xAOD::TrigEMClusterContainer* sclusterLAr(nullptr);
        if ( evtStore()->retrieve(sclusterLAr,m_inputLArName).isFailure() ){
                msg << MSG::WARNING << "did not find LArFex container" << endreq;
                //return StatusCode::SUCCESS;
        }
        const xAOD::EmTauRoIContainer* lvl1(nullptr);
        if ( evtStore()->retrieve(lvl1,m_inputLvl1Name).isFailure() ){
                msg << MSG::WARNING << "did not find old l1 container" << endreq;
                //return StatusCode::SUCCESS;
        }
	const xAOD::TruthParticleContainer* truth;
        if ( evtStore()->retrieve(truth,"TruthParticles").isFailure() ) {
                msg << MSG::WARNING << "did not find truth particle container" << endreq;
                return StatusCode::SUCCESS;
        }

	const xAOD::ElectronContainer* electrons;
        if ( evtStore()->retrieve(electrons,"Electrons").isFailure() ) {
                msg << MSG::WARNING << "did not find electron offline container" << endreq;
                return StatusCode::SUCCESS;
        }

	bool caloavail=true;
	const CaloCellContainer* allcalo;
        if ( evtStore()->retrieve(allcalo,"AllCalo").isFailure() ) {
                msg << MSG::WARNING << "did not find allcalo container" << endreq;
		caloavail=false;
        }

	const xAOD::CaloClusterContainer* topoClusters(0);
	if ( evtStore()->retrieve(topoClusters, "CaloCalTopoClusters").isFailure() ) {
                msg << MSG::WARNING << "did not find topo cluster container" << endreq;
	}

        // for(auto digit : *allcalo) {
        // m_selectron_et.clear();
        ResetAllBranches();

        for(auto scl : *sclusterLAr) {
           m_selectronLAr_eta.push_back(scl->eta());
           m_selectronLAr_phi.push_back(scl->phi());
	   float den = (scl->emaxs1() + scl->e2tsts1() );
	   if ( den > 0.1 ) m_selectronLAr_Eratio.push_back((scl->emaxs1() - scl->e2tsts1() )/den );
	   else m_selectronLAr_Eratio.push_back( 999.0 );
           m_selectronLAr_fracs1.push_back(scl->fracs1());
           m_selectronLAr_wstot.push_back(scl->wstot());
	}
	m_selectronLAr->Fill();

        for(auto scl : *scluster) {
	   m_selectron_et.push_back(scl->et());
	   m_selectron_eta.push_back(scl->eta());
	   m_selectron_phi.push_back(scl->phi());
	   m_selectron_energy.push_back(scl->energy());
	   m_selectron_e237.push_back(scl->e237());
	   m_selectron_e277.push_back(scl->e277());
	   m_selectron_ehad1.push_back(scl->ehad1());
	   m_selectron_e0.push_back(scl->energy(CaloSampling::PreSamplerB)+scl->energy(CaloSampling::PreSamplerE));
	   m_selectron_e1.push_back(scl->energy(CaloSampling::EMB1) + scl->energy(CaloSampling::EME1) );
	   m_selectron_e2.push_back(scl->energy(CaloSampling::EMB2) + scl->energy(CaloSampling::EME2) );
	   m_selectron_e3.push_back(scl->energy(CaloSampling::EMB3) + scl->energy(CaloSampling::EME3) );
	   m_selectron_wstot.push_back(scl->wstot() );
	   m_selectron_time.push_back(scl->e233() );
	   m_selectron_emaxs1.push_back(scl->emaxs1() );
	   if ( caloavail ) {
		float etaCl = scl->eta();
		float phiCl = scl->phi();
		std::vector< float > e; e.reserve(200);
		std::vector< float > eta; eta.reserve(200);
		std::vector< float > phi; phi.reserve(200);
		std::vector< float > layer; layer.reserve(200);
		std::vector< float > time; time.reserve(200);
		std::vector< float > quality; quality.reserve(200);
		for(auto c : *allcalo) {
			if ( fabsf( etaCl-c->eta() ) > 0.14 ) continue;
			float dphi = fabsf( phiCl-c->phi() );
			dphi = fabsf(M_PI - dphi);
			dphi = fabsf(M_PI - dphi);
			if ( dphi > 0.15 ) continue;
			int l = c->caloDDE()->getSampling();
			if ( (l!=1) && (l!=5) ) continue;
			e.push_back ( c->energy() );
			eta.push_back ( c->eta() );
			phi.push_back ( c->phi() );
			layer.push_back ( l );
			time.push_back ( c->time() );
			quality.push_back ( c->quality() );
		}
		m_cell_infront_e.push_back(e);
		m_cell_infront_eta.push_back(eta);
		m_cell_infront_phi.push_back(phi);
		m_cell_infront_layer.push_back(layer);
		m_cell_infront_time.push_back(time);
		m_cell_infront_quality.push_back(quality);
	   }
	   // try to calculate isolation
	   float etaCl = scl->eta();
	   float phiCl = scl->phi();
	   float isolcell = 0.0;
	   float isolscell = 0.0;
	   //std::cout << "eFex Cluster : " << scl->et() << " " << etaCl << " " << phiCl << std::endl;
	   for(auto c : *allcalo) {
		float deta = fabsf( etaCl-c->eta() );
		if ( deta > 0.14 ) continue;
                float dphi = fabsf( phiCl-c->phi() );
                dphi = fabsf(M_PI - dphi);
                dphi = fabsf(M_PI - dphi);
                if ( dphi > 0.2 ) continue;
		if ( deta < 0.05 ) continue;
		if ( dphi < 0.15 ) continue;
		isolcell+=c->et();
		//std::cout << "isol cell " << c->et() << " " << c->eta() << " " << c->phi() << std::endl;
	   } // end of isolation
	   //std::cout << std::endl;
           for(auto c : *scells) {
                float deta = fabsf( etaCl-c->eta() );
                if ( deta > 0.14 ) continue;
                float dphi = fabsf( phiCl-c->phi() );
                dphi = fabsf(M_PI - dphi);
                dphi = fabsf(M_PI - dphi);
                if ( dphi > 0.2 ) continue;
                if ( deta < 0.05 ) continue;
                if ( dphi < 0.15 ) continue;
		isolscell+=c->et();
                //std::cout << "isol scell " << c->et() << " " << c->eta() << " " << c->phi() << std::endl;
           } // end of isolation
	   m_selectron_isolcell.push_back(isolcell);
	   m_selectron_isolscell.push_back(isolscell);
	   xAOD::Electron elFI;
	   elFI.makePrivateStore();
	   elFI.setAuthor(xAOD::EgammaParameters::AuthorElectron);
	   xAOD::CaloClusterContainer * clusters = new xAOD::CaloClusterContainer();
	   xAOD::CaloClusterAuxContainer * clustersAux = new xAOD::CaloClusterAuxContainer();
	   clusters->setStore(clustersAux);
	   xAOD::CaloCluster *cl = new xAOD::CaloCluster();
	   clusters->push_back( cl );
	   cl->setE(scl->et());
	   cl->setEta(etaCl);
	   cl->setPhi(phiCl);
	   std::vector< ElementLink< xAOD::CaloClusterContainer > > links_clusters;
	   links_clusters.push_back(ElementLink< xAOD::CaloClusterContainer >( cl, *clusters ));
	   
	   elFI.setP4(scl->et(), etaCl, phiCl, 0.51);
	   elFI.setCaloClusterLinks(links_clusters);
	   xAOD::CaloCorrection corrlist;
	   corrlist.calobitset.set(static_cast<unsigned int>(xAOD::Iso::core57cells));
	   corrlist.calobitset.set(static_cast<unsigned int>(xAOD::Iso::ptCorrection));
	   std::vector<xAOD::Iso::IsolationType> isoTypes;
	   isoTypes.push_back( xAOD::Iso::topoetcone40 );
	   isoTypes.push_back( xAOD::Iso::topoetcone30 );
	   isoTypes.push_back( xAOD::Iso::topoetcone20 );
	   if ( topoClusters ) m_topoIsolationTool->decorateParticle_topoClusterIso(elFI, isoTypes, corrlist, topoClusters);
	   m_selectron_isoltopo20.push_back( elFI.isolationValue(xAOD::Iso::topoetcone20) );
	   m_selectron_isoltopo30.push_back( elFI.isolationValue(xAOD::Iso::topoetcone30) );
	   m_selectron_isoltopo40.push_back( elFI.isolationValue(xAOD::Iso::topoetcone40) );
	   //delete cl;
	   delete clusters;
	   delete clustersAux;
	}

	if ( caloavail && m_saveLa1Cells ) {
		std::vector< float > all_idx;
		std::vector< float > all_e;
		std::vector< float > all_eta;
		std::vector< float > all_phi;
		std::vector< float > all_layer;
		std::vector< float > all_time;
		std::vector< float > all_quality;
		int index = 0;
                for(auto c : *allcalo) {
                        int l = c->caloDDE()->getSampling();
                        if ( (l!=1) && (l!=5) ) continue;
			float noiseToCompare = m_et;
			if ( m_etInSigma >= 0.0 ) 
			  noiseToCompare = m_etInSigma*m_noiseTool->getNoise(&(*c),ICalorimeterNoiseTool::TOTALNOISE);
                        if ( c->energy() > noiseToCompare ) { // found a seed
                           float seedEta=c->eta();
                           float absSeedEta = TMath::Abs( seedEta );
                           float seedPhi=c->phi();
                           for(auto d : *allcalo) {
                                float limit = 0.066;
                                if ( absSeedEta > 1.7 ) limit = 0.088;
                                if ( absSeedEta > 1.9 ) limit = 0.132;
                                if ( fabsf( seedEta-d->eta() ) > limit ) continue;
                                float dphi = fabsf( seedPhi-d->phi() );
                                dphi = fabsf(M_PI - dphi);
                                dphi = fabsf(M_PI - dphi);
                                if ( dphi > 0.12 ) continue;
                                int k = d->caloDDE()->getSampling();
                                if ( (k!=1) && (k!=5) ) continue;
                                all_idx.push_back( index );
                                all_e.push_back ( d->energy() );
                                all_eta.push_back ( d->eta() );
                                all_phi.push_back ( d->phi() );
                                all_layer.push_back ( k );
                                all_time.push_back ( d->time() );
                                all_quality.push_back ( d->quality() );
                           }
                           index++;
                        }
		}
		if ( m_saveLa1Cells ){
		m_allcell_infront_idx.push_back(all_idx);
		m_allcell_infront_e.push_back(all_e);
		m_allcell_infront_eta.push_back(all_eta);
		m_allcell_infront_phi.push_back(all_phi);
		m_allcell_infront_layer.push_back(all_layer);
		m_allcell_infront_time.push_back(all_time);
		m_allcell_infront_quality.push_back(all_quality);
		}
	}

	if ( caloavail ) {
		if ( m_saveLa1Cells ) m_allcell_infront->Fill();
		m_cell_infront->Fill();
	}
	m_selectron->Fill();
        int nvtxs=-1;
	if ( nvtx != NULL) nvtxs = nvtx->size();
	((TNtuple*)m_evt)->Fill(m_counter,event_number,bunch_crossing,bunch_crossingNor,nvtxs);

	for( auto tt : *truth ){
                if ( fabsf(tt->absPdgId()) != 11 ) continue;
		if ( tt->status() != 1 ) continue;
                if ( tt->barcode() >= 10000 ) continue;
		m_truth_pt.push_back( tt->pt() );
		m_truth_eta.push_back( tt->eta() );
		m_truth_phi.push_back( tt->phi() );
		m_truth_pdg.push_back( tt->pdgId() );
		if ( tt->parent() != NULL )
		m_truth_pdgM.push_back( tt->parent()->pdgId() );
		else 
		m_truth_pdgM.push_back( -999999.0 );
		m_truth_barcode.push_back( tt->barcode() );
		m_truth_charge.push_back( tt->charge() );
		const ElementLink<xAOD::TruthVertexContainer>& v = tt->prodVtxLink();
		const xAOD::TruthVertex* vtx;
		if ( v.isValid() ) {
			vtx = *v;
			m_truth_z.push_back( vtx->z() );
		} else m_truth_z.push_back( -9999.0 );
	}
	m_truth->Fill();

	if ( lvl1 ) {
                for(auto l1 : *lvl1 ){
		//((TNtuple*)m_l1)->Fill(m_counter,l1->eT(),l1->eta(),l1->phi(), l1->emClus(), l1->hadCore(), l1->emIsol(), l1->hadIsol() );
		m_l1_et.push_back( l1->eT() );
		m_l1_eta.push_back( l1->eta() );
		m_l1_phi.push_back( l1->phi() );
		m_l1_emclus.push_back( l1->emClus() );
		m_l1_hadcore.push_back( l1->hadCore() );
		m_l1_emisol.push_back( l1->emIsol() );
		m_l1_hadisol.push_back( l1->hadIsol() );
		}
	m_l1->Fill();
        }
	for( auto el : *electrons ){
		if ( el->pt() < 1e3 ) continue; // dont care about too low energy things
		m_offel_et.push_back(el->pt() );
		m_offel_eta.push_back(el->eta() );
		m_offel_phi.push_back(el->phi() );
		m_offel_cleta.push_back(el->caloCluster()->eta() );
		m_offel_clphi.push_back(el->caloCluster()->phi() );
		m_offel_reta.push_back(el->auxdata<float>("Reta") );
		m_offel_f1.push_back(el->auxdata<float>("f1") );
		m_offel_f3.push_back(el->auxdata<float>("f3") );
		m_offel_had.push_back( 0.0 ); // not yet there
		m_offel_istight.push_back( (el->passSelection( xAOD::EgammaParameters::LHTight ) ? 1 : 0 ) );
		m_offel_ismedium.push_back( (el->passSelection( xAOD::EgammaParameters::LHMedium ) ? 1 : 0 ) );
		m_offel_isloose.push_back( (el->passSelection( xAOD::EgammaParameters::LHLoose ) ? 1 : 0 ) );
	} // end of electron
	m_offelectron->Fill();


	m_counter++;
	gettimeofday(&t2,NULL);
	
	return StatusCode::SUCCESS;
}

void DumpAll::ResetAllBranches(){
        m_selectron_et.clear();
        m_selectron_eta.clear();
        m_selectron_phi.clear();
        m_selectron_energy.clear();
        m_selectron_e237.clear();
        m_selectron_e277.clear();
        m_selectron_ehad1.clear();
        m_selectron_e0.clear();
        m_selectron_e1.clear();
        m_selectron_e2.clear();
        m_selectron_e3.clear();
	m_selectron_wstot.clear();
	m_selectron_time.clear();
	m_selectron_isolcell.clear();
	m_selectron_isolscell.clear();
	m_selectron_e233.clear();
	m_selectron_emaxs1.clear();
	m_selectron_isoltopo20.clear();
	m_selectron_isoltopo30.clear();
	m_selectron_isoltopo40.clear();

	m_selectronLAr_eta.clear();
	m_selectronLAr_phi.clear();
	m_selectronLAr_Eratio.clear();
	m_selectronLAr_fracs1.clear();
	m_selectronLAr_wstot.clear();

        m_truth_pt.clear();
        m_truth_eta.clear();
        m_truth_phi.clear();
        m_truth_pdg.clear();
        m_truth_pdgM.clear();
        m_truth_barcode.clear();
        m_truth_charge.clear();
        m_truth_z.clear();

        m_l1_et.clear();
        m_l1_eta.clear();
        m_l1_phi.clear();
        m_l1_emclus.clear();
        m_l1_hadcore.clear();
        m_l1_hadisol.clear();
        m_l1_emisol.clear();

        m_offel_et.clear();
        m_offel_eta.clear();
        m_offel_phi.clear();
        m_offel_cleta.clear();
        m_offel_clphi.clear();
        m_offel_reta.clear();
        m_offel_f1.clear();
        m_offel_f3.clear();
        m_offel_had.clear();
        m_offel_istight.clear();
        m_offel_ismedium.clear();
        m_offel_isloose.clear();

	m_cell_infront_e.clear();
	m_cell_infront_eta.clear();
	m_cell_infront_phi.clear();
	m_cell_infront_layer.clear();
	m_cell_infront_time.clear();
	m_cell_infront_quality.clear();

	if ( m_saveLa1Cells ) {
	m_allcell_infront_idx.clear();
	m_allcell_infront_e.clear();
	m_allcell_infront_eta.clear();
	m_allcell_infront_phi.clear();
	m_allcell_infront_layer.clear();
	m_allcell_infront_time.clear();
	m_allcell_infront_quality.clear();
	}

}


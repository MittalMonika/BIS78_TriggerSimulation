/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//macro to read in D3PD files and run some tools
//should get same results as running MissingETPerformance package over AOD/ESD/DPD
//to run it:
//root runD3PD_withSelection.C
//
#include <TFile.h>
#include <TChain.h>
#include <iostream>
#include <fstream>
using namespace std;

void runD3PD_MinBiastest() {
  //compile libraries and load them
  gSystem->Load("/afs/cern.ch/user/a/ayurkewi/scratch0/GRL/DataQuality/GoodRunsLists/i686-slc5-gcc43-opt/libGoodRunsListsLib.so");
  gROOT->LoadMacro("D3PD.h+");
  gROOT->LoadMacro("MissingETData.C+");
  gROOT->LoadMacro("BasicPlotsTool.C+");
  gROOT->LoadMacro("ResolutionTool.C+");
  gROOT->LoadMacro("MissingETEventSelector.C+");

  Root::TGoodRunsListReader reader;
  reader.AddXMLFile("GRLs/data10_7TeV.periodA.152166-153200_LBSUMM_DetStatus-v03-repro04-01_jetetmiss_7TeV.xml");
  reader.AddXMLFile("GRLs/data10_7TeV.periodB.153565-155160_LBSUMM_DetStatus-v03-repro04-01_jetetmiss_7TeV.xml");
  reader.AddXMLFile("GRLs/data10_7TeV.periodC.155228-156682_LBSUMM_DetStatus-v03-pass1-analysis-2010C_data-jetetmiss_7TeV.xml");
  reader.Interpret();
  Root::TGoodRunsList m_grl = reader.GetMergedGoodRunsList() ;

  bool grl(D3PD* filedata2,Root::TGoodRunsList &m_grl){ return  m_grl.HasRunLumiBlock(filedata2->RunNumber,filedata2->lbn); }

  fstream file_counters;
  file_counters.open("EventCounters_L1CaloData.txt", ios::out);

  fstream file4;
  file4.open("EventInfotails_L1CaloData.txt", ios::out);

  //input files
  //  TChain *chain = new TChain("JetMETD3PD");
  TChain *chain = new TChain("qcd");

  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run152878/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run152933/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run153134/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run153200/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run153565/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run153565/MERGED_1.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run154810/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run154813/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run154817/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run154817/MERGED_1.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155073/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155073/MERGED_1.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155073/MERGED_2.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155112/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155112/MERGED_1.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155112/MERGED_2.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155112/MERGED_3.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155112/MERGED_4.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155112/MERGED_5.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155112/MERGED_6.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155116/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155118/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155160/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155160/MERGED_1.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155160/MERGED_2.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155228/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155569/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155569/MERGED_1.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155634/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155634/MERGED_1.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155669/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155678/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155678/MERGED_1.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155697/MERGED.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155697/MERGED_1.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155697/MERGED_2.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155697/MERGED_3.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155697/MERGED_4.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155697/MERGED_5.root");
  chain->Add("/scratch2/ayurkewi/L1Calo_THINNED_files/merged/run155697/MERGED_6.root");

  //pass the input file chain to our data structure
  //The D3PD class is made from MakeClass, with only very small modifications
  //If we are using a 3PD with different variables, it should be simple to use MakeClass
  //on the new files and not have to change much

  D3PD *metFromD3PD = new D3PD(chain);
  //MissingETData class mimics MissingETPerformance/MissingETData class
  //to present (almost) the same interface
  //We use this intermediary data structure to avoid having to change much when we switch to
  //a different D3PD with different variables
  //The MissingET variables should be the same, and we just point to those in this class

  MissingETData data(metFromD3PD);
  //create the output file for our histograms
  TFile *out = new TFile("L1CaloData.root", "RECREATE");

  //create event selectors
  MissingETEventSelector METEvtSel;
  METEvtSel.initialize(metFromD3PD);

  //create our histogram-making tools
  //give them the output file pointer to create directories and histograms
  BasicPlotsTool basicTool;
  basicTool.m_iset=5001;
  basicTool.m_nbinp=150;
  basicTool.m_suptmi=150.;  
  basicTool.m_nbinE=100;
  basicTool.m_lowET=-5.;
  basicTool.m_nbinpxy=120;
  basicTool.m_suptmixy=60.;
  basicTool.m_nbinphi=63;
  basicTool.m_binphi=3.15;
  basicTool.m_nbinsig=500;
  basicTool.m_binsig=50.;
  basicTool.m_nbineta=50;
  basicTool.m_bineta=5.0;
  basicTool.m_folderName="nocut";
  basicTool.m_suET=500.+basicTool.m_lowET; 
  basicTool.initialize(out);

  //make copies
  BasicPlotsTool basicTool3(basicTool);
  BasicPlotsTool basicTool4(basicTool);
  BasicPlotsTool basicTool5(basicTool);
  BasicPlotsTool basicTool6(basicTool);

  basicTool3.m_folderName = "COLLCAND_VERTEX_CLEANING_CUTS"; 
  basicTool4.m_folderName = "COLLCAND_VERTEX_CLEANING_CUTS_NOJETS"; 
  basicTool5.m_folderName = "COLLCAND_VERTEX_CLEANING_CUTS_JETS"; 
  basicTool6.m_folderName = "COLLCAND_VERTEX_CLEANING_CUTS_JETS20"; 
 
  basicTool3.initialize(out);
  basicTool4.initialize(out);
  basicTool5.initialize(out);
  basicTool6.initialize(out);
  
  ResolutionTool resolutionTool;
  resolutionTool.m_numptt=100;
  resolutionTool.m_startpt=0.;
  resolutionTool.m_binpt=5.;
  resolutionTool.m_scares=30.;
  resolutionTool.m_nbinres=100;
  resolutionTool.m_curvasres=10.;
  resolutionTool.m_fitrange=3;
  resolutionTool.m_ns=2.;
  resolutionTool.m_percerr=0.15;
  resolutionTool.m_entries=100;
  resolutionTool.m_iset=5001;
  resolutionTool.m_nbinres1=100;
  resolutionTool.m_scares1=50.;
  resolutionTool.m_nbinres2=100;
  resolutionTool.m_scares2=500.;
  resolutionTool.m_nbinres3=200;
  resolutionTool.m_scares3=100.;
  resolutionTool.m_nbinphires1=315;
  resolutionTool.m_lowET=-5.;
  resolutionTool.m_suET=495.;
  resolutionTool.m_useTruth=0;
  resolutionTool.m_use_sumet=2;
  resolutionTool.m_folderName="nocut";
  resolutionTool.initialize(out);

  //make copies
  ResolutionTool resolutionTool3(resolutionTool);
  ResolutionTool resolutionTool4(resolutionTool);
  ResolutionTool resolutionTool5(resolutionTool);
  ResolutionTool resolutionTool6(resolutionTool);
  ResolutionTool resolutionTool_em(resolutionTool);

  resolutionTool3.m_folderName = "COLLCAND_VERTEX_CLEANING_CUTS"; 
  resolutionTool4.m_folderName = "COLLCAND_VERTEX_CLEANING_CUTS_NOJETS"; 
  resolutionTool5.m_folderName = "COLLCAND_VERTEX_CLEANING_CUTS_JETS"; 
  resolutionTool6.m_folderName = "COLLCAND_VERTEX_CLEANING_CUTS_JETS20"; 

  resolutionTool3.initialize(out);
  resolutionTool4.initialize(out);
  resolutionTool5.initialize(out);
  resolutionTool6.initialize(out);

  resolutionTool_em.m_use_sumet=3;
  resolutionTool_em.m_folderName = "SUMETEM";
  resolutionTool_em.initialize(out);

  //make copies
  ResolutionTool resolutionTool_em3(resolutionTool_em);
  ResolutionTool resolutionTool_em4(resolutionTool_em);
  ResolutionTool resolutionTool_em5(resolutionTool_em);
  ResolutionTool resolutionTool_em6(resolutionTool_em);

  resolutionTool_em3.m_folderName = "SUMETEM_COLLCAND_VERTEX_CLEANING_CUTS"; 
  resolutionTool_em4.m_folderName = "SUMETEM_COLLCAND_VERTEX_CLEANING_CUTS_NOJETS"; 
  resolutionTool_em5.m_folderName = "SUMETEM_COLLCAND_VERTEX_CLEANING_CUTS_JETS"; 
  resolutionTool_em6.m_folderName = "SUMETEM_COLLCAND_VERTEX_CLEANING_CUTS_JETS20"; 

  resolutionTool_em3.initialize(out);
  resolutionTool_em4.initialize(out); 
  resolutionTool_em5.initialize(out);
  resolutionTool_em6.initialize(out);

  //get number of entries in the input chain
  Long64_t nentries = chain->GetEntries();
  //nentries = 10000;
  cout << "Processing " << nentries << " events." << endl;

  chain->SetBranchStatus("*",0);
  chain->SetBranchStatus("MET*",1);

  chain->SetBranchStatus("MET_Muid*",0);
//   chain->SetBranchStatus("MET_Track*",0);
//   chain->SetBranchStatus("MET_RefGamma*",0);
//   chain->SetBranchStatus("MET_RefMuon*",0);
//   chain->SetBranchStatus("MET_RefTau*",0);
  chain->SetBranchStatus("MET_CellOutMiniJet*",0);
  chain->SetBranchStatus("MET_MuonBoyTrack*",0);
  chain->SetBranchStatus("MET_MuonBoySpectro*",0);
  chain->SetBranchStatus("MET_RefMuonTrack*",0);
  chain->SetBranchStatus("MET_CryoCone*",0);
  chain->SetBranchStatus("MET_Muon*",0);
  chain->SetBranchStatus("MET_Final*",0);
  chain->SetBranchStatus("MET_MuidTrack*",0);
  chain->SetBranchStatus("MET_MuidSpectro*",0);
  chain->SetBranchStatus("MET_MuonMuid*",0);
  chain->SetBranchStatus("MET_TopoObj*",0);
  chain->SetBranchStatus("MET_LocHadTopoObj*",0);
  chain->SetBranchStatus("MET_Base*",0);
  chain->SetBranchStatus("MET_Base0*",0);
  chain->SetBranchStatus("MET_Calib*",0);

  chain->SetBranchStatus("RunNumber",1);
  chain->SetBranchStatus("EventNumber",1);
  chain->SetBranchStatus("lbn",1);
  chain->SetBranchStatus("L1_J5",1);
  chain->SetBranchStatus("jet_AntiKt4H1TopoNew_n",1);
  chain->SetBranchStatus("jet_AntiKt4H1TopoNew_n90",1);
  chain->SetBranchStatus("jet_AntiKt4H1TopoNew_emscale_pt",1);
  chain->SetBranchStatus("jet_AntiKt4H1TopoNew_pt",1);
  chain->SetBranchStatus("jet_AntiKt4H1TopoNew_eta",1);
  chain->SetBranchStatus("jet_AntiKt4H1TopoNew_phi",1);
  chain->SetBranchStatus("jet_AntiKt4H1TopoNew_hecf",1);
  chain->SetBranchStatus("jet_AntiKt4H1TopoNew_emfrac",1);
  chain->SetBranchStatus("jet_AntiKt4H1TopoNew_timing",1);
  chain->SetBranchStatus("jet_AntiKt4H1TopoNew_quality",1);
  chain->SetBranchStatus("vxp_n",1);
  chain->SetBranchStatus("vxp_x",1);
  chain->SetBranchStatus("vxp_y",1);
  chain->SetBranchStatus("vxp_z",1);
  chain->SetBranchStatus("vxp_nTracks",1);
  chain->SetBranchStatus("vxp_type",1);
  chain->SetBranchStatus("collcand_passMBTSTime",1);
  chain->SetBranchStatus("collcand_passCaloTime",1);

  unsigned int i_GRL = 0;
  unsigned int i_COLLCAND = 0;
  unsigned int i_COLLCAND_CLEANING=0;
  unsigned int i_COLLCAND_CLEANING_NOJETS=0;
  unsigned int i_COLLCAND_CLEANING_ONEJET=0;
  unsigned int i_COLLCAND_CLEANING_ONEJETGT20=0;
  unsigned int num_outliers = 0;

  //loop over all entries
  for (Long64_t jentry=0; jentry < nentries; ++jentry) {
 
    //load the current event
    Long64_t ientry = metFromD3PD->LoadTree(jentry);
    if (ientry < 0) break;
    //chain->GetEntry(jentry);
    chain->GetEntry(jentry,0);

    //print out what event we are processing once in a while
    if (jentry%1000==0)  cout << "event " << jentry << endl;

    //call execute() for each of our tools to fill histograms 
    //nocuts
    basicTool.execute(&data);
    resolutionTool.execute(&data);
    resolutionTool_em.execute(&data);

    //"GRL"
    if  (grl(metFromD3PD,m_grl)){ 

      i_GRL++;
      
      //"COLLCAND"
      if ( METEvtSel.passesL1J5() && METEvtSel.passesVertex() && 
	   (METEvtSel.passesMBTSTime() || METEvtSel.passesCaloTime()) ) {

	i_COLLCAND++;
       
	// "COLLCAND_CLEANING
	if (METEvtSel.passesN90AndHECFracCut() && METEvtSel.passes_fbadQCutAndEmFracCut() 
	    && METEvtSel.passes_JetTime() && METEvtSel.passes_hecf_quality()) {

	  i_COLLCAND_CLEANING++;
      
	  basicTool3.execute(&data);
	  resolutionTool3.execute(&data);
	  resolutionTool_em3.execute(&data);
	  //printEventInfo(metFromD3PD, file4);

	  if (metFromD3PD-> MET_Topo_et > 40000 || metFromD3PD-> MET_LocHadTopo_et > 40000 || metFromD3PD-> MET_CorrTopo_et > 40000) {
	    num_outliers++;
	    file4 <<  "run number " << metFromD3PD->RunNumber << endl;
	    file4 <<  "event number " << metFromD3PD->EventNumber << endl;
	    file4 <<  "MET_Topo " << metFromD3PD->MET_Topo_et << endl;
	    file4 <<  "METx_Topo " << metFromD3PD->MET_Topo_etx << endl;
	    file4 <<  "METy_Topo " << metFromD3PD->MET_Topo_ety << endl;
	    file4 <<  "MET_LocHadTopo " << metFromD3PD->MET_LocHadTopo_et << endl;
	    file4 <<  "METx_LocHadTopo " << metFromD3PD->MET_LocHadTopo_etx << endl;
	    file4 <<  "METy_LocHadTopo " << metFromD3PD->MET_LocHadTopo_ety << endl;
	    file4 <<  "MET_CorrTopo " << metFromD3PD->MET_CorrTopo_et << endl;
	    file4 <<  "METx_CorrTopo " << metFromD3PD->MET_CorrTopo_etx << endl;
	    file4 <<  "METy_CorrTopo " << metFromD3PD->MET_CorrTopo_ety << endl;

	    file4 <<  "jet_AntiKt4H1TopoNew_n " << metFromD3PD->jet_AntiKt4H1TopoNew_n << endl;
	    for ( int i = 0; i < metFromD3PD->jet_AntiKt4H1TopoNew_n; i++ ) {
	      float HECfrac =metFromD3PD->jet_AntiKt4H1TopoNew_hecf->at(i);

	      file4 <<  "Jet pt (EM)" << metFromD3PD-> jet_AntiKt4H1TopoNew_emscale_pt->at(i) << endl;
	      file4 <<  "Jet eta " << metFromD3PD->jet_AntiKt4H1TopoNew_eta->at(i) << endl;
	      file4 <<  "Jet emfrac " << metFromD3PD-> jet_AntiKt4H1TopoNew_emfrac->at(i) << endl;
	      file4 <<  "Jet HECfrac " << HECfrac << endl;
	      file4 <<  "Jet n90 " << metFromD3PD-> jet_AntiKt4H1TopoNew_n90->at(i)  << endl;
	      file4 <<  "Jet Q factor " << metFromD3PD-> jet_AntiKt4H1TopoNew_quality->at(i) << endl;
	      file4 <<  "Jet timing " << metFromD3PD-> jet_AntiKt4H1TopoNew_timing->at(i)<< endl;
	    }
	    file4 <<  endl;
	  }//outlier

	  // "COLLCAND_CLEANING_CUTS_NOJET"
	  if (METEvtSel.hasNoJets()) {       
	    basicTool4.execute(&data);
	    resolutionTool4.execute(&data);
	    resolutionTool_em4.execute(&data);	

	    i_COLLCAND_CLEANING_NOJETS++;
	  }

	  // "COLLCAND_CLEANING_CUTS_JET"
	  if (METEvtSel.hasOneJet()) {       
	    basicTool5.execute(&data);
	    resolutionTool5.execute(&data);
	    resolutionTool_em5.execute(&data);	
       
	    i_COLLCAND_CLEANING_ONEJET++;

	    // "COLLCAND_CLEANING_CUTS_JET20"
	    if ( METEvtSel.hasOneJetAbove20()){       
	      basicTool6.execute(&data);
	      resolutionTool6.execute(&data);
	      resolutionTool_em6.execute(&data);

	      i_COLLCAND_CLEANING_ONEJETGT20++;

	    }//chiudo jet20
	  } //chiudo njet             
	}//chiudo cleaning
      }//chiudo collcand
    }//chiudoGRL
  }//done loop over all entries

  file_counters << "Total number of events                    = " << nentries << endl;
  file_counters << "passed GRL                                = " << i_GRL << endl;
  file_counters << "passed COLLCAND                           = " << i_COLLCAND  << endl;
  file_counters << "passed COLLCAND + CLEANING                = " << i_COLLCAND_CLEANING << endl;
  file_counters << "passed COLLCAND + CLEANING, NO JETS       = " << i_COLLCAND_CLEANING_NOJETS << endl;
  file_counters << "passed COLLCAND + CLEANING, ONE JET       = " << i_COLLCAND_CLEANING_ONEJET << endl;
  file_counters << "passed COLLCAND + CLEANING, ONE JET > 20  = " << i_COLLCAND_CLEANING_ONEJETGT20 << endl;

  file_counters.close();
  file_counters.clear();
  cout << "writing outliers file, " << num_outliers << " outliers." << endl;
  file4.close();
  file4.clear();

  cout << "writing output histogram file" << endl;

  //write output root file
  out->Write();
  out->Close();

}//end runD3PD_withSelection()

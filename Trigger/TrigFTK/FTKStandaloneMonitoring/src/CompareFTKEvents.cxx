#include "FTKStandaloneMonitoring/CompareFTKEvents.h"
#include <iostream>
#include <string>
CompareFTKEvents::CompareFTKEvents()
{
    std::cout<<"Empty constructor"<<std::endl;
}
//CompareFTKEvents::~CompareFTKEvents()
//{
//    std::cout<<"Destructor"<<std::endl;
//}
CompareFTKEvents::CompareFTKEvents(const std::string &BSfile, const std::string &NTUP_FTK_file):
   m_BSfile(BSfile),
   m_NTUP_FTK_file(NTUP_FTK_file)
{
    std::cout<<"Input files constructor"<<std::endl;
    std::cout<<m_BSfile<<std::endl;
    std::cout<<m_NTUP_FTK_file<<std::endl;
    m_myBSfile.open(m_BSfile.c_str(),std::ios::in | std::ios::binary);
    if (m_myBSfile.fail()){
        std::cout << "ERROR! cannot open file "<<m_BSfile.c_str();
	m_myBSfile.clear( );
    }
}
void CompareFTKEvents::CreateHistos(){
    TH1D * h_res_pt= new TH1D("h_res_pt","#Delta p_{T} (test-ref)/ p_{T}^{ref};( p_{T}^{test} - p_{T}^{ref} )/ p_{T}^{ref};FTK Tracks",2000,-1.,1.);
    TH1D * h_res_d0= new TH1D("h_res_d0","#Delta d_{0} (test-ref)/ d_{0}^{ref};( d_{0}^{test} - d_{0}^{ref} )/ d_{0}^{ref};FTK Tracks",4000,-2.,2.);
    TH1D * h_ref_ntrk= new TH1D("h_ref_ntrk","Reference Nr. of tracks;N_{trk}^{ref};Events",100,0,100);
    TH1D * h_test_ntrk= new TH1D("h_test_ntrk","Test Nr. of tracks;N_{trk}^{test};Events",100,0,100);
    vec_histo.push_back(h_res_pt);
    vec_histo.push_back(h_res_d0);
    vec_histo.push_back(h_ref_ntrk);
    vec_histo.push_back(h_test_ntrk);
//      std::cout<<"bin "<<h_res_pt->GetBinContent(501)<<std::endl;

}
void CompareFTKEvents::SetHistos(std::vector<std::string> histo_list){
    m_histo_list=histo_list;
    for (auto & istr : m_histo_list){
	std::size_t pos = istr.find("_");      
        std::string str3 = istr.substr (pos+1);
        std::stringstream ss;
        if (istr.find("HWSW")!=std::string::npos)        ss<<"HW=SIM;"<<str3<<";FTK HW=SIM Tracks";
        else if (istr.find("HWonly")!=std::string::npos) ss<<"HW only;"<<str3<<";FTK HW Tracks w/o SW match";
        else if (istr.find("nTrk")!=std::string::npos) ss<<istr<<";N tracks;Events";
	std::string title = ss.str();
        if (istr.find("ETA_PHI")==std::string::npos){
            if (istr.find("nTrk")!=std::string::npos) map_histo.insert(std::map<std::string, TH1D *>::value_type(istr, new TH1D(istr.c_str(),title.c_str(),100,0,100)));
 	    else if (istr.find("HWSW")!=std::string::npos|| istr.find("HWonly")!=std::string::npos) map_histo.insert(std::map<std::string, TH1D *>::value_type(istr, new TH1D(istr.c_str(),title.c_str(),histo_param[str3].at(0),histo_param[str3].at(1),histo_param[str3].at(2))));
            if (istr.find("res_pt")!=std::string::npos){
		//TH1D *h= new TH1D("m_res_pt","#Delta p_{T} (test-ref)/ p_{T}^{ref};( p_{T}^{test} - p_{T}^{ref} )/ p_{T}^{ref};FTK Tracks",2000,-1.,1.);
		//map_histo.insert(std::map<std::string, TH1D *>::value_type(istr, h));
		map_histo.insert(std::map<std::string, TH1D *>::value_type(istr, new TH1D("m_res_pt","#Delta p_{T} (test-ref)/ p_{T}^{ref};( p_{T}^{test} - p_{T}^{ref} )/ p_{T}^{ref};FTK Tracks",2000,-1.,1.)));
	    }
            if (istr.find("res_d0")!=std::string::npos) 
		map_histo.insert(std::map<std::string, TH1D *>::value_type(istr, new TH1D("m_res_d0","#Delta d_{0} (test-ref)/ d_{0}^{ref};( d_{0}^{test} - d_{0}^{ref} )/ d_{0}^{ref};FTK Tracks",4000,-2.,2.)));
	}
	else {
 	    map_histo_2D.insert(std::map<std::string, TH2D *>::value_type(istr, new TH2D(istr.c_str(),title.c_str(),100,-2.5,2.5,100,-3.2,3.2)));	
	}
    }	
    
}
void CompareFTKEvents::Execute()
{
    CreateHistos();
    m_Nevents_BS=GetNEventsBS();
    std::cout<<"Input BS file has N events: "<<m_Nevents_BS<<std::endl;
    readNTUP_FTKfile();
    if (m_Nevents_BS!=m_Nevents_NTUPFTK) {
	std::cout<<"different number of events: "<<m_Nevents_BS<<" "<<m_Nevents_NTUPFTK<<std::endl;
	m_allmatched=false;
    }
    EventLoop();
    WriteHistos();
}
void CompareFTKEvents::readNTUP_FTKfile()
{
    std::cout<<"Reading NTUP_FTK file "<<m_NTUP_FTK_file<<std::endl;
    m_fntupftk = new TFile(m_NTUP_FTK_file.c_str());
    if (!m_fntupftk) std::cout<<"ERROR!! NTUP_FTK file could not be opened: "<<m_NTUP_FTK_file.c_str()<<std::endl;
    if (!m_fntupftk->GetListOfKeys()->Contains(m_str_tree_evtinfo.c_str())) std::cout<<"ERROR!! NTUP_FTK file does not contain tree: "<<m_str_tree_evtinfo.c_str()<<std::endl;
    m_tevtinfo = (TTree*)m_fntupftk->Get(m_str_tree_evtinfo.c_str());
    m_tevtinfo->SetBranchAddress("RunNumber",&RN);  
    m_tevtinfo->SetBranchAddress("EventNumber",&EN);  
    m_tevtinfo->SetBranchAddress("LB",&LB);  
    m_tevtinfo->SetBranchAddress("BCID",&BCID);  
    m_tevtinfo->SetBranchAddress("ExtendedLevel1ID",&EL1ID);  
    m_tevtinfo->SetBranchAddress("Level1TriggerType",&l1TT); 
    if (!m_fntupftk->GetListOfKeys()->Contains(m_str_tree_ftkdata.c_str())) std::cout<<"ERROR!! NTUP_FTK file does not contain tree: "<<m_str_tree_ftkdata.c_str()<<std::endl;
    m_theTree=(TTree *)m_fntupftk->Get(m_str_tree_ftkdata.c_str());
    m_ft=new FTKTrackStream();
    if (!m_theTree->GetListOfBranches()->Contains(m_str_tree_ftkstream.c_str()))std::cout<<"ERROR!! NTUP_FTK file does not contain branch: "<<m_str_tree_ftkstream.c_str()<<std::endl;
    auto branch  = m_theTree->GetBranch(m_str_tree_ftkstream.c_str());
    branch->SetAddress(&m_ft);
    m_Nevents_NTUPFTK=m_theTree->GetEntries();
}
StatusCode CompareFTKEvents::decode(uint32_t nTracks, OFFLINE_FRAGMENTS_NAMESPACE::PointerType rodData, FTK_RawTrackContainer* result) {

  result->reserve(result->size() + nTracks);
  for ( size_t i = 0; i < nTracks; ++i ) {
    FTK_RawTrack* track = unpackFTTrack( rodData );
    rodData += TrackBlobSize;
    result->push_back(track);
  }
  return StatusCode::SUCCESS;
}
FTK_RawTrack* CompareFTKEvents::unpackFTTrack( OFFLINE_FRAGMENTS_NAMESPACE::PointerType data) {  
  FTK_RawTrack* track = new FTK_RawTrack(data[0], data[1], data[2], data[3], data[4], data[5]); // first six words are track params
  data += TrackParamsBlobSize;

  // get pixel hits  
  track->getPixelClusters().resize(NPixLayers);
  for ( size_t i = 0; i < size_t(NPixLayers); ++i) {
    size_t offset = PixHitParamsBlobSize*i;
    unpackPixCluster(data+offset, track->getPixelCluster(i) );    
  }
  data += PixHitParamsBlobSize*NPixLayers;

  // gets SCT hits
  track->getSCTClusters().resize(NSCTLayers);
  for ( size_t i = 0; i < size_t(NSCTLayers); ++i) {
    size_t offset = SCTHitParamsBlobSize*i;
    unpackSCTCluster(data+offset, track->getSCTCluster(i) );    
  }
  // no more shifts needed
  return track;
}  
void CompareFTKEvents::unpackPixCluster(OFFLINE_FRAGMENTS_NAMESPACE::PointerType data, FTK_RawPixelCluster& cluster) {
  cluster.setWordA(*data);
  cluster.setWordB(*(data+1));
}
void CompareFTKEvents::EventLoop()
{
    std::cout<<"Starting Loop"<<std::endl;
    m_tmpbufpos=0;
    if (m_nevtmax==0)m_nevtmax=std::min(m_Nevents_BS,m_Nevents_NTUPFTK);
    for (int ievent=1;ievent<=m_nevtmax;ievent++){
       m_tmpbufpos =readBSevent( 1,m_tmpbufpos);  
       int lengthdata=m_data.size();
       uint32_t * data32=NULL;
       data32=new uint32_t[lengthdata]; 
       for(int ij=0; ij<lengthdata; ij++) {
           data32[ij]=m_data[ij];
       }
       OFFLINE_FRAGMENTS_NAMESPACE::PointerType pdata=data32;
       //event info are given to FullEventFragment
       eformat::read::FullEventFragment fe(pdata);
       std::vector< eformat::read::ROBFragment > ROBFragments_v;
       if (m_verbose)std::cout<<"Run Number "<<fe.run_no()<<" Lvl1 "<<fe.lvl1_id()<<" BCID "<<fe.bc_id()<<std::endl;
       fe.robs( ROBFragments_v );
       FTK_RawTrackContainer* trkcontainer= new FTK_RawTrackContainer();
       std::vector<const FTK_RawTrack *> ftkBSref;
       int nrob=0;
       int nTrks=0;
       for(auto& ROBFragment : ROBFragments_v){
           unsigned int niTrks=0;
           eformat::helper::SourceIdentifier ROBSource_id = eformat::helper::SourceIdentifier( ROBFragment.source_id() );
           if(ROBSource_id.subdetector_id() != eformat::TDAQ_FTK ){
              continue;
           }
           const uint32_t *rod_data = ROBFragment.rod_data();
           uint32_t rod_ndata = ROBFragment.rod_ndata();
	   if( ( rod_ndata - trackBlockOffsetStart - trackBlockOffsetEnd ) % TrackBlobSize != 0 ){
	      std::cout<<"Error: wrong size of rod"<<rod_ndata<<std::endl;
	      continue;
	   }
           niTrks = ( rod_ndata - trackBlockOffsetStart - trackBlockOffsetEnd ) / TrackBlobSize;
	   nTrks+=niTrks;
           rod_data += trackBlockOffsetStart;            // moving data pointer to first track block
	   if (m_verbose){
	      std::cout<<"test of working rod l1 "<<ROBFragment.rod_lvl1_id()<<" ndata "<<rod_ndata<<std::endl;
	      //std::cout<< "decode N tracks "<< decodeNumberOfTracks(rod_data)<< "Ntracks "<<nTrks<<std::endl;
	   }
	   // decoding the rod info into FTK track container
           StatusCode sc;
	   sc=decode(niTrks,rod_data, trkcontainer);
	   if (!sc.isSuccess()) std::cout<<"ERROR!! decode function did not return success"<<std::endl;
	   if (trkcontainer->size()==0) {
               std::cout<<"no FTK tracks from BS file continue"<<std::endl;
	       continue;
	   }
	   if (m_verbose){
	      std::cout<<sc.isSuccess()<<std::endl;
              std::cout<< "collection of size " << trkcontainer->size() << std::endl;
              //for ( unsigned int i = 0 ; i < nTrks; ++i ) {
	      //	 std::cout<<"D0"<<trkcontainer->at(i)->getD0()<<std::endl;
	      //}
	   }
           for ( unsigned int i = 0 ; i < niTrks; ++i ) {
	       ftkBSref.push_back(trkcontainer->at(i));
	   }
	   nrob+=1;	  
	   //delete [] rod_data;
       }
       vec_histo.at(2)->Fill(nTrks);
       if (nrob>1){std::cout<<"!!!!!!!!!!SOMETHING WRONG number of robs >1:"<<nrob<<std::endl;}
       std::cout<<"Event "<<ievent<<" N tracks BS file "<<trkcontainer->size()<<std::endl;
       // end reading info from BS file
       //---------------------------------------------------------------------//
       // start reading info from NTUP_FTK file
       m_theTree->GetEvent(ievent-1); 
       FTK_RawTrackContainer* trkcontainerNTUP= new FTK_RawTrackContainer();
       std::vector<const FTK_RawTrack *> ftkNTUPtest;
       int NTracksNTUP;
       NTracksNTUP=m_ft->getNTracks();
       vec_histo.at(3)->Fill(NTracksNTUP);
       for(int it=0; it<NTracksNTUP;it++){
         FTKTrack* ftktrk=m_ft->getTrack(it);
         FTK_RawTrack* rawftktrk=new FTK_RawTrack();
	 rawftktrk->setD0(ftktrk->getIP());
	 rawftktrk->setZ0(ftktrk->getZ0());
	 rawftktrk->setPhi(ftktrk->getPhi());
	 rawftktrk->setCotTh(ftktrk->getCotTheta());
	 //cout<<"Eta comp"<<TMath::ASinH(rawftktrk->getCotTh())<<" "<<ftktrk->getEta()<<std::endl;
	 rawftktrk->setInvPt(2*ftktrk->getHalfInvPt());
	 rawftktrk->setChi2(ftktrk->getChi2());
	 rawftktrk->setBarcode(ftktrk->getBarcode());
	 trkcontainerNTUP->push_back(rawftktrk);
         ftkNTUPtest.push_back(trkcontainerNTUP->at(it));
       }
       std::cout<<"Event "<<ievent<<" N tracks NTUP_FTK "<<trkcontainerNTUP->size()<<std::endl;
       // end reading info from NTUP_FTK file
       //---------------------------------------------------------------------//
       // start comparing infos
       m_tevtinfo->GetEntry(ievent-1);
       if (NTracksNTUP!=nTrks){std::cout<<"different N tracks: "<<NTracksNTUP<<" "<<nTrks<<std::endl;m_allmatched=false;}
       if (fe.run_no()!=(unsigned int)RN){std::cout<<"different RunNUmber: "<<fe.run_no()<<" "<<RN<<std::endl; m_allmatched=false;}
       if (fe.bc_id()!=(unsigned int)BCID){std::cout<<"different BCID: "<<fe.bc_id()<<" "<<BCID<<std::endl;m_allmatched=false;}
       // end comparing infos
       //---------------------------------------------------------------------//
       // start matching
       for ( int i = 0 ; i < std::min((int) nTrks,NTracksNTUP); ++i ) {
           if(trkcontainerNTUP->at(i)->getD0()!=0&&trkcontainerNTUP->at(i)->getZ0()!=0&&trkcontainerNTUP->at(i)->getInvPt()!=0&&
	      trkcontainerNTUP->at(i)->getPhi()!=0&&trkcontainerNTUP->at(i)->getCotTh()!=0&&trkcontainerNTUP->at(i)->getChi2()!=0
	      &&(
	       abs(trkcontainer->at(i)->getD0()/trkcontainerNTUP->at(i)->getD0()-1.)>0.001||
               abs(trkcontainer->at(i)->getZ0()/trkcontainerNTUP->at(i)->getZ0()-1.)>0.001||
               abs(trkcontainer->at(i)->getInvPt()/trkcontainerNTUP->at(i)->getInvPt()-1.)>0.001||
               abs(trkcontainer->at(i)->getPhi()/trkcontainerNTUP->at(i)->getPhi()-1.)>0.001||
               abs(trkcontainer->at(i)->getCotTh()/trkcontainerNTUP->at(i)->getCotTh()-1.)>0.001||
               abs(trkcontainer->at(i)->getChi2()/trkcontainerNTUP->at(i)->getChi2()-1.)>0.001)){
		   std::cout<<"Difference in comparing trk "<<i<<std::endl;
		   std::cout<<"\td0 : \t"<<trkcontainer->at(i)->getD0()<<"\t"<<trkcontainerNTUP->at(i)->getD0()<<std::endl;
		   std::cout<<"\tz0 : \t"<<trkcontainer->at(i)->getZ0()<<"\t"<<trkcontainerNTUP->at(i)->getZ0()<<std::endl;
		   std::cout<<"\tipt : \t"<<trkcontainer->at(i)->getInvPt()<<"\t"<<trkcontainerNTUP->at(i)->getInvPt()<<std::endl;
		   std::cout<<"\tPhi : \t"<<trkcontainer->at(i)->getPhi()<<"\t"<<trkcontainerNTUP->at(i)->getPhi()<<std::endl;
		   std::cout<<"\tCotTh : \t"<<trkcontainer->at(i)->getCotTh()<<"\t"<<trkcontainerNTUP->at(i)->getCotTh()<<std::endl;
		   std::cout<<"\tChi2 : \t"<<trkcontainer->at(i)->getChi2()<<"\t"<<trkcontainerNTUP->at(i)->getChi2()<<std::endl;
		   m_allmatched=false;
               }
	   else {if (i%std::min(nTrks,NTracksNTUP)==0 && m_allmatched){std::cout<<"Track Parameters btw BS_FTK and NTUP_FTK are the same: good!"<<std::endl;}}
       }
       m_compTrk= new CompareFTKTracks(ftkBSref,ftkNTUPtest,vec_histo, map_histo, map_histo_2D);
       m_compTrk->AssociateTracks();
       //m_compTrk->TestEquivalence();
       m_compTrk->FillHistos();
     }
}
void CompareFTKEvents::WriteHistos(){
    m_fout= new TFile("./out.histo.root","RECREATE");
    m_fout->cd();
    for(unsigned int ih=0; ih<vec_histo.size();ih++){
        vec_histo.at(ih)->Write();
    }
    for(auto & imap : map_histo){
        imap.second->Write();
    }
    m_fout->Write();
    m_fout->Close();
    std::cout<<"Histo written into file "<<m_fout->GetName()<<std::endl;
}
std::streampos CompareFTKEvents::readBSevent(int ievent,std::streampos startbufpos)
{
    if (m_verbose) std::cout<<"Reading BS event"<<std::endl;
    m_data.clear();
    uint32_t word;
    int iil=0;
    int nprocessedevents=0;
    std::streampos lastpos;
    if (m_verbose) std::cout<<"pointer position start"<<m_myBSfile.tellg()<<std::endl;
    if (ievent ==1){
       if (m_verbose) std::cout<<"going to buffer position "<<startbufpos<<std::endl;
       m_myBSfile.seekg(startbufpos);
    }
    if (m_verbose) std::cout<<"starting reading words "<<startbufpos<<std::endl;
    while(m_myBSfile.is_open())
    {
       if(m_myBSfile.eof()){  break; }//printf("END\n");
       word=0;
       m_myBSfile.read ((char*)&word, sizeof(word));
       //if (m_verbose) std::cout<<"word" <<word<<std::endl;
       if (word==m_checkword) {
	  nprocessedevents+=1;
          if (m_verbose) std::cout<<"pointer position start event"<<m_myBSfile.tellg()<<std::endl;
       }
       if (nprocessedevents==ievent){ 
          m_data.push_back(word);
          iil++;
	  lastpos=m_myBSfile.tellg();
       }
       else if (nprocessedevents>ievent) {
          if (m_verbose) std::cout<<"pointer position end event "<<lastpos<<std::endl;
          break;
       }
    }
    if (m_verbose) std::cout<<"pointer position before closing"<<m_myBSfile.tellg()<<std::endl;
    return lastpos;    
}
int CompareFTKEvents::GetNEventsBS()
{
    std::cout<<"Getting N events BS file"<<std::endl;
    uint32_t word;
    int nevents=0;
    while(m_myBSfile.is_open())
    {
       if(m_myBSfile.eof()){ break; }
       word=0;
       m_myBSfile.read ((char*)&word, sizeof(word));
       if (word==m_checkword) {
	  nevents+=1;
       }
    }
    m_myBSfile.clear();
    m_myBSfile.seekg(0,std::ios::beg);
    return nevents;    
}
void CompareFTKEvents::PrintFiles()
{
    std::cout<<"Print Input files"<<std::endl;
    std::cout<<m_BSfile<<std::endl;
    std::cout<<m_NTUP_FTK_file<<std::endl;    
}
CompareFTKEvents::~CompareFTKEvents()
{
    m_myBSfile.close();
    if (m_allmatched) std::cout<<"====>Finished successfully: the two files contain the same infos"<<std::endl;
    else std::cout<<"====>There was something different, check the printouts"<<std::endl;
    
}

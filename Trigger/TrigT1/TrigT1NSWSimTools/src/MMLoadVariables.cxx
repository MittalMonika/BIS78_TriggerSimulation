/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// local includes
#include "MMLoadVariables.h"

// //Event info includes
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

// random numbers
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"


// using namespace std;

MMLoadVariables::MMLoadVariables(StoreGateSvc* evtStore, const MuonGM::MuonDetectorManager* detManager, const MmIdHelper* idhelper, MMT_Parameters *par){
      m_par = par;
      m_evtStore = evtStore;
      m_detManager = detManager;
      m_MmIdHelper = idhelper;
}

MMLoadVariables::~MMLoadVariables() {
}

    void MMLoadVariables::getMMDigitsInfo(vector<athena_entry>& entries, map<hdst_key,hdst_entry>& Hits_Data_Set_Time, map<int,evInf_entry>& Event_Info){
      //*******Following MuonPRD code to access all the variables**********

      histogramVariables fillVars;

      //Get truth variables, vertex
      const McEventCollection *truthContainer = nullptr;
      StatusCode sc1 = m_evtStore->retrieve(truthContainer,"TruthEvent");

      //Get MuEntry variables TBD if still necessary
      const TrackRecordCollection* trackRecordCollection = nullptr;
      StatusCode sc2 = m_evtStore->retrieve(trackRecordCollection,"MuonEntryLayer") ;

      //Get truth information container of digitization
      const MuonSimDataCollection* nsw_MmSdoContainer = nullptr;
      StatusCode sc3 = m_evtStore->retrieve(nsw_MmSdoContainer,"MM_SDO");

      // get digit container (a container corresponds to a multilayer of a module)
      const MmDigitContainer *nsw_MmDigitContainer = nullptr;
      StatusCode sc4 = m_evtStore->retrieve(nsw_MmDigitContainer,"MM_DIGITS");

      std::string wedgeType = getWedgeType(nsw_MmDigitContainer);

      float phiEntry = 0, phiPosition = 0, etaEntry = 0, etaPosition = 0, chargeThreshold = m_par->chargeThreshold, theTheta = 0;//, avTheta = 0;

      TLorentzVector thePart, theInfo;
      TVector3 vertex;int pdg=0;
      auto MuEntry_Particle_n = trackRecordCollection->size();
      int j=0; //# iteration of particle entries

      for(auto it : *truthContainer) {  //first loop in MMT_loader::load_event

        const HepMC::GenEvent *subEvent = it;


        HepMC::ConstGenEventParticleRange particle_range = subEvent->particle_range();

        for(auto pit : particle_range) {
          const HepMC::GenParticle *particle = pit;
          const HepMC::FourVector momentum = particle->momentum();

          int k=0;
          for(auto mit : *trackRecordCollection ) {k++;} //number of mu entries
          if(particle->barcode() == 10001 && abs(particle->pdg_id())==13){

            thePart.SetPtEtaPhiE(momentum.perp(),momentum.eta(),momentum.phi(),momentum.e());

            for(auto mit : *trackRecordCollection ) {
              if(k>0&&j<k){

                const CLHEP::Hep3Vector mumomentum = mit.GetMomentum();
                const CLHEP::Hep3Vector muposition = mit.GetPosition();

                pdg=particle->barcode();
                phiEntry = mumomentum.getPhi();
                etaEntry = mumomentum.getEta();
                phiPosition = muposition.getPhi();
                etaPosition = muposition.getEta();
                std::cout << "THEPARTXY " << thePart.X() << " " << thePart.Y() << " " << thePart.Phi() << std::endl;

              }
            }//muentry loop
              int l=0;
              HepMC::ConstGenEventVertexRange vertex_range = subEvent->vertex_range();



              for(auto vit : vertex_range) {
                if(l!=0){break;}//get first vertex of iteration, may want to change this
                const HepMC::GenVertex *vertex1 = vit;
                const HepMC::FourVector position = vertex1->position();



                vertex=TVector3(position.x(),position.y(),position.z());

                l++;
              }//end vertex loop
            }
            j++;
        } //end particle loop
      } //end container loop (should be only 1 container per event)
      const EventInfo* pevt = 0;
      StatusCode sc = m_evtStore->retrieve(pevt);

      int event = pevt->event_ID()->event_number();
      if(j==0){cout << "ZERO PARTICLES IN EVENT";} //debug
      int TruthParticle_n = j;
      evFit_entry fit;fit.athena_event=event;//-1;  **May not be necessary

      //Truth information for theta, phi
      double theta_pos=atan(exp(-etaPosition))*2,theta_ent=atan(exp(-etaEntry))*2,phi_pos=phiPosition;


      //may need rename
      // vector<athena_entry> entries;

      //get hits container
      const GenericMuonSimHitCollection *nswContainer = nullptr;
      StatusCode sc5 = m_evtStore->retrieve(nswContainer,"MicromegasSensitiveDetector");


      //Second loop in MMT_loader
      int TruthDigit_n = 0;
      if(nsw_MmDigitContainer->size()==0) {cout <<"EMPTY";}
      for(auto dit : *nsw_MmDigitContainer) {
        // a digit collection is instanciated for each container, i.e. holds all digits of a multilayer
        const MmDigitCollection* coll = dit;
        // loop on all digits inside a collection, i.e. multilayer
        int digit_count =0;

        cout << "ITEMSIZE  " << coll->size() << endl;
        for (unsigned int item=0; item<coll->size(); item++) {
          //*******THIS COULD BE AN ISSUE SEE MMDIGITVARIABLES IF VALIDATIONS DON'T MATCH


          // get specific digit and identify it
          const MmDigit* digit = coll->at(item);
          Identifier Id = digit->identify();


          //if (nsw_MmSdoContainer /* && nsw_MmSdoContainer->size()*/) {

            // search the truth container with the Id of the digit
            //const MuonSimData mm_sdo = (nsw_MmSdoContainer->find(Id))->second;
            //std::vector<MuonSimData::Deposit> deposits;
            // get the truth deposits
            //mm_sdo.deposits(deposits);
            // use the information of the first deposit
            // int    truth_barcode   = deposits[0].first.barcode();
            // double truth_localPosX = deposits[0].second.firstEntry();
            // double truth_localPosY = deposits[0].second.secondEntry();
            // float  truth_angle     = mm_sdo.word()/1000.;




            //Amg::Vector2D hit_on_surface(truth_localPosX, truth_localPosY);
            Amg::Vector3D hit_gpos(0., 0., 0.);


            std::string stName   = m_MmIdHelper->stationNameString(m_MmIdHelper->stationName(Id));
            int stationEta       = m_MmIdHelper->stationEta(Id);
            int stationPhi       = m_MmIdHelper->stationPhi(Id);
            int multiplet        = m_MmIdHelper->multilayer(Id);
            int gas_gap          = m_MmIdHelper->gasGap(Id);

            int channel          = m_MmIdHelper->channel(Id);



            int isSmall = (stName[2] == 'S');
            const MuonGM::MMReadoutElement* rdoEl = m_detManager->getMMRElement_fromIdFields(isSmall, stationEta, stationPhi, multiplet );

            std::vector<float>  time          = digit->chipResponseTime();
            std::vector<float>  charge        = digit->chipResponseCharge();
            std::vector<int>    stripPosition = digit->chipResponsePosition();

            bool isValid;
            int stationName  = m_MmIdHelper->stationName(Id);

            string sname(stName);

            int mult=m_MmIdHelper->multilayer(Id),gap=m_MmIdHelper->gasGap(Id);

            fillVars.NSWMM_dig_stationEta.push_back(stationEta);
            fillVars.NSWMM_dig_stationPhi.push_back(stationPhi);
            fillVars.NSWMM_dig_multiplet.push_back(multiplet);
            fillVars.NSWMM_dig_gas_gap.push_back(gas_gap);
            fillVars.NSWMM_dig_channel.push_back(channel);

            //match to truth particle
            TLorentzVector truthPart;
            for(auto it1 : *truthContainer) {  //Must be a more elegant way... should work for now though

              const HepMC::GenEvent *subEvent1 = it1;
              HepMC::ConstGenEventParticleRange particle_range1 = subEvent1->particle_range();

              for(auto pit1 : particle_range1) {
                const HepMC::GenParticle *particle1 = pit1;
                const HepMC::FourVector momentum1 = particle1->momentum();

                //if( particle1->barcode() == deposits[0].first.barcode()) {
                  truthPart.SetPtEtaPhiE(momentum1.perp(),momentum1.eta(),momentum1.phi(),momentum1.e());

               // }//end if barcode matching

              }//end particle loop
            }//end truth container loop (1 iteration) for matching

            theTheta = truthPart.Theta(); //not used yet??

            std::vector<double> localPosX;
            std::vector<double> localPosY;
            std::vector<double> globalPosX;
            std::vector<double> globalPosY;
            std::vector<double> globalPosZ;


            for (unsigned int i=0;i<stripPosition.size();i++) {
              // take strip index form chip information
              int cr_strip = stripPosition.at(i);


              localPosX.push_back(0.);
              localPosY.push_back(0.);
              globalPosX.push_back(0.);
              globalPosY.push_back(0.);
              globalPosZ.push_back(0.);


              Identifier cr_id = m_MmIdHelper->channelID(stationName, stationEta, stationPhi, multiplet, gas_gap, cr_strip, true, &isValid);
              if (!isValid) {
              //ATH_MSG_WARNING("MicroMegas digitization: failed to create a valid ID for (chip response) strip n. " << cr_strip
               //                << "; associated positions will be set to 0.0.");
              } else {
                  // asking the detector element to get local position of strip
                  Amg::Vector2D cr_strip_pos(0., 0.);
                  if ( !rdoEl->stripPosition(cr_id,cr_strip_pos) ) {
                  //  ATH_MSG_WARNING("MicroMegas digitization: failed to associate a valid local position for (chip response) strip n. " << cr_strip
                  //               << "; associated positions will be set to 0.0.");
                  } else {
                    localPosX.at(i) = cr_strip_pos.x();
                    localPosY.at(i) = cr_strip_pos.y();
                    }

                  // asking the detector element to transform this local to the global position
                  Amg::Vector3D cr_strip_gpos(0., 0., 0.);
                  rdoEl->surface(cr_id).localToGlobal(cr_strip_pos, Amg::Vector3D(0., 0., 0.), cr_strip_gpos);
                  globalPosX.at(i) = cr_strip_gpos[0];
                  globalPosY.at(i) = cr_strip_gpos[1];
                  globalPosZ.at(i) = cr_strip_gpos[2];



                  // check if local and global position are congruent with the transform
                  Amg::Vector3D lpos = rdoEl->transform(cr_id).inverse() * cr_strip_gpos;
                  double dx = cr_strip_pos.x() - lpos.x();
                  double dy = cr_strip_pos.y() - lpos.y();
                }

            }//end of strip position loop
            //NTUPLE FILL DIGITS
            fillVars.NSWMM_dig_time.push_back(time);
            fillVars.NSWMM_dig_charge.push_back(charge);
            fillVars.NSWMM_dig_stripPosition.push_back(stripPosition);
            fillVars.NSWMM_dig_stripLposX.push_back(localPosX);
            fillVars.NSWMM_dig_stripLposY.push_back(localPosY);
            fillVars.NSWMM_dig_stripGposX.push_back(globalPosX);
            fillVars.NSWMM_dig_stripGposY.push_back(globalPosY);
            fillVars.NSWMM_dig_stripGposZ.push_back(globalPosZ);
            if(globalPosY.size() == 0) continue;

            //if(globalPosY[0]<-100 || globalPosY[0]>100 ) continue; //vector access [] not .at().. be careful
            int indForPos = -1;
            float earliestTime = 100000;
            for (unsigned int i=0;i<stripPosition.size();i++) {

              //if(charge[i]<chargeThreshold) continue;

              if(time[i]<earliestTime){
                earliestTime =time[i];
                indForPos=i;

              }
            }//end of strip poistion loop 2

            if(indForPos == -1) continue;
            int hit_count=0;


            MicromegasHitIdHelper* hitHelper = MicromegasHitIdHelper::GetHelper();
            MM_SimIdToOfflineId simToOffline(*m_MmIdHelper);
            for( auto it2 : *nswContainer ) { //get hit variables
              const GenericMuonSimHit hit = it2;
              fillVars.NSWMM_globalTime.push_back(hit.globalTime());

              const Amg::Vector3D globalPosition = hit.globalPosition();
              if(digit_count==0){
                fillVars.NSWMM_hitGlobalPositionX.push_back(globalPosition.x());
                fillVars.NSWMM_hitGlobalPositionY.push_back(globalPosition.y());
                fillVars.NSWMM_hitGlobalPositionZ.push_back(globalPosition.z());
                fillVars.NSWMM_hitGlobalPositionR.push_back(globalPosition.perp());
                fillVars.NSWMM_hitGlobalPositionP.push_back(globalPosition.phi());
                const Amg::Vector3D globalDirection = hit.globalDirection();
                fillVars.NSWMM_hitGlobalDirectionX.push_back(globalDirection.x());
                fillVars.NSWMM_hitGlobalDirectionY.push_back(globalDirection.y());
                fillVars.NSWMM_hitGlobalDirectionZ.push_back(globalDirection.z());

                const Amg::Vector3D localPosition = hit.localPosition();
                fillVars.NSWMM_hitLocalPositionX.push_back(localPosition.x());
                fillVars.NSWMM_hitLocalPositionY.push_back(localPosition.y());
                fillVars.NSWMM_hitLocalPositionZ.push_back(localPosition.z());

                fillVars.NSWMM_particleEncoding.push_back(hit.particleEncoding());
                fillVars.NSWMM_kineticEnergy.push_back(hit.kineticEnergy());
                fillVars.NSWMM_depositEnergy.push_back(hit.depositEnergy());
                fillVars.NSWMM_StepLength.push_back(hit.StepLength());
              }

              int simId = hit.GenericId();
              std::string sim_stationName = hitHelper->GetStationName(simId);
              int sim_stationEta  = hitHelper->GetZSector(simId);
              int sim_stationPhi  = hitHelper->GetPhiSector(simId);
              int sim_multilayer  = hitHelper->GetMultiLayer(simId);
              int sim_layer       = hitHelper->GetLayer(simId);
              int sim_side        = hitHelper->GetSide(simId);


              // Fill Ntuple with SimId data
              //fillVars.NSWMM_sim_stationName .push_back(sim_stationName);
              if(digit_count){
                fillVars.NSWMM_sim_stationEta  .push_back(sim_stationEta);
                fillVars.NSWMM_sim_stationPhi  .push_back(sim_stationPhi);
                fillVars.NSWMM_sim_multilayer  .push_back(sim_multilayer);
                fillVars.NSWMM_sim_layer       .push_back(sim_layer);
                fillVars.NSWMM_sim_side        .push_back(sim_side);
              }

              if(hit.depositEnergy()==0.) continue; // SimHits without energy loss are not recorded.

              if(digit_count==hit_count) {

                entries.push_back(athena_entry(mult,gap, hit.globalTime(),time[indForPos],
                TVector3(-999,-99,-999),//Digits_MM_truth_localPosZ->at(i)),
                TVector3(localPosX[indForPos],localPosY[indForPos],-999),//Digits_MM_stripLposZ->at(i)[indForPos]),
                TVector3(globalPosX[indForPos],globalPosY[indForPos],globalPosZ[indForPos]),
                charge[indForPos],stripPosition[indForPos],abs(stationEta),stationPhi));

                // cout << "HITX    " << globalPosition.x() << " HITY " << globalPosition.y() << endl;
                // cout << "HITPHI  " << atan2(globalPosition.y(),globalPosition.x())  << " " << phi_shift(atan2(globalPosition.y(),globalPosition.x()),wedgeType,stationPhi) << endl;
                // cout << "LOCALX  " << localPosX[indForPos] << " LOCALY " << localPosY[indForPos] << endl;
                // cout << "GLOBALX " << globalPosX[indForPos] << " GLOBALY " << globalPosY[indForPos] << endl;
                // cout << "CHARGE  " << charge[indForPos] << " STRIPPOS " << stripPosition[indForPos] << " STATIONETA " << stationEta << endl;
                // cout << "MULT    " << mult << " GAP " << gap << " GLOBALTIME " << hit.globalTime() << " TIME " << time[indForPos] << endl;
                // cout << "GLOBALZ " << globalPosZ[indForPos] << std::endl;
                // cout << "STATIONPHI " << stationPhi << std::endl;

              }
              hit_count++;
            }//end of hit cotainer loop */
         // } //end if sdo
        digit_count++;
        } //end iterator digit loop
      } // end digit container loop (1 for event?)
      vector<athena_entry> dummy;
      vector<int> indices;
      //Truth info for particle (originally primer)
      int phiSt = 0;
      if(entries.size() > 0) phiSt = entries.at(0).phi_station;
      evInf_entry particle_info(event,pdg,thePart.E(),thePart.Pt(),thePart.Eta(),etaPosition,etaEntry,phi_shift(thePart.Phi(),wedgeType,phiSt),phi_shift(phi_pos,wedgeType,phiSt),phi_shift(phiEntry,wedgeType,phiSt),thePart.Theta(),theta_pos,theta_ent,theta_ent-theta_pos,TruthParticle_n,MuEntry_Particle_n,vertex);
      if(wedgeType == "Neither") particle_info.bad_wedge=true;
      else particle_info.bad_wedge=false;

      vector<athena_entry> origEntries = entries;
      for(unsigned int i=0; i<entries.size(); i++){
        if(entries.size() < 8) continue;
        if(entries[i].multiplet==1 and entries[i].gas_gap==1) entries[i].gtime = origEntries[0].gtime;
        if(entries[i].multiplet==1 and entries[i].gas_gap==2) entries[i].gtime = origEntries[1].gtime;
        if(entries[i].multiplet==1 and entries[i].gas_gap==3) entries[i].gtime = origEntries[2].gtime;
        if(entries[i].multiplet==1 and entries[i].gas_gap==4) entries[i].gtime = origEntries[3].gtime;
        if(entries[i].multiplet==2 and entries[i].gas_gap==1) entries[i].gtime = origEntries[4].gtime;
        if(entries[i].multiplet==2 and entries[i].gas_gap==2) entries[i].gtime = origEntries[5].gtime;
        if(entries[i].multiplet==2 and entries[i].gas_gap==3) entries[i].gtime = origEntries[6].gtime;
        if(entries[i].multiplet==2 and entries[i].gas_gap==4) entries[i].gtime = origEntries[7].gtime;
      }
      for(unsigned int i=0; i<entries.size(); i++){
        float min = 100000;
        int minIndice=-999;
        for(unsigned int j=0; j<entries.size(); j++){
          bool notmindex = true;
          for (int k=0; k<indices.size(); k++){
            if(j==indices[k]) notmindex=false;
          }
          if(notmindex){
            if(min > entries[j].gtime ){
              minIndice = j;
              min = entries[minIndice].gtime ;
            }
          }
        }
        if(minIndice < 0) minIndice = i;
        dummy.push_back(entries[minIndice]);
        indices.push_back(minIndice);

      }
      entries = dummy;
      int min_hits = 1,max_hits = 10000,nent=entries.size();

      uvxxmod=(m_par->setup.compare("xxuvuvxx")==0);
      //Number of hits cut
      if(!particle_info.bad_wedge)particle_info.pass_cut=true;//default is false
      if(nent<min_hits||nent>max_hits) particle_info.pass_cut=false;
      if(!particle_info.pass_cut)cout<<"event FAIL at max hit mark...nent="<<nent<<endl;
      //double theta_min = m_par->minimum_large_theta,theta_max =m_par->maximum_large_theta,phi_min = m_par->minimum_large_phi,phi_max = m_par->maximum_large_phi;
      double theta_min = m_par->minimum_large_theta.getFloat();
      double theta_max =m_par->maximum_large_theta.getFloat();
      double phi_min = m_par->minimum_large_phi.getFloat();
      double phi_max = m_par->maximum_large_phi.getFloat();
      double tru_phi = -999;
      if (entries.size() >0) tru_phi=phi_shift(thePart.Phi(),wedgeType,entries.at(0).phi_station);
      double tru_theta=thePart.Theta();


      //Theta cut //ALTER THIS FOR BACKGROUND!!!!!
      cout<<"tru_theta is "<<tru_theta<<" which should be in ["<<theta_min<<","<<theta_max<<"]"<<endl;
      //TESTING
      //if(tru_theta<theta_min||tru_theta>theta_max) particle_info.pass_cut=false;//*** do a theta cut?
      if(!particle_info.pass_cut)cout<<"THETAFAIL"<<endl;

      //Phi cut
      cout<<"tru_phi is "<<tru_phi<<" which should be in ["<<phi_min<<","<<phi_max<<"]"<<endl;
      //TESTING
      //if(tru_phi<phi_min||tru_phi>phi_max) particle_info.pass_cut=false;
      if(!particle_info.pass_cut)cout<<"PHIPHAIL"<<endl;


      //Hit information in Stephen's code... Starts getting a little weird.
      map<hdst_key,hdst_entry> hit_info; //Originally "targaryen"
      vector<hdst_key> keys;int fstation=0;

      //Loop over entries, which has digitization info for each event
      for(unsigned int ient=0; ient<entries.size(); ient++){
        athena_entry examine=entries[ient];
        //DLM_NEW plane assignments
        //stated [3,2,1,0;7,6,5,4]
        int plane=(examine.multiplet-1)*4+examine.gas_gap-1;
        // cout<<"SUBSTR CALL MMT_L--0...plane: "<<plane<<", multiplet: "<<examine.multiplet<<endl;
        int BC_id=ceil(examine.time/25.);

        TVector3 mazin_check(examine.strip_gpos.X(),examine.strip_gpos.Y(),examine.strip_gpos.Z());
        TVector3 athena_tru(examine.strip_gpos.X(),examine.strip_gpos.Y()-examine.truth_lpos.Y(),examine.strip_gpos.Z());
        if(m_par->dlm_new){
          athena_tru.SetX(examine.strip_gpos.X()-examine.strip_lpos.X());
        //       cerr<<"IT'S THE NEW TIME!"<<endl;
        }
        TVector3 athena_rec(examine.strip_gpos);
        //now store some variables BLC initializes; we might trim this down for efficiency later
        //the following line should be rather easy to one-to-one replace

        TVector3 truth(athena_tru.Y(),-athena_tru.X(),athena_tru.Z()), recon(athena_rec.Y(),-athena_rec.X(),athena_rec.Z());

        if(uvxxmod){
          xxuv_to_uvxx(truth,plane);xxuv_to_uvxx(recon,plane);
        }
        double charge = examine.charge;
        int strip = Get_Strip_ID(recon.X(),recon.Y(),plane), strip_pos=examine.strip_pos, station=examine.eta_station;  //theta_strip_id,module_y_center,plane); // true_x,true_y,plane);
        fstation=station;
        string schar=m_par->setup.substr(plane,1);
        strip=strip_number(station,plane,strip_pos);
        //diagnostics Stephen used (not needed ?)
        if(m_par->H<recon.Y()&&m_par->diag){
          double width=m_par->strip_width.getFloat(),base=m_par->ybases[plane][station-1].getFloat(),yhere=recon.Y(),xhere=truth.X(),msl=0;//recon.Y();//(schar.compare("x")==0?recon.Y():truth.Y());
          if(schar=="u"||schar=="v")width/=cos(TMath::DegToRad()*(m_par->stereo_degree.getFloat()));
          if(schar=="u"){
            msl=-tan(TMath::DegToRad()*(m_par->stereo_degree.getFloat()));
          }
          if(schar=="v"){
            msl=tan(TMath::DegToRad()*(m_par->stereo_degree.getFloat()));
          }
        }
        int VMM_chip = Get_VMM_chip(strip);

        //we're doing everything by the variable known as "athena_event" to reflect C++ vs MATLAB indexing
        int btime=(event+1)*10+(BC_id-1);
        particle_info.NUV_bg_preVMM = 0;   //examine.gtime;
        int special_time = examine.time + (event+1)*100;
        // Originally shaka
        hdst_entry hit_entry(event,examine.gtime,charge,VMM_chip,plane,strip_pos,station,tru_theta,tru_phi,true,btime,special_time,mazin_check,mazin_check);//truth,recon);//leave the rest of the info as 0's
        hit_info[hit_entry.entry_key()]=hit_entry;
    //     if(debug){ cout<<"Filling hit_info slot: "; hit_entry.entry_key().print();}
        keys.push_back(hit_entry.entry_key()); //may be only used when "incoherent background" is generated (not included for now)

      }//end entries loop

      //Did not include Stephen's generate bkg

      particle_info.N_hits_preVMM=hit_info.size();

      particle_info.N_hits_postVMM=0;
      unsigned int ir=0;

      //might want to move these somewhere smarter in future
      VMM_deadtime = 100;
      num_VMM_per_plane = 1000;
      VMM_chip_status=vector<vector<bool> >(num_VMM_per_plane,vector<bool>(8,true));
      VMM__chip_last_hit_time=vector<vector<int> >(num_VMM_per_plane,vector<int>(8,0));

      //*** FIGURE OUT WHAT TO DO WITH THE TIES--IS MIMIC VMM BUSTED? DO WE PLACE THE HIT REQUIREMENTS HERE? (PROBABLY)
      int xhit=0,uvhit=0,strip_X_tot=0,strip_UV_tot=0;
      vector<bool>plane_hit(m_par->setup.size(),false);

      for(map<hdst_key,hdst_entry>::iterator it=hit_info.begin(); it!=hit_info.end(); ++it){
        int plane=it->second.plane;
        plane_hit[plane]=true;
        particle_info.N_hits_postVMM++;
    //     if(Hits_Data_Set_Time.find(aegon)!=Hits_Data_Set_Time.end()) continue;
        Hits_Data_Set_Time[it->first]=it->second;
        if(m_par->setup.substr(plane,1).compare("x")==0){//if(debug)cout<<"ADD X STRIP VALUE "<<it->second.strip<<endl;
          strip_X_tot+=it->second.strip;
        }
        else{//if(debug)cout<<"ADD UV STRIP VALUE "<<it->second.strip<<endl;
          strip_UV_tot+=it->second.strip;
        }
    //     if(debug)cout<<"(hit"<<rae<<",pl"<<rhaegar[rae].plane<<")...";
      }//end map iterator loop

      for(unsigned int ipl=0;ipl<plane_hit.size();ipl++){
        if(plane_hit[ipl]){
          if(m_par->setup.substr(ipl,1)=="x") xhit++;
          else if(m_par->setup.substr(ipl,1)=="u"||m_par->setup.substr(ipl,1)=="v") uvhit++;
        }
      }
      if(xhit==4&&uvhit==4){
      //  cout<<"and so it's uv("<<strip_UV_tot<<") minus x("<<strip_X_tot<<")*0.25="<<0.25*(strip_UV_tot-strip_X_tot)<<endl;
        //m_diff_xuv[fstation-1]->Fill(0.25*(strip_UV_tot-strip_X_tot));
      }
      particle_info.N_X_hits=xhit;
      particle_info.N_UV_hits=uvhit;
      //X and UV hits minumum cut
      if(xhit<m_par->CT_x) particle_info.pass_cut=false;//return;
      if(uvhit<m_par->CT_uv) particle_info.pass_cut=false;//return;
      if(!particle_info.pass_cut) cout<<"event FAIL at CT cut"<<endl;
      //*** place any cuts on n_x, n_uv, n_postvmm here...



      //Moved the removing of some entries to the end of ~Trigger
      Event_Info[event]=particle_info;



      cout<<"Event "<<event<<" did "<<(particle_info.pass_cut?"":"(NOT) ")<<"pass cuts."<<endl;

      histVars = fillVars;

      //ATH_MSG_DEBUG( "fill_mmstrip_cache: end of processing" );
      }

  double MMLoadVariables::phi_shift(double athena_phi,std::string wedgeType, int stationPhi) const{
    float n = 2*(stationPhi-1);
    float index = stationPhi;
    std::cout << "BEFORE PHI " << athena_phi << " STATION " << stationPhi << std::endl;
    if(wedgeType=="Small") n+=1;
    float sectorPi = n*TMath::Pi()/8.;
    if(n>8) sectorPi = (16.-n)*TMath::Pi()/8.;
    std::cout << "N " << n << " SHIFT "<< sectorPi << std::endl;

    if(n<8)       return (athena_phi-sectorPi);
    else if(n==8) return (athena_phi + (athena_phi >= 0? -1:1)*sectorPi);
    else if(n>8)  return (athena_phi+sectorPi);
    else return athena_phi;

  }
  void MMLoadVariables::xxuv_to_uvxx(TVector3& hit,int plane)const{
    if(plane<4)return;
    else if(plane==4)hit_rot_stereo_bck(hit);//x to u
    else if(plane==5)hit_rot_stereo_fwd(hit);//x to v
    else if(plane==6)hit_rot_stereo_fwd(hit);//u to x
    else if(plane==7)hit_rot_stereo_bck(hit);//v to x
  }

  void MMLoadVariables::hit_rot_stereo_fwd(TVector3& hit)const{
    double degree=TMath::DegToRad()*(m_par->stereo_degree.getFloat());
    if(striphack) hit.SetY(hit.Y()*cos(degree));
    else{
      double xnew=hit.X()*cos(degree)+hit.Y()*sin(degree),ynew=-hit.X()*sin(degree)+hit.Y()*cos(degree);
      hit.SetX(xnew);hit.SetY(ynew);
    }
  }

  void MMLoadVariables::hit_rot_stereo_bck(TVector3& hit)const{
    double degree=-TMath::DegToRad()*(m_par->stereo_degree.getFloat());
    if(striphack) hit.SetY(hit.Y()*cos(degree));
    else{
      double xnew=hit.X()*cos(degree)+hit.Y()*sin(degree),ynew=-hit.X()*sin(degree)+hit.Y()*cos(degree);
      hit.SetX(xnew);hit.SetY(ynew);
    }
  }


  int MMLoadVariables::Get_Strip_ID(double X,double Y,int plane) const{  //athena_strip_id,module_y_center,plane)
    if(Y==-9999) return -1;
    string setup(m_par->setup);
    double strip_width=m_par->strip_width.getFloat(), degree=TMath::DegToRad()*(m_par->stereo_degree.getFloat());//,vertical_strip_width_UV = strip_width/cos(degree);
    double y_hit=Y;
    int setl=setup.length();
    if(plane>=setl||plane<0){
      cerr<<"Pick a plane in [0,"<<setup.length()<<"] not "<<plane<<endl; exit(1);
    }
  //   if(debug) cout<<"SUBSTR CALL MMT_L--2\n";
    string xuv=setup.substr(plane,1);
    if(xuv=="u"){//||xuv=="v"){
      if(striphack)return ceil(Y*cos(degree)/strip_width);
      y_hit = X*sin(degree)+Y*cos(degree);
    }
    else if(xuv=="v"){
      if(striphack)return ceil(Y*cos(degree)/strip_width);
      y_hit = -X*sin(degree)+Y*cos(degree);
      cout<<"-X*sin("<<degree<<")+Y*cos(degree) is"<<-X*sin(degree)+Y*cos(degree)<<endl;
    }
    else if(xuv!="x"){
      cerr<<"Invalid plane option " << xuv << endl; exit(2);
    }
    double strip_hit = ceil(y_hit*1./strip_width);
    cout <<"(y_hit="<<y_hit<<"), "<< strip_hit<<endl;
    return strip_hit;
  }

  int MMLoadVariables::Get_VMM_chip(int strip) const{  //Not Finished... Rough
    int strips_per_VMM = 64;
    return ceil(1.*strip/strips_per_VMM);
  }

  int MMLoadVariables::strip_number(int station,int plane,int spos)const{
    //assert(station>0&&station<=m_par->n_stations_eta);
    //assert(plane>=0&&plane<(int)m_par->setup.size());
    if (station<=0||station>m_par->n_stations_eta) {
      int base_strip = 0;

      return base_strip;
    }
    if (plane<0||plane>(int)m_par->setup.size()) {
      int base_strip = 0;

      return base_strip;
    }
    bool do_auto=false;
    //if true do strip # (ceil(Y/strip_width); what's currently fed into the algorithm)  calculation based on evenly spaced eta assumption of stations
    double H=m_par->H.getFloat()/*,h=m_par->h1,z=m_par->z_nominal[plane],z0=m_par->z_nominal.front()*/,ybase=m_par->ybases[plane][station-1].getFloat();
    if(do_auto){
      //-log(tan(0.5(atan(y/z))))=eta
      //this is the even y spacing
      if(m_par->dlm_new) ybase=H+1100.*(station-1);
      else ybase=H+950.*(station-1);
      /*//this is the even eta spacing version
      double etalo=-log(tan(0.5*atan((h+H)/z))),etahi=-log(tan(0.5*atan(H/z))),inc=(etahi-etalo)/m_par->n_stations_eta;
      double this_eta=etalo+inc*(station-1);
      ybase=z*tan(2*atan(exp(-1.*this_eta)));
      */
    }
    double width=m_par->strip_width.getFloat(); string plane_char=m_par->setup.substr(plane,1);
  //   if(plane_char.compare("u")==0||plane_char.compare("v")==0) width/=cos(TMath::DegToRad()*(m_par->stereo_degree));
    int base_strip=/*ceil(ybase/width)+*/spos;
    return base_strip;
  }
  std::string MMLoadVariables::getWedgeType(const MmDigitContainer *nsw_MmDigitContainer){
    std::vector<bool> isLargeWedge;
    //Digit loop to match to truth
    for(auto dit : *nsw_MmDigitContainer) {

      const MmDigitCollection* coll = dit;
      for (unsigned int item=0; item<coll->size(); item++) {

        const MmDigit* digit = coll->at(item);
        Identifier Id = digit->identify();

          std::string stName   = m_MmIdHelper->stationNameString(m_MmIdHelper->stationName(Id));
          string sname(stName);
          if (sname.compare("MML")==0)isLargeWedge.push_back(true);
          else isLargeWedge.push_back(false);
      }
    }
    bool allLarge = true;
    bool allSmall = true;
    for(int i=0; i<isLargeWedge.size(); i++){
      if (isLargeWedge.at(i)) allSmall = false;
      else allLarge = false;
    }
    std::string wedgeType = "Neither";
    if (allLarge) wedgeType = "Large";
    if (allSmall) wedgeType = "Small";
    return wedgeType;
  }

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Athena/Gaudi includes
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/IIncidentSvc.h"

// local includes
#include "TrigT1NSWSimTools/StripSegmentTool.h"
#include "TrigT1NSWSimTools/StripOfflineData.h"
#include "TrigT1NSWSimTools/tdr_compat_enum.h"

//Event info includes
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

// Muon software includes
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "MuonDigitContainer/sTgcDigit.h"
#include "MuonSimData/MuonSimDataCollection.h"
#include "MuonSimData/MuonSimData.h"
// random numbers
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"

// local includes
#include "TTree.h"
#include "TVector3.h"

#include <functional>
#include <algorithm>
#include <map>
#include <utility>
#include <math.h>      

namespace NSWL1 {

    StripSegmentTool::StripSegmentTool( const std::string& type, const std::string& name, const IInterface* parent) :
      AthAlgTool(type,name,parent),
      m_incidentSvc("IncidentSvc",name),
      m_rndmSvc("AtRndmGenSvc",name),
      m_rndmEngine(0),
      m_detManager(0),
      m_sTgcIdHelper(0),
      m_tree(0)

    {
      declareInterface<NSWL1::IStripSegmentTool>(this);

      declareProperty("DoNtuple", m_doNtuple = true, "input the StripTds branches into the analysis ntuple"); 
      declareProperty("sTGC_SdoContainerName", m_sTgcSdoContainer = "sTGC_SDO", "the name of the sTGC SDO container");
      declareProperty("NSWTrigRDOContainerName", m_trigRdoContainer = "NSWTRGRDO");

    }

    StripSegmentTool::~StripSegmentTool() {

      // Clear Ntuple variables
      //      if(m_cl_charge) delete m_cl_charge;
      //if(m_cl_eta) delete m_cl_eta;
      //if(m_cl_phi) delete m_cl_phi;


    }

 
  
  StatusCode StripSegmentTool::initialize() {
    ATH_MSG_INFO( "initializing " << name() ); 
    
    ATH_MSG_INFO( name() << " configuration:"); 
    ATH_CHECK( m_trigRdoContainer.initialize() );   
    
    
      const IInterface* parent = this->parent();
      const INamedInterface* pnamed = dynamic_cast<const INamedInterface*>(parent);
      std::string algo_name = pnamed->name();

      if ( m_doNtuple && algo_name=="NSWL1Simulation" ) {
        ITHistSvc* tHistSvc;
        StatusCode sc = service("THistSvc", tHistSvc);
        if(sc.isFailure()) {
          ATH_MSG_FATAL("Unable to retrieve THistSvc");
          return sc;
        }

        char ntuple_name[40]={'\0'};
        //memset(ntuple_name,'\0',40*sizeof(char));
        sprintf(ntuple_name,"%sTree",algo_name.c_str());

        m_tree = 0;
        sc = tHistSvc->getTree(ntuple_name,m_tree);

        if (sc.isFailure()) {
          ATH_MSG_FATAL("Could not retrieve the analysis ntuple from the THistSvc");
          return sc;
        } else {
          ATH_MSG_INFO("Analysis ntuple succesfully retrieved");
          sc = this->book_branches();
          if (sc.isFailure()) {
            ATH_MSG_ERROR("Cannot book the branches for the analysis ntuple");
          }
        }

      }


      // retrieve the Incident Service
      if( m_incidentSvc.retrieve().isFailure() ) {
        ATH_MSG_FATAL("Failed to retrieve the Incident Service");
        return StatusCode::FAILURE;
      } else {
        ATH_MSG_INFO("Incident Service successfully rertieved");
      }
      m_incidentSvc->addListener(this,IncidentType::BeginEvent);

      // retrieve the Random Service
      if( m_rndmSvc.retrieve().isFailure() ) {
        ATH_MSG_FATAL("Failed to retrieve the Random Number Service");
        return StatusCode::FAILURE;
      } else {
        ATH_MSG_INFO("Random Number Service successfully retrieved");
      }

      //  retrieve the MuonDetectormanager
      if( detStore()->retrieve( m_detManager ).isFailure() ) {
        ATH_MSG_FATAL("Failed to retrieve the MuonDetectorManager");
        return StatusCode::FAILURE;
      } else {
        ATH_MSG_INFO("MuonDetectorManager successfully retrieved");
      }

      //  retrieve the sTGC offline Id helper
      if( detStore()->retrieve( m_sTgcIdHelper ).isFailure() ){
        ATH_MSG_FATAL("Failed to retrieve sTgcIdHelper");
        return StatusCode::FAILURE;
      } else {
        ATH_MSG_INFO("sTgcIdHelper successfully retrieved");
      }
 
      return StatusCode::SUCCESS;
    }

    void StripSegmentTool::handle(const Incident& inc) {
      if( inc.type()==IncidentType::BeginEvent ) {
        this->reset_ntuple_variables();
      }
    }

  
    StatusCode StripSegmentTool::find_segments(std::vector< std::unique_ptr<StripClusterData> >& clusters){
      
      SG::WriteHandle<Muon::NSW_TrigRawDataContainer> trgRdos (m_trigRdoContainer);
       auto recordp=std::make_unique<Muon::NSW_TrigRawDataContainer>();
       ATH_CHECK( trgRdos.record( std::move(recordp)));
      //auto p=std::make_unique<Muon::NSW_TrigRawDataContainer>();
      
      
      //TODO : put  sector Id and BCID in the ctor of NSW_TrigRawData below
      Muon::NSW_TrigRawData trgRawData;//like vector<trigger segment>
      
      std::map<int, std::vector<std::unique_ptr<StripClusterData>>[2] > cluster_map; // gather clusters by bandID and seperate in wedge


      for(  auto& cl : clusters){
	    auto item =cluster_map.find(cl->bandId());
	    if (item != cluster_map.end()){
	      item->second[cl->wedge()-1].push_back(std::move(cl));
	    }
	    else{
	      cluster_map[cl->bandId()][cl->wedge()-1].push_back(std::move(cl));
	    }
      }
      
      for(const auto& band : cluster_map){//main cluster loop
	    
          
          
       int bandId=band.first;
       if (band.second[0].size() == 0){
           ATH_MSG_WARNING("Cluster size is zero for inner wedge trg with bandId "<<bandId<<"...skipping");
           continue;
           
      }
      
      if(band.second[1].size() == 0){
           ATH_MSG_WARNING("Cluster size is zero for outer wedge trg with bandId "<<bandId<<"...skipping");
           continue;
      }
        int phiId=band.second[0].at(0)->phiId();
	    float glx1=0;
	    float gly1=0;
        float glx2=0;
        float gly2=0;
        float glx=0;
        float gly=0;
	    float phi=0;
	    float eta=0;
	    float charge1=0;
        float charge2=0;

	    //first measuement
	    float r1=0;
	    float z1=0;
	    for( const auto& cl : band.second[0] ){//inner
	      r1+=sqrt(pow(cl->globX()*cl->charge(),2)+pow(cl->globY()*cl->charge(),2));
	      z1+=cl->globZ()*cl->charge();
	      glx1+=cl->globX()*cl->charge();
	      gly1+=cl->globY()*cl->charge();
	      charge1+=cl->charge();
	    }

	    //first measuement
	    float r2=0;
	    float z2=0;
	    for( const auto& cl : band.second[1] ){//outer
            r2+=sqrt(pow(cl->globX()*cl->charge(),2)+pow(cl->globY()*cl->charge(),2));
            z2+=cl->globZ()*cl->charge();
            glx2+=cl->globX()*cl->charge();
            gly2+=cl->globY()*cl->charge();
            charge2+=cl->charge();
        }
        if(charge1!=0){
            r1=r1/charge1;
            z1=z1/charge1;
            glx1=glx1/charge1;
            gly1=gly1/charge1;
        }
        if(charge2!=0){
            r2=r2/charge2;
            z2=z2/charge2;
            glx2=glx2/charge2;
            gly2=gly2/charge2;
        }
        glx=(glx1+glx2)/2.;
        gly=(gly1+gly2)/2.;
        float slope=(r2-r1)/(z2-z1);
        float avg_r=(r1+r2)/2.;//S.I is it possible in the Hardware implementation?
        float avg_z=(z1+z2)/2.;
        float inf_slope=(avg_r/avg_z);
        //float dR=slope-inf_slope;
        float theta_inf=atan(inf_slope);
        float theta=atan(slope);
        float dtheta=(theta_inf-theta)*1000;//In Milliradian
        if(avg_z>0){
            eta=-log(tan(theta/2));
        }
        else if(avg_z<0){
            eta=log(tan(-theta/2));
        }
        else{
            ATH_MSG_ERROR("Segment Global Z at IP");
        }
        
        if(glx>=0 && gly>=0){
            phi=atan(gly/glx);
        }
        else if(glx<0 && gly>0){
            phi=PI-atan(abs(gly/glx));
        }
        else if(glx<0 && gly<0){
            phi=-1*PI+atan(gly/glx);
        }
        else if(glx>0 && gly<0){
            phi=-atan(abs(gly/glx));
        }
        else{
            ATH_MSG_ERROR("Unexpected error, global x or global y are not a number");
        }
        //R index calculation below :  needs to be replaced by a generic solution. Probably theres a tool to access these boundaries
        static std::vector<float> bw_roi_boundaries={
                //FWD
                2.4148,
                2.3777,
                2.3439,
                2.3100,
                2.2755,
                2.2417,
                2.2070,
                2.1734,
                2.1411,
                2.1088,
                2.0781,
                2.0476,
                2.0166,
                1.9855,
                1.9537,
                1.9214,
                1.9163,
                //EC
                1.9172,
                1.8932,
                1.8679,
                1.8420,
                1.8153,
                1.7882,
                1.7608,
                1.7328,
                1.7050,
                1.6781,
                1.6524,
                1.6274,
                1.6038,
                1.5810,
                1.5591,
                1.5385,
                1.5187,
                1.4997,
                1.4811,
                1.4622,
                1.4428,
                1.4227,
                1.4015,
                1.3783,
                1.3528,
                1.3257,
                1.2972,
                1.2681,
                1.2384,
                1.2086,
                1.1798,
                1.1528,
                1.1275,
                1.1036,
                1.0807,
                1.0587,
                1.0375,
                1.0334,
        };
        //just to make sure sort eta boundaries.
        std::sort(bw_roi_boundaries.begin(),bw_roi_boundaries.end());//or in the reverse order depends on what people want
        if(fabs(dtheta)>15) return StatusCode::SUCCESS;//it seems to be the most optimistic hw scenario . However it needs to be kept an eye on... will be something in between 7 and 15 mrad needs to be decided by hw ppl
        
        static auto calc_rIndex=[](const float &eta){
            for(unsigned int i=0;i<bw_roi_boundaries.size();i++){
                if(i==bw_roi_boundaries.size()-1) return 0;
                float eta_b0=bw_roi_boundaries.at(i);
                float eta_b1=bw_roi_boundaries.at(i+1);
                if(eta>eta_b0 && eta<eta_b1) return (int)(i+1);
            }
            
            return 0;
        };
        uint8_t rIndex=(uint8_t)calc_rIndex(eta);
        
        m_seg_wedge1_size->push_back(band.second[0].size());
        m_seg_wedge2_size->push_back(band.second[1].size());

        m_seg_bandId->push_back(bandId);
        m_seg_phiId->push_back(phiId);
        m_seg_rIdx->push_back(rIndex);
        m_seg_theta->push_back(theta);
        m_seg_dtheta->push_back(dtheta);
        m_seg_eta->push_back(eta);
        m_seg_phi->push_back(phi);
        m_seg_global_r->push_back(avg_r); 
        m_seg_global_x->push_back(glx);
        m_seg_global_y->push_back(gly); 
        m_seg_global_z->push_back(avg_z); 
        m_seg_dir_r->push_back(slope); 
        m_seg_dir_y->push_back(-99); 
        m_seg_dir_z->push_back(-99); 
       
        
        
       //Use these only for salt.... True values will be provided later 
       bool phiRes=true;
       bool lowRes=false;//we do not have singlewedge trigger. not implemented in the HW. yet  so lowres is always false for now
       uint8_t phiIndex=(uint8_t)phiId;
       uint8_t deltaTheta=uint8_t(dtheta);
       ATH_MSG_DEBUG("dTheta="<<dtheta<<" phiIndex="<<phiIndex<<" lowRes="<<lowRes<<" phiRes="<<phiRes);
       //S.I As far as I understand memory is handled by the DataVector so we shoul not delete the pointer
       auto* rdo_segment= new Muon::NSW_TrigRawDataSegment( deltaTheta,  phiIndex,  rIndex, lowRes,  phiRes);      
       trgRawData.push_back(rdo_segment);
     
     }//end of clmap loop
     
    

      return StatusCode::SUCCESS;
    }

    StatusCode StripSegmentTool::book_branches() {
      // m_cl_n= 0;
      // m_cl_charge = new std::vector< int >();

      m_seg_theta = new std::vector< float >();    
      m_seg_dtheta = new std::vector< float >();
      m_seg_eta = new std::vector< float >();   
      m_seg_phi = new std::vector< float >();
      m_seg_global_r = new std::vector< float >();
      m_seg_global_x = new std::vector< float >();
      m_seg_global_y = new std::vector< float >();
      m_seg_global_z = new std::vector< float >();
      m_seg_dir_r = new std::vector< float >();
      m_seg_dir_y = new std::vector< float >();
      m_seg_dir_z = new std::vector< float >();
      m_seg_bandId = new std::vector< int >();
      m_seg_phiId = new std::vector< int >();
      m_seg_rIdx=new std::vector< int >();
      m_seg_wedge1_size = new std::vector< int >();
      m_seg_wedge2_size = new std::vector< int >();

       if (m_tree) {
	     std::string ToolName = name().substr(  name().find("::")+2,std::string::npos );
         const char* n = ToolName.c_str();
         m_tree->Branch(TString::Format("%s_seg_theta",n).Data(),&m_seg_theta);
         m_tree->Branch(TString::Format("%s_seg_dtheta",n).Data(),&m_seg_dtheta);

         m_tree->Branch(TString::Format("%s_seg_eta",n).Data(),&m_seg_eta);
         m_tree->Branch(TString::Format("%s_seg_phi",n).Data(),&m_seg_phi);

         m_tree->Branch(TString::Format("%s_seg_global_r",n).Data(),&m_seg_global_r);
         m_tree->Branch(TString::Format("%s_seg_global_x",n).Data(),&m_seg_global_x);
         m_tree->Branch(TString::Format("%s_seg_global_y",n).Data(),&m_seg_global_y);
         m_tree->Branch(TString::Format("%s_seg_global_z",n).Data(),&m_seg_global_z);


         m_tree->Branch(TString::Format("%s_seg_dir_r",n).Data(),&m_seg_dir_r);
         m_tree->Branch(TString::Format("%s_seg_dir_y",n).Data(),&m_seg_dir_y);
         m_tree->Branch(TString::Format("%s_seg_dir_z",n).Data(),&m_seg_dir_z);
         m_tree->Branch(TString::Format("%s_seg_bandId",n).Data(),&m_seg_bandId);
         m_tree->Branch(TString::Format("%s_seg_phiId",n).Data(),&m_seg_phiId);
         m_tree->Branch(TString::Format("%s_seg_rIdx",n).Data(),&m_seg_rIdx);
         m_tree->Branch(TString::Format("%s_seg_wedge1_size",n).Data(),&m_seg_wedge1_size);
         m_tree->Branch(TString::Format("%s_seg_wedge2_size",n).Data(),&m_seg_wedge2_size);


       }
      // else { 
      //    return StatusCode::FAILURE;
      // }
      return StatusCode::SUCCESS;
    }



    void StripSegmentTool::reset_ntuple_variables() {
      // if ntuple is not booked nothing to do
      if ( m_tree==0 ) return;
      //reset the ntuple variables
      clear_ntuple_variables();
    }

    void StripSegmentTool::clear_ntuple_variables() {
      m_seg_theta->clear();    
      m_seg_dtheta->clear();
      m_seg_eta->clear();   
      m_seg_phi->clear();
      m_seg_global_r->clear();
      m_seg_global_x->clear();
      m_seg_global_y->clear();
      m_seg_global_z->clear();
      m_seg_dir_r->clear();
      m_seg_dir_y->clear();
      m_seg_dir_z->clear();
      m_seg_bandId->clear();
      m_seg_phiId->clear();
      m_seg_rIdx->clear();
      m_seg_wedge2_size->clear();
      m_seg_wedge1_size->clear();
  }


 




}

//  LocalWords:  pos lpos

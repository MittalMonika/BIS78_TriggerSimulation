/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "InDetVKalVxInJetTool/InDetVKalVxInJetTool.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "VxSecVertex/VxSecVKalVertexInfo.h"
#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"
#include "TrkToolInterfaces/ITrackParticleCreatorTool.h"
#include "GaudiKernel/ITHistSvc.h"
#include "TMath.h"
//
//-------------------------------------------------
// Other stuff
#include "GaudiKernel/ToolFactory.h"
//#include "GaudiKernel/MsgStream.h"
//
#include<iostream>

//extern "C" {
//  float chisin_(const float & prob, const long int& ndf);             
//}

namespace InDet {
//
//Constructor-------------------------------------------------------------- 
InDetVKalVxInJetTool::InDetVKalVxInJetTool(const std::string& type,
                                           const std::string& name,
                                           const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_CutSctHits(4),
    m_CutPixelHits(1),
    m_CutSiHits(7),
    m_CutBLayHits(0),
    m_CutSharedHits(1),
    m_CutPt(700.),
    m_CutZVrt(25.),
    m_CutA0(5.),
    m_CutChi2(3.),
    m_SecTrkChi2Cut(10.),
    m_ConeForTag(0.4),
    m_Sel2VrtChi2Cut(4.5),
    m_Sel2VrtSigCut(3.0),
    m_Sel2VrtSigNTrkDep(0.4),
    m_TrkSigCut(2.0),
    m_TrkSigSumCut(1.),
    m_TrkSigSumNTrkDep(0.2),
    m_A0TrkErrorCut(1.0),
    m_ZTrkErrorCut(5.0),
    m_AntiPileupSigRCut(2.0),
    m_AntiPileupSigZCut(6.0),
    m_AntiFake2trVrtCut(0.5),
    m_JetPtFractionCut(0.01),
    m_FillHist(false),
    m_RobustFit(5),
    m_Xbeampipe (0.),
    m_Ybeampipe (0.),
    m_XlayerB (0.),
    m_YlayerB (0.),
    m_Xlayer1 (0.),
    m_Ylayer1 (0.),
    m_Xlayer2 (0.),
    m_Ylayer2 (0.),
    m_Rbeampipe (29.4),    
    m_RlayerB (50.5),
    m_Rlayer1 (88.0),
    m_Rlayer2 (123.0),
//    m_Rbeampipe (25.4), //IBL
//    m_RlayerB (34.0),   //IBL
//    m_Rlayer1 (50.5),   //IBL
//    m_Rlayer2 (88.0),   //IBL
    m_MassType (1),
    m_MultiVertex(false),
    m_MultiWithPrimary(false),
    m_getNegativeTail(false),
    m_getNegativeTag(false),
    m_MultiWithOneTrkVrt(true),
    m_VertexMergeCut(3.),
    m_TrackDetachCut(6.),
    m_fitterSvc("Trk::TrkVKalVrtFitter/VertexFitterTool",this),
    m_trackToVertexIP("Trk::TrackToVertexIPEstimator/TrackToVertexIPEstimator"),
    m_trkPartCreator("Trk::TrackParticleCreatorTool/InDetParticleCreatorTool")
//    m_fitSvc("Trk::TrkVKalVrtFitter/VKalVrtFitter",this)
   {
//
// Declare additional interface
//
    declareInterface< ISecVertexInJetFinder >(this);
// Properties
//
//
    declareProperty("CutSctHits",    m_CutSctHits ,  "Remove track is it has less SCT hits" );
    declareProperty("CutPixelHits",  m_CutPixelHits, "Remove track is it has less Pixel hits");
    declareProperty("CutSiHits",     m_CutSiHits,    "Remove track is it has less Pixel+SCT hits"  );
    declareProperty("CutBLayHits",   m_CutBLayHits,  "Remove track is it has less B-layer hits"   );
    declareProperty("CutSharedHits", m_CutSharedHits,"Reject final 2tr vertices if tracks have shared hits" );

    declareProperty("CutPt",         m_CutPt,     "Track Pt selection cut"  );
    declareProperty("CutA0",         m_CutA0,     "Track A0 selection cut"  );
    declareProperty("CutZVrt",       m_CutZVrt,   "Track Z impact selection cut"   );
    declareProperty("ConeForTag",    m_ConeForTag,"Cone around jet direction for track selection"  );
    declareProperty("CutChi2",       m_CutChi2,   "Track Chi2 selection cut" );
    declareProperty("TrkSigCut",     m_TrkSigCut, "Track 3D impact significance w/r primary vertex" );
    declareProperty("TrkSigSumCut",  m_TrkSigSumCut, "Sum of 3D track significances cut for 2tr vertex search" );
    declareProperty("TrkSigSumNTrkDep", m_TrkSigSumNTrkDep, "NTrack in jet dependent increase of TrkSigSumCut " );
    declareProperty("SecTrkChi2Cut", m_SecTrkChi2Cut,"Track - common secondary vertex association cut. Single Vertex Finder only" );

    declareProperty("A0TrkErrorCut",  m_A0TrkErrorCut, "Track A0 error cut" );
    declareProperty("ZTrkErrorCut",   m_ZTrkErrorCut,  "Track Z impact error cut" );

    declareProperty("Sel2VrtChi2Cut",    m_Sel2VrtChi2Cut, "Cut on Chi2 of 2-track vertex for initial selection"  );
    declareProperty("Sel2VrtSigCut",     m_Sel2VrtSigCut,  "Cut on significance of 3D distance between initial 2-track vertex and PV"  );
    declareProperty("Sel2VrtSigNTrkDep", m_Sel2VrtSigNTrkDep, "NTrack in jet dependent increase of Sel2VrtSigCut");
    declareProperty("AntiPileupSigRCut",   m_AntiPileupSigRCut, "Remove tracks with low Rphi and big Z impacts presumably coming from pileup"  );
    declareProperty("AntiPileupSigZCut",   m_AntiPileupSigZCut, "Remove tracks with low Rphi and big Z impacts presumably coming from pileup"  );
    declareProperty("AntiFake2trVrtCut",   m_AntiFake2trVrtCut, "Cut to reduce fake 2-track vertices contribution.Single Vertex Finder only"  );
    declareProperty("JetPtFractionCut",    m_JetPtFractionCut,  "Reduce high Pt fakes. Jet HLV input is mandatory, direction is not enough. Multi and single vertex versions are affected"  );

    declareProperty("FillHist",   m_FillHist, "Fill technical histograms"  );

    declareProperty("RobustFit",  m_RobustFit, "Use vertex fit with RobustFit functional(VKalVrt) for common secondary vertex fit" );

    declareProperty("Xbeampipe", m_Xbeampipe);
    declareProperty("Ybeampipe", m_Ybeampipe);
    declareProperty("XlayerB",   m_XlayerB  );
    declareProperty("YlayerB",   m_YlayerB  );
    declareProperty("Xlayer1",   m_Xlayer1  );
    declareProperty("Ylayer1",   m_Ylayer1  );
    declareProperty("Xlayer2",   m_Xlayer2  );
    declareProperty("Ylayer2",   m_Ylayer2  );
    declareProperty("Rbeampipe", m_Rbeampipe);
    declareProperty("RlayerB",   m_RlayerB  );
    declareProperty("Rlayer1",   m_Rlayer1  );
    declareProperty("Rlayer2",   m_Rlayer2  );

    declareProperty("MassType",  m_MassType, "Type of vertex mass returned by finder. Single Vertex Finder only!" );
    declareProperty("MultiVertex",        m_MultiVertex,       "Run Multiple Secondary Vertices in jet finder"  );
    declareProperty("MultiWithPrimary",   m_MultiWithPrimary,  "Find Multiple Secondary Vertices + primary vertex in jet. MultiVertex Finder only!"  );
    declareProperty("MultiWithOneTrkVrt", m_MultiWithOneTrkVrt,"Allow one-track-vertex addition to already found secondary vertices. MultiVertex Finder only! ");
    declareProperty("getNegativeTail", m_getNegativeTail, "Allow secondary vertex behind the primary one (negative) w/r jet direction (not for multivertex!)" );
    declareProperty("getNegativeTag",  m_getNegativeTag,  "Return ONLY negative secondary vertices (not for multivertex!)"   );

    declareProperty("VertexMergeCut",	  m_VertexMergeCut, "To allow vertex merging for MultiVertex Finder" );
    declareProperty("TrackDetachCut",	  m_TrackDetachCut, "To allow track from vertex detachment for MultiVertex Finder" );

    declareProperty("VertexFitterTool", m_fitterSvc);
    declareProperty("TrackToVertexTool", m_trackToVertexIP);
    declareProperty("TrackParticleCreator", m_trkPartCreator);
//    declareProperty("TrkVKalVrtFitter", m_fitSvc);
//
    m_iflag=0;
    m_massPi  = 139.5702 ;
    m_massP   = 938.272  ;
    m_massE   =   0.511  ;
    m_massK0  = 497.648  ;
    m_massLam =1115.683  ;
    m_WorkArray = 0;

   }

//Destructor---------------------------------------------------------------
    InDetVKalVxInJetTool::~InDetVKalVxInJetTool(){
     //MsgStream log( msgSvc(), name() ) ;
     //log << MSG::DEBUG << "InDetVKalVxInJetTool destructor called" << endreq;
     if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<< "InDetVKalVxInJetTool destructor called" << endreq;
     if(m_WorkArray) delete m_WorkArray;
   }

//Initialize---------------------------------------------------------------
   StatusCode InDetVKalVxInJetTool::initialize(){
     //MsgStream log( msgSvc(), name() ) ;
     //log << MSG::DEBUG << "InDetVKalVxInJetTool initialize() called" << endreq;
     if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<< "InDetVKalVxInJetTool initialize() called" << endreq;
     m_WorkArray = new VKalVxInJetTemp;

//     IToolSvc* toolSvc;                    //needed for old style TrkVKalVrtFitter retrieval
//     StatusCode sc = service("ToolSvc",toolSvc);
//     if (sc.isFailure()) { 
//        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << " No ToolSvc for InDetVKalVxInJetTool !" << endreq;
//        return StatusCode::SUCCESS;
//     }

 
     if (m_fitterSvc.retrieve().isFailure()) {
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "Could not find Trk::TrkVKalVrtFitter" << endreq;
        return StatusCode::SUCCESS;
     } else {
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "InDetVKalVxInJetTool TrkVKalVrtFitter found" << endreq;
     }
     m_fitSvc = dynamic_cast<Trk::TrkVKalVrtFitter*>(&(*m_fitterSvc));
     if(!m_fitSvc){
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<<" No implemented Trk::ITrkVKalVrtFitter interface" << endreq;
        return StatusCode::SUCCESS;
     }
//------
     if ( m_trackToVertexIP.retrieve().isFailure() ) {
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<< "Failed to retrieve trackToVertexIPEstimator tool. Used for tests only, so safe!" << endreq;
     } else {
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "Retrieved Trk::TrackToVertexIPEstimator tool. Only for tests!" << m_trackToVertexIP<<endreq;
     }
//-------
     if(m_MultiVertex && m_MultiWithOneTrkVrt) {
       if ( m_trkPartCreator.retrieve().isFailure() ) {
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<< "Failed to retrieve TrackParticleCreator tool " << endreq;
       } else {
        if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "Retrieved Trk::TrackParticleCreator tool" << m_trkPartCreator<<endreq;
       }
     }
//-------
//     Trk::IVertexFitter * tmp;
//     sc = toolSvc->retrieveTool("Trk::TrkVKalVrtFitter",tmp,this);
//     if (sc.isFailure()) { 
//        log << MSG::DEBUG << " No Trk::TrkVKalVrtFitter for InDetVKalVxInJetTool !" << endreq;
//     }else{
//        m_fitSvc = dynamic_cast<Trk::TrkVKalVrtFitter*>(tmp);
//        if(!m_fitSvc)log<<MSG::DEBUG<<" No Trk::TrkVKalVrtFitter" << endreq;
//     }



     ITHistSvc*     hist_root=0;
     if(m_FillHist){

       StatusCode sc = service( "THistSvc", hist_root); 
       if( sc.isFailure() ) {
          if(msgLvl(MSG::ERROR))msg(MSG::ERROR)<< "Could not find THistSvc service" << endreq;
       }
       if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "InDetVKalVxInJetTool Histograms found" << endreq;
 
       m_hb_massPiPi   = new TH1D("massPiPi"," massPiPi",160,200., 1000.);
       m_hb_massPPi    = new TH1D("massPPi"," massPPi", 100,1000., 1250.);
       m_hb_massEE     = new TH1D("massEE"," massEE", 100,0., 200.);
       m_hb_nvrt2      = new TH1D("nvrt2"," vertices2", 50,0., 50.);
       m_hb_ratio      = new TH1D("ratio"," ratio", 51,0., 1.02);
       m_hb_totmass    = new TH1D("totmass"," totmass", 250,0., 10000.);
       m_hb_totmassEE  = new TH1D("massEEcomvrt"," totmass EE common vertex", 100,0., 1000.);
       m_hb_totmass2T0 = new TH1D("mass2trcomvrt0"," totmass 2tr common vertex", 800,0., 4000.);
       m_hb_totmass2T1 = new TH1D("mass2trcomvrt1"," totmass 2tr common vertex", 100,0., 4000.);
       m_hb_totmass2T2 = new TH1D("mass2trcomvrt2"," totmass 2tr common vertex", 100,0., 4000.);
       m_hb_impact     = new TH1D("impact", " impact", 100,0., 20.);
       m_hb_impactR    = new TH1D("impactR"," impactR", 100,-30., 70.);
       m_hb_impactZ    = new TH1D("impactZ"," impactZ", 100,-30., 70.);
       m_hb_impactRZ   = new TH2D("impactRZ"," impactRZ", 40,-10., 10., 60, -30.,30. );
       m_hb_r2d        = new TH1D("r2interact"," interaction", 150,0., 150.);
       m_hb_impV0      = new TH1D("impactV0"," V0 impact", 100,0., 50.);
       m_hb_ntrkjet    = new TH1D("ntrkjet"," NTrk in jet", 50,0., 50.);
       m_hb_jmom       = new TH1D("jetmom"," Jet mom", 200,0., 1000000.);
       m_hb_mom        = new TH1D("jetmomvrt"," Jet mom with sec. vertex", 200,0., 1000000.);
       m_hb_signif3D   = new TH1D("signif3D"," Signif3D for initial 2tr vertex", 140,-20., 50.);
       m_hb_massPiPi1  = new TH1D("massPiPi1"," massPiPi",100,200., 800.);
       m_hb_massPiPi2  = new TH1D("massPiPi2"," massPiPi",100,0., 4000.);
       m_hb_sig3DTot   = new TH1D("sig3dcommon"," Signif3D for common vertex", 140,-20., 50.);
       m_hb_sig3D2tr   = new TH1D("sig3D2tr","Signif2D for 2tr single vertex", 140,-20., 50.);
       m_hb_goodvrtN   = new TH1F("goodvrtN","Number of good vertices", 20,0., 20.);
       m_hb_distVV     = new TH1D("distvv","Vertex-Vertex dist", 100,0., 20.);
       m_hb_diffPS     = new TH1D("diffPS","Primary-Secondary assoc", 200,-20., 20.);
       m_hb_trkPtMax   = new TH1D("trkPtMax","Maximal track Pt to jet", 100, 0., 10000.);
       m_hb_tr2SelVar  = new TH1D("tr2SelVar","New 2tr variable", 200, 0., 10.);
       m_hb_blshared   = new TH1F("blshared","Number of shared hits in B-layer for R<BL", 5, 0., 5.);
       m_hb_pxshared   = new TH1F("pxshared","Number of shared hits in pixel for R>BL", 5, 0., 5.);
       m_hb_addRatioMV = new TH1F("addRatioMV","Distance ratio for added vertices multivertex case", 100, 0., 2.5);
       m_hb_addChi2MV  = new TH1F("addChi2MV","Chi2 for added vertices multivertex case", 100, 0., 10.);
       m_hb_addNVrtMV  = new TH1F("addNVrtMV","N of added vertices multivertex case", 10, 0., 10.);
       m_pr_effVrt2tr  = new TProfile("effVrt2tr"," 2tr vertex efficiency vs track", 50, 0., 50.);
       std::string histDir;
       if(m_MultiVertex) histDir="/file1/stat/MSVrtInJet/";
       else              histDir="/file1/stat/SVrtInJet/";
       sc = hist_root->regHist(histDir+"massPiPi", m_hb_massPiPi);
       sc = hist_root->regHist(histDir+"massPPi", m_hb_massPPi);
       sc = hist_root->regHist(histDir+"massEE", m_hb_massEE );
       sc = hist_root->regHist(histDir+"nvrt2", m_hb_nvrt2);
       sc = hist_root->regHist(histDir+"ratio", m_hb_ratio);
       sc = hist_root->regHist(histDir+"totmass", m_hb_totmass);
       sc = hist_root->regHist(histDir+"massEEcomvrt", m_hb_totmassEE);
       sc = hist_root->regHist(histDir+"mass2trcomvrt0", m_hb_totmass2T0);
       sc = hist_root->regHist(histDir+"mass2trcomvrt1", m_hb_totmass2T1);
       sc = hist_root->regHist(histDir+"mass2trcomvrt2", m_hb_totmass2T2);
       sc = hist_root->regHist(histDir+"impact", m_hb_impact);
       sc = hist_root->regHist(histDir+"impactR", m_hb_impactR);
       sc = hist_root->regHist(histDir+"impactZ", m_hb_impactZ);
       sc = hist_root->regHist(histDir+"impactRZ", m_hb_impactRZ);
       sc = hist_root->regHist(histDir+"r2interact", m_hb_r2d);
       sc = hist_root->regHist(histDir+"impactV0", m_hb_impV0);
       sc = hist_root->regHist(histDir+"ntrkjet", m_hb_ntrkjet);
       sc = hist_root->regHist(histDir+"jetmom", m_hb_jmom);
       sc = hist_root->regHist(histDir+"jetmomvrt", m_hb_mom);
       sc = hist_root->regHist(histDir+"sig3d2tr", m_hb_signif3D);
       sc = hist_root->regHist(histDir+"massPiPi1", m_hb_massPiPi1);
       sc = hist_root->regHist(histDir+"massPiPi2", m_hb_massPiPi2);
       sc = hist_root->regHist(histDir+"sig3dcommon", m_hb_sig3DTot);
       sc = hist_root->regHist(histDir+"sig3D2tr", m_hb_sig3D2tr);
       sc = hist_root->regHist(histDir+"goodvrtN", m_hb_goodvrtN);
       sc = hist_root->regHist(histDir+"distVV", m_hb_distVV);
       sc = hist_root->regHist(histDir+"diffPS", m_hb_diffPS);
       sc = hist_root->regHist(histDir+"trkPtMax", m_hb_trkPtMax);
       sc = hist_root->regHist(histDir+"tr2SelVar", m_hb_tr2SelVar);
       sc = hist_root->regHist(histDir+"blshared", m_hb_blshared);
       sc = hist_root->regHist(histDir+"pxshared", m_hb_pxshared);
       sc = hist_root->regHist(histDir+"addRatioMV", m_hb_addRatioMV);
       sc = hist_root->regHist(histDir+"addChi2MV", m_hb_addChi2MV);
       sc = hist_root->regHist(histDir+"addNVrtMV", m_hb_addNVrtMV);
       sc = hist_root->regHist(histDir+"effVrt2tr", m_pr_effVrt2tr);
       if( sc.isFailure() ) {     // Check of StatusCode
         if(msgLvl(MSG::INFO))msg(MSG::INFO) << "BTagVrtSec Histogram registration failure!!!" << endreq;
       }
       w_1 = 1.;

     }

     if(!m_MultiVertex)m_MultiWithPrimary = false; 

     if(m_getNegativeTag){
        if(msgLvl(MSG::INFO))msg(MSG::INFO) << " Negative TAG is requested! " << endreq;
        if(msgLvl(MSG::INFO))msg(MSG::INFO) << "Not compatible with negativeTAIL option, so getNegativeTail is set to FALSE." << endreq;
        m_getNegativeTail=false;
     }

     //for(int ntv=2; ntv<=10; ntv++) m_chiScale[ntv]=chisin_(0.9,2*ntv-3)/ntv; m_chiScale[0]=m_chiScale[2];
     for(int ntv=2; ntv<=10; ntv++) m_chiScale[ntv]=TMath::ChisquareQuantile(0.9,2.*ntv-3.)/ntv; m_chiScale[0]=m_chiScale[2];
     for(int ntv=2; ntv<=10; ntv++) m_chiScale[ntv]/=m_chiScale[0];

     if(m_RobustFit>7)m_RobustFit=7;
     if(m_RobustFit<0)m_RobustFit=0;

     return StatusCode::SUCCESS;

   }




  StatusCode InDetVKalVxInJetTool::finalize()
  {
    //MsgStream log( msgSvc(), name() );
    if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) <<"InDetVKalVxInJetTool finalize()" << endreq;
    return StatusCode::SUCCESS; 
  }
  



   const Trk::VxSecVertexInfo* InDetVKalVxInJetTool::findSecVertex(const xAOD::Vertex & PrimVrt,
							           const TLorentzVector & JetDir,
						 	           const std::vector<const xAOD::IParticle*> & IInpTrk)
    const  {
    std::vector<double>     Results;
    std::vector<const xAOD::TrackParticle*>            InpTrk;
    std::vector<const xAOD::TrackParticle*>            SelSecTrk;
    std::vector< std::vector<const xAOD::TrackParticle*> >  SelSecTrkPerVrt;
    std::vector<const xAOD::TrackParticle*>        xaodTrkFromV0;
    std::vector<xAOD::Vertex*> listVrtSec(0);
    double SecVtxMass =   0.;
    double RatioE     =   0.;
    double EnergyJet  =   0.;
    int N2trVertices  =   0 ;

    InpTrk.clear();
    std::vector<const xAOD::IParticle*>::const_iterator   i_itrk;
    for (i_itrk = IInpTrk.begin(); i_itrk < IInpTrk.end(); ++i_itrk) {
          const xAOD::TrackParticle * tmp=dynamic_cast<const xAOD::TrackParticle *> ((*i_itrk));
          if(tmp)InpTrk.push_back(tmp);
    }

    if(m_MultiVertex){
      workVectorArrxAOD * tmpVectxAOD=new workVectorArrxAOD();
      tmpVectxAOD->InpTrk.resize(InpTrk.size());
      std::copy(InpTrk.begin(),InpTrk.end(), tmpVectxAOD->InpTrk.begin());
      //////listVrtSec = GetVrtSecMulti(InpTrk,PrimVrt,JetDir,Results,SelSecTrkPerVrt,TrkFromV0);
      listVrtSec = GetVrtSecMulti(tmpVectxAOD,0,PrimVrt,JetDir,Results);
      SelSecTrkPerVrt.swap(tmpVectxAOD->FoundSecondTracks);
      xaodTrkFromV0.swap(tmpVectxAOD->TrkFromV0);
      delete tmpVectxAOD;
    }else{
       xAOD::Vertex* secVrt = GetVrtSec( InpTrk,PrimVrt,JetDir,Results,SelSecTrk,xaodTrkFromV0);
       if(secVrt != 0) listVrtSec.push_back(secVrt);
    }
    if(Results.size()<3) {
       listVrtSec.clear();
    }else{
       SecVtxMass =      Results[0];
       RatioE     =      Results[1];
       N2trVertices  = (int)Results[2];
       EnergyJet     =      Results[7];
    }

    std::vector<const xAOD::IParticle*>  iparTrkFromV0(0); 
    for(int i=0; i<(int)xaodTrkFromV0.size(); i++)iparTrkFromV0.push_back(xaodTrkFromV0[i]);

    const Trk::VxSecVertexInfo* res = 
          new Trk::VxSecVKalVertexInfo(listVrtSec, SecVtxMass, RatioE, N2trVertices, EnergyJet, iparTrkFromV0 );
    m_fitSvc->clearMemory();
    return res;
   }



   const Trk::VxSecVertexInfo* InDetVKalVxInJetTool::findSecVertex(const Trk::RecVertex & PrimVrt,
							           const TLorentzVector & JetDir,
						 	           const std::vector<const Rec::TrackParticle*> & InpTrk)
    const  {
    std::vector<double>     Results;
    std::vector<const Rec::TrackParticle*>            SelSecTrk;
    std::vector< std::vector<const Rec::TrackParticle*> >  SelSecTrkPerVrt;
    std::vector<const Rec::TrackParticle*>            TrkFromV0;
    std::vector<xAOD::Vertex*> listVrtSec(0);
    double SecVtxMass =   0.;
    double RatioE     =   0.;
    double EnergyJet  =   0.;
    int N2trVertices  =   0 ;

    xAOD::Vertex xaodPrimVrt; 
                            xaodPrimVrt.setPosition(PrimVrt.position());
                            xaodPrimVrt.setCovariancePosition(PrimVrt.covariancePosition());

    if(m_MultiVertex){
       workVectorArrREC * tmpVectREC=new workVectorArrREC();
       tmpVectREC->InpTrk.resize(InpTrk.size());
       std::copy(InpTrk.begin(),InpTrk.end(), tmpVectREC->InpTrk.begin());
       //////listVrtSec = GetVrtSecMulti(InpTrk,xaodPrimVrt,JetDir,Results,SelSecTrkPerVrt,TrkFromV0);
       listVrtSec = GetVrtSecMulti(0,tmpVectREC,xaodPrimVrt,JetDir,Results);
       SelSecTrkPerVrt.swap(tmpVectREC->FoundSecondTracks);
       TrkFromV0.swap(tmpVectREC->TrkFromV0);
       delete tmpVectREC;
    }else{
       xAOD::Vertex * secVrt = GetVrtSec( InpTrk,xaodPrimVrt,JetDir,Results,SelSecTrk,TrkFromV0);
       if(secVrt != 0) listVrtSec.push_back(secVrt);
    }
    if(Results.size()<3) {
       listVrtSec.clear();
    }else{
       SecVtxMass =      Results[0];
       RatioE     =      Results[1];
       N2trVertices  = (int)Results[2];
       EnergyJet     =      Results[7];
    }
    const Trk::VxSecVertexInfo* res = 
          new Trk::VxSecVKalVertexInfo(listVrtSec, SecVtxMass, RatioE, N2trVertices, EnergyJet, PartToBase(TrkFromV0) );
    m_fitSvc->clearMemory();
    return res;
        
//    return new Trk::VxSecVertexInfo(listVrtSec);

   }

   const Trk::VxSecVertexInfo* InDetVKalVxInJetTool::findSecVertex(const Trk::RecVertex & PrimVrt,
							           const TLorentzVector & JetDir,
						 	           const std::vector<const Trk::TrackParticleBase*> & InpTrkBase)
    const  {
    std::vector<double>     Results;
    std::vector<const Rec::TrackParticle*>            SelSecTrk;
    std::vector< std::vector<const Rec::TrackParticle*> >  SelSecTrkPerVrt;
    std::vector<const Rec::TrackParticle*>            TrkFromV0;
    std::vector<xAOD::Vertex*> listVrtSec(0);
    std::vector<const Rec::TrackParticle*> InpTrk = BaseToPart(InpTrkBase) ;
    double SecVtxMass =   0.;
    double RatioE     =   0.;
    double EnergyJet  =   0.;
    int N2trVertices  =   0 ;

    xAOD::Vertex xaodPrimVrt; 
                            xaodPrimVrt.setPosition(PrimVrt.position());
                            xaodPrimVrt.setCovariancePosition(PrimVrt.covariancePosition());

    if(m_MultiVertex){
       workVectorArrREC * tmpVectREC=new workVectorArrREC();
       tmpVectREC->InpTrk.resize(InpTrk.size());
       std::copy(InpTrk.begin(),InpTrk.end(), tmpVectREC->InpTrk.begin());
       //////listVrtSec = GetVrtSecMulti(InpTrk,xaodPrimVrt,JetDir,Results,SelSecTrkPerVrt,TrkFromV0);
       listVrtSec = GetVrtSecMulti(0,tmpVectREC,xaodPrimVrt,JetDir,Results);
       SelSecTrkPerVrt.swap(tmpVectREC->FoundSecondTracks);
       TrkFromV0.swap(tmpVectREC->TrkFromV0);
       delete tmpVectREC;
    }else{
       xAOD::Vertex* secVrt = GetVrtSec( InpTrk,xaodPrimVrt,JetDir,Results,SelSecTrk,TrkFromV0);
       if(secVrt != 0) listVrtSec.push_back(secVrt);
    }
    if(Results.size()<3) {
       listVrtSec.clear();
    }else{
       SecVtxMass =      Results[0];
       RatioE     =      Results[1];
       N2trVertices  = (int)Results[2];
       EnergyJet     =      Results[7];
    }
    const Trk::VxSecVertexInfo* res = 
          new Trk::VxSecVKalVertexInfo(listVrtSec, SecVtxMass, RatioE, N2trVertices, EnergyJet, PartToBase(TrkFromV0) );
    m_fitSvc->clearMemory();
    return res;
       
//    return new Trk::VxSecVertexInfo(listVrtSec);

   }

}  // end InDet namespace

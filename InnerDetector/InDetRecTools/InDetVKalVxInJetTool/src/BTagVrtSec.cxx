/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "InDetVKalVxInJetTool/InDetVKalVxInJetTool.h"
//-------------------------------------------------
// Other stuff
//#include  "TrkTrackLink/ITrackLink.h"
//#include  "TrkTrack/LinkToTrack.h"
#include  "TrkParticleBase/LinkToTrackParticleBase.h"
#include  "VxVertex/ExtendedVxCandidate.h"
#include  "VxVertex/VxTrackAtVertex.h"
#include  "AnalysisUtils/AnalysisMisc.h"
#include  "TrkToolInterfaces/ITrackSummaryTool.h"
#include  "TrkParticleBase/TrackParticleBaseCollection.h"
#include  "TMath.h"
//
#include<iostream>

//----------------------------------------------------------------------------------------
//  GetVrtSec resurns the vector Results with the following
//   1) Vertex mass
//   2) Vertex/jet energy fraction
//   3) Number of initially selected 2tr vertices
//   4) Number of selected for vertexing tracks in jet 
//   5) Number of track in secondary vertex
//   6) 3D SV-PV significance with sign
//   7) Maximal track Pt with respect to jet axis
//   8) Jet energy used in (2) calculation 
//---------------------------------------------------------------------------------------- 
namespace InDet{




  xAOD::Vertex* InDetVKalVxInJetTool::GetVrtSec(const std::vector<const Rec::TrackParticle*>& InpTrk,
                                                const xAOD::Vertex                          & PrimVrt,
	                                        const TLorentzVector                        & JetDir,
                                                std::vector<double>                         & Results,
                                                std::vector<const Rec::TrackParticle*> & ListSecondTracks,
                                                std::vector<const Rec::TrackParticle*> & TrkFromV0) 
  const
  {

      if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "GetVrtSec() called with Rec::TrackParticle=" <<InpTrk.size()<< endreq;
   
      std::vector<const Rec::TrackParticle*> SelectedTracks,SelectedTracksRelax;
      SelectedTracks.clear(); SelectedTracksRelax.clear();
      ListSecondTracks.clear();
      Results.clear();        

      if( InpTrk.size() < 2 ) { return 0;} // 0,1 track => nothing to do!


      int NPVParticle = SelGoodTrkParticle( InpTrk, PrimVrt, JetDir, SelectedTracks);

      long int NTracks = (int) (SelectedTracks.size());
      if(m_FillHist){m_hb_ntrkjet->Fill( (double) NTracks, w_1); }
      if( NTracks < 2 ) { return 0;} // 0,1 selected track => nothing to do!

      if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "Number of selected tracks inside jet= " <<NTracks << endreq;
      
      SelGoodTrkParticleRelax( InpTrk, PrimVrt, JetDir, SelectedTracksRelax);
      TLorentzVector MomentumJet = TotalMom(GetPerigeeVector(SelectedTracksRelax));
      if(m_FillHist){m_hb_jmom->Fill( MomentumJet.E(), w_1); }


//--------------------------------------------------------------------------------------------	 
//                    Initial Rec::TrackParticle list ready


      std::vector<const Rec::TrackParticle*>  TracksForFit;
      //std::vector<double> InpMass; for(int i=0; i<NTracks; i++){InpMass.push_back(m_massPi);}
      std::vector<double> InpMass(NTracks,m_massPi);

      xAOD::Vertex xaodPrimVrt; xaodPrimVrt.setPosition(PrimVrt.position());
                                xaodPrimVrt.setCovariancePosition(PrimVrt.covariancePosition());
      Select2TrVrt(SelectedTracks, TracksForFit, xaodPrimVrt, JetDir, InpMass, 
	                           TrkFromV0, ListSecondTracks);
      m_WorkArray->m_Incomp.clear();  // Not needed for single vertex version
//
//--- Cleaning
// 
      if( TrkFromV0.size() > 1) {
        RemoveDoubleEntries(TrkFromV0);
        AnalysisUtils::Sort::pT (&TrkFromV0);
      }
//---
      double Vrt2TrackNumber = (double) ListSecondTracks.size()/2.;
      RemoveDoubleEntries(ListSecondTracks);
      AnalysisUtils::Sort::pT (&ListSecondTracks);
//---
      if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<<" Found different tracks in pairs="<< ListSecondTracks.size()<<endreq;
      if(ListSecondTracks.size() < 2 ) { return 0;} // Less than 2 tracks left

//
//-----------------------------------------------------------------------------------------------------
//            Secondary track list is ready
//            Now common vertex fit
//
      std::vector<const Rec::TrackParticle*>::const_iterator   i_ntrk, i_found;
      Amg::Vector3D          FitVertex;
      std::vector<double> ErrorMatrix;
      std::vector< std::vector<double> > TrkAtVrt; 
      TLorentzVector    Momentum;
      double Signif3D, Signif3DP=0, Signif3DS=0;
      std::vector<double> Impact,ImpactError;

      double Chi2 =  FitCommonVrt( ListSecondTracks, xaodPrimVrt, InpMass, FitVertex, ErrorMatrix, Momentum, TrkAtVrt);
//
      if( Chi2 < 0) { return 0; }      // Vertex not reconstructed
//
// Check jet tracks not in secondary vertex
      std::vector<const Rec::TrackParticle*> AdditionalTracks;
      VrtVrtDist(PrimVrt, FitVertex, ErrorMatrix, Signif3D);
      if(Signif3D>8.){
       for (i_ntrk = SelectedTracks.begin(); i_ntrk < SelectedTracks.end(); ++i_ntrk) {
         i_found = find( ListSecondTracks.begin(), ListSecondTracks.end(), (*i_ntrk));
	 if( i_found != ListSecondTracks.end() ) continue;
         Signif3DS = m_fitSvc->VKalGetImpact((*i_ntrk), FitVertex         , 1, Impact, ImpactError);
         if( Signif3DS > 10.) continue;
         Signif3DP = m_fitSvc->VKalGetImpact((*i_ntrk), PrimVrt.position(), 1, Impact, ImpactError);
         if(m_FillHist){ m_hb_diffPS->Fill( Signif3DP-Signif3DS, w_1); }
	 if(Signif3DP-Signif3DS>1.0) AdditionalTracks.push_back((*i_ntrk));
       }
      }
//
// Add found tracks and refit
      if( AdditionalTracks.size() > 0){
        for (i_ntrk = AdditionalTracks.begin(); i_ntrk < AdditionalTracks.end(); ++i_ntrk)
                      ListSecondTracks.push_back((*i_ntrk));
        Chi2 =  FitCommonVrt( ListSecondTracks, xaodPrimVrt, InpMass, FitVertex, ErrorMatrix, Momentum, TrkAtVrt);
      }
//
//  Saving of results
//
//
      if( Chi2 < 0) { return 0; }      // Vertex not reconstructed
//
// Find highest track Pt with respect to jet direction
      double trackPt, trackPtMax=0.;
      for (int tr=0; tr<(int)ListSecondTracks.size(); tr++) {
         trackPt=pTvsDir(Amg::Vector3D(JetDir.Vect().X(),JetDir.Vect().Y(),JetDir.Vect().Z()) , TrkAtVrt[tr]);
	 if(trackPt>trackPtMax)trackPtMax=trackPt;
      }

// VK 15.10.2007 Wrong idea. Kills high Pt jets 
// VK 12.02.2008 Idea correct, implementation was wrong!!! Tracks must have BL hits only if vertex is inside BL!! 
      if( ListSecondTracks.size()==2 ){         // If there are 2 only tracks
        int Charge=0, tCnt=0;
	int blTrk[2]={0,0}; 
	int l1Trk[2]={0,0}; 
	int l2Trk[2]={0,0}; 
	int BLshared=0;
	int PXshared=0;
        std::vector<const Rec::TrackParticle*>::const_iterator   i_ntrk;
        for (i_ntrk = ListSecondTracks.begin(); i_ntrk < ListSecondTracks.end(); ++i_ntrk) {
            Charge +=  (int) (*i_ntrk)->charge();
            const Trk::TrackSummary* testSum = (*i_ntrk)->trackSummary();
            if(testSum){ 
	       if(testSum->isHit(Trk::pixelBarrel0))blTrk[tCnt]=1;
	       if(testSum->isHit(Trk::pixelBarrel1))l1Trk[tCnt]=1;
	       if(testSum->isHit(Trk::pixelBarrel2))l2Trk[tCnt]=1;
               BLshared   += testSum->get(Trk::numberOfBLayerSharedHits);
               PXshared   += testSum->get(Trk::numberOfPixelSharedHits);
	    }
            tCnt++;
        }
        double xDif=FitVertex.x()-m_XlayerB, yDif=FitVertex.y()-m_YlayerB ; 
        double Dist2D=sqrt(xDif*xDif+yDif*yDif);
        if (Dist2D < m_RlayerB) {              // Inside B-layer
          if(m_FillHist){ if(Charge){m_hb_totmass2T1->Fill(Momentum.M(),w_1);}else{m_hb_totmass2T0->Fill(Momentum.M(),w_1);} }
//          if( Momentum.m() < 400.)         {return 0;}
          if( blTrk[0]==0 || blTrk[1]==0 ) {return 0;}
          if(m_FillHist){ m_hb_blshared->Fill((float)BLshared,w_1); }
          if(BLshared>m_CutSharedHits) return 0;
        } else {                              //Outside b-layer
//          if(m_FillHist){ if(!Charge){m_hb_totmass2T2->Fill(Momentum.m(),w_1);} }
          if( blTrk[0]==1 || blTrk[1]==1 ) {return 0;}
          if(m_FillHist){ m_hb_pxshared->Fill((float)PXshared,w_1); }
          if(PXshared>m_CutSharedHits) return 0;
        }
	if(fabs(FitVertex.z())<400.){                            //Only if vertex is in acceptance
          xDif=FitVertex.x()-m_Xlayer1, yDif=FitVertex.y()-m_Ylayer1 ; 
          Dist2D=sqrt(xDif*xDif+yDif*yDif);
          if (Dist2D < m_Rlayer1) {  if( l1Trk[0]==0 || l1Trk[1]==0 ) return 0; }  // Inside 1st-layer
          else                    {  if( l1Trk[0]==1 || l1Trk[1]==1 ) return 0; }  // Outside 1st-layer
          xDif=FitVertex.x()-m_Xlayer2, yDif=FitVertex.y()-m_Ylayer2 ; 
          Dist2D=sqrt(xDif*xDif+yDif*yDif);
          if (Dist2D < m_Rlayer2) {  if( l2Trk[0]==0 || l2Trk[1]==0 ) return 0; }  // Inside 2nd-layer
          else                    {  if( l2Trk[0]==1 || l2Trk[1]==1 ) return 0; }  // Outside 2nd-layer
        }
//
        if( !Charge && fabs(Momentum.M()-m_massK0)<15. ) {       // Final rejection of K0
	  TrkFromV0.push_back(ListSecondTracks[0]);
	  TrkFromV0.push_back(ListSecondTracks[1]);
          if( TrkFromV0.size() > 1) {
	     RemoveDoubleEntries(TrkFromV0);
             AnalysisUtils::Sort::pT (&TrkFromV0);
          }
          return 0;
        }
//-- Protection against fake vertices far from interaction point 
	if(NPVParticle<1)NPVParticle=1;
        double vvdist3D=VrtVrtDist(PrimVrt, FitVertex, ErrorMatrix, Signif3D);
        double t3Dimp1= m_fitSvc->VKalGetImpact(ListSecondTracks[0], PrimVrt.position(), 1, Impact, ImpactError)/fabs(TrkAtVrt[0][2]);
        double t3Dimp2= m_fitSvc->VKalGetImpact(ListSecondTracks[1], PrimVrt.position(), 1, Impact, ImpactError)/fabs(TrkAtVrt[1][2]);
	double selVar=(t3Dimp1<t3Dimp2 ? t3Dimp1 : t3Dimp2)/sqrt((double)NPVParticle)/vvdist3D/500.;
        if(m_FillHist){ m_hb_tr2SelVar->Fill( selVar , w_1); }
	if(selVar<m_AntiFake2trVrtCut)return 0;
        if(m_FillHist){ m_hb_totmass2T2->Fill(Momentum.M(),w_1); }
      }


      double JetVrtDir =
             JetDir.Px()*(FitVertex.x()-PrimVrt.position().x())
           + JetDir.Py()*(FitVertex.y()-PrimVrt.position().y())
 	   + JetDir.Pz()*(FitVertex.z()-PrimVrt.position().z());
      if(  m_getNegativeTag )
         { if( JetVrtDir>0. )   return 0; }
      else if( m_getNegativeTail )
         { ; }
      else 
         { if( JetVrtDir<0. ) return 0; } 


      Amg::Vector3D DirForPt( FitVertex.x()-PrimVrt.position().x(),
                              FitVertex.y()-PrimVrt.position().y(),
		              FitVertex.z()-PrimVrt.position().z());
      if( m_MassType == 3 ) Results.push_back( TotalTVMom(DirForPt, GetPerigeeVector(ListSecondTracks))); 
      if( m_MassType == 2 ) Results.push_back(TotalTMom(GetPerigeeVector(ListSecondTracks))*1.15); 
      if( m_MassType == 1 ) Results.push_back(Momentum.M()); 

      if(m_FillHist){m_hb_mom->Fill( MomentumJet.E(), w_1);}       //1st
      double eRatio = Momentum.E()/MomentumJet.E(); 
      Results.push_back(  eRatio<0.99999 ? eRatio : 0.99999);      //2nd
      Results.push_back(Vrt2TrackNumber);                          //3rd
      Results.push_back((double)NTracks);                          //4th
      Results.push_back((double)ListSecondTracks.size());          //5th
      VrtVrtDist(PrimVrt, FitVertex, ErrorMatrix, Signif3D);
      if(JetVrtDir < 0) Signif3D = -Signif3D;
      Results.push_back(Signif3D);                                 //6th
      Results.push_back(trackPtMax);                               //7th
      Results.push_back(MomentumJet.E());                          //8th

      if(m_FillHist){m_hb_ratio->Fill( Results[1], w_1); }
      if(m_FillHist){m_hb_totmass->Fill( Results[0], w_1); }
      if(m_FillHist){m_hb_nvrt2->Fill( Results[2], w_1); }
      if(m_FillHist){m_hb_sig3DTot->Fill( Signif3D, w_1); }
      if(m_FillHist){ m_hb_trkPtMax->Fill( trackPtMax, w_1); }


//-------------------------------------------------------------------------------------
//Return Trk::VxCandidate 
//      Trk::RecVertex * tmpRecV = new Trk::RecVertex(FitVertex, makeVrtCovMatrix(ErrorMatrix), ListSecondTracks.size()*2.-3., Chi2); 
//      std::vector<Trk::VxTrackAtVertex*> * tmpVTAV = new std::vector<Trk::VxTrackAtVertex*>();
//      Trk::Perigee         * measPerigee;
//      Trk::VxTrackAtVertex * tmpPointer;
//      for(int ii=0; ii<(int)ListSecondTracks.size(); ii++) {
//           AmgSymMatrix(5) * tmpCovMatr=new AmgSymMatrix(5); (*tmpCovMatr).setIdentity(); 
//           measPerigee =  new Trk::Perigee( 0.,0., TrkAtVrt[ii][0],
//                                                   TrkAtVrt[ii][1],
//                                                   TrkAtVrt[ii][2],
//                                                   Trk::PerigeeSurface(FitVertex),
//                                                   tmpCovMatr );
//           tmpPointer = new Trk::VxTrackAtVertex( 1., measPerigee ) ;
//           ElementLink<Trk::TrackParticleBaseCollection> TEL;  TEL.setElement( (Trk::TrackParticleBase*) ListSecondTracks[ii]);
//           Trk::LinkToTrackParticleBase * ITL = new Trk::LinkToTrackParticleBase(TEL); //pointer to initial Trk
//           tmpPointer->setOrigTrack(ITL);              //pointer to initial TrackParticle
//
//           tmpPointer->setWeight(1.);
//           tmpVTAV->push_back( tmpPointer );
//       }
//       Trk::VxCandidate* tmpVx = new Trk::VxCandidate(*tmpRecV,*tmpVTAV);
//       tmpVx->setVertexType(Trk::SecVtx);
//       delete tmpVTAV;delete tmpRecV;
//       return tmpVx;
//-------------------------------------------------------------------------------------
//Return xAOD::Vertex
       xAOD::Vertex * tmpVertex=new xAOD::Vertex();
       tmpVertex->makePrivateStore();
       tmpVertex->setPosition(FitVertex);
       std::vector<float> floatErrMtx; floatErrMtx.resize(ErrorMatrix.size());
       for(int i=0; i<(int)ErrorMatrix.size(); i++) floatErrMtx[i]=ErrorMatrix[i];
       tmpVertex->setCovariance(floatErrMtx);
       tmpVertex->setFitQuality(Chi2, (float)(ListSecondTracks.size()*2.-3.));

       std::vector<Trk::VxTrackAtVertex> & tmpVTAV=tmpVertex->vxTrackAtVertex();    tmpVTAV.clear();
       for(int ii=0; ii<(int)ListSecondTracks.size(); ii++) {
         AmgSymMatrix(5) *CovMtxP=new AmgSymMatrix(5);    (*CovMtxP).setIdentity(); 
         Trk::Perigee * tmpMeasPer  =  new Trk::Perigee( 0.,0., TrkAtVrt[ii][0], TrkAtVrt[ii][1], TrkAtVrt[ii][2],
                                                                Trk::PerigeeSurface(FitVertex), CovMtxP );
         tmpVTAV.push_back( Trk::VxTrackAtVertex( 1., tmpMeasPer) );
       }
       return tmpVertex;
//==========================================Test of ExtendedVxCandidate(do not forget to comment tmpVTAV deletion!!!)
//      std::vector <double> CovFull;
//      StatusCode sc = m_fitSvc->VKalGetFullCov( (long int)ListSecondTracks.size(), CovFull); 
//      Trk::ExtendedVxCandidate * tmpVertex = new Trk::ExtendedVxCandidate(*tmpRecV,*tmpVTAV, 
//               new Amg::MatrixX( SetFullMatrix((int)ListSecondTracks.size(),CovFull) )  );
//std::cout<<" Error="<<massError(tmpVertex, 139.5702)<<'\n';
//      double dMass,dMassErr;std::vector<int> ListOfTracks((int)ListSecondTracks.size(),1);
//      sc = m_fitSvc->VKalGetMassError( ListOfTracks,dMass, dMassErr);
//std::cout<<" Error1="<<dMass<<", "<<dMassErr<<'\n';
//      Trk::TrackParticleBase * tp=CreateParticle(tmpVertex);
//===================================================
  }









  xAOD::Vertex* InDetVKalVxInJetTool::GetVrtSec(const std::vector<const xAOD::TrackParticle*>& InpTrk,
                                                const xAOD::Vertex                           & PrimVrt,
                                                const TLorentzVector                         & JetDir,
	                                        std::vector<double>                          & Results,
                                                std::vector<const xAOD::TrackParticle*>      & ListSecondTracks,
				                std::vector<const xAOD::TrackParticle*>      & TrkFromV0) 
  const
  {

      if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "GetVrtSec() called with xAOD::TrackParticle=" <<InpTrk.size()<< endreq;
   
      std::vector<const xAOD::TrackParticle*> SelectedTracks,SelectedTracksRelax;
      SelectedTracks.clear(); SelectedTracksRelax.clear();
      ListSecondTracks.clear(); Results.clear();        

      if( InpTrk.size() < 2 ) { return 0;} // 0,1 track => nothing to do!
      int NPVParticle = SelGoodTrkParticle( InpTrk, PrimVrt, JetDir, SelectedTracks);

      long int NTracks = (int) (SelectedTracks.size());
      if(m_FillHist){m_hb_ntrkjet->Fill( (double) NTracks, w_1); }
      if( NTracks < 2 ) { return 0;} // 0,1 selected track => nothing to do!

      if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "Number of selected tracks inside jet= " <<NTracks << endreq;
      
      SelGoodTrkParticleRelax( InpTrk, PrimVrt, JetDir, SelectedTracksRelax);
      TLorentzVector MomentumJet = TotalMom(SelectedTracksRelax);
      if(m_FillHist){m_hb_jmom->Fill( MomentumJet.E(), w_1); }


//--------------------------------------------------------------------------------------------	 
//                    Initial xAOD::TrackParticle list ready


      std::vector<const xAOD::TrackParticle*>  TracksForFit;
      //std::vector<double> InpMass; for(int i=0; i<NTracks; i++){InpMass.push_back(m_massPi);}
      std::vector<double> InpMass(NTracks,m_massPi);

      Select2TrVrt(SelectedTracks, TracksForFit, PrimVrt, JetDir, 
				   InpMass, TrkFromV0, ListSecondTracks);
      m_WorkArray->m_Incomp.clear();  // Not needed for single vertex version
//
//--- Cleaning
// 
      if( TrkFromV0.size() > 1) {
        RemoveDoubleEntries(TrkFromV0);
        AnalysisUtils::Sort::pT (&TrkFromV0);            //no sorting for xAOD
      }
//---
      double Vrt2TrackNumber = (double) ListSecondTracks.size()/2.;
      RemoveDoubleEntries(ListSecondTracks);
      AnalysisUtils::Sort::pT (&ListSecondTracks);      //no sorting for xAOD
//---
      if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<<" Found different xAOD tracks in pairs="<< ListSecondTracks.size()<<endreq;
      if(ListSecondTracks.size() < 2 ) { return 0;} // Less than 2 tracks left

//
//-----------------------------------------------------------------------------------------------------
//            Secondary track list is ready
//            Now common vertex fit
//
      std::vector<const xAOD::TrackParticle*>::const_iterator   i_ntrk, i_found;
      Amg::Vector3D          FitVertex;
      std::vector<double> ErrorMatrix;
      std::vector< std::vector<double> > TrkAtVrt; 
      TLorentzVector    Momentum;
      double Signif3D, Signif3DP=0, Signif3DS=0;
      std::vector<double> Impact,ImpactError;

      double Chi2 =  FitCommonVrt( ListSecondTracks, PrimVrt, InpMass, FitVertex, ErrorMatrix, Momentum, TrkAtVrt);
//
      if( Chi2 < 0) { return 0; }      // Vertex not reconstructed
//
// Check jet tracks not in secondary vertex
      std::vector<const xAOD::TrackParticle*> AdditionalTracks;
      VrtVrtDist(PrimVrt, FitVertex, ErrorMatrix, Signif3D);
      if(Signif3D>8.){
       for (i_ntrk = SelectedTracks.begin(); i_ntrk < SelectedTracks.end(); ++i_ntrk) {
         i_found = find( ListSecondTracks.begin(), ListSecondTracks.end(), (*i_ntrk));
	 if( i_found != ListSecondTracks.end() ) continue;
         Signif3DS = m_fitSvc->VKalGetImpact((*i_ntrk), FitVertex         , 1, Impact, ImpactError);
         if( Signif3DS > 10.) continue;
         Signif3DP = m_fitSvc->VKalGetImpact((*i_ntrk), PrimVrt.position(), 1, Impact, ImpactError);
         if(m_FillHist){ m_hb_diffPS->Fill( Signif3DP-Signif3DS, w_1); }
	 if(Signif3DP-Signif3DS>1.0) AdditionalTracks.push_back((*i_ntrk));
       }
      }
//
// Add found tracks and refit
      if( AdditionalTracks.size() > 0){
        for (i_ntrk = AdditionalTracks.begin(); i_ntrk < AdditionalTracks.end(); ++i_ntrk)
                      ListSecondTracks.push_back((*i_ntrk));
        Chi2 =  FitCommonVrt( ListSecondTracks, PrimVrt, InpMass, FitVertex, ErrorMatrix, Momentum, TrkAtVrt);
      }
//
//  Saving of results
//
//
      if( Chi2 < 0) { return 0; }      // Vertex not reconstructed
//
// Find highest track Pt with respect to jet direction
      double trackPt, trackPtMax=0.;
      for (int tr=0; tr<(int)ListSecondTracks.size(); tr++) {
         trackPt=pTvsDir(Amg::Vector3D(JetDir.X(),JetDir.Y(),JetDir.Z()) , TrkAtVrt[tr]);
	 if(trackPt>trackPtMax)trackPtMax=trackPt;
      }

      if( ListSecondTracks.size()==2 ){         // If there are 2 only tracks
        int Charge=0, tCnt=0;
	int blTrk[2]={0,0}; 
	int l1Trk[2]={0,0}; 
	int l2Trk[2]={0,0}; 
	uint8_t BLshared=0;
	uint8_t PXshared=0;
        for (i_ntrk = ListSecondTracks.begin(); i_ntrk < ListSecondTracks.end(); ++i_ntrk) {
            Charge +=  (int) (*i_ntrk)->charge();
            getPixelLayers( (*i_ntrk), blTrk[tCnt] , l1Trk[tCnt], l2Trk[tCnt] );

            uint8_t retval;
            if( (*i_ntrk)->summaryValue( retval, xAOD::numberOfPixelSharedHits)  )  PXshared  += retval;
            if( (*i_ntrk)->summaryValue( retval, xAOD::numberOfBLayerSharedHits) )  BLshared  += retval;

            tCnt++;
        }
        double xDif=FitVertex.x()-m_XlayerB, yDif=FitVertex.y()-m_YlayerB ; 
        double Dist2D=sqrt(xDif*xDif+yDif*yDif);
        if (Dist2D < m_RlayerB) {              // Inside B-layer
          if(m_FillHist){ if(Charge){m_hb_totmass2T1->Fill(Momentum.M(),w_1);}else{m_hb_totmass2T0->Fill(Momentum.M(),w_1);} }
          if( blTrk[0]==0 || blTrk[1]==0 ) {return 0;}
          if(m_FillHist){ m_hb_blshared->Fill((float)BLshared,w_1); }
          if(BLshared>m_CutSharedHits) return 0;
        } else {                              //Outside b-layer
          if( blTrk[0]==1 || blTrk[1]==1 ) {return 0;}
          if(m_FillHist){ m_hb_pxshared->Fill((float)PXshared,w_1); }
          if(PXshared>m_CutSharedHits) return 0;
        }
	if(fabs(FitVertex.z())<400.){                            //Only if vertex is in acceptance
          xDif=FitVertex.x()-m_Xlayer1, yDif=FitVertex.y()-m_Ylayer1 ; 
          Dist2D=sqrt(xDif*xDif+yDif*yDif);
          if (Dist2D < m_Rlayer1) {  if( l1Trk[0]==0 || l1Trk[1]==0 ) return 0; }  // Inside 1st-layer
          else                    {  if( l1Trk[0]==1 || l1Trk[1]==1 ) return 0; }  // Outside 1st-layer
          xDif=FitVertex.x()-m_Xlayer2, yDif=FitVertex.y()-m_Ylayer2 ; 
          Dist2D=sqrt(xDif*xDif+yDif*yDif);
          if (Dist2D < m_Rlayer2) {  if( l2Trk[0]==0 || l2Trk[1]==0 ) return 0; }  // Inside 2nd-layer
          else                    {  if( l2Trk[0]==1 || l2Trk[1]==1 ) return 0; }  // Outside 2nd-layer
        }
//
        if( !Charge && fabs(Momentum.M()-m_massK0)<15. ) {       // Final rejection of K0
	  TrkFromV0.push_back(ListSecondTracks[0]);
	  TrkFromV0.push_back(ListSecondTracks[1]);
          if( TrkFromV0.size() > 1) {
             RemoveDoubleEntries(TrkFromV0);
             AnalysisUtils::Sort::pT (&TrkFromV0);
          }
          return 0;
        }
//-- Protection against fake vertices far from interaction point 
	if(NPVParticle<1)NPVParticle=1;
        double vvdist3D=VrtVrtDist(PrimVrt, FitVertex, ErrorMatrix, Signif3D);
        double t3Dimp1= m_fitSvc->VKalGetImpact(ListSecondTracks[0], PrimVrt.position(), 1, Impact, ImpactError)/fabs(TrkAtVrt[0][2]);
        double t3Dimp2= m_fitSvc->VKalGetImpact(ListSecondTracks[1], PrimVrt.position(), 1, Impact, ImpactError)/fabs(TrkAtVrt[1][2]);
	double selVar=(t3Dimp1<t3Dimp2 ? t3Dimp1 : t3Dimp2)/sqrt((double)NPVParticle)/vvdist3D/500.;
        if(m_FillHist){ m_hb_tr2SelVar->Fill( selVar , w_1); }
	if(selVar<m_AntiFake2trVrtCut)return 0;
        if(m_FillHist){ m_hb_totmass2T2->Fill(Momentum.M(),w_1); }
      }


      double JetVrtDir =
             JetDir.Px()*(FitVertex.x()-PrimVrt.x())
           + JetDir.Py()*(FitVertex.y()-PrimVrt.y())
 	   + JetDir.Pz()*(FitVertex.z()-PrimVrt.z());
      if(  m_getNegativeTag )
         { if( JetVrtDir>0. )   return 0; }
      else if( m_getNegativeTail )
         { ; }
      else 
         { if( JetVrtDir<0. ) return 0; } 


      Amg::Vector3D DirForPt( FitVertex.x()-PrimVrt.x(),
                              FitVertex.y()-PrimVrt.y(),
		              FitVertex.z()-PrimVrt.z());
      //if( m_MassType == 3 ) Results.push_back( TotalTVMom(DirForPt, GetPerigeeVector(ListSecondTracks))); 
      //if( m_MassType == 2 ) Results.push_back(TotalTMom(GetPerigeeVector(ListSecondTracks))*1.15); 
      if( m_MassType == 1 ) Results.push_back(Momentum.M()); 

      if(m_FillHist){m_hb_mom->Fill( MomentumJet.E(), w_1);}       //1st
      double eRatio = Momentum.E()/MomentumJet.E(); 
      Results.push_back(  eRatio<0.99999 ? eRatio : 0.99999);      //2nd
      Results.push_back(Vrt2TrackNumber);                          //3rd
      Results.push_back((double)NTracks);                          //4th
      Results.push_back((double)ListSecondTracks.size());          //5th
      VrtVrtDist(PrimVrt, FitVertex, ErrorMatrix, Signif3D);
      if(JetVrtDir < 0) Signif3D = -Signif3D;
      Results.push_back(Signif3D);                                 //6th
      Results.push_back(trackPtMax);                               //7th
      Results.push_back(MomentumJet.E());                          //8th

      if(m_FillHist){m_hb_ratio->Fill( Results[1], w_1); }
      if(m_FillHist){m_hb_totmass->Fill( Results[0], w_1); }
      if(m_FillHist){m_hb_nvrt2->Fill( Results[2], w_1); }
      if(m_FillHist){m_hb_sig3DTot->Fill( Signif3D, w_1); }
      if(m_FillHist){ m_hb_trkPtMax->Fill( trackPtMax, w_1); }

//-------------------------------------------------------------------------------------
//Return Trk::VxCandidate 
//      Trk::RecVertex * tmpRecV = new Trk::RecVertex(FitVertex, makeVrtCovMatrix(ErrorMatrix), ListSecondTracks.size()*2.-3., Chi2); 
//      std::vector<Trk::VxTrackAtVertex*> * tmpVTAV = new std::vector<Trk::VxTrackAtVertex*>();
//      Trk::Perigee         * measPerigee;
//      Trk::VxTrackAtVertex * tmpPointer;
//      for(int ii=0; ii<(int)ListSecondTracks.size(); ii++) {
//           AmgSymMatrix(5) * tmpCovMatr=new AmgSymMatrix(5); (*tmpCovMatr).setIdentity(); 
//           measPerigee =  new Trk::Perigee( 0.,0., TrkAtVrt[ii][0], TrkAtVrt[ii][1], TrkAtVrt[ii][2],
//                                                   Trk::PerigeeSurface(FitVertex), tmpCovMatr );
//           tmpPointer = new Trk::VxTrackAtVertex( 1., measPerigee ) ;
//           ElementLink<Trk::TrackParticleBaseCollection> TEL;  TEL.setElement( (Trk::TrackParticleBase*) ListSecondTracks[ii]);
//           Trk::LinkToTrackParticleBase * ITL = new Trk::LinkToTrackParticleBase(TEL); //pointer to initial Trk
//           tmpPointer->setOrigTrack(ITL);              //pointer to initial TrackParticle
//
//           tmpPointer->setWeight(1.);
//           tmpVTAV->push_back( tmpPointer );
//       }
//       Trk::VxCandidate* tmpVx = new Trk::VxCandidate(*tmpRecV,*tmpVTAV);
//       tmpVx->setVertexType(Trk::SecVtx);
//       delete tmpVTAV;delete tmpRecV;
//       return tmpVx;
//-------------------------------------------------------------------------------------
//Return xAOD::Vertex
       xAOD::Vertex * tmpVertex=new xAOD::Vertex();
       tmpVertex->makePrivateStore();
       tmpVertex->setPosition(FitVertex);
       std::vector<float> floatErrMtx; floatErrMtx.resize(ErrorMatrix.size());
       for(int i=0; i<(int)ErrorMatrix.size(); i++) floatErrMtx[i]=ErrorMatrix[i];
       tmpVertex->setCovariance(floatErrMtx);
       tmpVertex->setFitQuality(Chi2, (float)(ListSecondTracks.size()*2.-3.));

       std::vector<Trk::VxTrackAtVertex> & tmpVTAV=tmpVertex->vxTrackAtVertex();    tmpVTAV.clear();
       for(int ii=0; ii<(int)ListSecondTracks.size(); ii++) {
         AmgSymMatrix(5) *CovMtxP=new AmgSymMatrix(5);   (*CovMtxP).setIdentity(); 
         Trk::Perigee * tmpMeasPer  =  new Trk::Perigee( 0.,0., TrkAtVrt[ii][0], TrkAtVrt[ii][1], TrkAtVrt[ii][2],
                                                                Trk::PerigeeSurface(FitVertex), CovMtxP );
         tmpVTAV.push_back( Trk::VxTrackAtVertex( 1., tmpMeasPer) );
         ElementLink<xAOD::TrackParticleContainer> TEL;  TEL.setElement( ListSecondTracks[ii] );
         const xAOD::TrackParticleContainer* cont = (const xAOD::TrackParticleContainer* ) (ListSecondTracks[ii]->container() );
	 TEL.setStorableObject(*cont);
         tmpVertex->addTrackAtVertex(TEL,1.);
       }
       return tmpVertex;


  }












//
//--------------------------------------------------------
//   Template routine for global secondary vertex fitting
//

  template <class Track>
  double InDetVKalVxInJetTool::FitCommonVrt(std::vector<const Track*>& ListSecondTracks,
                                  const xAOD::Vertex        & PrimVrt,
                                  std::vector<double>       & InpMass, 
	                          Amg::Vector3D             & FitVertex,
                                  std::vector<double>       & ErrorMatrix,
	                          TLorentzVector            & Momentum,
		        std::vector< std::vector<double> >  & TrkAtVrt)
  const
 {
      if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG) << "FitCommonVrt() called " <<ListSecondTracks.size()<< endreq;
//preparation
      StatusCode sc;
      std::vector<double> Chi2PerTrk;
//      vector< vector<double> > TrkAtVrt; 
      long int           Charge;
      double             Chi2 = 0.;
;
      int Outlier=1, i=0;
//
// Start of fit
//
      m_fitSvc->setDefault();
      m_fitSvc->setMassInputParticles( InpMass );            // Use pions masses
      m_fitSvc->setMomCovCalc(1);  /* Total momentum and its covariance matrix are calculated*/
      sc=VKalVrtFitFastBase(ListSecondTracks,FitVertex);          /* Fast crude estimation */
      if(sc.isFailure() || FitVertex.perp() > m_Rlayer2*2. ) {    /* No initial estimation */ 
         m_fitSvc->setApproximateVertex(PrimVrt.position().x(),   /* Use as starting point */
                                        PrimVrt.position().y(),
	  			        PrimVrt.position().z()); 
      } else {
         m_fitSvc->setApproximateVertex(FitVertex.x(),FitVertex.y(),FitVertex.z()); /*Use as starting point*/
      }
//      m_fitSvc-> setVertexForConstraint(PrimVrt.position().x(),PrimVrt.position().y(),PrimVrt.position().z());
//      m_fitSvc->setCovVrtForConstraint(PrimVrt.errorPosition().covValue(Trk::x,Trk::x),
//                                       PrimVrt.errorPosition().covValue(Trk::y,Trk::x),
//                                       PrimVrt.errorPosition().covValue(Trk::y,Trk::y),
//                                       PrimVrt.errorPosition().covValue(Trk::z,Trk::x),
//                                       PrimVrt.errorPosition().covValue(Trk::z,Trk::y),
//                                       PrimVrt.errorPosition().covValue(Trk::z,Trk::z) );
//      m_fitSvc->setCnstType(7);
      if(m_RobustFit)m_fitSvc->setRobustness(m_RobustFit);
//fit itself
      int NTracksVrt = ListSecondTracks.size(); double FitProb=0.;
      for (i=0; i < NTracksVrt-1; i++) {
//         sc=m_fitSvc->VKalVrtFit(ListSecondTracks,FitVertex, Momentum,Charge,
//                                         ErrorMatrix,Chi2PerTrk,TrkAtVrt,Chi2);
         sc=VKalVrtFitBase(ListSecondTracks,FitVertex, Momentum,Charge,
                                 ErrorMatrix,Chi2PerTrk,TrkAtVrt,Chi2);
         if(sc.isFailure() ||  Chi2 > 1000000. ) { return -10000.;}    // No fit
         Outlier = FindMax( Chi2PerTrk ); 
//	 FitProb=prob_( (float)Chi2,(long int)(2*ListSecondTracks.size()-3));
	 FitProb=TMath::Prob( Chi2, 2*ListSecondTracks.size()-3);
	 if(ListSecondTracks.size() == 2 )                break;         // Only 2 tracks left
//	 if( Momentum.m() <6000.) {
//	   if( Chi2PerTrk[Outlier] < m_SecTrkChi2Cut && FitProb > 0.001)  break;  // Solution found
//	 }
	 if( Momentum.M() <6000. && FitProb > 0.001) {
	   if( Chi2PerTrk[Outlier] < m_SecTrkChi2Cut*m_chiScale[ListSecondTracks.size()<10?ListSecondTracks.size():10])  break;  // Solution found
	 }
         RemoveEntryInList(ListSecondTracks,Outlier);
         m_fitSvc->setApproximateVertex(FitVertex.x(),FitVertex.y(),FitVertex.z()); /*Use as starting point*/
      }
//--
      if(msgLvl(MSG::DEBUG))msg(MSG::DEBUG)<<" SecVrt fit converged="<< ListSecondTracks.size()<<", "
          <<Chi2<<", "<<Chi2PerTrk[Outlier]<<" Mass="<<Momentum.M()<<endreq;
//--
      if( ListSecondTracks.size()==2 ){
	 if( Momentum.M() > 6000. || FitProb < 0.001 || Chi2PerTrk[Outlier] > m_SecTrkChi2Cut) { return -10000.;  }  
      } 
//
//-- To kill remnants of conversion
      double Dist2D=sqrt(FitVertex.x()*FitVertex.x()+FitVertex.y()*FitVertex.y());
      if( ListSecondTracks.size()==2  && (Dist2D > 20.) && Charge==0 ) {      
        double mass_EE   =  massV0( TrkAtVrt,m_massE,m_massE);
        if(m_FillHist){m_hb_totmassEE->Fill( mass_EE, w_1); }
        if( mass_EE < 100. ) return -40.;
      }
//-- Test creation of Trk::Track
//      if( ListSecondTracks.size()==2 && Charge==0 ) {      
//        std::vector<double> VKPerigee,CovPerigee;
//        sc=m_fitSvc->VKalVrtCvtTool(FitVertex,Momentum,ErrorMatrix,0,VKPerigee,CovPerigee);
//        if(sc.isSuccess())const Trk::Track* TT = m_fitSvc->CreateTrkTrack(VKPerigee,CovPerigee); 
//      }
//      if( ListSecondTracks.size()==2  && (Dist2D > 20.) ) {    // Protection against fake vertices    
//         std::vector<double> Impact,ImpactError;double ImpactSignif;
//         ImpactSignif = m_fitSvc->VKalGetImpact(ListSecondTracks[0], PrimVrt.position(), 1, Impact, ImpactError);
//         if( ImpactSignif<4. && ImpactSignif>-3. ) return -41.;
//         ImpactSignif = m_fitSvc->VKalGetImpact(ListSecondTracks[1], PrimVrt.position(), 1, Impact, ImpactError);
//         if( ImpactSignif<4. && ImpactSignif>-3. ) return -41.;
//      }
      return Chi2;
}







//
//
//--------------------------------------------------------
//   Template routine for 2track secondary vertices selection
//

    template <class Track>
    void InDetVKalVxInJetTool::Select2TrVrt(std::vector<const Track*>  & SelectedTracks,
                                  std::vector<const Track*>            & TracksForFit,
                                  const xAOD::Vertex                   & PrimVrt,
	                          const TLorentzVector & JetDir,
                                  std::vector<double>         & InpMass, 
	                          std::vector<const Track*>   & TrkFromV0,
	                          std::vector<const Track*>   & ListSecondTracks)
    const
    {
      Amg::Vector3D          FitVertex, vDist;
      std::vector<double> ErrorMatrix,Chi2PerTrk,VKPerigee,CovPerigee;
      std::vector< std::vector<double> > TrkAtVrt; 
      TLorentzVector   Momentum;
      std::vector<double> Impact,ImpactError;
      double ImpactSignif=0;
      double             Chi2, Signif3D, Dist2D, JetVrtDir;
      long int           Charge;
      int i,j;
      double  saveSignif3D=0.;
//
      long int NTracks = (int) (SelectedTracks.size());

      m_WorkArray->m_Incomp.clear();   // For multivertex
      m_WorkArray->m_Prmtrack.clear();   // For multivertex
//
//  Impact parameters with sign calculations
//
      double SignifR,SignifZ;
      std::vector<double> TrackSignif,TrackPt;
      AmgVector(5) tmpPerigee; tmpPerigee<<0.,0.,0.,0.,0.;
      int NPrimTrk=0, NSecTrk=0, NRefTrk=0;
      for (i=0; i<NTracks; i++) {
         ImpactSignif = m_fitSvc->VKalGetImpact(SelectedTracks[i], PrimVrt.position(), 1, Impact, ImpactError);
         tmpPerigee = GetPerigee(SelectedTracks[i])->parameters(); 
         if( sin(tmpPerigee[2]-JetDir.Phi())*Impact[0] < 0 ){ Impact[0] = -fabs(Impact[0]);}
	                                                else{ Impact[0] =  fabs(Impact[0]);}
         if(  (tmpPerigee[3]-JetDir.Theta())*Impact[1] < 0 ){ Impact[1] = -fabs(Impact[1]);}
	                                                else{ Impact[1] =  fabs(Impact[1]);}
	 SignifR = Impact[0]/ sqrt(ImpactError[0]);
	 SignifZ = Impact[1]/ sqrt(ImpactError[2]);
         if(m_FillHist){
	    m_hb_impactR->Fill( SignifR, w_1); 
            m_hb_impactZ->Fill( SignifZ, w_1); 
            m_hb_impactRZ->Fill(SignifR, SignifZ, w_1); 
         }
	 if(ImpactSignif < 3.) { NPrimTrk += 1;}
         else{NSecTrk += 1;}
         if( SignifR<0 || SignifZ<0 ) NRefTrk++;
         if(m_getNegativeTail){
  	    ImpactSignif = sqrt( SignifR*SignifR + SignifZ*SignifZ);
  	 }else if(m_getNegativeTag){
  	    ImpactSignif = sqrt(  (SignifR-0.6)*(SignifR-0.6)
  	                        + (SignifZ-0.6)*(SignifZ-0.6) );
  	 }else{
  	    ImpactSignif = sqrt(  (SignifR+0.6)*(SignifR+0.6)
  	                        + (SignifZ+0.6)*(SignifZ+0.6) );
  	 }
         if(fabs(SignifR) < m_AntiPileupSigRCut) {   // cut against tracks from pileup vertices  
           if(SignifZ >  m_AntiPileupSigZCut ) ImpactSignif=0.;  
           if(SignifZ < -m_AntiPileupSigZCut ) ImpactSignif=0.;  
         }
      
         TrackSignif.push_back(ImpactSignif); TrackPt.push_back( sin(tmpPerigee[3])/fabs(tmpPerigee[4]) ) ;
         if(m_FillHist){m_hb_impact->Fill( ImpactSignif, w_1);}
         //if( ImpactSignif < m_TrkSigCut) m_WorkArray->m_Prmtrack.push_back(i);  //For multivertex  xAODproblem
      }

      int NTrDiff = NPrimTrk-NSecTrk; if(NTrDiff < 1) NTrDiff=1;
//      double Chi2Corr =   sqrt( chisin_((float)(1.-0.1/NTrDiff),2) )
//                        - sqrt( chisin_((float)(1.-0.1)        ,2) );
      double Chi2Corr =   sqrt( TMath::ChisquareQuantile((1.-0.1/NTrDiff),2.) )
                        - sqrt( TMath::ChisquareQuantile((1.-0.1)        ,2.) );
      Chi2Corr = 0.;  //Remove correction   28.10.2007 - this correction worsen result
      double SelLim = m_TrkSigCut + Chi2Corr;

      StatusCode sc; if(sc.isSuccess())ImpactSignif=0; //Safety !

      m_fitSvc->setDefault();
      m_fitSvc->setMassInputParticles( InpMass );     // Use pion masses for fit
      m_fitSvc->setMomCovCalc(1);                     // Total momentum and its covariance matrix are calculated
      for (i=0; i<NTracks-1; i++) {
         //if(TrackPt[i]<m_JetPtFractionCut*JetDir.perp())continue;
	 double adp1=TrackPt[i]/64000.; if(adp1<0.)adp1=0;if(adp1>1.)adp1=1.;adp1=sqrt(adp1)/2.;
         for (j=i+1; j<NTracks; j++) {
             //if(TrackPt[j]<m_JetPtFractionCut*JetDir.perp())continue;
             m_WorkArray->m_Incomp.push_back(i);m_WorkArray->m_Incomp.push_back(j); // For multivertex
	     double adp2=TrackPt[j]/64000.; if(adp2<0.)adp2=0;if(adp2>1.)adp2=1.;adp2=sqrt(adp2)/2.;
	     if(!m_MultiWithPrimary) {  // Not used for multi-vertex with primary one search
	        if(TrackSignif[i]<SelLim +adp1 )                    continue;
 	        if(TrackSignif[j]<SelLim +adp2 )                    continue;
	        if(TrackSignif[i]+TrackSignif[j]  < 2.*SelLim + m_TrkSigSumCut + adp1 + adp2 + m_TrkSigSumNTrkDep*NRefTrk) continue;
	     }
	     int BadTracks = 0;                                       //Bad tracks identification 
             TracksForFit.clear();
             m_fitSvc->setDefault();                          //Reset VKalVrt settings
             m_fitSvc->setMomCovCalc(1);                     // Total momentum and its covariance matrix are calculated
             TracksForFit.push_back( SelectedTracks[i] );
             TracksForFit.push_back( SelectedTracks[j] );
//             sc=m_fitSvc->VKalVrtFitFast(TracksForFit,FitVertex);      /* Fast crude estimation*/
             sc=VKalVrtFitFastBase(TracksForFit,FitVertex);              /* Fast crude estimation*/
             if( sc.isFailure() || FitVertex.perp() > m_Rlayer2*2. ) {   /* No initial estimation */ 
                m_fitSvc->setApproximateVertex(PrimVrt.position().x(),   /*Use as starting point*/
		                               PrimVrt.position().y(),
					       PrimVrt.position().z()); 
                if( m_MultiWithPrimary ) m_fitSvc->setApproximateVertex(0., 0., 0.); 
 	     } else {
	        vDist=FitVertex-PrimVrt.position();
                JetVrtDir = JetDir.Px()*vDist.x() + JetDir.Py()*vDist.y() + JetDir.Pz()*vDist.z();
                if( m_MultiWithPrimary ) JetVrtDir=fabs(JetVrtDir); /* Always positive when primary vertex is seeked for*/ 
                if( JetVrtDir>0. ) {                           /* Good initial estimation */ 
                  m_fitSvc->setApproximateVertex(FitVertex.x(),FitVertex.y(),FitVertex.z()); /*Use as starting point*/
	        }else{
                  m_fitSvc->setApproximateVertex(PrimVrt.position().x(), PrimVrt.position().y(), PrimVrt.position().z()); 
                }
             }
//             sc=m_fitSvc->VKalVrtFit(TracksForFit,FitVertex, Momentum,Charge,
//                                         ErrorMatrix,Chi2PerTrk,TrkAtVrt,Chi2);
             sc=VKalVrtFitBase(TracksForFit,FitVertex, Momentum,Charge,
                               ErrorMatrix,Chi2PerTrk,TrkAtVrt,Chi2);
             if(sc.isFailure())                continue;          /* No fit */ 
             if(Chi2 > m_Sel2VrtChi2Cut)       continue;          /* Bad Chi2 */
	     vDist=FitVertex-PrimVrt.position();
             JetVrtDir = JetDir.Px()*vDist.x() + JetDir.Py()*vDist.y() + JetDir.Pz()*vDist.z();
	     double vPos=(vDist.x()*Momentum.Px()+vDist.y()*Momentum.Py()+vDist.z()*Momentum.Pz())/Momentum.Rho();
  	     if((!m_MultiWithPrimary) && (!m_getNegativeTail) && (!m_getNegativeTag) && (JetVrtDir<0.) )  continue; /* secondary vertex behind primary*/
	     if(vPos<-150.) continue;                                              /* Secondary vertex is too far behind primary*/
//
// Check pixel hit - vertex position. If Rv>Rlayer->tracks must not have given layer hits
	     int blTrk[2]={0,0};  int l1Trk[2]={0,0};  int l2Trk[2]={0,0}; 
             getPixelLayers(SelectedTracks[i], blTrk[0] , l1Trk[0], l2Trk[0] );
             getPixelLayers(SelectedTracks[j], blTrk[1] , l1Trk[1], l2Trk[1] );
	     double xdf=FitVertex.x()-m_XlayerB; double ydf=FitVertex.y()-m_YlayerB;
             if( sqrt(xdf*xdf+ydf*ydf) > (m_RlayerB+4.)  && (blTrk[0] || blTrk[1])) continue;
	     if(fabs(FitVertex.z())<400.){                            //Only if vertex is in acceptance
 	        xdf=FitVertex.x()-m_Xlayer1;
		ydf=FitVertex.y()-m_Ylayer1;
                if( sqrt(xdf*xdf+ydf*ydf) > (m_Rlayer1+4.)  && (l1Trk[0] || l1Trk[1])) continue;
  	        xdf=FitVertex.x()-m_Xlayer2;
		ydf=FitVertex.y()-m_Ylayer2;
                if( sqrt(xdf*xdf+ydf*ydf) > (m_Rlayer2+4.)  && (l2Trk[0] || l2Trk[1])) continue;
	     }
// Additional check for multivertex
             if( m_MultiWithPrimary || m_MultiVertex ) {
 	        xdf=FitVertex.x()-m_XlayerB;
	        ydf=FitVertex.y()-m_YlayerB;
                if( sqrt(xdf*xdf+ydf*ydf) < (m_RlayerB-5.)      &&   (blTrk[0] ==0 || blTrk[1] ==0)   ) continue;
                if(TrackPt[i]<m_JetPtFractionCut*JetDir.Perp() && m_MultiWithPrimary )continue;
                if(TrackPt[j]<m_JetPtFractionCut*JetDir.Perp() && m_MultiWithPrimary )continue;
             }
             m_WorkArray->m_Incomp.pop_back();m_WorkArray->m_Incomp.pop_back();   // For multivertex

  	     if( m_MultiWithPrimary )   continue;   /* Multivertex with primary one. All below is not needed */
//--------
//
 	     VrtVrtDist(PrimVrt, FitVertex, ErrorMatrix, Signif3D);
             Dist2D=sqrt(FitVertex.x()*FitVertex.x()+FitVertex.y()*FitVertex.y()); 
             double Signif3Dproj=VrtVrtDist( Trk::RecVertex(PrimVrt.position(),PrimVrt.covariancePosition()), FitVertex, ErrorMatrix, JetDir);
             double Signif3DSign=Signif3D;
             double mass_PiPi =  Momentum.M();  
  	     if(  JetVrtDir < 0) Signif3DSign=-Signif3D;
  	     if(m_FillHist){m_hb_signif3D->Fill( Signif3DSign, w_1); }
             if(m_FillHist){m_hb_r2d->Fill( Dist2D, w_1); }
//
	     if(mass_PiPi > 6000.)  continue;  // can't be from B decay
	     if(Dist2D    > 180. )  continue;  // can't be from B decay
	     if(m_FillHist)m_hb_massPiPi2->Fill( mass_PiPi, w_1);
//
//  Check if V0 or material interaction on Pixel layer is present
//
	     if( Charge == 0 && Signif3D>8. && mass_PiPi<900.) {
               double mass_PPi  =  massV0( TrkAtVrt,m_massP,m_massPi);
               double mass_EE   =  massV0( TrkAtVrt,m_massE,m_massE);
               if(m_FillHist){m_hb_massEE->Fill( mass_EE, w_1);} 
	       if(       mass_EE <  40.)  { 
	         BadTracks = 3;
	       }else{
                 if(m_FillHist){m_hb_massPiPi->Fill( mass_PiPi, w_1);}     /* Total mass with input particles masses*/
                 if(m_FillHist){m_hb_massPPi->Fill( mass_PPi, w_1);} 
	         if( fabs(mass_PiPi-m_massK0) < 22. )  BadTracks = 1;
	         if( fabs(mass_PPi-m_massLam) <  8. )  BadTracks = 2;
	         //double TransMass=TotalTMom(GetPerigeeVector(TracksForFit));
	         //if( TransMass<400. && m_FillHist)m_hb_massPiPi1->Fill( mass_PiPi, w_1);
	       }
//
//  Creation on V0 tracks
//
	       if(BadTracks){
	          std::vector<double> InpMassV0;
                  m_fitSvc->setDefault();                     //Reset VKalVrt settings
                  m_fitSvc->setMomCovCalc(1);                 //Total momentum and its covariance 
		                                              //matrix are calculated
		  if( BadTracks == 1 ) {  // K0 case
		    InpMassV0.push_back(m_massPi);InpMassV0.push_back(m_massPi);
                    m_fitSvc->setMassInputParticles( InpMassV0 );
                    m_fitSvc->setMassForConstraint(m_massK0);
                    m_fitSvc->setCnstType(1);       // Set mass  constraint
                  }
		  if( BadTracks == 2 ) {  // Lambda case
	            if( fabs(1./TrkAtVrt[0][2]) > fabs(1./TrkAtVrt[1][2]) ) {
		            InpMassV0.push_back(m_massP);InpMassV0.push_back(m_massPi);
	            }else{  InpMassV0.push_back(m_massPi);InpMassV0.push_back(m_massP); }
                    m_fitSvc->setMassInputParticles( InpMassV0 );
                    m_fitSvc->setMassForConstraint(m_massLam);
                    m_fitSvc->setCnstType(1);       // Set mass  constraint
                  }
		  if( BadTracks == 3 ) {  // Gamma case
		    InpMassV0.push_back(m_massE);InpMassV0.push_back(m_massE);
                    m_fitSvc->setMassInputParticles( InpMassV0 );
                    m_fitSvc->setCnstType(12);       // Set 3d angular constraint
//                    m_fitSvc->setVertexForConstraint(PrimVrt.position().x(),PrimVrt.position().y(),PrimVrt.position().z());
//                    m_fitSvc->setCovVrtForConstraint(0.015*0.015,0.,0.015*0.015,0.,0.,56.*56);
//                    m_fitSvc->setCnstType(7);          // Set pointing to primary vertex constraint
                  }
                  m_fitSvc->setApproximateVertex(FitVertex.x(),FitVertex.y(),FitVertex.z()); 
                  sc=VKalVrtFitBase(TracksForFit,FitVertex, Momentum,Charge,
                                    ErrorMatrix,Chi2PerTrk,TrkAtVrt,Chi2);
                  if(sc.isSuccess()) {
                    //std::cout<<" cnst result="<< massV0( TrkAtVrt,m_massPi,m_massPi)<<", "
                    //                          << massV0( TrkAtVrt,m_massP,m_massPi)<<", "
                    //                          << massV0( TrkAtVrt,m_massE,m_massE)<<", "<<i<<", "<<j<<'\n';
                    sc=m_fitSvc->VKalVrtCvtTool(FitVertex,Momentum,ErrorMatrix,0,VKPerigee,CovPerigee);
                    if(sc.isSuccess()) {
                      const Trk::Track* TT = m_fitSvc->CreateTrkTrack(VKPerigee,CovPerigee); 
                      ImpactSignif=m_fitSvc->VKalGetImpact(TT, PrimVrt.position(), 0, Impact, ImpactError);
                      if(m_FillHist){m_hb_impV0->Fill( ImpactSignif, w_1); }
	              if(ImpactSignif>5.) BadTracks=0;
		      delete TT;
	            } else { BadTracks=0;}
	         }  // else { BadTracks=0;}
               }
             }
//
//  Check interactions on pixel layers
//
	     double xvt=FitVertex.x();
	     double yvt=FitVertex.y();
             Dist2D=sqrt( (xvt-m_Xbeampipe)*(xvt-m_Xbeampipe) + (yvt-m_Ybeampipe)*(yvt-m_Ybeampipe) ); 
             if( fabs(Dist2D-m_Rbeampipe)< 1.5) BadTracks = 4;           // Beam Pipe removal
           //Dist2D=sqrt(pow(xvt-m_XlayerB,2.)+pow(yvt-m_YlayerB,2.)); 
             Dist2D=sqrt( (xvt-m_XlayerB)*(xvt-m_XlayerB) + (yvt-m_YlayerB)*(yvt-m_YlayerB) ); 
	     if( fabs(Dist2D-m_RlayerB) < 3.5)  BadTracks = 4;
           //Dist2D=sqrt(pow(xvt-m_Xlayer1,2.)+pow(yvt-m_Ylayer1,2.)); 
	     Dist2D=sqrt( (xvt-m_Xlayer1)*(xvt-m_Xlayer1) + (yvt-m_Ylayer1)*(yvt-m_Ylayer1) );
	     if( fabs(Dist2D-m_Rlayer1) < 4.0)  BadTracks = 4;
	     Dist2D=sqrt( (xvt-m_Xlayer2)*(xvt-m_Xlayer2) + (yvt-m_Ylayer2)*(yvt-m_Ylayer2) );
	     if( fabs(Dist2D-m_Rlayer2) < 5.0)  BadTracks = 4;
//
//  Creation of tracks from V0 list
//	     
	     if( BadTracks ){
	        TrkFromV0.push_back(SelectedTracks[i]);
	        TrkFromV0.push_back(SelectedTracks[j]);
	     }else{
                Dist2D=sqrt( (xvt-m_XlayerB)*(xvt-m_XlayerB) + (yvt-m_YlayerB)*(yvt-m_YlayerB) ); 
		int hBL1,hBL2,hhh1,hhh2;
                getPixelLayers(TracksForFit[0], hBL1 , hhh1, hhh2 );
                getPixelLayers(TracksForFit[1], hBL2 , hhh1, hhh2 );

                if( Dist2D > (m_RlayerB+5.) &&  ( hBL1 || hBL2 ) ) continue;
                if( Dist2D < (m_RlayerB-5.) ) {
		    if( hBL1==0 && hBL2==0  ) continue;
	            if( Signif3D <10. && (hBL1*hBL2)==0)continue;
		}
                if(TrackPt[i]<m_JetPtFractionCut*JetDir.Perp())continue;
                if(TrackPt[j]<m_JetPtFractionCut*JetDir.Perp())continue;
//
  		double JetVrtDir =
   	          JetDir.Px()*(FitVertex.x()-PrimVrt.position().x())
                + JetDir.Py()*(FitVertex.y()-PrimVrt.position().y())
 		+ JetDir.Pz()*(FitVertex.z()-PrimVrt.position().z());
                if(m_getNegativeTail) JetVrtDir=fabs(JetVrtDir);  // For negative TAIL
  	                                                          // accepts also negative
  	                                                          // track pairs
                if(m_getNegativeTag) JetVrtDir=-JetVrtDir;        // For negative TAG
  	                                                          // accepts only negative
  	                                                          // track pairs
	        if( (Signif3D>m_Sel2VrtSigCut) && (JetVrtDir>0) ) {
                  if(Signif3Dproj<m_Sel2VrtSigCut)continue;
	          ListSecondTracks.push_back(SelectedTracks[i]);
	          ListSecondTracks.push_back(SelectedTracks[j]);
                  saveSignif3D = Signif3D;
                }
	     }
         }
      } 
      if(ListSecondTracks.size()>0 ){ 
        if(ListSecondTracks.size()<3 && saveSignif3D < (m_Sel2VrtSigCut+m_Sel2VrtSigNTrkDep*NRefTrk)){ ListSecondTracks.clear(); return; }
        if(m_FillHist)m_pr_effVrt2tr->Fill((float)NRefTrk,1.);
      } else if(ListSecondTracks.size()==0) { if(m_FillHist)m_pr_effVrt2tr->Fill((float)NRefTrk,0.);}
   }




   Amg::MatrixX InDetVKalVxInJetTool::makeVrtCovMatrix( std::vector<double> & ErrorMatrix )
   const
   {
      Amg::MatrixX VrtCovMtx(3,3);  
      VrtCovMtx(0,0)                  = ErrorMatrix[0];
      VrtCovMtx(0,1) = VrtCovMtx(1,0) = ErrorMatrix[1];
      VrtCovMtx(1,1)                  = ErrorMatrix[2];
      VrtCovMtx(0,2) = VrtCovMtx(2,0) = ErrorMatrix[3];
      VrtCovMtx(1,2) = VrtCovMtx(2,1) = ErrorMatrix[4];
      VrtCovMtx(2,2)                  = ErrorMatrix[5];
      return VrtCovMtx;
   }


}  //end of namespace

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//
// InDetVKalVxInJetTool.h - Description
//
/*
   Tool for secondary vertex inside jet reconstruction.
   It returns a pointer to Trk::VxSecVertexInfo object which contains
   vector of pointers to VxCandidates of found secondary verteces.
   In case of failure pointer to Trk::VxSecVertexInfo is 0.
   

   Package creates a derivative object VxSecVKalVertexInfo which contains also additional variables
   see  Tracking/TrkEvent/VxSecVertex/VxSecVertex/VxSecVKalVertexInfo.h
   
    
   Additional returned values:
      Results    - vector of variables for b-tagging
                   [0] - secondary vertex mass
                   [1] - energy ratio   Esec/Ejet
		   [2] - number of 2-track vertices
		   [3] - pointers to "bad" tracks
		   [4] - pointers to identified V0 tracks

  Package has "single vertex" and "multiple vertices" modes of work.

  More details at

     https://twiki.cern.ch/twiki/bin/save/Atlas/BTagVrtSec

    Author: Vadim Kostyukhin
    e-mail: vadim.kostyukhin@cern.ch

-----------------------------------------------------------------------------*/



#ifndef _VKalVrt_InDetVKalVxInJetTool_H
#define _VKalVrt_InDetVKalVxInJetTool_H
// Normal STL and physical vectors
#include <vector>
// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
//
//#include  "TrkTrack/Track.h"
#include  "Particle/TrackParticle.h"
#include  "xAODTracking/TrackParticleContainer.h"
//
#include  "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
//
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"

#include "InDetRecToolInterfaces/ISecVertexInJetFinder.h"



  typedef std::vector<double> dvect;

  namespace Trk { 
    class ITrackToVertexIPEstimator;
    class ITrackParticleCreatorTool;
  }
  
//------------------------------------------------------------------------
namespace InDet {


  class VKalVxInJetTemp{
    public:
      std::vector<int> m_Incomp;
      std::vector<int> m_Prmtrack;

  };

  struct workVectorArrxAOD{
        std::vector<const xAOD::TrackParticle*> listJetTracks;
        std::vector<const xAOD::TrackParticle*> tmpListTracks;
        std::vector<const xAOD::TrackParticle*> listSecondTracks;
        std::vector<const xAOD::TrackParticle*> TracksForFit;
        std::vector<const xAOD::TrackParticle*> InpTrk;
        std::vector< std::vector<const xAOD::TrackParticle*> >  FoundSecondTracks;
		     std::vector<const xAOD::TrackParticle*>    TrkFromV0; 
  };

  struct workVectorArrREC{
        std::vector<const Rec::TrackParticle*> listJetTracks;
        std::vector<const Rec::TrackParticle*> tmpListTracks;
        std::vector<const Rec::TrackParticle*> listSecondTracks;
        std::vector<const Rec::TrackParticle*> TracksForFit;
        std::vector<const Rec::TrackParticle*> InpTrk;
        std::vector< std::vector<const Rec::TrackParticle*> >  FoundSecondTracks;
		     std::vector<const Rec::TrackParticle*>    TrkFromV0;
  };

  class InDetVKalVxInJetTool : public AthAlgTool, virtual public ISecVertexInJetFinder{


  public:
       /* Constructor */
      InDetVKalVxInJetTool(const std::string& type, const std::string& name, const IInterface* parent);
       /* Destructor */
      virtual ~InDetVKalVxInJetTool();


      StatusCode initialize();
      StatusCode finalize();

    
      const Trk::VxSecVertexInfo* findSecVertex(const Trk::RecVertex & primaryVertex,
					        const TLorentzVector & jetMomentum,
					        const std::vector<const Rec::TrackParticle*> & inputTracks) const;

      const Trk::VxSecVertexInfo* findSecVertex(const Trk::RecVertex & primaryVertex,
					        const TLorentzVector & jetMomentum,
					        const std::vector<const Trk::TrackParticleBase*> & inputTracks) const;

      const Trk::VxSecVertexInfo* findSecVertex(const xAOD::Vertex & primaryVertex,
                                                const TLorentzVector & jetMomentum,
                                                const std::vector<const xAOD::IParticle*> & inputTracks) const;

//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//

    private:

      double w_1;
      TH1D* m_hb_massPiPi;
      TH1D* m_hb_massPiPi1;
      TH1D* m_hb_massPiPi2;
      TH1D* m_hb_massPPi;
      TH1D* m_hb_massEE;
      TH1D* m_hb_totmassEE;
      TH1D* m_hb_totmass2T0;
      TH1D* m_hb_totmass2T1;
      TH1D* m_hb_totmass2T2;
      TH1D* m_hb_nvrt2;
      TH1D* m_hb_ratio;
      TH1D* m_hb_totmass;
      TH1D* m_hb_impact;
      TH1D* m_hb_impactR;
      TH2D* m_hb_impactRZ;
      TH1D* m_hb_ntrkjet;
      TH1D* m_hb_impactZ;
      TH1D* m_hb_r2d;
      TH1D* m_hb_jmom;
      TH1D* m_hb_mom;
      TH1D* m_hb_signif3D;
      TH1D* m_hb_impV0;
      TH1D* m_hb_sig3DTot;
      TH1F* m_hb_goodvrtN;
      TH1D* m_hb_distVV;
      TH1D* m_hb_diffPS;
      TH1D* m_hb_sig3D2tr;
      TH1D* m_hb_trkPtMax;
      TH1D* m_hb_tr2SelVar;
      TH1F* m_hb_blshared;
      TH1F* m_hb_pxshared;
      TH1F* m_hb_addRatioMV;
      TH1F* m_hb_addChi2MV;
      TH1F* m_hb_addNVrtMV;
      TProfile * m_pr_effVrt2tr;
      long int m_iflag;

      SimpleProperty<int>    m_Robustness;
      SimpleProperty<double> m_MassForConstraint;
      std::vector<double>    m_MassInputParticles;



      long int m_CutSctHits;
      long int m_CutPixelHits;
      long int m_CutSiHits;
      long int m_CutBLayHits;
      long int m_CutSharedHits;
      double m_CutPt;
      double m_CutZVrt;
      double m_CutA0;
      double m_CutChi2;
      double m_SecTrkChi2Cut;
      double m_ConeForTag;
      double m_Sel2VrtChi2Cut;
      double m_Sel2VrtSigCut;
      double m_Sel2VrtSigNTrkDep;
      double m_TrkSigCut;
      double m_TrkSigSumCut;
      double m_TrkSigSumNTrkDep;
      double m_A0TrkErrorCut;
      double m_ZTrkErrorCut;
      double m_AntiPileupSigRCut;
      double m_AntiPileupSigZCut;
      double m_AntiFake2trVrtCut;
      double m_JetPtFractionCut;

      bool m_FillHist;

      long int m_RobustFit;

      double m_Xbeampipe;
      double m_Ybeampipe;
      double m_XlayerB;
      double m_YlayerB;
      double m_Xlayer1;
      double m_Ylayer1;
      double m_Xlayer2;
      double m_Ylayer2;
      double m_Rbeampipe;
      double m_RlayerB;
      double m_Rlayer1;
      double m_Rlayer2;

      long int m_MassType;
      bool     m_MultiVertex;
      bool     m_MultiWithPrimary;
      bool     m_getNegativeTail;
      bool     m_getNegativeTag;
      bool     m_MultiWithOneTrkVrt;

      double    m_VertexMergeCut;
      double    m_TrackDetachCut;


      ToolHandle < Trk::IVertexFitter >       m_fitterSvc;
//      ToolHandle < Trk::ITrkVKalVrtFitter >   m_fitSvc;
      ToolHandle < Trk::ITrackToVertexIPEstimator > m_trackToVertexIP;
      ToolHandle< Trk::ITrackParticleCreatorTool > m_trkPartCreator;
      Trk::TrkVKalVrtFitter*   m_fitSvc;
 

      double m_massPi ;
      double m_massP ;
      double m_massE;
      double m_massK0;
      double m_massLam;

// For multivertex version only

      float m_chiScale[11];
      VKalVxInJetTemp*  m_WorkArray;     
      struct WrkVrt 
     {   bool Good;
         std::deque<long int> SelTrk;
         Amg::Vector3D     vertex;
         TLorentzVector    vertexMom;
         long int   vertexCharge;
         std::vector<double> vertexCov;
         std::vector<double> Chi2PerTrk;
         std::vector< std::vector<double> > TrkAtVrt;
         double Chi2;
         int nCloseVrt;
         double dCloseVrt;
	 double ProjectedVrt;
	 };


//
//   Private technical functions
//
//
      xAOD::Vertex*      GetVrtSec(const std::vector<const xAOD::TrackParticle*> & InpTrk,
                                   const xAOD::Vertex                            & PrimVrt,
                                   const TLorentzVector                          & JetDir,
                                   std::vector<double>                           & Results,
                                   std::vector<const xAOD::TrackParticle*>       & SelSecTrk,
                                   std::vector<const xAOD::TrackParticle*>       & TrkFromV0) const;
      xAOD::Vertex*      GetVrtSec(const std::vector<const Rec::TrackParticle*> & InpTrk,
                                   const xAOD::Vertex                           & PrimVrt,
                                   const TLorentzVector                         & JetDir,
                                   std::vector<double>                          & Results,
                                   std::vector<const Rec::TrackParticle*>       & SelSecTrk,
                                   std::vector<const Rec::TrackParticle*>       & TrkFromV0) const;
      std::vector<xAOD::Vertex*> GetVrtSecMulti(
                                         workVectorArrxAOD * ,
                                         workVectorArrREC  * ,
                                   const xAOD::Vertex                          & PrimVrt,
	                           const TLorentzVector                        & JetDir,
	                           std::vector<double>                         & Results) const;


      int CheckKilledVertices( WrkVrt *KilledVertex, std::vector<WrkVrt> *VrtSet) const;

      void  TrackClassification(std::vector<WrkVrt> *WrkVrtSet, 
                                std::vector< std::deque<long int> > *TrkInVrt) const;

      double MaxOfShared(std::vector<WrkVrt> *WrkVrtSet, 
                         std::vector< std::deque<long int> > *TrkInVrt,
			 long int & SelectedTrack,
			 long int & SelectedVertex) const;
      void RemoveTrackFromVertex(std::vector<WrkVrt> *WrkVrtSet, 
                                 std::vector< std::deque<long int> > *TrkInVrt,
				 long int & SelectedTrack,
				 long int & SelectedVertex) const;
//
//

      void printWrkSet(const std::vector<WrkVrt> * WrkSet, const std::string name ) const;


      StatusCode CutTrk(double,double, double , double , double , 
         long int ,long int ,long int , long int ) const;
      double ConeDist(const AmgVector(5) & , const TLorentzVector & ) const;
//
// Gives correct mass assignment in case of nonequal masses
      double massV0( std::vector< std::vector<double> >& TrkAtVrt, double massP, double massPi ) const;
      int FindMax( std::vector<double>& Chi2PerTrk) const;


      TLorentzVector TotalMom(const std::vector<const Trk::Perigee*>& InpTrk) const; 
      TLorentzVector TotalMom(const std::vector<const xAOD::TrackParticle*>& InpTrk) const; 
      TLorentzVector MomAtVrt(const std::vector<double>& InpTrk) const; 
      double           TotalTMom(const std::vector<const Trk::Perigee*> & InpTrk) const; 
      double           TotalTVMom(const Amg::Vector3D &Dir, const std::vector<const Trk::Perigee*>& InpTrk) const; 
      double           pTvsDir(const Amg::Vector3D &Dir, const std::vector<double>& InpTrk) const; 


      double minVrtVrtDist( std::vector<WrkVrt> *WrkVrtSet, int & V1, int & V2) const;
      double minVrtVrtDistNext( std::vector<WrkVrt> *WrkVrtSet, int & V1, int & V2) const;
      void MergeVertices( std::vector<WrkVrt> *WrkVrtSet, int & V1, int & V2) const;
      bool isPart( std::deque<long int> test, std::deque<long int> base) const;

      double VrtVrtDist(const Trk::RecVertex & PrimVrt, const Amg::Vector3D & SecVrt, 
                                  const std::vector<double> VrtErr,double& Signif ) const;
      double VrtVrtDist(const xAOD::Vertex & PrimVrt, const Amg::Vector3D & SecVrt, 
                                  const std::vector<double> VrtErr,double& Signif ) const;
      double VrtVrtDist(const Trk::RecVertex & PrimVrt, const Amg::Vector3D & SecVrt, 
                                  const std::vector<double> SecVrtErr, const TLorentzVector & JetDir) const;
      double VrtVrtDist(const Amg::Vector3D & Vrt1, const std::vector<double>& VrtErr1,
                        const Amg::Vector3D & Vrt2, const std::vector<double>& VrtErr2) const;

      template <class Particle>
      void DisassembleVertex(std::vector<WrkVrt> *WrkVrtSet, int iv, 
                         std::vector<const Particle*>  AllTracks) const;
					  
      double ProjPos(const Amg::Vector3D & Vrt,const TLorentzVector & JetDir) const;

      const Trk::Perigee* GetPerigee( const xAOD::TrackParticle* ) const;
      const Trk::Perigee* GetPerigee( const Rec::TrackParticle* ) const;
      std::vector<const Trk::Perigee*> GetPerigeeVector( const std::vector<const Rec::TrackParticle*>& ) const;
      std::vector<const Trk::Perigee*> GetPerigeeVector( const std::vector<const Trk::TrackParticleBase*>& ) const;


      template <class Trk>
      double FitCommonVrt(std::vector<const Trk*>& ListSecondTracks,
                          const xAOD::Vertex   & PrimVrt,
                          std::vector<double>  & InpMass, 
	                  Amg::Vector3D        & FitVertex,
                          std::vector<double>  & ErrorMatrix,
	                  TLorentzVector       & Momentum,
		     std::vector< std::vector<double> >  & TrkAtVrt) const; 

      template <class Trk>
      void RemoveEntryInList(std::vector<const Trk*>& , int) const;
      template <class Trk>
      void RemoveDoubleEntries(std::vector<const Trk*>& ) const;


      template <class Particle>
      StatusCode RefitVertex( std::vector<WrkVrt> *WrkVrtSet, int SelectedVertex,
                              std::vector<const Particle*> & SelectedTracks) const;

      int   SelGoodTrkParticle( const std::vector<const Rec::TrackParticle*>& InpPart,
                                const xAOD::Vertex                          & PrimVrt,
	                        const TLorentzVector                        & JetDir,
                                      std::vector<const Rec::TrackParticle*>& SelPart) const;
      int   SelGoodTrkParticle( const std::vector<const xAOD::TrackParticle*>& InpPart,
                                const xAOD::Vertex                           & PrimVrt,
	                        const TLorentzVector                         & JetDir,
                                      std::vector<const xAOD::TrackParticle*>& SelPart) const;
      int   SelGoodTrkParticleRelax( const std::vector<const Rec::TrackParticle*>& InpPart,
                                const xAOD::Vertex                               & PrimVrt,
	                        const TLorentzVector                             & JetDir,
                                      std::vector<const Rec::TrackParticle*>& SelPart) const;
      int   SelGoodTrkParticleRelax( const std::vector<const xAOD::TrackParticle*>& InpPart,
                                const xAOD::Vertex                                & PrimVrt,
	                        const TLorentzVector                              & JetDir,
                                           std::vector<const xAOD::TrackParticle*>& SelPart) const;


      template <class Trk>
      void Select2TrVrt(std::vector<const Trk*>  & SelectedTracks,
                        std::vector<const Trk*>  & TracksForFit,
                        const xAOD::Vertex       & PrimVrt,
 	                const TLorentzVector     & JetDir,
                        std::vector<double>      & InpMass, 
	                std::vector<const Trk*>  & TrkFromV0,
	                std::vector<const Trk*>  & ListSecondTracks) const;

     Amg::MatrixX  makeVrtCovMatrix( std::vector<double> & ErrorMatrix ) const;

     Trk::TrackParticleBase *  CreateParticle(const Trk::ExtendedVxCandidate * vxCandidate) const;
     double massError(const Trk::ExtendedVxCandidate * vxCandidate, double mass) const;
     Amg::MatrixX SetFullMatrix(int NTrk, std::vector<double> & Matrix) const;


//////////////////////////////////////////////////////////////////////////////////////////
//   Needed for TrackParticle<->TrackParticleBase story!!!!
//
//     template <class Track>
//     StatusCode VKalVrtFitFastBase(const std::vector<const Track*>& listPart,Amg::Vector3D& Vertex) const;
     StatusCode VKalVrtFitFastBase(const std::vector<const Rec::TrackParticle*>& listPart,Amg::Vector3D& Vertex) const;
     StatusCode VKalVrtFitFastBase(const std::vector<const xAOD::TrackParticle*>& listPart,Amg::Vector3D& Vertex) const;


     void  getPixelLayers(const Rec::TrackParticle* Part, int &blHit, int &l1Hit, int &l2Hit ) const;
     void  getPixelLayers(const xAOD::TrackParticle* Part, int &blHit, int &l1Hit, int &l2Hit ) const;


     StatusCode VKalVrtFitBase(const std::vector<const Rec::TrackParticle*> & listPart,
                                                  Amg::Vector3D&                 Vertex,
                                                  TLorentzVector&                Momentum,
                                                  long int&                      Charge,
                                                  std::vector<double>&           ErrorMatrix,
                                                  std::vector<double>&           Chi2PerTrk,
                                                  std::vector< std::vector<double> >& TrkAtVrt,
                                                  double& Chi2 ) const;
      StatusCode VKalVrtFitBase(const std::vector<const xAOD::TrackParticle*> & listPart,
                                                  Amg::Vector3D&                 Vertex,
                                                  TLorentzVector&                Momentum,
                                                  long int&                      Charge,
                                                  std::vector<double>&           ErrorMatrix,
                                                  std::vector<double>&           Chi2PerTrk,
                                                  std::vector< std::vector<double> >& TrkAtVrt,
                                                  double& Chi2 ) const;
    const std::vector<const Trk::TrackParticleBase*> 
               PartToBase(const std::vector<const Rec::TrackParticle*> & listPart) const;
     const std::vector<const Rec::TrackParticle*> 
               BaseToPart(const std::vector<const Trk::TrackParticleBase*> & listBase) const;
   };


  template <class Trk>
  void InDetVKalVxInJetTool::RemoveEntryInList(std::vector<const Trk*>& ListTracks, int Outlier) const
  {
    if(Outlier < 0 ) return;
    if(Outlier >= (int)ListTracks.size() ) return;
    typename std::vector<const Trk*>::iterator   TransfEnd;
    TransfEnd = remove( ListTracks.begin(), ListTracks.end(), ListTracks[Outlier]);
    ListTracks.erase( TransfEnd,ListTracks.end());
  }     

  template <class Trk>
  void InDetVKalVxInJetTool::RemoveDoubleEntries(std::vector<const Trk*>& ListTracks) const
  {
    typename std::vector<const Trk*>::iterator   TransfEnd;
    sort(ListTracks.begin(),ListTracks.end());
    TransfEnd =  unique(ListTracks.begin(),ListTracks.end());
    ListTracks.erase( TransfEnd, ListTracks.end());
  }     


}  //end namespace

#endif

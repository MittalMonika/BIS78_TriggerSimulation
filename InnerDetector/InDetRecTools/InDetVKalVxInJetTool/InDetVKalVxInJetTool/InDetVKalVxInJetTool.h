/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//
// InDetVKalVxInJetTool.h - Description
//
/*
   Tool for secondary vertex inside jet reconstruction.
   It returns a pointer to Trk::VxSecVertexInfo object which contains
   vector of pointers to xAOD::Vertex's of found secondary verteces.
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
#include "boost/graph/adjacency_list.hpp"
//
#include  "Particle/TrackParticle.h"
#include  "xAODTracking/TrackParticleContainer.h"
//
//

#include "InDetRecToolInterfaces/ISecVertexInJetFinder.h"

class TH1D;
class TH2D;
class TH1F;
class TProfile;

namespace Trk{
  class TrkVKalVrtFitter;
  class IVertexFitter;
}


typedef std::vector<double> dvect;

 
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
					        const std::vector<const Trk::TrackParticleBase*> & inputTracks) const;

      const Trk::VxSecVertexInfo* findSecVertex(const xAOD::Vertex & primaryVertex,
                                                const TLorentzVector & jetMomentum,
                                                const std::vector<const xAOD::IParticle*> & inputTracks) const;

//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//

    private:

      double m_w_1;
      TH1D* m_hb_massPiPi;
      TH1D* m_hb_massPiPi1;
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
      TH1D* m_hb_r1dc;
      TH1D* m_hb_r2dc;
      TH1D* m_hb_r3dc;
      TH1D* m_hb_rNdc;
      TH1D* m_hb_dstToMat;
      TH1D* m_hb_jmom;
      TH1D* m_hb_mom;
      TH1D* m_hb_signif3D;
      TH1D* m_hb_impV0;
      TH1D* m_hb_sig3DTot;
      TH1F* m_hb_goodvrtN;
      TH1D* m_hb_distVV;
      TH1D* m_hb_diffPS;
      TH1D* m_hb_sig3D1tr;
      TH1D* m_hb_sig3D2tr;
      TH1D* m_hb_sig3DNtr;
      TH1D* m_hb_trkPtMax;
      TH1D* m_hb_tr2SelVar;
      TH1F* m_hb_blshared;
      TH1F* m_hb_pxshared;
      TH1F* m_hb_rawVrtN;
      TH1F* m_hb_lifetime;
      TH1F* m_hb_trkPErr;
      TH1F* m_hb_deltaRSVPV;
//--
      TH1D*  m_hb_massJetTrkSV;
      TH1D*  m_hb_ratioJetTrkSV;
      TH1D*  m_hb_DST_JetTrkSV;
      TH1D*  m_hb_NImpJetTrkSV;
      TH1D*  m_hb_nHImpTrkCnt;
//--
      TProfile * m_pr_effVrt2tr;
      TProfile * m_pr_effVrt2trEta;
      TProfile * m_pr_effVrt;
      TProfile * m_pr_effVrtEta;
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
      double m_TrkSigCut;
      double m_TrkSigNTrkDep;
      double m_TrkSigSumCut;
      double m_A0TrkErrorCut;
      double m_ZTrkErrorCut;
      double m_AntiPileupSigRCut;
      double m_AntiPileupSigZCut;
      double m_AntiFake2trVrtCut;
      double m_JetPtFractionCut;
      int    m_TrackInJetNumberLimit;
      double m_pseudoSigCut;
      double m_hadronIntPtCut;

      bool m_FillHist;

      bool m_existIBL;

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
      double m_Rlayer3;
      double m_SVResolutionR;

      bool     m_useMaterialRejection;
      bool     m_useVertexCleaning;
      int      m_MassType;
      bool     m_MultiVertex;
      bool     m_MultiWithPrimary;
      bool     m_getNegativeTail;
      bool     m_getNegativeTag;
      bool     m_MultiWithOneTrkVrt;

      double    m_VertexMergeCut;
      double    m_TrackDetachCut;


      ToolHandle < Trk::IVertexFitter >       m_fitterSvc;
      Trk::TrkVKalVrtFitter*   m_fitSvc;
 

      double m_massPi ;
      double m_massP ;
      double m_massE;
      double m_massK0;
      double m_massLam;
      double m_massB;
      mutable int m_NRefTrk;    //Measure of track in jet multiplicity
      std::string m_instanceName;


// For multivertex version only

      boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS> *m_compatibilityGraph;
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
         int detachedTrack=-1;
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

      xAOD::Vertex* tryPseudoVertex(const std::vector<const xAOD::TrackParticle*>& Tracks,
                                                      const xAOD::Vertex         & PrimVrt,
                                                      const TLorentzVector       & JetDir,
                                                      const TLorentzVector       & TrkJet,
						      const int                  & nTrkLead,
	                                              std::vector<double>        & Results)  const;

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
      int FindMax( std::vector<double>& Chi2PerTrk, std::vector<int>&  countT) const;


      TLorentzVector TotalMom(const std::vector<const Trk::Perigee*>& InpTrk) const; 
      TLorentzVector TotalMom(const std::vector<const xAOD::TrackParticle*>& InpTrk) const; 
      TLorentzVector MomAtVrt(const std::vector<double>& InpTrk) const; 
      double           TotalTMom(const std::vector<const Trk::Perigee*> & InpTrk) const; 
      double           TotalTVMom(const Amg::Vector3D &Dir, const std::vector<const Trk::Perigee*>& InpTrk) const; 
      double           pTvsDir(const Amg::Vector3D &Dir, const std::vector<double>& InpTrk) const; 

      bool   insideMatLayer(float ,float ) const;

      TLorentzVector GetBDir( const xAOD::TrackParticle* trk1,
                              const xAOD::TrackParticle* trk2,
                              const xAOD::Vertex    & PrimVrt,
			      Amg::Vector3D &V1, Amg::Vector3D &V2) const;

      int   nTrkCommon( std::vector<WrkVrt> *WrkVrtSet, int V1, int V2) const;
      double minVrtVrtDist( std::vector<WrkVrt> *WrkVrtSet, int & V1, int & V2) const;
      double minVrtVrtDistNext( std::vector<WrkVrt> *WrkVrtSet, int & V1, int & V2) const;
      bool isPart( std::deque<long int> test, std::deque<long int> base) const;
      void Clean1TrVertexSet(std::vector<WrkVrt> *WrkVrtSet) const;
      double JetProjDist(Amg::Vector3D &SecVrt, const xAOD::Vertex &PrimVrt, const TLorentzVector &JetDir) const;

      double VrtVrtDist(const Trk::RecVertex & PrimVrt, const Amg::Vector3D & SecVrt, 
                                  const std::vector<double> VrtErr,double& Signif ) const;
      double VrtVrtDist(const xAOD::Vertex & PrimVrt, const Amg::Vector3D & SecVrt, 
                                  const std::vector<double> VrtErr,double& Signif ) const;
      double VrtVrtDist(const Trk::RecVertex & PrimVrt, const Amg::Vector3D & SecVrt, 
                                  const std::vector<double> SecVrtErr, const TLorentzVector & JetDir) const;
      double VrtVrtDist(const xAOD::Vertex & PrimVrt, const Amg::Vector3D & SecVrt, 
                                  const std::vector<double> SecVrtErr, const TLorentzVector & JetDir) const;
      double VrtVrtDist(const Amg::Vector3D & Vrt1, const std::vector<double>& VrtErr1,
                        const Amg::Vector3D & Vrt2, const std::vector<double>& VrtErr2) const;
 
      template <class Particle>
      void DisassembleVertex(std::vector<WrkVrt> *WrkVrtSet, int iv, 
                         std::vector<const Particle*>  AllTracks) const;
					  
      double ProjPos(const Amg::Vector3D & Vrt, const TLorentzVector & JetDir) const;
      double ProjPosT(const Amg::Vector3D & Vrt, const TLorentzVector & JetDir) const;

      const Trk::Perigee* GetPerigee( const xAOD::TrackParticle* ) const;
      const Trk::Perigee* GetPerigee( const Rec::TrackParticle* ) const;
      std::vector<const Trk::Perigee*> GetPerigeeVector( const std::vector<const Rec::TrackParticle*>& ) const;
      std::vector<const Trk::Perigee*> GetPerigeeVector( const std::vector<const Trk::TrackParticleBase*>& ) const;


      template <class Trk>
      double FitCommonVrt(std::vector<const Trk*>& ListSecondTracks,
                          std::vector<int>     & cntComb,
                          const xAOD::Vertex   & PrimVrt,
 	                  const TLorentzVector & JetDir,
                          std::vector<double>  & InpMass, 
	                  Amg::Vector3D        & FitVertex,
                          std::vector<double>  & ErrorMatrix,
	                  TLorentzVector       & Momentum,
		     std::vector< std::vector<double> >  & TrkAtVrt) const; 

      template <class Trk>
      void RemoveEntryInList(std::vector<const Trk*>& , std::vector<int>&, int) const;
      template <class Trk>
      void RemoveDoubleEntries(std::vector<const Trk*>& ) const;

      template <class Trk>
      double RemoveNegImpact(std::vector<const Trk*>& , const xAOD::Vertex &, const TLorentzVector&, const double ) const;

      template <class Particle>
      StatusCode RefitVertex( std::vector<WrkVrt> *WrkVrtSet, int SelectedVertex,
                              std::vector<const Particle*> & SelectedTracks) const;
      template <class Particle>
      double RefitVertex( WrkVrt &Vrt,std::vector<const Particle*> & SelectedTracks) const;

      template <class Particle>
      double FitVertexWithPV( std::vector<WrkVrt> *WrkVrtSet, int SelectedVertex, const xAOD::Vertex & PV,
                              std::vector<const Particle*> & SelectedTracks) const;

      template <class Particle>
      double mergeAndRefitVertices( std::vector<WrkVrt> *WrkVrtSet, int V1, int V2, WrkVrt & newvrt,
                                     std::vector<const Particle*> & AllTrackList) const;

      template <class Particle>
      double  improveVertexChi2( std::vector<WrkVrt> *WrkVrtSet, int V, std::vector<const Particle*> & AllTracks)const;

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

     double trkPtCorr(double pT) const;
     Amg::MatrixX SetFullMatrix(int NTrk, std::vector<double> & Matrix) const;


//////////////////////////////////////////////////////////////////////////////////////////
//   Needed for TrackParticle<->TrackParticleBase story!!!!
//
//     template <class Track>
//     StatusCode VKalVrtFitFastBase(const std::vector<const Track*>& listPart,Amg::Vector3D& Vertex) const;
     StatusCode VKalVrtFitFastBase(const std::vector<const Rec::TrackParticle*>& listPart,Amg::Vector3D& Vertex) const;
     StatusCode VKalVrtFitFastBase(const std::vector<const xAOD::TrackParticle*>& listPart,Amg::Vector3D& Vertex) const;

     template <class Track>
     bool  Check2TrVertexInPixel( const Track* p1, const Track* p2, Amg::Vector3D &, double ) const;
     template <class Track>
     bool  Check1TrVertexInPixel( const Track* p1, Amg::Vector3D &) const;
     template <class Track>
     double medianPtF( std::vector<const Track*> & Trks) const;

     void  getPixelLayers(const  Rec::TrackParticle* Part, int &blHit, int &l1Hit, int &l2Hit, int &nLay) const;
     void  getPixelLayers(const xAOD::TrackParticle* Part, int &blHit, int &l1Hit, int &l2Hit, int &nLay) const;
     void  getPixelDiscs(const xAOD::TrackParticle* Part, int &d0Hit, int &d1Hit, int &d2Hit) const;
     void  getPixelDiscs(const  Rec::TrackParticle* Part, int &d0Hit, int &d1Hit, int &d2Hit) const;
     void  getPixelProblems(const xAOD::TrackParticle* Part, int &splshIBL, int &splshBL ) const;
     void  getPixelProblems(const Rec::TrackParticle* Part, int &splshIBL, int &splshBL ) const;


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
  void InDetVKalVxInJetTool::RemoveEntryInList(std::vector<const Trk*>& ListTracks, std::vector<int> &cnt, int Outlier) const
  {
    if(Outlier < 0 ) return;
    if(Outlier >= (int)ListTracks.size() ) return;
    ListTracks.erase( ListTracks.begin()+Outlier);
    cnt.erase( cnt.begin()+Outlier);
  }     

  template <class Trk>
  void InDetVKalVxInJetTool::RemoveDoubleEntries(std::vector<const Trk*>& ListTracks) const
  {
    typename std::vector<const Trk*>::iterator   TransfEnd;
    sort(ListTracks.begin(),ListTracks.end());
    TransfEnd =  unique(ListTracks.begin(),ListTracks.end());
    ListTracks.erase( TransfEnd, ListTracks.end());
  }     

  struct clique_visitor
  {
    clique_visitor(std::vector< std::vector<int> > & input): m_allCliques(input){ input.clear();}
    
    template <typename Clique, typename Graph>
    void clique(const Clique& clq, Graph& )
    { 
      std::vector<int> new_clique(0);
      for(auto i = clq.begin(); i != clq.end(); ++i) new_clique.push_back(*i);
      m_allCliques.push_back(new_clique);
    }

    std::vector< std::vector<int> > & m_allCliques;

  };

   template <class Track>
   bool InDetVKalVxInJetTool::Check1TrVertexInPixel( const Track* p1, Amg::Vector3D &FitVertex)
   const
   {
	int blTrk=0, blP=0, l1Trk=0, l1P=0, l2Trk=0, nLays=0; 
        getPixelLayers( p1, blTrk , l1Trk, l2Trk, nLays );
        getPixelProblems(p1, blP, l1P );
        double xDif=FitVertex.x()-m_XlayerB, yDif=FitVertex.y()-m_YlayerB ; 
        double Dist2DBL=sqrt(xDif*xDif+yDif*yDif);
        if      (Dist2DBL < m_RlayerB-m_SVResolutionR){       //----------------------------------------- Inside B-layer
          if( blTrk<1  && l1Trk<1  )  return false;
          if(  nLays           <2 )   return false;  // Less than 2 layers on track 0
	  return true;
        }else if(Dist2DBL > m_RlayerB+m_SVResolutionR){      //----------------------------------------- Outside b-layer
          if( blTrk>0 && blP==0 ) return false;  // Good hit in b-layer is present
       }
// 
// L1 and L2 are considered only if vertex is in acceptance
//
	if(fabs(FitVertex.z())<400.){
          xDif=FitVertex.x()-m_Xlayer1, yDif=FitVertex.y()-m_Ylayer1 ;
	  double Dist2DL1=sqrt(xDif*xDif+yDif*yDif);
          xDif=FitVertex.x()-m_Xlayer2, yDif=FitVertex.y()-m_Ylayer2 ; 
	  double Dist2DL2=sqrt(xDif*xDif+yDif*yDif);
          if      (Dist2DL1 < m_Rlayer1-m_SVResolutionR) {   //------------------------------------------ Inside 1st-layer
             if( l1Trk<1  && l2Trk<1  )     return false;  // Less than 1 hits on track 0
             return true;
          }else if(Dist2DL1 > m_Rlayer1+m_SVResolutionR) {  //------------------------------------------- Outside 1st-layer
	     if( l1Trk>0 && l1P==0 )       return false;  //  Good L1 hit is present
          }
          
          if      (Dist2DL2 < m_Rlayer2-m_SVResolutionR) {  //------------------------------------------- Inside 2nd-layer
	     if( l2Trk==0 )  return false;           // At least one L2 hit must be present
          }else if(Dist2DL2 > m_Rlayer2+m_SVResolutionR) {  
	  //   if( l2Trk>0  )  return false;           // L2 hits are present
	  }           
        } else {
	  int d0Trk=0, d1Trk=0, d2Trk=0; 
          getPixelDiscs( p1, d0Trk , d1Trk, d2Trk );
          if( d0Trk+d1Trk+d2Trk ==0 )return false;
        }
        return true;
   }

   template <class Track>
   double InDetVKalVxInJetTool::medianPtF( std::vector<const Track*> & Trks) const{
     int NT=Trks.size(); if(NT==0)return 0.;
     return (Trks[(NT-1)/2]->pt()+Trks[NT/2]->pt())/2.;
   }
}  //end namespace

#endif

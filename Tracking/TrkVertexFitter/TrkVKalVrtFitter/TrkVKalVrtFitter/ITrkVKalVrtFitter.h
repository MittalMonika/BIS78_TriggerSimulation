/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// ITrkVKalVrtFitter.h  - old VKalVrtInterface
//
//  Old interface of VKalVrt.
//  Fully implemented also in Reconstruction/VKalVrt/VKalVrtFitSvc
//  
//---------------------------------------------------------------
#ifndef TRKVKALVRTFITTER_ITRKVKALVRTFITTER_H
#define TRKVKALVRTFITTER_ITRKVKALVRTFITTER_H
// Normal STL and physical vectors
#include <vector>
#include "GeoPrimitives/GeoPrimitives.h"
#include "EventPrimitives/EventPrimitives.h"
// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
//#include "GaudiKernel/IToolSvc.h"
//
#include  "TrkVKalVrtFitter/IVKalState.h"
#include  "TrkTrack/Track.h"
#include  "TrkParticleBase/TrackParticleBase.h"
#include  "xAODTracking/TrackParticleContainer.h" 
#include  "xAODTracking/NeutralParticleContainer.h" 
#include  "xAODTracking/VertexContainer.h"

namespace MagField{
   class IMagFieldSvc;
}


namespace Trk{


   class IExtrapolator;


//------------------------------------------------------------------------
  static const InterfaceID IID_ITrkVKalVrtFitter("ITrkVKalVrtFitter", 1, 0);

  class ITrkVKalVrtFitter : virtual public IAlgTool {
    public:
      static const InterfaceID& interfaceID() { return IID_ITrkVKalVrtFitter;}
//---------------------------------------------------------------------------
//Interface itself

      virtual std::unique_ptr<IVKalState> makeState() = 0;

      virtual StatusCode VKalVrtFit(const  std::vector<const xAOD::TrackParticle*>   & listC,
                                    const  std::vector<const xAOD::NeutralParticle*> & listN,
                       Amg::Vector3D&         Vertex,
                       TLorentzVector&     Momentum,
                       long int&           Charge,
                       std::vector<double>&     ErrorMatrix,
                       std::vector<double>&     Chi2PerTrk,
                       std::vector< std::vector<double> >& TrkAtVrt,
                       double& Chi2,
                       IVKalState& istate = tmpVKalState,
                       bool ifCovV0 = false) =0;
      virtual StatusCode VKalVrtFit(const  std::vector<const Track*> & list,
                       Amg::Vector3D&         Vertex,
                       TLorentzVector&   Momentum,
                       long int&           Charge,
                       std::vector<double>&     ErrorMatrix,
                       std::vector<double>&     Chi2PerTrk,
                       std::vector< std::vector<double> >& TrkAtVrt,
                       double& Chi2,
                       IVKalState& istate = tmpVKalState,
                       bool ifCovV0 = false) =0;
      virtual StatusCode VKalVrtFit(const  std::vector<const TrackParticleBase*> & list,
                       Amg::Vector3D&         Vertex,
                       TLorentzVector&   Momentum,
                       long int&           Charge,
                       std::vector<double>&     ErrorMatrix,
                       std::vector<double>&     Chi2PerTrk,
                       std::vector< std::vector<double> >& TrkAtVrt,
                       double& Chi2,
                       IVKalState& istate = tmpVKalState,
                       bool ifCovV0 = false) =0;
      virtual StatusCode VKalVrtFit(const std::vector<const TrackParameters*> & listC,
                                    const std::vector<const NeutralParameters*>& listN,
                       Amg::Vector3D&         Vertex,
                       TLorentzVector&   Momentum,
                       long int&           Charge,
                       std::vector<double>&     ErrorMatrix,
                       std::vector<double>&     Chi2PerTrk,
                       std::vector< std::vector<double> >& TrkAtVrt,
                       double& Chi2,
                       IVKalState& istate = tmpVKalState,
                       bool ifCovV0 = false) =0;
//------
      virtual StatusCode VKalVrtCvtTool(const Amg::Vector3D& Vertex,
	                          const TLorentzVector& Momentum,
	                          const std::vector<double>& CovVrtMom,
				  const long int& Charge,
				  std::vector<double>& Perigee,
				  std::vector<double>& CovPerigee,
                                  IVKalState& istate = tmpVKalState) =0;
//.........................................................................................
      virtual StatusCode VKalVrtFitFast(const std::vector<const Track*>& list,Amg::Vector3D& Vertex, IVKalState& istate = tmpVKalState) =0;
      virtual StatusCode VKalVrtFitFast(const std::vector<const xAOD::TrackParticle*>& list, Amg::Vector3D& Vertex, IVKalState& istate = tmpVKalState) =0;
      virtual StatusCode VKalVrtFitFast(const std::vector<const TrackParticleBase*>& list, Amg::Vector3D& Vertex, IVKalState& istate = tmpVKalState) =0;
      virtual StatusCode VKalVrtFitFast(const std::vector<const TrackParameters*>& list, Amg::Vector3D& Vertex, IVKalState& istate = tmpVKalState) =0;
//.........................................................................................


      virtual Track* CreateTrkTrack(const std::vector<double>& VKPerigee, const std::vector<double>& VKCov, IVKalState& istate = tmpVKalState) =0;

      virtual StatusCode VKalGetTrkWeights(std::vector<double>& Weights,
                                           IVKalState& istate = tmpVKalState) =0;
      virtual StatusCode VKalGetTrkCov(long int, long int, std::vector<double>& CovMtx) =0;
      virtual StatusCode VKalGetFullCov(long int, std::vector<double>& CovMtx,
                                        IVKalState& istate = tmpVKalState, bool = false) =0;
      virtual StatusCode VKalGetMassError(double& Mass, double& MassError,
                                          IVKalState& istate = tmpVKalState) =0;
      virtual int VKalGetNDOF() =0;

      virtual void setApproximateVertex(double,double,double,
                                        IVKalState& istate = tmpVKalState)=0;
      virtual void setMassForConstraint(double,
                                        IVKalState& istate = tmpVKalState) =0;
      virtual void setMassForConstraint(double, const std::vector<int>&,
                                        IVKalState& istate = tmpVKalState) =0;
      virtual void setRobustness(int, IVKalState& istate = tmpVKalState) =0;
      virtual void setRobustScale(double, IVKalState& istate = tmpVKalState) =0;
      virtual void setCascadeCnstPrec(double)=0;
      virtual void setCnstType(int, IVKalState& istate = tmpVKalState) =0;
      virtual void setMomCovCalc(int)=0;
      virtual void setIterations(int, double)=0;
      virtual void setVertexForConstraint(const xAOD::Vertex &,
                                          IVKalState& istate = tmpVKalState)=0;
      virtual void setVertexForConstraint(double,double,double,
                                          IVKalState& istate = tmpVKalState)=0;
      virtual void setCovVrtForConstraint(double,double,double,
                                          double,double,double,
                                          IVKalState& istate = tmpVKalState)=0;
      virtual void setMassInputParticles( const std::vector<double>&,
                                          IVKalState& istate = tmpVKalState)=0;
      virtual void setDefault() =0;
      virtual void setZeroCharge(int)=0;
//
//  Interface with ATLAS reconstruction software
//
//      virtual void setAthenaField(MagField::IMagFieldSvc*)=0;
//      virtual void setAthenaPropagator(const IExtrapolator*)=0;
//----------------------------------------------------------------------------------------------------

      virtual double VKalGetImpact(const xAOD::TrackParticle*,const Amg::Vector3D& Vertex, const long int Charge,
                               std::vector<double>& Impact,std::vector<double>& ImpactError,
                                   IVKalState& istate = tmpVKalState) = 0;
      virtual double VKalGetImpact(const TrackParticleBase*,const Amg::Vector3D& Vertex, const long int Charge,
                               std::vector<double>& Impact,std::vector<double>& ImpactError,
                                   IVKalState& istate = tmpVKalState) = 0;
      virtual double VKalGetImpact(const Track*,const Amg::Vector3D& Vertex, const long int Charge,
                              std::vector<double>& Impact,std::vector<double>& ImpactError,
                                   IVKalState& istate = tmpVKalState) = 0;

//----------------------------------------------------------------------------------------------------

   };

} //end of namespace

#endif

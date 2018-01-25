/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_PVCASCADETOOLS_H
#define DERIVATIONFRAMEWORK_PVCASCADETOOLS_H

#include "GaudiKernel/ToolHandle.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "DerivationFrameworkBPhys/CascadeTools.h"
#include <vector>

// Authors: Adam Barton <abarton@SPAMMENOTTtttcern.ch>
//          Eva Bouhova <bouhova@SPAMMENOTTtttcern.ch>


//class CascadeTools;
namespace Trk {
  class V0Tools;
}

namespace Analysis{
  class PrimaryVertexRefitter;
}
class IBeamCondSvc;

namespace DerivationFramework {
  
  class BPhysPVCascadeTools {

  private:
       const Trk::V0Tools *m_v0Tools;
       const CascadeTools *m_cascadeTools;
       const ServiceHandle<IBeamCondSvc> *m_beamSpotSvc;

       // minimum number of tracks required in PVs considered
       size_t m_PV_minNTracks;

       // beamspot cache
       mutable Amg::Vector3D m_beamspot;
       mutable bool          m_BScached;
       
  public:
  
       BPhysPVCascadeTools(CascadeTools *cascadeTools);
       BPhysPVCascadeTools(CascadeTools *cascadeTools,
                           const ServiceHandle<IBeamCondSvc> *beamSpotSvc);
         
       
       void ProcessVertex(const std::vector<TLorentzVector> &mom, Amg::MatrixX cov, xAOD::BPhysHypoHelper &vtx, xAOD::BPhysHelper::pv_type pvtype, double mass) const;
    
       static void FillBPhysHelperNULL(xAOD::BPhysHelper &vtx, const xAOD::VertexContainer* PvContainer,
           xAOD::BPhysHelper::pv_type pvtype);
       
       //Fills the BPhysHelper object with the standard parameters
       void FillBPhysHelper(const std::vector<TLorentzVector> &mom, Amg::MatrixX cov, xAOD::BPhysHelper &vtx, const xAOD::Vertex* refPV,const xAOD::VertexContainer* refPvContainer,
                    xAOD::BPhysHelper::pv_type pvtype, int) const;
    
       //Returns the index integer of the vertex with the lowest Z in relation to the given vertex
       size_t FindLowZIndex(const std::vector<TLorentzVector> &mom, const xAOD::BPhysHelper &Obj,
			    const std::vector<const xAOD::Vertex*> &PVlist,
			    const size_t PV_minNTracks=0) const;
       //Returns the index integer of the vertex with the lowest A0 in relation to the given vertex
       size_t FindLowA0Index(const std::vector<TLorentzVector> &mom, const xAOD::BPhysHelper &Obj,
			     const std::vector<const xAOD::Vertex*> &PVlist,
			     const size_t PV_minNTracks=0) const;
       
       static size_t FindHighPtIndex(const std::vector<const xAOD::Vertex*> &PVlist);
       
       // Static method call with
       // DerivationFramework::BPhysDerHelpers::GetGoodPV
       // Returns a std::vector containing only PVs of type 1 and 3 - HighPt
       // and Pileup, which have at least PV_minNTracks tracks.
       static std::vector<const xAOD::Vertex*> GetGoodPV(const xAOD::VertexContainer* pvContainer);
       
       // Set the minimum number of tracks required for primary vertices to be
       // considered for primary vertex association to a secondary vertex.
       // Note that this requirement will not be applied for finding
       // the vertex with the highest pT sum (FindHighPtIndex()) since
       // it would possibly exclude this vertex which has been marked
       // earlier in the tool chain.
       void SetMinNTracksInPV(size_t PV_minNTracks);

       // Get the current beamspot position either from cache or from
       // BeamCondSvc.
       // If resetCache = true, the caching flag will be set to false.
       // Before processing a new event, make sure to call
       // GetBeamSpot(true);
       Amg::Vector3D GetBeamSpot(bool resetCache=false) const;

       // Find the index for the PV with the lowest distance in z of
       // the SV's DOCA point w.r.t. the beamline and the PV.
       size_t FindLowZ0BAIndex(const std::vector<TLorentzVector> &mom, const xAOD::BPhysHelper &obj,
			       const std::vector<const xAOD::Vertex*> &PVlist,
			       const size_t PV_minNTracks=0) const;
       // Calculate the distance along z axis between the PV and
       //  SV's DOCA point w.r.t. the beamline.
       double DistInZtoDOCA(const std::vector<TLorentzVector> &mom, const xAOD::BPhysHelper &obj,
			    const xAOD::Vertex* vertex) const;
       // Point of DOCA w.r.t. the beamline backward extrapolated
       // along the B candidate's momentum direction. 
       Amg::Vector3D DocaExtrapToBeamSpot(const std::vector<TLorentzVector> &mom, const xAOD::BPhysHelper &obj) const;


  }; // class BPhysPVCascadeTools

} // namespace DerivationFramework


#endif // DERIVATIONFRAMEWORK_PVCASCADETOOLS_H

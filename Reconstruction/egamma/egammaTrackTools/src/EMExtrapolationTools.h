/*
   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EGAMMATRACKTOOLS_EMEXTRAPOLATIONTOOLS_H
#define EGAMMATRACKTOOLS_EMEXTRAPOLATIONTOOLS_H
/** 
  @class EMExtrapolationTools
  Tools for track extrapolation to the calorimeter
  @author Thomas Koffas, Christos Anastopoulos
  */

/********************************************************************

NAME:     EMExtrapolationTools.h
PACKAGE:  offline/Reconstruction/egammaTrackTools
AUTHORS:  T. Koffas, C.Anastopoulos
PURPOSE:  Tool which propagate track to 
- calorimeter
- cluster
 ********************************************************************/

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "egammaInterfaces/IEMExtrapolationTools.h"

#include "TrkNeutralParameters/NeutralParameters.h"
#include "TrkParameters/TrackParameters.h"
#include "xAODCaloEvent/CaloClusterFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTracking/VertexFwd.h"

#include "TrkCaloExtension/CaloExtensionHelpers.h"
#include "TrkCaloExtension/CaloExtensionCollection.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "TrkExInterfaces/IExtrapolator.h"

class TRT_ID;
class CaloDepthTool;

namespace Trk
{
class INeutralParticleParameterCalculator;
}


class EMExtrapolationTools : virtual public IEMExtrapolationTools, public AthAlgTool {

public:

  /** @brief Constructor with parameters */
  EMExtrapolationTools (const std::string& type,const std::string& name, const IInterface* parent);
  /** @brief Destructor */
  virtual ~EMExtrapolationTools();

  /** @brief initialize method */
  virtual StatusCode initialize() override final;
  /** @brief finalize method */
  virtual StatusCode finalize() override final;


  typedef  std::unordered_map<size_t,std::unique_ptr<Trk::CaloExtension>> Cache;
 
  /**  test for cluster/extrapolated track match, from xAOD::TrackParticle,
   * returns true for good match using sampling 2, and
   *  the values for eta/phi, deltaEta/deltaPhi 
   */
  virtual bool  matchesAtCalo(const xAOD::CaloCluster*      cluster, 
                              const xAOD::TrackParticle*    trkPB, 
                              bool                          isTRT, 
                              Trk::PropDirection            direction,
                              std::vector<double>&          eta,
                              std::vector<double>&          phi,
                              std::vector<double>&          deltaEta,
                              std::vector<double>&          deltaPhi,
                              unsigned int                  extrapFrom = fromPerigee,
                              Cache* cache=nullptr) const override final;


  /**   get eta, phi, deltaEta, and deltaPhi at the four calorimeter
   *    layers given the Trk::ParametersBase.  */
  virtual StatusCode getMatchAtCalo (const xAOD::CaloCluster*      cluster, 
                                     const xAOD::TrackParticle*    trkPB,
                                     bool                          isTRT, 
                                     Trk::PropDirection            direction,
                                     std::vector<double>&          eta,
                                     std::vector<double>&          phi,
                                     std::vector<double>&          deltaEta,
                                     std::vector<double>&          deltaPhi,
                                     unsigned int                  extrapFrom = fromPerigee,
                                     Cache* cache=nullptr) const override final;



  /** test for vertex-to-cluster match given also the positions 
    at the calorimeter from the vertex extrapolation  **/
  virtual bool matchesAtCalo(const xAOD::CaloCluster* cluster,
                             const xAOD::Vertex *vertex,
                             float etaAtCalo,
                             float phiAtCalo)  const override final;


  /** get eta, phi at EM2 given a vertex which is converted to NeutralParameters.
    Return false if the extrapolation fails **/
  virtual bool getEtaPhiAtCalo (const xAOD::Vertex* vertex, 
                                float *etaAtCalo,
                                float *phiAtCalo) const override final;

  /** get eta, phi at EM2 given NeutralParameters.
    Return false if the extrapolation fails **/
  virtual bool getEtaPhiAtCalo (const Trk::TrackParameters* trkPar, 
                                float *etaAtCalo,
                                float *phiAtCalo) const override final;

  /** get the momentum of the i-th at the vertex (designed for conversions) **/
  Amg::Vector3D getMomentumAtVertex(const xAOD::Vertex&, unsigned int) const override final;

  /** get sum of the momenta at the vertex (designed for conversions). Retrieve from auxdata if available and \<reuse\> is true **/
  Amg::Vector3D getMomentumAtVertex(const xAOD::Vertex&, bool reuse = true) const override final;


private:

  /** @Helper to get the per Layer Intersections **/ 
  CaloExtensionHelpers::EtaPhiPerLayerVector getIntersections (const Trk::CaloExtension& extension,const xAOD::CaloCluster* cluster) const;

  /** @Perform the Rescaling of the perigee parameters with the cluster energy **/
  const Trk::TrackParameters*  getRescaledPerigee(const xAOD::TrackParticle* trkPB, const xAOD::CaloCluster* cluster) const;

  /** @brief Return +/- 1 (2) if track is in positive/negative TRT barrel (endcap) **/
  int getTRTsection(const xAOD::TrackParticle* trkPB) const;

  ToolHandle<Trk::IParticleCaloExtensionTool> m_lastParticleCaloExtensionTool {this,
    "LastCaloExtentionTool", "Trk::ParticleCaloExtensionTool/EMLastCaloExtensionTool"};

  ToolHandle<Trk::IParticleCaloExtensionTool> m_perigeeParticleCaloExtensionTool {this,
    "PerigeeCaloExtentionTool", "Trk::ParticleCaloExtensionTool/EMParticleCaloExtensionTool"};

  ToolHandle<Trk::IExtrapolator> m_extrapolator {this, 
    "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator"};

  // Track-to-cluster match cuts
  Gaudi::Property<double> m_broadDeltaEta{this, "BroadDeltaEta", 0.05};
  Gaudi::Property<double> m_broadDeltaPhi{this, "BroadDeltaPhi", 0.10};
  Gaudi::Property<double> m_narrowDeltaEta{this, "NarrowDeltaEta", 0.05};
  Gaudi::Property<double> m_narrowDeltaPhi{this, "NarrowDeltaPhi", 0.05};
  Gaudi::Property<double> m_narrowDeltaPhiBrem{this,
    "NarrowDeltaPhiBrem", 0.10};
  Gaudi::Property<double> m_narrowDeltaPhiTRTbarrel{this,
    "NarrowDeltaPhiTRTbarrel", 0.02};
  Gaudi::Property<double> m_narrowDeltaPhiBremTRTbarrel{this,
    "NarrowDeltaPhiBremTRTbarrel", 0.03};
  Gaudi::Property<double> m_narrowDeltaPhiTRTendcap{this,
    "NarrowDeltaPhiTRTendcap", 0.02};
  Gaudi::Property<double> m_narrowDeltaPhiBremTRTendcap{this,
    "NarrowDeltaPhiBremTRTendcap", 0.03};
  Gaudi::Property<double> m_TRTbarrelDeltaEta{this, "TRTbarrelDeltaEta", 0.35};
  Gaudi::Property<double> m_TRTendcapDeltaEta{this, "TRTendcapDeltaEta", 0.2};

  // ID TRT helper
  const TRT_ID*                         m_trtId;

  //Cache collections for GSF Track Particle extrapolation Perigee
  SG::ReadHandleKey<CaloExtensionCollection>  m_GSFPerigeeCacheKey{this,
  "GSFPerigeeCache", "GSFPerigeeCaloExtension", "Name of GSF Perigee extrapolation cache"};
  SG::ReadHandleKey<CaloExtensionCollection>  m_GSFLastCacheKey{this,
  "GSFLastCache", "GSFLastCaloExtension", "Name of GSF Last measurement extrapolation cache"};
 //Use the a cache for GSF Track Particle extrapolation
  Gaudi::Property<bool>  m_useCaching {this,"useCaching", false, "Use a CaloExtension Collection as cache"};

};


#endif

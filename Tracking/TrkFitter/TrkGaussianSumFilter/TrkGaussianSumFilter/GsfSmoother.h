/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/* *******************************************************************************
      GsfSmoother.h  -  description
      -----------------------------
begin                : Wednesday 9th March 2005
author               : atkinson
email                : Tom.Atkinson@cern.ch
decription           : Class definition for the GSF smoother
********************************************************************************** */

#ifndef TrkGsfSmoother_H
#define TrkGsfSmoother_H

#include "TrkGaussianSumFilter/IMultiComponentStateCombiner.h"
#include "TrkGaussianSumFilter/IGsfSmoother.h"

#include "TrkMultiComponentStateOnSurface/MultiComponentState.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkFitterUtils/FitterTypes.h"

#include "TrkGaussianSumFilter/IMultiComponentStateMerger.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

namespace Trk {

class IMultiStateMeasurementUpdator;
class IMultiStateExtrapolator;
class CaloCluster_OnTrack;

class GsfSmoother : public AthAlgTool, virtual public IGsfSmoother {

 public:

  /** Constructor with AlgTool parameters */
  GsfSmoother(const std::string&, const std::string&, const IInterface*);

  /** Virtual destructor */
  virtual ~GsfSmoother() {};

  /** AlgTool initialise method */
  StatusCode initialize();

  /** AlgTool finalise method */
  StatusCode finalize();

  /** Configure the GSF smoother
      - Configure the extrapolator
      - Configure the measurement updator */
  virtual StatusCode configureTools ( const ToolHandle<IMultiStateExtrapolator> &, 
                                      const ToolHandle<IMultiStateMeasurementUpdator> &);

  /** Gsf smoother method */
  virtual SmoothedTrajectory* fit (const ForwardTrajectory&, 
                                   const ParticleHypothesis particleHypothesis = nonInteracting, 
                                   const CaloCluster_OnTrack* ccot = 0  ) const;

 private:

  /** Method for combining the forwards fitted state and the smoothed state */
  const MultiComponentState* combine (const MultiComponentState&, const MultiComponentState&) const;
  
  /** Methof to add the CaloCluster onto the track */
  const MultiComponentState* addCCOT( const Trk::TrackStateOnSurface* currentState, 
                                      const Trk::CaloCluster_OnTrack* ccot,  
                                      Trk::SmoothedTrajectory* smoothedTrajectory) const;


 private:
  bool                                      m_combineWithFitter;
 ToolHandle<IMultiComponentStateMerger>    m_merger{this,
    "ComponentMerger", "Trk::QuickCloseComponentsMultiStateMerger/CloseComponentsMultiStateMerger",""};
  ToolHandle<IMultiComponentStateCombiner>  m_combiner{this,    
    "MultiComponentStateCombiner","Trk::MultiComponentStateCombiner/GsfSmootherCombiner",""};
  /*
   * Special Tool Handles set by the configureTools
   */
  ToolHandle<IMultiStateExtrapolator>       m_extrapolator;
  ToolHandle<IMultiStateMeasurementUpdator> m_updator;
 

};

} // end Trk namespace

#endif

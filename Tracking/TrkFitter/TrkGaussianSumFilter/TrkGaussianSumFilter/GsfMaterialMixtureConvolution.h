/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/*************************************************************************************
                        GsfMaterialMixtureConvolution.h  -  description
                        -----------------------------------------------
begin                : Thursday 7th September 2006
author               : atkinson
email                : Tom.Atkinson@cern.ch
decription           : Class description for convolution of GSF material mixture
************************************************************************************/

#ifndef TrkGsfMaterialMixtureConvolution_H
#define TrkGsfMaterialMixtureConvolution_H

#include "TrkGaussianSumFilter/IMaterialMixtureConvolution.h"
#include "TrkGaussianSumFilter/IMultiComponentStateAssembler.h"
#include "TrkGaussianSumFilter/IMultiComponentStateCombiner.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

namespace Trk {

class IMultiStateMaterialEffectsUpdator;
class MultiComponentState;
class Layer;

class GsfMaterialMixtureConvolution
  : public AthAlgTool
  , virtual public IMaterialMixtureConvolution
{

public:
  //!< Constructor with AlgTool parameters
  GsfMaterialMixtureConvolution(const std::string&, const std::string&, const IInterface*);

  //!< Destructor
  ~GsfMaterialMixtureConvolution();

  //!< AlgTool initialise method
  StatusCode initialize();

  //!< AlgTool finalize method
  StatusCode finalize();

  //!< Convolution with full material properties
  virtual const MultiComponentState* update(const MultiComponentState&,
                                            const Layer&,
                                            PropDirection direction = anyDirection,
                                            ParticleHypothesis particleHypothesis = nonInteracting) const;

  //!< Convolution with pre-measurement-update material properties
  virtual const MultiComponentState* preUpdate(const MultiComponentState&,
                                               const Layer&,
                                               PropDirection direction = anyDirection,
                                               ParticleHypothesis particleHypothesis = nonInteracting) const;

  //!< Convolution with post-measurement-update material properties
  virtual const MultiComponentState* postUpdate(const MultiComponentState&,
                                                const Layer&,
                                                PropDirection direction = anyDirection,
                                                ParticleHypothesis particleHypothesis = nonInteracting) const;

  //!< Retain for now redundant simplified material effects
  virtual const MultiComponentState* simpliedMaterialUpdate(
    const MultiComponentState& multiComponentState,
    PropDirection direction = anyDirection,
    ParticleHypothesis particleHypothesis = nonInteracting) const;

private:
  ToolHandle<IMultiStateMaterialEffectsUpdator> m_updator{ this,
                                                           "MaterialEffectsUpdator",
                                                           "Trk::GsfMaterialEffectsUpdator/GsfMaterialEffectsUpdator",
                                                           "" };
  ToolHandle<IMultiComponentStateCombiner> m_stateCombiner{
    this,
    "MultiComponentStateCombiner",
    "Trk::MultiComponentStateCombiner/MultiComponentStateCombiner",
    ""
  };
  ToolHandle<IMultiComponentStateAssembler> m_stateAssembler{
    this,
    "MultiComponentStateAssembler",
    "Trk::MultiComponentStateAssembler/MaterialConvolutionAssembler",
    ""
  };
};

} // end Trk namespace

#endif

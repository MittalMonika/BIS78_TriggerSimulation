/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/*********************************************************************************
      GsfMeasurementUpdator.h  -  description
      ---------------------------------------
begin                : Friday 25th February 2005
author               : atkinson
email                : Tom.Atkinson@cern.ch
decription           : Class for performing updates on multi-component states for
                       the gaussian-sum filter. Now an AlgTool
*********************************************************************************/

#ifndef TrkGsfMeasurementUpdator_H
#define TrkGsfMeasurementUpdator_H

#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkGaussianSumFilter/MultiComponentStateAssembler.h"

#include "TrkGaussianSumFilter/IMultiStateMeasurementUpdator.h"
#include "TrkToolInterfaces/IUpdator.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

namespace Trk {

class LocalParameters;

class GsfMeasurementUpdator
  : public AthAlgTool
  , virtual public IMultiStateMeasurementUpdator
{

private:
  /** Private typedef for calling the correct updator member function depending of direction of fitting */
  typedef Trk::TrackParameters* (Trk::IUpdator::*Updator)(const Trk::TrackParameters&,
                                                          const LocalParameters&,
                                                          const Amg::MatrixX&,
                                                          FitQualityOnSurface*&)const;

public:
  /** Constructor with parameters to be passed to AlgTool */
  GsfMeasurementUpdator(const std::string, const std::string, const IInterface*);

  /** Virtual destructor */
  virtual ~GsfMeasurementUpdator(){};

  /** AlgTool initialise method */
  StatusCode initialize() override;

  /** AlgTool finalise method */
  StatusCode finalize() override;

  /** Method for updating the multi-state with a new measurement */
  virtual std::unique_ptr<MultiComponentState> update(MultiComponentState&&,
                                                      const MeasurementBase&) const override final;

  /** Method for updating the multi-state with a new measurement and calculate the fit qaulity at the same time*/
  virtual std::unique_ptr<MultiComponentState> update(
    Trk::MultiComponentState&&,
    const Trk::MeasurementBase&,
    std::unique_ptr<FitQualityOnSurface>& fitQoS) const override final;

  /** Method for GSF smoother to calculate unbiased parameters of the multi-component state */
  virtual std::unique_ptr<MultiComponentState> 
    getUnbiasedTrackParameters(MultiComponentState&&,
                               const MeasurementBase&) const override final;

  /** Method for determining the chi2 of the multi-component state and the number of degrees of freedom */
  virtual const FitQualityOnSurface* fitQuality(const MultiComponentState&, const MeasurementBase&) const override;

private:
  std::unique_ptr<MultiComponentState> calculateFilterStep(MultiComponentState&&,
                                                           const MeasurementBase&,
                                                           const Updator) const;

  std::unique_ptr<MultiComponentState> calculateFilterStep(MultiComponentState&&,
                                                           const MeasurementBase&,
                                                           std::unique_ptr<FitQualityOnSurface>& fitQoS) const;

  bool invalidComponent(const Trk::TrackParameters* trackParameters) const;

  std::unique_ptr<MultiComponentState> rebuildState(Trk::MultiComponentState&& stateBeforeUpdate) const;

private:
  ToolHandle<IUpdator> m_updator{ this, "Updator", "Trk::KalmanUpdator/KalmanUpdator", "" };
};
}

#endif

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/*********************************************************************************
      PosteriorWeightsCalculator.cxx  -  description
      -----------------------------------------------
created              : Wednesday 22nd December 2004
author               : amorley, atkinson
email                : Anthony.Morley@cern.ch, Tom.Atkinson@cern.ch
description          : Implementation code for PosteriorWeightsCalculator class
*********************************************************************************/

#include "TrkGaussianSumFilter/PosteriorWeightsCalculator.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkToolInterfaces/IUpdator.h"

Trk::PosteriorWeightsCalculator::PosteriorWeightsCalculator(const std::string& type,
                                                            const std::string& name,
                                                            const IInterface* parent)
  : AthAlgTool(type, name, parent)
  , m_reMatrices(5)
{

  declareInterface<IPosteriorWeightsCalculator>(this);
}

StatusCode
Trk::PosteriorWeightsCalculator::initialize()
{

  ATH_MSG_INFO("Initialisation of " << name() << " was successful");

  return StatusCode::SUCCESS;
}

StatusCode
Trk::PosteriorWeightsCalculator::finalize()
{

  ATH_MSG_INFO("Finalisation of " << name() << " was successful");

  return StatusCode::SUCCESS;
}

std::unique_ptr<Trk::MultiComponentState>
Trk::PosteriorWeightsCalculator::weights(const MultiComponentState& predictedState,
                                         const MeasurementBase& measurement) const
{

  ATH_MSG_VERBOSE("Calculating Posterior Weights");
  const size_t predictedStateSize=predictedState.size();
  if (predictedStateSize==0) {
    ATH_MSG_WARNING("Predicted state is empty... Exiting!");
    return nullptr;
  }

  ATH_MSG_VERBOSE("State for update is valid!");

  std::unique_ptr<Trk::MultiComponentState> returnMultiComponentState = std::make_unique<Trk::MultiComponentState>();

  std::vector<double> componentDeterminantR;
  std::vector<double> componentChi2;
  componentDeterminantR.reserve(predictedStateSize);
  componentChi2.reserve(predictedStateSize);
  // Calculate chi2 and determinant of each component.
  double minimumChi2(10.e10); // Initalise high

  // Loop over all components in the prediction state
  Trk::MultiComponentState::const_iterator component = predictedState.begin();

  for (; component != predictedState.end(); ++component) {

    const Trk::TrackParameters* componentTrackParameters = (*component).first;

    if (!componentTrackParameters) {
      ATH_MSG_DEBUG("Component in the state prepared for update is invalid... Ignoring!");
      continue;
    }

    const AmgSymMatrix(5)* predictedCov = componentTrackParameters->covariance();

    if (!predictedCov) {
      ATH_MSG_WARNING("No measurement associated with track parameters... Ignoring!");
      continue;
    }

    ATH_MSG_VERBOSE("Component for update is valid!");

    const Trk::LocalParameters& measurementLocalParameters = measurement.localParameters();
    std::pair<double, double> result(0, 0);
    int nLocCoord = measurement.localParameters().dimension();
    if (nLocCoord == 1) {
      result = calculateWeight_1D(componentTrackParameters,
                                  predictedCov,
                                  measurementLocalParameters(0),
                                  measurement.localCovariance()(0, 0),
                                  measurementLocalParameters.parameterKey());
    } else if (nLocCoord == 2) {
      if (measurementLocalParameters.parameterKey() == 3) {
        result = calculateWeight_2D_3(componentTrackParameters,
                                      predictedCov,
                                      measurementLocalParameters.block<2, 1>(0, 0),
                                      measurement.localCovariance().block<2, 2>(0, 0));
      } else {
        result = calculateWeight_T<2>(componentTrackParameters,
                                      predictedCov,
                                      measurementLocalParameters.block<2, 1>(0, 0),
                                      measurement.localCovariance().block<2, 2>(0, 0),
                                      measurementLocalParameters.parameterKey());
      }
    } else if (nLocCoord == 3) {
      result = calculateWeight_T<3>(componentTrackParameters,
                                    predictedCov,
                                    measurementLocalParameters.block<3, 1>(0, 0),
                                    measurement.localCovariance().block<3, 3>(0, 0),
                                    measurementLocalParameters.parameterKey());
    } else if (nLocCoord == 4) {
      result = calculateWeight_T<4>(componentTrackParameters,
                                    predictedCov,
                                    measurementLocalParameters.block<4, 1>(0, 0),
                                    measurement.localCovariance().block<4, 4>(0, 0),
                                    measurementLocalParameters.parameterKey());
    } else if (nLocCoord == 5) {
      result = calculateWeight_T<5>(componentTrackParameters,
                                    predictedCov,
                                    measurementLocalParameters.block<5, 1>(0, 0),
                                    measurement.localCovariance().block<5, 5>(0, 0),
                                    measurementLocalParameters.parameterKey());
    } else {
      ATH_MSG_ERROR("Dimension error in PosteriorWeightsCalculator::weights.");
      break;
    }

    if (result.first == 0) {
      ATH_MSG_WARNING("Determinant is 0, cannot invert matrix... Ignoring component");
      continue;
    }
    // Compute Chi2

    ATH_MSG_VERBOSE("determinant R / chiSquared: " << result.first << '\t' << result.second);

    componentDeterminantR.push_back(result.first);
    componentChi2.push_back(result.second);

    if (result.second < minimumChi2)
      minimumChi2 = result.second;

  } // end loop over components

  if (componentDeterminantR.size() != predictedState.size() || componentChi2.size() != predictedState.size()) {
    ATH_MSG_WARNING("Inconsistent number of components in chi2 and detR vectors... Exiting!");
    returnMultiComponentState.reset();
    return nullptr;
  }

  // Calculate posterior weights.

  unsigned int index(0);
  double sumWeights(0.);

  component = predictedState.begin();

  for (; component != predictedState.end(); ++component, ++index) {

    double priorWeight = (*component).second;

    // Extract common factor to avoid numerical problems during exponentiation
    double chi2 = componentChi2[index] - minimumChi2;

    double updatedWeight(0.);
    // Determinant can not be belowe 1e-19 in CLHEP .... rather ugly but protect against 0 determinants
    // Normally occur when the component is a poor fit
    if (componentDeterminantR[index] > 1e-20)
      updatedWeight = priorWeight * sqrt(1. / componentDeterminantR[index]) * exp(-0.5 * chi2);
    else
      updatedWeight = 1e-10;

    ATH_MSG_VERBOSE(" Prior Weight: " << priorWeight << "  Updated Weight:  " << updatedWeight);
    Trk::ComponentParameters componentWithNewWeight(component->first->clone(), updatedWeight);
    returnMultiComponentState->push_back(componentWithNewWeight);
    sumWeights += updatedWeight;
  }

  if (returnMultiComponentState->size() != predictedState.size()) {
    ATH_MSG_WARNING("Inconsistent number of components between initial and final states... Exiting!");
    returnMultiComponentState.reset();
    return nullptr;
  }

  // Renormalise the state to total weight = 1
  Trk::MultiComponentState::iterator returnComponent = returnMultiComponentState->begin();
  component = predictedState.begin();

  for (; returnComponent != returnMultiComponentState->end(); ++returnComponent, ++component) {
    if (sumWeights > 0.) {
      (*returnComponent).second /= sumWeights;
    } else {
      (*returnComponent).second = component->second;
    }
  }

  return returnMultiComponentState;
}

std::pair<double, double>
Trk::PosteriorWeightsCalculator::calculateWeight_1D(const TrackParameters* componentTrackParameters,
                                                    const AmgSymMatrix(5) * predictedCov,
                                                    const double measPar,
                                                    const double measCov,
                                                    int paramKey) const
{
  std::pair<double, double> result(0, 0);
  // use measuring coordinate (variable "mk") instead of reduction matrix
  int mk = 0;
  if (paramKey != 1)
    for (int i = 0; i < 5; ++i)
      if (paramKey & (1 << i)) {
        mk = i;
        break;
      }

  // Extract predicted state track parameters
  const AmgVector(5)& trackParameters = componentTrackParameters->parameters();

  // Calculate the residual
  double r = measPar - trackParameters(mk);

  // Residual covariance. Posterior weights is calculated used predicted state and measurement. Therefore add
  // covariances
  double R = measCov + (*predictedCov)(mk, mk);

  // compute determinant of residual
  result.first = R;

  if (result.first == 0) {
    // ATH_MSG_WARNING( "Determinant is 0, cannot invert matrix... Ignoring component" );
    return result;
  }
  // Compute Chi2
  result.second = r * r / R;
  return result;
}

std::pair<double, double>
Trk::PosteriorWeightsCalculator::calculateWeight_2D_3(const TrackParameters* componentTrackParameters,
                                                      const AmgSymMatrix(5) * predictedCov,
                                                      const AmgVector(2) & measPar,
                                                      const AmgSymMatrix(2) & measCov) const
{
  std::pair<double, double> result(0, 0);
  // Extract predicted state track parameters
  const AmgVector(5)& trackParameters = componentTrackParameters->parameters();

  // Calculate the residual
  AmgVector(2) r = measPar - trackParameters.block<2, 1>(0, 0);

  // Residual covariance. Posterior weights is calculated used predicted state and measurement. Therefore add
  // covariances
  AmgSymMatrix(2) R(measCov + predictedCov->block<2, 2>(0, 0));

  // compute determinant of residual
  result.first = R.determinant();

  if (result.first == 0) {
    // ATH_MSG_WARNING( "Determinant is 0, cannot invert matrix... Ignoring component" );
    return result;
  }
  // Compute Chi2
  result.second = 0.5 * ((r.transpose() * R.inverse() * r)(0, 0));
  return result;
}

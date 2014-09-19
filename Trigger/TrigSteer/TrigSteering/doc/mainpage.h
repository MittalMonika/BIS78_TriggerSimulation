/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
@mainpage TrigSteering Package


New classes:
@author Till.Eifert@cern.ch
@author Nicolas.Berger@cern.ch
@author Tomasz.Bold@cern.ch

@section TrigSteeringIntro Introduction

This package is used for the High Level Trigger (HLT), both
in the Level 2 (L2) and Event Filter (EF).
It provides one Gaudi TopAlgorithm (StepController) that runs all other
sub algorithms.
This package provides: tools to convert the results from previous trigger
levels; the actual controlling unit, which is responsible for
managing (scheduling) the execution of all HLT PESA
algorithms.



@section TrigSteeringOverview Class Overview
  The TrigSteering package contains of following classes:

  - HLT::TrigSteer : Gaudi TopAlgorithm, controlling all other sub algorithms
  - HLT::Chain : representing one chain of signatures
  - HLT::Signature : representing one signature
  - HLT::Sequence : one sequence = PESA algorithm(s), input type(s), output type
  - HLT::LvlXConverter : various tools to do the trigger level conversion
  - Scaler : various classes which provide scaler engines for both: pass-throughs and prescales

@ref used_TrigSteering

@ref requirements_TrigSteering

*/

/**
@page used_TrigSteering Used Packages
@htmlinclude used_packages.html
*/

/**
@page requirements_TrigSteering Requirements
@include requirements
*/

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**

 <i>This page has been automatically generated by the tool. Please remove this line when you start creating your own mainpage</i>

@mainpage

New Extrapolation package
=========================

The TrkExEngine package was created to eventually repace the TrkExTools package by redefining and centralising the interface of all extrapolation operations needed for reconstruction and fast track simulation.
It has been re-designed in accordance to the updated track parameterisation (TrkParametersBase).

Design concepts and implementation
==================================

Data Flow
---------

The major design choice of the new TrkExEngine package is a division into optimised engines for the given geometry setup. 
The tracking geometry (TrkGeometry) supports the following different geometyr types:
- static   : a layer based setup that is interlinked and pre-ordered at the tracking geometry building
- dense    : a dense volume description, especially made for the STEP propagator (TrkExSTEP_Propagator)
- detached : a mixed setup of freely based volumes, especially made fort the STEP propagator

One single master engine (Trk::ExtrapolationEngine) steers the dataflow and the navigation through the high level tracking volumes,
calling the optimised sub engine for the different tracking volumes. The type of the tracking volume is provided by the node itself via an enumaration object.

The extrapolation, navigation and eventually needed material or jacobian collection is done using a cache object (Trk::ExtrapolationCell) which is handed over from engine to engine.


The ExtrapolationCell
---------------------

The key object of the extrapolation engine is the Trk::ExtrapolationCell class, which is used to start, configure and steer the extrapolation process.
This class is a templated object and can be used with charged (TrkParameters) and neutral (TrkNeutralParameters) parameters.
The ExtrapolationEngine itself thus supports the exact same interface for charged and neutral parameters.

The ExtrapolationCell is the single cache allowed in the new ExtrapolationEngine and --- since it is the input/output object --- turns the master engine into a thread -safe component.


The ExtrapolationCode
---------------------

The main return class of the ExtrapolationEngine package is a ExtrapolationCode class, which carries the current status of the extrapoaltion.
During the extrapolation process, the code forwarded between the different components is usually Trk::ExtrapolationCode::InProgress,
while success and failure codes are used to indicate different destination parameters or eventual failures during the extrapolation progress.


@htmlinclude used_packages.html

@include requirements

*/

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
@mainpage RecoToolInterfaces Package

This package contains abstract interfaces for common reconstruction tools.
Concrete implementations can be found in each leaf package.

@author Reconstruction Software Group

@section RecoToolInterfacesList List of Defined Interfaces

Abstract interfaces for the following combined reconstruction tasks are defined:

@htmlinclude annotated.html

   - IExtrapolateToCaloTool : Extrapolates Trk::Track, Trk::TrackParameters to the calorimeter samplings using the Trk::IExtrapolator.
   - Rec::IMuonTrackThroughCalo : Extrapolates the parameters associated to muons across the calorimeter,
            adding or subtracting energyloss from either parameterisation or measurement.
   - Rec::ITrackThroughDetectorBoundaries : Makes an extrapolation of ID track parameters outwards, saving
            the extrapolated parameters at each detector boundary and identification layer.

This package is not built as any library, it is a simple include package.   

@section ExtrasRecoToolInterfaces Extra Pages

 - @ref UsedRecoToolInterfaces
 - @ref requirementsRecoToolInterfaces
*/

/**
@page UsedRecoToolInterfaces Used Packages
@htmlinclude used_packages.html
*/

/**
@page requirementsRecoToolInterfaces Requirements
@include requirements
*/


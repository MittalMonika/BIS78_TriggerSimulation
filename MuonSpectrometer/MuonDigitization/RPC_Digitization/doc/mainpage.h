/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
@mainpage RPC_Digitization Package
@author Andrea.Di.Simone@cern.ch

@section RPC_DigitizationIntro Introduction

Package to generate RPC digits starting from Geant4 hits.

@section RPC_DigitizationOverview Class Overview
  
This package contains the RPC_Digitizer algorithm. It takes care of all the steps necessary for RPC hits
digitization:

      - Conversion from SimID to OfflineID
      - Cluster simulation (According to experimental data)
      - Propagation time calculation.

@section  RPC_DigitizationDetails Working principles

RPC hits are generated by the SensitiveDetector (SD) which assigns to them a 
Simulation Identifier (SimID), uniquely identifying the gas gap each hit is 
registered in. The position of the hit in the reference system of the gas gap 
is also stored, together with the time from the beginning of the event, i.e. 
the time of flight of the particle generating the hit. RPC hits are represented 
in the simulation code by instances of the class RPCHit.
The digitization process takes care of adding to the hits the information 
necessary for further analysis (for example trigger algorithm simulation and 
track reconstruction). It translates any SimID to a Standard Offline Identifier 
(OID), which is used by the other ATHENA algorithms to uniquely identify RPC 
strips in the muon spectrometer. Using the position information provided by 
the hits, the digitization can properly calculate the propagation time of each 
electronic signal along its strip, add it to the time of flight of the hit and 
assign this global time to the digit. The information obtained is 
stored in a new instance of the class RPCDigit and posted in StoreGate 
for further processing.

@section  RPC_DigitizationCluster Cluster simulation

When a particle generates an avalanche in an RPC, charge signals are 
induced (and detected) on the readout strips. A set of $n$ adjacent strips 
with signals is called a cluster of size n. In RPC operation, due 
to possible signal induction on more than one strip, cluster sizes are in 
general greater than 1, with an average cluster size at working point 
typically of 1.3. The hit production mechanism provided by the ATLAS Geant4 
simulation does not include a tool for proper simulation of clusters. Thus 
a particle generates hits on only one strip, except when secondaries are produced
and detected by neighboring strips.
The digitization algorithm reproduces the observed cluster sizes by generating, 
when necessary, digits on strips adjacent to the one actually crossed by the 
particle. Cluster simulation is carried on in three steps:

    - experimental distributions are used to decide, according to the impact 
      point of the particle along the strip, whether the cluster size will be 1 or 2.
    - experimental distributions are used to decide what the final size of the 
      simulated cluster will be.
    - digits are created according to the results of the above steps.

Particular attention needs to be paid to the way the additional digits are 
created around the one actually crossed by the muon. The decision of which strip
to turn on is made according to experimental distributions.

@section Req Requirements

@include requirements

*/

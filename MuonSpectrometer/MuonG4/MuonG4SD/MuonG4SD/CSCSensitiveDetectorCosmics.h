/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/** @class CSCSensitiveDetectorCosmics
	@author ketevi@bnl.gov
	
	
    @section CSCSensitiveDetectorCosmics Class methods and properties
    
The method CSCSensitiveDetectorCosmics::ProcessHits is executed by the G4 kernel each
time a charged particle (or a geantino) crosses one of the CSC Gas Gaps.

Navigating with the toucahbleHistory method GetHistoryDepth()
through the hierarchy of the
volumes crossed by the particles, the CSCSensitiveDetectorCosmics determinates the
correct set of geometry parameters to be folded in the Simulation Identifier
associated to each hit. The CSC SimIDs are 32-bit unsigned integers, built 
using the MuonSimEvent/CscHitIdHelper class
which inherits from the MuonHitIdHelper base class. 

The modified version (Daniela Rebuzzi, 16/12/2005) calculates the tof for the
cosmics, using as t0 the time to the closest approach of the track to (0,0,0).
This implementation works also for slow particles, since the discrimination
physics event/cosmics is done relying on the G4Track vertex (not on the tof). 

We describe in the following, how each field of the identifier is retrieved.

1) stationName, stationEta, stationPhi: when a volume is found in the hierarchy,
   whose name contains the substring "station", the stationName is extracted from
   the volume's name; stationPhi and stationEta are calculated starting from the
   volume copy number, assigned by MuonGeoModel.

2) multilayer: when a volume is found in the hierarchy,
   whose name contains the substring "component", the multilayer is set to 1 or to
   2, according to the component number (multilayer=1 if the component copy number 
   is 1, multilayer=3 if the component copy number is 2).

3) wireLayer: when a volume is found in the hierarchy,
   whose name contains the substring "CscArCO2", the wire Layer is calculated,
   increasing with |z| Atlas global coordinate.

    @section Some notes:

1) for each hit, the time of flight (the G4 globalTime), is recorded and
   associated to the hit.

2) the CSCHit object contains: the SimID, the globalTime, the energy deposited,
   the start and end point of the step in local coordinates (chamber reference
   system), the particle pdg and the track identifier.
    
 
*/
   

#ifndef CSCSensitiveDetectorCosmics_H
#define CSCSensitiveDetectorCosmics_H

#include "globals.hh"

#include "FadsSensitiveDetector/FadsSensitiveDetector.h"

#include "MuonSimEvent/CSCSimHitCollection.h"
#include "MuonSimEvent/CscHitIdHelper.h"
#include "SimHelpers/AthenaHitsCollectionHelper.h"

using namespace FADS;

class CSCSensitiveDetectorCosmics : public FadsSensitiveDetector {
    
public:
    /** construction/destruction */
    CSCSensitiveDetectorCosmics(std::string name);
    ~CSCSensitiveDetectorCosmics() {}
    
    
    /** member functions */
    void   Initialize(G4HCofThisEvent* HCE);
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
    void   EndOfEvent(G4HCofThisEvent* HCE); 
    
private:
    Amg::Vector3D mom;           
    double momMag;           
    Amg::Vector3D vertex; 
    Amg::Vector3D currVertex;          
    Amg::Vector3D globH;
    double m_globalTime;

    /** member data */
    CSCSimHitCollection*  myCSCHitColl;
    CscHitIdHelper* muonHelper;
    AthenaHitsCollectionHelper m_hitCollHelp;
};

#endif

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/** @class TGCSensitiveDetector
	@author hasegawa@azusa.shinshu-u.ac.jp   
	
    @section TGCSensitiveDetector Class methods and properties
    
The method TGCSensitiveDetector::ProcessHits is executed by the G4 kernel each
time a charged particle (or a geantino) crosses one of the TGC Gas
volumes.

Navigating with the touchableHistory method GetHistoryDepth()
through the hierarchy of the
volumes crossed by the particles, the TGCSensitiveDetector determinates the
correct set of geometry parameters to be folded in the Simulation Identifier
associated to each hit. The TGC SimIDs are 32-bit unsigned integers, built 
using the MuonSimEvent/TgcHitIdHelper class
which inherits from the MuonHitIdHelper base class. 

We describe in the following, how each field of the identifier is retrieved.

1) stationName and stationPhi: when a volume is found in the hierarchy,
   whose name contains the substring "station", the stationName is extracted from
   the volume's name; stationPhi is calculated starting from the
   volume copy number assigned by MuonGeoModel. Separate cases when the station 
   are "F" or "E" type. 

2) stationEta: when a volume is found in the hierarchy,
   whose name contains the substring "tgccomponent", stationEta is calculated 
   starting from the volume copy number assigned by MuonGeoModel. Possibility to
   calculate the stationPhi at this level, from the copy number assigned by
   MuonGeoModel (if the copyNo > 1000).

3) gasGap: when a volume is found in the hierarchy,
   whose name contains the substring "Gas Volume Layer" or "TGCGas", the gasGap is calculated
   from the volume copy number, according to the positive or negative Z value, in
   the Atlas global reference system.


    @section Some notes:

1) since the volume copy numbers strongly depend on the database
   layout, the volume name (whether it contains the string "Q02", for instance)
   is used to retrieve geometry information.

2) for each hit, the time of flight (the G4 globalTime), is recorded and
   associated to the hit.

3) the TGCHit object contains: the SimID, the globalTime, the hit position in
   the local reference system, the track director cosine in the local reference
   system and the track identifier.

 
 
 */   
#ifndef TGCSENSITIVEDETECTOR_H
#define TGCSENSITIVEDETECTOR_H


#include "globals.hh"


#include "FadsSensitiveDetector/FadsSensitiveDetector.h"

#include "MuonSimEvent/TGCSimHitCollection.h"
#include "MuonSimEvent/TgcHitIdHelper.h"
#include "SimHelpers/AthenaHitsCollectionHelper.h"


class TGCSensitiveDetector : public FADS::FadsSensitiveDetector {

public:
    /** construction/destruction */
    TGCSensitiveDetector(std::string name);
    ~TGCSensitiveDetector() {}

    /** member functions */
    void Initialize(G4HCofThisEvent* HCE);
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROHist);
    void EndOfEvent(G4HCofThisEvent* HCE); 
    
private:
    /** member data */
    TGCSimHitCollection*  myTGCHitColl;
    TgcHitIdHelper* muonHelper;
    AthenaHitsCollectionHelper m_hitCollHelp;
    std::string m_layout;
};

#endif

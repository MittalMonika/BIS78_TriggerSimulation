/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_TRACKDATA_H
#define  TRIGL2MUONSA_TRACKDATA_H

#include "TrigL2MuonSA/MdtData.h"
#include "TrigL2MuonSA/SuperPointData.h"

#define NCHAMBER 10

namespace TrigL2MuonSA {

class TrackPattern
{
 public:
 TrackPattern() :
   pt(0),
   charge(0),
   etaVtx(0),
   phiVtx(0),
   deltaPt(0),
   deltaEtaVtx(0),
   deltaPhiVtx(0),
   s_address(-1),
   phiMS(0),
   phiMSDir(0),
   etaMap(0),
   phiMap(0),
   etaBin(0),
   phiBin(0),
   phiBin24(0),
   smallLarge(-1),
   barrelRadius(0),
   barrelSagitta(0),
   endcapAlpha(0),
   endcapBeta(0),
   endcapRadius(0),
   endcapRadius3P(0),
   slope(0),
   intercept(0),
   deltaR(0),
   ptEndcapAlpha(0),
   ptEndcapBeta(0),
   ptEndcapRadius(0),
   isRpcFailure(false),
   isTgcFailure(false)
     {};
   
    ~TrackPattern() {};
    
 public:
    TrigL2MuonSA::MdtHits    mdtSegments[NCHAMBER]; // MDT hits associated with the track
    TrigL2MuonSA::SuperPoint superPoints[NCHAMBER]; // Super points at each station
    
    double pt;
    double charge;

    double etaVtx;
    double phiVtx;

    double deltaPt;
    double deltaEtaVtx;
    double deltaPhiVtx;

    int    s_address;

    double phiMS;
    double phiMSDir;

    double etaMap;
    double phiMap;  

    int    etaBin;
    int    phiBin;  
    int    phiBin24; 
    int    smallLarge;

    double barrelRadius;
    double barrelSagitta;

    double endcapAlpha;
    double endcapBeta;
    double endcapRadius;
    double endcapRadius3P;

    double slope;
    double intercept;
    double deltaR;

    double ptEndcapAlpha;
    double ptEndcapBeta;
    double ptEndcapRadius;
    
    bool   isRpcFailure;
    bool   isTgcFailure;
};
 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

}

#endif  // TRIGL2MUONSA_TRACKDATA_H

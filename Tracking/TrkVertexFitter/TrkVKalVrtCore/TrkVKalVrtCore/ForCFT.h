/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _TrkVKalVrtCore_ForCFT_H
#define _TrkVKalVrtCore_ForCFT_H

#include "TrkVKalVrtCore/CommonPars.h"

namespace Trk {

  struct ForCFT 
  {
//
    int useMassCnst;
    int usePhiCnst;
    int useThetaCnst;
    int usePointingCnst;
    int useAprioriVrt;
    int usePassNear;
    int usePlaneCnst;

//
// For several(up to 8) mass constraints
    int nmcnst;
    double wm[vkalNTrkM];
    double wmfit[vkalMaxNMassCnst];
    double localbmag;
// Since 20/09/2009 - Vertex in plane constraint 
    double Ap,Bp,Cp,Dp;

//
// Vertex, covariance and it's weight (inverse covariance)
// Used for pointing and a priori vertex constraints
// Since 15/03/2009: 
//    For "passing near" vrt+covvrt+charge from ForVrtClose structure is used. 
//    For pointing cnst vertex from constraint object itself is used
//---------------------------------------------------------------------------
    double vrt[3], covvrt[6], wgtvrt[6];
//
// temporary vertex in global ref.frame
    double vrtstp[3];
    int irob;
    double RobustScale;
    double robres[vkalNTrkM];
    int indtrkmc[vkalMaxNMassCnst][vkalNTrkM];
    int IterationNumber;
    double IterationPrecision;
 
    ForCFT(){
      nmcnst=0;
      useMassCnst=0; usePhiCnst=0; useThetaCnst=0; usePointingCnst=0; usePlaneCnst=0;
      useAprioriVrt=0; usePassNear=0;
      Ap=Bp=Dp=Cp=0.;
      IterationNumber = 50;
      IterationPrecision=1.e-3;
      RobustScale = 1.; irob=0;
      for (int ic=0; ic<vkalMaxNMassCnst; ++ic) wmfit[ic] = -10000.;
      for (int it=0; it<vkalNTrkM; ++it) {
         wm[it] = 139.57018;
         for(int ic=0; ic<vkalMaxNMassCnst; ic++) indtrkmc[ic][it]=0;
      }
      localbmag=1.997;   // Safety: standard magnetic field in ID 
    };
    ~ForCFT() {};
  };

}
#endif

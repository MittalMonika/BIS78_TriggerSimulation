/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1EVENTTPCNV_CPMHITSCOLLECTION_P1_H
#define TRIGT1EVENTTPCNV_CPMHITSCOLLECTION_P1_H

#include "AthenaPoolUtilities/TPObjRef.h"
#include <vector>

class CPMHitsCollection_p1 : public std::vector<TPObjRef>
{
 public:
 
  CPMHitsCollection_p1() {}

};

#endif

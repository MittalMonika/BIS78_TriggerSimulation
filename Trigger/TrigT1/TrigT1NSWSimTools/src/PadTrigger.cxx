/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/PadTrigger.h"
#include "TrigT1NSWSimTools/SectorTriggerCandidate.h"

#include "TrigT1NSWSimTools/PadData.h"
#include <iostream>

namespace NSWL1 {

BcTag_t PadTrigger::bctag() const {
    return m_pads.size() ? m_pads[0]->BC_Tag() : 0x0;
}

// Side ID
int PadTrigger::sideId() const {
  return m_pads.size() ? m_pads[0]->sideId() : -1;
} 

// Sector ID
int PadTrigger::sectorId() const {
  return m_pads.size() ? m_pads[0]->sectorId() : -1;
}


} // NSWL1



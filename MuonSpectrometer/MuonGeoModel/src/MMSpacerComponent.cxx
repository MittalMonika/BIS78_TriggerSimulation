/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/MMSpacerComponent.h"

namespace MuonGM {

MMSpacerComponent:: MMSpacerComponent():StandardComponent()
{
}


std::ostream& operator<<(std::ostream& os,const MMSpacerComponent& c)
{
  os << "Component " << c.name << std::endl;
  return os;
}

} // namespace MuonGM

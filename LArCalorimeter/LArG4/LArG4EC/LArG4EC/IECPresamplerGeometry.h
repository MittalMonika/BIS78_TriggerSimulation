/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// IGeometryCalculator.h
// Common interface for Geometry Calculator classes

#ifndef __LARG4EC_IECPRESAMPLERGEOMETRY_H__
#define __LARG4EC_IECPRESAMPLERGEOMETRY_H__

#include "GaudiKernel/IService.h"

//#include "globals.hh"

class LArG4Identifier;
class G4Step;

namespace LArG4
{

  class IECPresamplerGeometry : virtual public IService
  {
  public:
    IECPresamplerGeometry() {}
    virtual ~IECPresamplerGeometry() {}

    static const InterfaceID& interfaceID() {
      static const InterfaceID  IID_IECPresamplerGeometry("IECPresamplerGeometry",1,0);
      return IID_IECPresamplerGeometry;
    }

    // Full identifier computation from a G4 step
    virtual LArG4Identifier CalculateIdentifier( const G4Step*) const = 0;
  };
}
#endif //__LARG4EC_IECPRESAMPLERGEOMETRY_H__

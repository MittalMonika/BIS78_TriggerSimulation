/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// RALEmec.h
// JFB Sep 

// Access the HEC parameters from the NOVA database.

#ifndef _LArGeo_RALEmec_h_
#define _LArGeo_RALEmec_h_

#include "LArGeoCode/VDetectorParameters.h"

#include <limits.h>
#include <string>

namespace LArGeo {

  class RALEmec : public VDetectorParameters {

  public:

    RALEmec();
    virtual ~RALEmec();

    virtual double GetValue(const std::string&, 
                            const int i0 = INT_MIN,
                            const int i1 = INT_MIN,
                            const int i2 = INT_MIN,
                            const int i3 = INT_MIN,
                            const int i4 = INT_MIN );

  private:


    class Clockwork;
    Clockwork *c;

    RALEmec (const RALEmec&);
    RALEmec& operator= (const RALEmec&);
  };

} // namespace LArGeo

#endif

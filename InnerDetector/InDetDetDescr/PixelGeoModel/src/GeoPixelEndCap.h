/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELENDCAP_H
#define GEOPIXELENDCAP_H

#include "GeoVPixelFactory.h"
class GeoPixelServices;

class GeoPixelEndCap : public GeoVPixelFactory {
 public:
  GeoPixelEndCap(const GeoPixelServices * pixServices);
  virtual GeoVPhysVol* Build();
 private:
  const GeoPixelServices * m_pixServices;
};

#endif
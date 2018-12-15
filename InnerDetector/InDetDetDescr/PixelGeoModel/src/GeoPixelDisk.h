/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELDISK_H
#define GEOPIXELDISK_H

#include "GeoVPixelFactory.h"
class GeoLogVol;

class GeoPixelDisk : public GeoVPixelFactory {
 public:
  GeoPixelDisk();
  virtual GeoVPhysVol* Build();
  double Thickness();
  double RMax();
  double RMin();
};

#endif

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELIBLFWDSVCMODEL1_H
#define GEOPIXELIBLFWDSVCMODEL1_H

#include "PixelGeoModel/GeoVPixelFactory.h"

#include "PixelGeoModel/GeoPixelStaveSupport.h"
#include "PixelGeoModel/GeoPixelLadder.h"

#include "CLHEP/Geometry/Transform3D.h"
#include "GeoModelKernel/GeoPhysVol.h"


class GeoTransform;
class GeoPhysVol;

class GeoPixelIBLFwdSvcModel1 :  public GeoVPixelFactory {

public:  

  GeoPixelIBLFwdSvcModel1(int);
  virtual GeoVPhysVol* Build();

  GeoPhysVol* getSupportA() const { return m_supportPhysA; }
  GeoPhysVol* getSupportC() const { return m_supportPhysC; }
  GeoTransform* getSupportTrfA() const { return m_xformSupportA; }
  GeoTransform* getSupportTrfC() const { return m_xformSupportC; }

 private:

  int m_section;
  GeoPhysVol *m_supportPhysA;
  GeoPhysVol *m_supportPhysC;
  GeoTransform *m_xformSupportA;
  GeoTransform *m_xformSupportC;

  const GeoShape* addShape(const GeoShape * lastShape, const GeoShape * nextShape, const HepGeom::Transform3D & trans);
};

#endif


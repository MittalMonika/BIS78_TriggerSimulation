/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

//
// This is the base class for all the pieces of the Pixel detector.
//
//
#ifndef GEOVPIXELFACTORY_H
#define GEOVPIXELFACTORY_H

#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "PixelGeometryManager.h"
#include "InDetGeoModelUtils/InDetMaterialManager.h"

// fwd declaration
namespace InDetDD {class PixelDetectorManager;}

class GeoVPixelFactory {
 public:
  GeoVPixelFactory();
  virtual ~GeoVPixelFactory();
  virtual GeoVPhysVol* Build( )=0;
  static void SetDDMgr(InDetDD::PixelDetectorManager* mgr);
  static void setGeometryManager(PixelGeometryManager * geometryManger);
     
 protected:
  PixelGeometryManager* m_gmt_mgr;
  InDetMaterialManager* m_mat_mgr;
  static InDetDD::PixelDetectorManager* m_DDmgr;
  static PixelGeometryManager * s_geometryManager;
  const double m_epsilon;

 private:
};


#endif

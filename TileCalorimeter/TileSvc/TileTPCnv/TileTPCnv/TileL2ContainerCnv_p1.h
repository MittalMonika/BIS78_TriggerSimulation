///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// TileL2ContainerCnv_p1.h 
// Header file for class TileL2ContainerCnv_p1
// Author: Aranzazu Ruiz <aranzazu.ruiz.martinez@cern.ch>
// Date:   July 2008
/////////////////////////////////////////////////////////////////// 
#ifndef TILETPCNV_TILEL2CONTAINERCNV_P1_H
#define TILETPCNV_TILEL2CONTAINERCNV_P1_H

// AthenaPoolCnvSvc includes
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// TileTPCnv includes
#include "TileTPCnv/TileL2Container_p1.h"
#include "TileTPCnv/TileL2Cnv_p1.h"

// TileEvent includes
#include "TileEvent/TileL2Container.h"

typedef T_AthenaPoolTPCnvVector<
            TileL2Container,
            TileL2Container_p1,
            TileL2Cnv_p1
       > TileL2ContainerCnv_p1;

#endif //> TILETPCNV_TILEL2CONTAINERCNV_P1_H

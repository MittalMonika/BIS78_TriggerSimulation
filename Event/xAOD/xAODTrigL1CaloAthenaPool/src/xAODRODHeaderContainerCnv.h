/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1CALOATHENAPOOL_XAODRODHEADERCONTAINERCNV_H
#define XAODTRIGL1CALOATHENAPOOL_XAODRODHEADERCONTAINERCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolxAODCnv.h"
#include "xAODTrigL1Calo/RODHeaderContainer.h"

typedef T_AthenaPoolxAODCnv<xAOD::RODHeaderContainer>
  xAODRODHeaderContainerCnv;
  
#endif   

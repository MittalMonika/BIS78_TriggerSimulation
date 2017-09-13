/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENINTERFACES_GENINTERFACESDICT_H
#define GENINTERFACES_GENINTERFACESDICT_H

#if defined(__GCCXML__) and not defined(EIGEN_DONT_VECTORIZE)
#define EIGEN_DONT_VECTORIZE
#endif // __GCCXML__

// Includes for the dictionary generation:
#include "GenInterfaces/IEvgenOTFTopUpSvc.h"
#include "GenInterfaces/IHepMCWeightSvc.h"
#include "GenInterfaces/IPDFReweightTool.h"
#include "GenInterfaces/ITruthSelector.h"
#include "GenInterfaces/IxAODtoHepMCTool.h"

#endif // GENINTERFACES_GENINTERFACESDICT_H
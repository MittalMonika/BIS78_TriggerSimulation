// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file TrkVKalVrtFitter/IVKalState.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2019
 * @brief Base class for VKal state object.
 */


#ifndef TRKVKALVRTFITTER_IVKALSTATE_H
#define TRKVKALVRTFITTER_IVKALSTATE_H

#include "CxxUtils/checker_macros.h"

namespace Trk {


class IVKalState
{
public:
  virtual ~IVKalState() = default;
};
extern IVKalState tmpVKalState ATLAS_THREAD_SAFE;


} // namespace Trk


#endif // not TRKVKALVRTFITTER_IVKALSTATE_H

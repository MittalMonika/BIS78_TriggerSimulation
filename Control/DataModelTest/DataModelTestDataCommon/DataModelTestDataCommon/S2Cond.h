// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration.
 */
// $Id$
/**
 * @file DataModelTestDataCommon/S2Cond.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2017
 * @brief Conditions declarations for S2.
 */


#ifndef DATAMODELTESTDATACOMMON_S2COND_H
#define DATAMODELTESTDATACOMMON_S2COND_H


#include "DataModelTestDataCommon/S2.h"
#include "DataModelTestDataCommon/S1Cond.h"
#include "SGTools/BaseInfo.h"
#include "AthenaKernel/CondCont.h"


CONDCONT_BASE (DMTest::S2, DMTest::S1);

CLASS_DEF (CondCont<DMTest::S2>, 243030042, 0)
SG_BASE (CondCont<DMTest::S2>, CondCont<DMTest::S1>);


#endif // not DATAMODELTESTDATACOMMON_S2COND_H

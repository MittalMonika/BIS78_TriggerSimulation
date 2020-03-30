/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// T/P converter for TruthTrajectory.
// Andrei Gaponenko <agaponenko@lbl.gov>, 2007
// Olivier Arnaez <olivier.arnaez@cern.ch>, 2015

#include "TrkTruthTPCnv/TruthTrajectoryCnv_p3.h"
#include "TrkTruthTPCnv/TruthTrajectory_p3.h"
#include "TrkTruthData/TruthTrajectory.h"

#include "GeneratorObjectsTPCnv/HepMcParticleLinkCnv_p3.h"

namespace {
  HepMcParticleLinkCnv_p3 particleLinkConverter;
}

void TruthTrajectoryCnv_p3::persToTrans( const Trk::TruthTrajectory_p3* pers,
                                         TruthTrajectory* trans,
                                         MsgStream& msg )
{
  trans->resize(pers->size());
  for(Trk::TruthTrajectory_p3::size_type i=0; i<trans->size(); i++) {
    particleLinkConverter.persToTrans( &((*pers)[i]), &((*trans)[i]), msg);
  }
}

void TruthTrajectoryCnv_p3::transToPers( const TruthTrajectory* trans,
                                         Trk::TruthTrajectory_p3* pers,
                                         MsgStream& msg )
{
  pers->resize(trans->size());
  for(TruthTrajectory::size_type i=0; i<trans->size(); i++) {
    particleLinkConverter.transToPers( &((*trans)[i]), &((*pers)[i]), msg);
  }
}

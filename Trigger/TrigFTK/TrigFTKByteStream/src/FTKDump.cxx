/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "FTKDump.h"
#include "TrigFTK_RawData/FTK_RawTrackContainer.h"
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/MsgStream.h"

FTKDump::FTKDump(const std::string &name, ISvcLocator *pSvcLocator)
   : AthAlgorithm( name, pSvcLocator ) {

}

FTKDump::~FTKDump(){
}

StatusCode FTKDump::initialize() {

  return StatusCode::SUCCESS;
}

StatusCode FTKDump::execute() {

  const FTK_RawTrackContainer* tracks_ptr;

  CHECK(evtStore()->retrieve(tracks_ptr, "dummy_tracks"));

  ATH_MSG_INFO( "Got no of tracks: " << tracks_ptr->size() );

  return StatusCode::SUCCESS;
}

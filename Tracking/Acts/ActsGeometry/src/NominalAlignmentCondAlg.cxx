/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ATHENA

#include "StoreGate/WriteCondHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/EventIDRange.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoNodeAction.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "InDetReadoutGeometry/PixelDetectorManager.h"
#include "InDetReadoutGeometry/TRT_DetectorManager.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "GeoModelUtilities/GeoAlignmentStore.h"
#include "InDetReadoutGeometry/ExtendedAlignableTransform.h"

// PACKAGE
#include "ActsGeometry/NominalAlignmentCondAlg.h"
#include "ActsGeometry/GeoModelDetectorElement.hpp"

// ACTS
#include "Acts/Utilities/Definitions.hpp"
#include "Acts/Detector/TrackingGeometry.hpp"
#include "Acts/Detector/TrackingVolume.hpp"

#include <thread>
#include <chrono>
#include <memory>
#include <iostream>

NominalAlignmentCondAlg::NominalAlignmentCondAlg( const std::string& name, 
            ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator ),
  m_cs("CondSvc",name),
  m_trackingGeometrySvc("ActsTrackingGeometrySvc", name)
{
}

NominalAlignmentCondAlg::~NominalAlignmentCondAlg() {}

StatusCode NominalAlignmentCondAlg::initialize() {
  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

  if (m_cs.retrieve().isFailure()) {
    ATH_MSG_ERROR("unable to retrieve CondSvc");
  }

  if (m_wchk.initialize().isFailure()) {
    ATH_MSG_ERROR("unable to initialize WriteCondHandle with key" << m_wchk.key() );
    return StatusCode::FAILURE;
  }

  if (m_cs->regHandle(this, m_wchk).isFailure()) {
    ATH_MSG_ERROR("unable to register WriteCondHandle " << m_wchk.fullKey() 
                  << " with CondSvc");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode NominalAlignmentCondAlg::finalize() {
  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);
  return StatusCode::SUCCESS;
}

StatusCode NominalAlignmentCondAlg::execute() {
  ATH_MSG_DEBUG(name() << "::" << __FUNCTION__);

  SG::WriteCondHandle<GeoAlignmentStore> wch(m_wchk);

  EventIDBase now(getContext().eventID());

  // do we have a valid m_wch for current time?
  if ( wch.isValid(now) ) {
    ATH_MSG_DEBUG("CondHandle is already valid for " << now
		  << ". In theory this should not be called, but may happen"
		  << " if multiple concurrent events are being processed out of order.");

  } else {

    ATH_MSG_DEBUG("  CondHandle " << wch.key() 
                  << " not valid now (" << now << "). Setting nominal alignment cond");


    EventIDBase start(1, EventIDBase::UNDEFEVT);
    EventIDBase end(1, EventIDBase::UNDEFEVT);
    start.set_lumi_block(0);
    end.set_lumi_block(9999); // this is not actually forever

    EventIDRange r(start, end);

    ATH_MSG_DEBUG("Will register nominal alignment for range: " << r);

    // create empty alignment store, no deltas
    GeoAlignmentStore* alignmentStore = new GeoAlignmentStore();

    // populate the alignment store with all detector elements
    auto trkGeom = m_trackingGeometrySvc->trackingGeometry();
    const Acts::TrackingVolume* trkVol = trkGeom->highestTrackingVolume();

    
    ATH_MSG_DEBUG("Populating GeoAlignmentStore for IOV");
    size_t nElems = 0;
    trkVol->visitDetectorElements(
      [alignmentStore, &nElems](const Acts::DetectorElementBase* detElem) {
      using GMDE = Acts::GeoModelDetectorElement;
      const GMDE* gmde = dynamic_cast<const GMDE*>(detElem);
      gmde->storeTransform(alignmentStore);
      nElems++;
    });
    ATH_MSG_DEBUG("GeoAlignmentStore populated for " << nElems << " detector elements");


    if (wch.record(r, alignmentStore).isFailure()) {
      ATH_MSG_ERROR("could not record nominal GeoAlignmentStore " << wch.key() 
		    << " = " << alignmentStore
                    << " with EventRange " << r);
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("WCH is valid now? " << wch.isValid(now));
  }
  return StatusCode::SUCCESS;
}

      // loop over the layers

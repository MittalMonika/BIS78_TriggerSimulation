/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSTRACKINGGEOMETRYSVC_H
#define ACTSGEOMETRY_ACTSTRACKINGGEOMETRYSVC_H

// ATHENA
#include "AthenaBaseComps/AthService.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/EventContext.h"

// PACKAGE
#include "ActsGeometry/IActsTrackingGeometrySvc.h"

// STL
#include <map>

namespace InDetDD {
  class InDetDetectorManager;
  class SiDetectorManager;
  class TRT_DetectorManager;
}

class TRT_ID;
class ActsAlignmentStore;

class ActsDetectorElement;

namespace Acts {

class TrackingGeometry;
class CylinderVolumeHelper;
class ITrackingVolumeBuilder;

class GeometryID;
class BinnedSurfaceMaterial;

}


class ActsTrackingGeometrySvc : public extends<AthService, IActsTrackingGeometrySvc> {
public:

  StatusCode initialize() override;
  //virtual StatusCode finalize() override;
    
  ActsTrackingGeometrySvc( const std::string& name, ISvcLocator* pSvcLocator );

  std::shared_ptr<const Acts::TrackingGeometry>
  trackingGeometry() override;
  
  void 
  setAlignmentStore(const ActsAlignmentStore* gas, const EventContext& ctx) override;
    
  const ActsAlignmentStore*
  getAlignmentStore(const EventContext& ctx) const override;


private:
  std::shared_ptr<const Acts::ITrackingVolumeBuilder> 
  makeVolumeBuilder(const InDetDD::InDetDetectorManager* manager, std::shared_ptr<const Acts::CylinderVolumeHelper> cvh, bool toBeamline = false);

  ServiceHandle<StoreGateSvc> m_detStore;
  const InDetDD::SiDetectorManager* p_pixelManager;
  const InDetDD::SiDetectorManager* p_SCTManager;
  const InDetDD::TRT_DetectorManager* p_TRTManager;
    
  std::shared_ptr<std::vector<std::shared_ptr<const ActsDetectorElement>>> m_elementStore;
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;
  
  const TRT_ID *m_TRT_idHelper;
    
  Gaudi::Property<bool> m_useMaterialMap{this, "UseMaterialMap", false, ""};
  Gaudi::Property<std::string> m_materialMapInputFile{this, "MaterialMapInputFile", "", ""};
  Gaudi::Property<std::vector<size_t>> m_barrelMaterialBins{this, "BarrelMaterialBins", {10, 10}};
  Gaudi::Property<std::vector<size_t>> m_endcapMaterialBins{this, "EndcapMaterialBins", {5, 20}};
  
  mutable std::unordered_map<EventContext::ContextID_t, const ActsAlignmentStore*> m_gasMap;
  mutable std::mutex m_gasMapMutex;


};



#endif 

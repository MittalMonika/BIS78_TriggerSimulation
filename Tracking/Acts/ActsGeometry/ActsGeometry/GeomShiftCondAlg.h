/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "EventInfo/EventInfo.h"
#include "GaudiKernel/ICondSvc.h"

#include "StoreGate/StoreGateSvc.h"
#include "GeoModelUtilities/GeoAlignmentStore.h"

#include <string>

namespace InDetDD {
  class InDetDetectorManager;
  class SiDetectorManager;
  class TRT_DetectorManager;
}


class GeomShiftCondAlg  :  public AthAlgorithm {
  
public:
    
  GeomShiftCondAlg (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~GeomShiftCondAlg();
  
  virtual bool isClonable() const override { return true; }

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:
  
  SG::ReadHandleKey<EventInfo> m_evt {this,"EvtInfo", "McEventInfo", "EventInfo name"};

  SG::WriteCondHandleKey<GeoAlignmentStore> m_wchk {this, "PixelAlignmentKey", "PixelAlignment", "cond handle key"};

  Gaudi::Property<std::string> m_dbKey {this, "Key_DB", "X2", "explicit dbKey for cond handle"};
  Gaudi::Property<double> m_zShiftPerLB {this, "ZShiftPerLB", 10.5, ""};

  ServiceHandle<ICondSvc> m_cs;
  //ServiceHandle<IASCIICondDbSvc> m_cds;

  ServiceHandle<StoreGateSvc> m_detStore;
  const InDetDD::SiDetectorManager* p_pixelManager;
  const InDetDD::SiDetectorManager* p_SCTManager;
  const InDetDD::TRT_DetectorManager* p_TRTManager;

  std::vector<const GeoAlignableTransform*> m_topAligns;

};


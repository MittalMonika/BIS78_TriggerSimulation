/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_GEOMODELSTRAWLAYERBUILDER_H
#define ACTSGEOMETRY_GEOMODELSTRAWLAYERBUILDER_H


// ATHENA
#include "InDetReadoutGeometry/TRT_DetectorManager.h"

// PACKAGE
#include "ActsGeometry/GeoModelDetectorElement.hpp"

// ACTS
#include "Acts/Layers/Layer.hpp"
#include "Acts/Layers/ProtoLayer.hpp"
#include "Acts/Tools/ILayerBuilder.hpp"
#include "Acts/Tools/LayerCreator.hpp"
#include "Acts/Utilities/Definitions.hpp"
#include "Acts/Utilities/Logger.hpp"

class TRT_ID;
class ActsTrackingGeomtrySvc;

namespace Acts {

class Surface;

/// @class GeoModelStrawLayerBuilder
class GeoModelStrawLayerBuilder : public ILayerBuilder
{
public:
  using ElementVector
      = std::vector<std::shared_ptr<const GeoModelDetectorElement>>;
  
  struct Config
  {
    /// string based identification
    std::string                          configurationName = "undefined";
    const InDetDD::TRT_DetectorManager*   mng;
    std::shared_ptr<const LayerCreator> layerCreator = nullptr;
    std::shared_ptr<ElementVector> elementStore;
    const ActsTrackingGeometrySvc* trackingGeometrySvc = nullptr;
    const TRT_ID* idHelper = nullptr;
  };
  
  /// Constructor
  /// @param cfg is the configuration struct
  /// @param logger the local logging instance
  GeoModelStrawLayerBuilder(const Config&           cfg,
                       std::unique_ptr<const Logger> logger
                       = getDefaultLogger("GMSLayBldr", Logging::INFO))
    : m_logger(std::move(logger))
  {
    m_cfg = cfg;
  }
  
  ~GeoModelStrawLayerBuilder() {}

  const LayerVector
  negativeLayers() const override;

  const LayerVector
  centralLayers() const override;

  const LayerVector
  centralLayers();

  const LayerVector
  positiveLayers() const override;
  
  const std::string&
  identification() const override
  {
    return m_cfg.configurationName;
  }

  const LayerVector
  endcapLayers(int side);

private:
  /// configruation object
  Config m_cfg;

  /// Private access to the logger
  const Logger&
  logger() const
  {
    return *m_logger;
  }
  
  /// logging instance
  std::unique_ptr<const Logger> m_logger;


};
}

#endif

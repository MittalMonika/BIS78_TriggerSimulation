/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////
// StagedTrackingGeometryBuilder.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// InDet
#include "InDetTrackingGeometry/StagedTrackingGeometryBuilder.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "BeamPipeGeoModel/BeamPipeDetectorManager.h"
#include "InDetTrackingGeometryUtils/DiscOverlapDescriptor.h"
// EnvelopeDefinitionService
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"
// Trk interfaces
#include "TrkDetDescrInterfaces/ILayerProvider.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeCreator.h"
#include "TrkDetDescrInterfaces/ILayerArrayCreator.h"
// Trk Geometry stuff
#include "TrkDetDescrGeoModelCnv/GeoMaterialConverter.h"
#include "TrkDetDescrGeoModelCnv/MaterialLayerHelper.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/BinnedArray1D1D.h"
#include "TrkVolumes/VolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkSurfaces/DiscBounds.h"
//Gaudi
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/MsgStream.h"
#include <boost/lexical_cast.hpp>
#include <algorithm>

// constructor
InDet::StagedTrackingGeometryBuilder::StagedTrackingGeometryBuilder(const std::string& t, const std::string& n, const IInterface* p) :
  AthAlgTool(t,n,p),
  Trk::TrackingVolumeManipulator(),
  m_trackingVolumeCreator("Trk::CylinderVolumeCreator/CylinderVolumeCreator"),  
  m_layerArrayCreator("Trk::LayerArrayCreator/LayerArrayCreator"),
  m_enclosingEnvelopeSvc("AtlasEnvelopeDefSvc", n),
  m_layerEnvelopeCover(2*Gaudi::Units::mm),
  m_buildBoundaryLayers(true),
  m_replaceJointBoundaries(true),
  m_materialProperties(0),
  m_magneticFieldProperties(0),
  m_indexStaticLayers(true),
  m_checkForRingLayout(false),
  m_ringTolerance(2.*Gaudi::Units::mm),
  m_namespace("InDet::"),
  m_exitVolume("InDet::Containers::InnerDetector")
  ,m_materialOnFly(false)
{
  declareInterface<Trk::IGeometryBuilder>(this);  
  // layer builders and their configurations
  declareProperty("LayerBuilders",                            m_layerProviders);
  declareProperty("LayerBinningTypeCenter",           m_layerBinningTypeCenter);
  declareProperty("LayerBinningTypeEndcap",          m_layerBinningTypeEndcap);
  declareProperty("ColorCodes",                               m_colorCodesConfig);  
  // envelope definition service
  declareProperty("EnvelopeDefinitionSvc",              m_enclosingEnvelopeSvc );
  declareProperty("VolumeEnclosureCylinderRadii",     m_enclosingCylinderRadius);
  declareProperty("VolumeEnclosureDiscPositions",     m_enclosingDiscPositionZ);
  // helper tools  
  declareProperty("TrackingVolumeCreator",            m_trackingVolumeCreator);
  declareProperty("LayerArrayCreator",                    m_layerArrayCreator);  
  // build the Boundary Layers
  declareProperty("EnvelopeCover",                         m_layerEnvelopeCover);
  declareProperty("BuildBoundaryLayers",               m_buildBoundaryLayers);
  declareProperty("ReplaceAllJointBoundaries",        m_replaceJointBoundaries);
  // force robust layer indexing  
  declareProperty("IndexStaticLayers",                    m_indexStaticLayers);
  declareProperty("CheckForRingLayout",                m_checkForRingLayout);
  // automatized transcript from GM
  declareProperty("MaterialOnFly",                          m_materialOnFly);
  // volume namespace & container name
  declareProperty("VolumeNamespace",                  m_namespace); 
  declareProperty("ExitVolumeName",                     m_exitVolume);
  // minimal radial distance between rings to allow a split
  declareProperty("MinimalRadialGapForVolumeSplit", m_ringTolerance);
}

// destructor
InDet::StagedTrackingGeometryBuilder::~StagedTrackingGeometryBuilder()
{
  delete m_materialProperties;
  delete m_magneticFieldProperties;
}

// Athena standard methods
// initialize
StatusCode InDet::StagedTrackingGeometryBuilder::initialize()
{
  
   // retrieve envelope definition service --------------------------------------------------
   if ( m_enclosingEnvelopeSvc.retrieve().isFailure() ){
     ATH_MSG_FATAL( "Could not retrieve EnvelopeDefinition service. Abort.");
     return StatusCode::FAILURE;
   }
           
   // Retrieve the layer builders -----------------------------------------------------------
   if (m_layerProviders.retrieve().isFailure())
   {
      ATH_MSG_FATAL( "Failed to retrieve tool(s) " << m_layerProviders );
      return StatusCode::FAILURE;
   } else
      ATH_MSG_DEBUG( "Retrieved tool " << m_layerProviders );

   // Retrieve the tracking volume creator  -------------------------------------------------
   if (m_trackingVolumeCreator.retrieve().isFailure())
   {
      ATH_MSG_FATAL( "Failed to retrieve tool " << m_trackingVolumeCreator );
      return StatusCode::FAILURE;
   } else
      ATH_MSG_DEBUG( "Retrieved tool " << m_trackingVolumeCreator );
      
   // Retrieve the layer array creator  ----------------------------------------------------
   if (m_layerArrayCreator.retrieve().isFailure())
   {
      ATH_MSG_FATAL( "Failed to retrieve tool " << m_layerArrayCreator );
      return StatusCode::FAILURE;
   } else
      ATH_MSG_INFO( "Retrieved tool " << m_layerArrayCreator );      

    // Dummy MaterialProerties
    m_materialProperties = new Trk::Material;

    ATH_MSG_INFO( "initialize() succesful" );
    
  return StatusCode::SUCCESS;
}


const Trk::TrackingGeometry* InDet::StagedTrackingGeometryBuilder::trackingGeometry(const Trk::TrackingVolume*) const
{
   // only one assumption: 
   // layer builders are ordered in increasing r
   ATH_MSG_DEBUG( "[ Start building the ID TrackingGeometry. ]"); 
   ATH_MSG_DEBUG( "[ STEP 0 ] : Getting overal dimensions from DetectorEnvelope service." );
   
   ////////////////////////////////////////////////////////////////////////////////////////////////////////    
   // The Overall Geometry
   Trk::TrackingGeometry* trackingGeometry = 0;   

   // get the dimensions from the envelope service 
   RZPairVector& envelopeDefs = m_enclosingEnvelopeSvc->getInDetRZValues();
   ATH_MSG_VERBOSE("       -> retrieved Inner Detector envelope definitions at size " << envelopeDefs.size());
   double envelopeVolumeRadius = envelopeDefs[1].first;
   double envelopeVolumeHalfZ  = fabs(envelopeDefs[1].second);
   float bpRadius = envelopeDefs[0].first;
   ATH_MSG_VERBOSE("       -> envelope R/Z defined as : " << envelopeVolumeRadius << " / " << envelopeVolumeHalfZ );
   ATH_MSG_VERBOSE("       -> beam pipe boundary at R = "<<envelopeDefs[0].first);

   ATH_MSG_DEBUG( "[ STEP 1 ] : Getting overal dimensions from the different layer builders." );
   size_t ilS = 0;
   double maximumLayerExtendZ   = 0.;
   double maximumLayerRadius    = 0.;
   std::vector<InDet::LayerSetup> layerSetups;
   for ( auto& lProvider : m_layerProviders){
       // screen output 
       ATH_MSG_DEBUG( "[ LayerBuilder : '" << lProvider->identification() << "' ] being processed. " );
       // retrieve the layers
       std::vector<const Trk::Layer*> centralLayers = lProvider->centralLayers();
       std::vector<const Trk::Layer*> negativeLayers = lProvider->negativeLayers();
       std::vector<const Trk::Layer*> positiveLayers = lProvider->positiveLayers();
       ATH_MSG_VERBOSE("       -> retrieved "  << centralLayers.size()  << " central layers.");
       ATH_MSG_VERBOSE("       -> retrieved "  << negativeLayers.size() << " layers on negative side.");
       ATH_MSG_VERBOSE("       -> retrieved "  << positiveLayers.size() << " layers on positive side.");
       // getting the Layer setup from parsing the builder output
       InDet::LayerSetup lSetup = estimateLayerSetup(lProvider->identification(), ilS, 
                                                     negativeLayers,centralLayers,positiveLayers,
                                                     envelopeVolumeRadius, envelopeVolumeHalfZ);
       // get the maxima - for R and Z
       takeBigger(maximumLayerRadius, lSetup.rMax);
       takeBigger(maximumLayerExtendZ, lSetup.zMax);
       //layer setups for the second run
       layerSetups.push_back(lSetup);
       // increase counter
       ++ilS;
   }       
   ATH_MSG_VERBOSE("       -> layer max R/Z defined as : " << maximumLayerRadius << " / " << maximumLayerExtendZ );
   
   // create a volume cache for:
   // - ID volume, i.e. those that can be stacked into the overall container
   std::vector<const Trk::TrackingVolume*> idVolumes;
   
   // create a layer setup cache for flushing when necessary
   std::vector<InDet::LayerSetup> layerSetupCache;
   
   // we only need to take care of the last flush radius
   double lastFlushRadius = 0.;
   
   // (I) PARSE THE LAYERS FOR OVERALL DIMENSIONS -------------------------------------------------------------
   ATH_MSG_DEBUG( "[ STEP 2 ] : Looping through the layer setups and flush them into the ID detector volume vector." );
   for ( auto& lSetup : layerSetups){
        // screen output
        ATH_MSG_DEBUG( "[ Layer setup: '" << lSetup.identification << "' ] being processed, current cache size is " << layerSetupCache.size() );
        ATH_MSG_VERBOSE("       -> estimated dimensions for this layer setup are");
        ATH_MSG_VERBOSE("       -> central sector rMin / rMax / zMax : " 
            << lSetup.minRadiusCenter << " / " << lSetup.maxRadiusCenter << " / " << lSetup.zExtendCenter);
        if (lSetup.buildEndcap)                                                                                                
           ATH_MSG_VERBOSE("       -> endcap  sector rMin / rMax / zMin / zMax : " 
               << lSetup.minRadiusEndcap << " / " << lSetup.maxRadiusEndcap << " / " << lSetup.minZextendEndcap << " / " << lSetup.maxZextendEndcap);
        else 
           ATH_MSG_VERBOSE("       -> endcap is not being built.");
        
        // now check what is in the cache 
           // [a] nothing in the cache or new setup is compatible (in this case sectorZ are updated in all setups)
        if (!layerSetupCache.size() || setupFitsCache(lSetup,layerSetupCache) ){
            ATH_MSG_VERBOSE("       -> cache is empty or new sector fits cache setup - add this one to the cache.");
        } else {
            // [b] cache is not empty - let's see what is going on:
            ATH_MSG_VERBOSE("       -> new sector does not fit the current cache specs -> flushing the cache." );
            // create the outer boundary
            double flushRadius = layerSetupCache[layerSetupCache.size()-1].rMax > lSetup.rMin ? 
	      0.5*(layerSetupCache[layerSetupCache.size()-1].rMax + lSetup.rMax) :  0.5*(layerSetupCache[layerSetupCache.size()-1].rMax + lSetup.rMin)  ;
            // create a flush volume - clears the cache
            const Trk::TrackingVolume* fVolume = createFlushVolume(layerSetupCache,lastFlushRadius,flushRadius,maximumLayerExtendZ);
            // stuff it into the idVolume
            idVolumes.push_back(fVolume);
            // remember the last flush radius                                                                                    
            lastFlushRadius = flushRadius; 
        }
        // in any case, this setup needs to go into the cache
        layerSetupCache.push_back(lSetup);
   }
   
   // check if the cache is empty
   if (layerSetupCache.size()){
       ATH_MSG_DEBUG( "[ STEP 3 ] : Flush the remaining cache into the ID detector volume vector." );
       // set the maximum radius to the last layer radius    
       double flushRadius = 0.5*(maximumLayerRadius  + envelopeVolumeRadius);
       const Trk::TrackingVolume* fVolume = createFlushVolume(layerSetupCache,lastFlushRadius,flushRadius,maximumLayerExtendZ);
       // push it into the vector
       idVolumes.push_back(fVolume);
       lastFlushRadius = flushRadius;
   }
   
   ATH_MSG_DEBUG( "[ STEP 4 ] : Create the ID detector volumes" );
   // build the central enclosure first
   const Trk::TrackingVolume* centralEnclosure =  m_trackingVolumeCreator->createGapTrackingVolume(*m_materialProperties,
                                                                           lastFlushRadius, envelopeVolumeRadius,
                                                                           -maximumLayerExtendZ, maximumLayerExtendZ,
                                                                           1, true,
                                                                           m_namespace+"Gaps::CentralEnclosure");
   idVolumes.push_back(centralEnclosure);
   // now lets create the container
   std::string volumeName = m_namespace+"Detectors::Container";
   const Trk::TrackingVolume* idContainer = 
         m_trackingVolumeCreator->createContainerTrackingVolume(idVolumes,
                                                                *m_materialProperties,
                                                                volumeName,
                                                                m_buildBoundaryLayers,
                                                                m_replaceJointBoundaries);
   // finally create the two endplates: negative
   const Trk::TrackingVolume* negativeEnclosure =  m_trackingVolumeCreator->createGapTrackingVolume(*m_materialProperties,
                                                                                                   0., envelopeVolumeRadius,
                                                                                                   -envelopeVolumeHalfZ, -maximumLayerExtendZ,
                                                                                                   1, false,
                                                                                                   m_namespace+"Gaps::NegativeEnclosure");
  
   // finally create the two endplates: positive
   const Trk::TrackingVolume* positiveEnclosure =  m_trackingVolumeCreator->createGapTrackingVolume(*m_materialProperties,
                                                                                                   0., envelopeVolumeRadius,
                                                                                                   maximumLayerExtendZ,envelopeVolumeHalfZ,
                                                                                                   1, false,
                                                                                                   m_namespace+"Gaps::PositiveEnclosure");
   // and the final tracking volume
   std::vector<const Trk::TrackingVolume*> enclosedVolumes;
   enclosedVolumes.push_back(negativeEnclosure);
   enclosedVolumes.push_back(idContainer);
   enclosedVolumes.push_back(positiveEnclosure);
   
   const Trk::TrackingVolume* enclosedDetector = 
       m_trackingVolumeCreator->createContainerTrackingVolume(enclosedVolumes,
                                                              *m_materialProperties,
                                                               m_exitVolume,
                                                               m_buildBoundaryLayers,
                                                               m_replaceJointBoundaries);

   //  create the TrackingGeometry ------------------------------------------------------  
   trackingGeometry = new Trk::TrackingGeometry(enclosedDetector);
   
   if (m_indexStaticLayers) {
      ATH_MSG_VERBOSE("Re-index the static layers ...");
      trackingGeometry->indexStaticLayers(Trk::Global);   
   }                       

   if (m_materialOnFly) addGMmaterial( enclosedDetector, bpRadius );

   return trackingGeometry;
}

// finalize
StatusCode InDet::StagedTrackingGeometryBuilder::finalize()
{
    ATH_MSG_INFO( "finalize() successful" );
    return StatusCode::SUCCESS;
}


const Trk::TrackingVolume* InDet::StagedTrackingGeometryBuilder::packVolumeTriple(const InDet::LayerSetup& layerSetup,
                                                                                  double rMin, double& rMax,
                                                                                  double zMax, double zPosCentral) const
{


  ATH_MSG_VERBOSE( '\t' << '\t'<< "Pack provided Layers from '" << layerSetup.identification << "' triple into a container volume. " );
  
  // create the strings
  std::string volumeBase = m_namespace+"Detectors::"+layerSetup.identification;
  
  const Trk::TrackingVolume* negativeVolume = createTrackingVolume(layerSetup.negativeLayers, 
                                                                   rMin,rMax,
                                                                   -zMax,-zPosCentral,
                                                                   volumeBase+"::NegativeEndcap",
                                                                   (Trk::BinningType)layerSetup.binningEndcap,
								   false);

  const Trk::TrackingVolume* centralVolume = 
         m_trackingVolumeCreator->createTrackingVolume(layerSetup.centralLayers,
                                                       *m_materialProperties,
                                                       rMin,rMax,
                                                       -zPosCentral,zPosCentral,
                                                       volumeBase+"::Barrel",
                                                       (Trk::BinningType)layerSetup.binningCenter);
                                                       
   const Trk::TrackingVolume* positiveVolume = createTrackingVolume(layerSetup.positiveLayers,
                                                                    rMin,rMax,
                                                                    zPosCentral,zMax,
                                                                    volumeBase+"::PositiveEndcap",
                                                                    (Trk::BinningType)layerSetup.binningEndcap,
								    false);
   
   // the base volumes have been created
   ATH_MSG_VERBOSE('\t' << '\t'<< "Volumes have been created, now pack them into a triple.");
   // registerColorCode                                                   
   negativeVolume->registerColorCode(layerSetup.colorCode);   
   centralVolume->registerColorCode(layerSetup.colorCode);
   positiveVolume->registerColorCode(layerSetup.colorCode);
                                                         
   // pack them together
   std::vector<const Trk::TrackingVolume*> tripleVolumes;
   tripleVolumes.push_back(negativeVolume);
   tripleVolumes.push_back(centralVolume);
   tripleVolumes.push_back(positiveVolume);
   
   // create the tiple container
   const Trk::TrackingVolume* tripleContainer = 
         m_trackingVolumeCreator->createContainerTrackingVolume(tripleVolumes,
                                                                *m_materialProperties,
                                                                volumeBase,
                                                                m_buildBoundaryLayers,
                                                                m_replaceJointBoundaries);
                                                                
   ATH_MSG_VERBOSE( '\t' << '\t'<< "Created container volume with bounds: " << tripleContainer->volumeBounds() );
                                                                
   return tripleContainer;
}


/** Private helper method, estimates the overal dimensions */
InDet::LayerSetup InDet::StagedTrackingGeometryBuilder::estimateLayerSetup(const std::string& idName, size_t ilS,
                                                                           const std::vector<const Trk::Layer*>& negLayers,
                                                                           const std::vector<const Trk::Layer*>& cenLayers,
                                                                           const std::vector<const Trk::Layer*>& posLayers,
                                                                           double maxR, double maxZ) const
{  
  // prepare the dimensions
  double cenMinR  = 10e10;
  double cenMaxR  = 0.;
  double cenMinZ  = 10e10;
  double cenMaxZ  = 0.;
  double posMinR  = 10e10;
  double posMaxR  = 0.;
  double posMinZ  = 10e10;
  double posMaxZ  = 0.;
  // parse through the central layers first
  estimateLayerDimensions(cenLayers, cenMinR, cenMaxR, cenMinZ, cenMaxZ);
  // parse throught the positive layers - we assume a symmetric setup 
  estimateLayerDimensions(posLayers, posMinR, posMaxR, posMinZ, posMaxZ);
  // reset to maxZ and and maxR if overs
  if (posMaxZ > maxZ) {
      ATH_MSG_WARNING("Estimated z extended of central sector bigger than maximal z extened. Resetting - may lose layers though.");
      cenMaxZ = maxZ;
  } else if (posMaxZ > maxZ) {
      ATH_MSG_WARNING("Estimated z extended of endcap sector bigger than maximal z extened. Resetting - may lose layers though.");
      posMaxZ = maxZ;
  }
  // reset the radial wones.
  if (cenMaxR > maxR) {
      ATH_MSG_WARNING("Estimated r extended of central sector bigger than maximal r extened. Resetting - may lose layers though.");
      cenMaxR = maxR;
  }
  if (posMaxR > maxR) {
      ATH_MSG_WARNING("Estimated r extended of endcap sector bigger than maximal r extened. Resetting - may lose layers though.");
      posMaxR = maxR;
  }
  // create the layer setup class  
  return InDet::LayerSetup(idName, m_colorCodesConfig[ilS], 
                           negLayers, cenLayers, posLayers,
                           cenMinR, cenMaxR, cenMaxZ, m_layerBinningTypeCenter[ilS], 
                           posLayers.size(), posMinR, posMaxR, posMinZ, posMaxZ, m_layerBinningTypeEndcap[ilS]);
}


/** Private helper method to estimate the layer dimensions */
void InDet::StagedTrackingGeometryBuilder::estimateLayerDimensions(const std::vector<const Trk::Layer*>& layers,
                                                                   double& rMin, double& rMax, double& zMin, double& zMax) const
{
    // parse through the layers and estimate
      for (auto& layer : layers){
          // the thickness of the layer needs to be taken into account 
          double thickness = layer->thickness();
          // get the center
          const Amg::Vector3D& center = layer->surfaceRepresentation().center();
          // check if it is a cylinder layer
          const Trk::CylinderLayer* cLayer = dynamic_cast<const Trk::CylinderLayer*>(layer);
          if (cLayer){
              // now we have access to all the information
              double rMinC  = cLayer->surfaceRepresentation().bounds().r()-0.5*thickness-m_layerEnvelopeCover;
              double rMaxC  = cLayer->surfaceRepresentation().bounds().r()+0.5*thickness+m_layerEnvelopeCover;
              double hZ = cLayer->surfaceRepresentation().bounds().halflengthZ();
              takeSmaller(rMin,rMinC);
              takeBigger(rMax,rMaxC);
              takeSmaller(zMin,center.z()-hZ);
              takeBigger(zMax,center.z()+hZ);
          } 
          // proceed further if it is a Disc layer
          const Trk::DiscBounds* dBounds = dynamic_cast<const Trk::DiscBounds*>(&(layer->surfaceRepresentation().bounds()));
          if (dBounds){
              // now we have access to all the information
              double rMinD =dBounds->rMin();
              double rMaxD =dBounds->rMax();
              double zMinD =  center.z()-0.5*thickness-m_layerEnvelopeCover;
              double zMaxD =  center.z()+0.5*thickness+m_layerEnvelopeCover;
              takeSmaller(rMin,rMinD);
              takeBigger(rMax,rMaxD);
              takeSmaller(zMin,zMinD);
              takeBigger(zMax,zMaxD);
          }
      }
}  
 

/** Private helper method to check if a sector is compatible with the cache */
bool InDet::StagedTrackingGeometryBuilder::setupFitsCache(LayerSetup& layerSetup, std::vector<InDet::LayerSetup>& layerSetupCache) const
{
    // the maximum center and overall extend of the cache
    double maxCenterCacheZ = 0.; 
    // the maximum endcap extend of the cache
    double minEndcapCacheZ = 10e10;
    // chech if he have a fullSectorSetup
    bool fullSectorSetup   = false;
    // 
    for (auto& lCacheSetup : layerSetupCache){
        takeBigger(maxCenterCacheZ, lCacheSetup.zExtendCenter);
        takeSmaller(minEndcapCacheZ, lCacheSetup.minZextendEndcap);
        // once true always true - otherwise it would have been flushed
        fullSectorSetup = lCacheSetup.buildEndcap ? true : fullSectorSetup;
        
    }    
    // if we do not have a full sector setup - > flush directly
    if (!fullSectorSetup) {
        ATH_MSG_VERBOSE("       -> only central sector being built, flush the cache ... ");
        return false;
    }
    // if the cached minimum z endcap z extend cuts within the new barrel -> flush it 
    if (minEndcapCacheZ < layerSetup.zExtendCenter){
        ATH_MSG_VERBOSE("       -> cache endcap extend reaches into new central sector, flush the cache ... ");
        return false;
    }
    // the cache center maximumg exceeds the new endcap minimum -> does not fit
    if (maxCenterCacheZ < layerSetup.minZextendEndcap ) {
        ATH_MSG_VERBOSE("       -> sector fully fits into cache! Add it and start synchronising ...");
        // calculate the new sector gap and synchronise
        double newCenterMaxZ = maxCenterCacheZ > layerSetup.zExtendCenter ? maxCenterCacheZ : layerSetup.zExtendCenter;
        double newEndcapMinZ = minEndcapCacheZ < layerSetup.minZextendEndcap ? minEndcapCacheZ : layerSetup.minZextendEndcap;
        double newSectorZ    = 0.5*(newCenterMaxZ+newEndcapMinZ);
        // and syncrhonise the boundaries 
        for (auto& lCacheSetup : layerSetupCache)
            lCacheSetup.zSector = newSectorZ;
        layerSetup.zSector = newSectorZ;
        return true;
    }
    // it simply does not fit so return false
    return false;
}
 
bool InDet::StagedTrackingGeometryBuilder::ringLayout(const std::vector<const Trk::Layer*>& layers, std::vector<double>& rmins, std::vector<double>& rmaxs) const {
  // get the maximum extent in z
  std::vector<std::pair<double,double>> radii;
  ATH_MSG_DEBUG("Checking for Ring layout ... ");
  for (auto& ring : layers) {
    // Surface
    const Trk::Surface&     ringSurface = ring->surfaceRepresentation(); 
    const Trk::DiscBounds*  ringBounds  = dynamic_cast<const Trk::DiscBounds*>(&(ringSurface.bounds()));
    if (ringBounds){
      // get the main parameters
      double zpos         = ringSurface.center().z();
      double rMin         = ringBounds->rMin();
      double rMax         = ringBounds->rMax();
      // take and check
      //checkForInsert(rmins,rMin);
      //checkForInsert(rmaxs,rMax);
      // take and check the couple rmin/rmax
      checkForInsert(rMin, rMax, radii);
      ATH_MSG_DEBUG(" -> Ring at z-position " << zpos << " - with rMin/rMax = " << rMin << "/" << rMax );
    }
  }
  
  // you need a post processing of the (rmin,rmax) in order to fit z-overlapping disks in the same ring
  std::vector<std::pair<double,double>> tmpradii;
  
  for (auto& rs: radii) {
    bool found = false;
    for (auto& tmprs: tmpradii) {
      if ((rs.first<tmprs.second and rs.second>tmprs.first) ) {
	tmprs.first  = std::min(tmprs.first ,rs.first );
	tmprs.second = std::max(tmprs.second,rs.second);
	found = true;
	break;
      }
    }
    if (found) continue;
    tmpradii.push_back(rs);
  }
   
  // now you fill rmin and rmax
  rmins.clear(); rmaxs.clear();
  for (auto& r: tmpradii) {
    rmins.push_back(r.first);
    rmaxs.push_back(r.second);
  }
   
  //add rmin and rmax
  return (rmins.size() > 1 );
}                                              

 

const Trk::TrackingVolume* InDet::StagedTrackingGeometryBuilder::createTrackingVolume(const std::vector<const Trk::Layer*>& layers, 
                                                                                      double innerRadius, double& outerRadius,
                                                                                      double zMin, double zMax,
                                                                                      const std::string& volumeName,
                                                                                      Trk::BinningType binningType,
										       bool doAdjustOuterRadius) const
{

    // first loop - this is for diagnostics for the radii 
    std::vector<double> ringRmins;
    std::vector<double> ringRmaxa;
    if (m_checkForRingLayout && ringLayout(layers,ringRmins, ringRmaxa)){
      ATH_MSG_DEBUG("Ring layout is present for volume '" << volumeName << "' dealing with it.");
      
      // create the vector for the sub volumes
      std::vector<const Trk::TrackingVolume* > ringVolumes;

      // now sort the necessary layers --- for the sub volumes
      std::vector< std::vector< const Trk::Layer*> > groupedDiscs(ringRmins.size(), std::vector< const Trk::Layer*>() );
      // second loop over the rings
      for (auto& ring : layers){
	// Surface
	const Trk::Surface&     ringSurface = ring->surfaceRepresentation(); 
	const Trk::DiscBounds*  ringBounds  = dynamic_cast<const Trk::DiscBounds*>(&(ringSurface.bounds()));
	if (ringBounds){
	  // get the main parameters
	  double rMax         = ringBounds->rMax();
	  size_t rPos         = 0;
	  // fill into the right group
	  for (auto& rm : ringRmaxa){
	    if (rMax <= rm+m_ringTolerance) break;
	    ++rPos;
	  }
	  // fill it it 
	  const Trk::DiscLayer* dring = dynamic_cast<const Trk::DiscLayer*>(ring);
	  if (dring) groupedDiscs[rPos].push_back(dring);
	}
      }
      // layer merging may be needed 
      std::vector< std::vector< const Trk::Layer*> > mergedLayers;
      std::vector< float > mergedRmax;
      std::vector< std::vector< int > > merge;
      std::vector<int> laySet(1,0); merge.push_back(laySet);
      double rCurr = ringRmaxa[0];
      mergedRmax.push_back(rCurr);
      for (int idset = 1; idset < int(groupedDiscs.size()); idset++){
	if (ringRmins[idset]<=rCurr + m_ringTolerance) {
	  merge.back().push_back(idset);
	  if (ringRmaxa[idset]>mergedRmax.back()) mergedRmax.back()=ringRmaxa[idset]; 
	} else {
	  merge.push_back(std::vector<int>(1,idset));
	  mergedRmax.push_back(ringRmaxa[idset]);
	} 
	rCurr = ringRmaxa[idset];
      } 
      for ( auto layset : merge ) {
	std::vector<const Trk::Layer*> ringSet;
	for ( auto lay : layset ) {
	  for ( auto ring : groupedDiscs[lay]) {
	    float zPos = ring->surfaceRepresentation().center().z();
	    if (!ringSet.size() || zPos>ringSet.back()->surfaceRepresentation().center().z()) ringSet.push_back(ring);
	    else {
	      std::vector<const Trk::Layer*>::iterator lit = ringSet.begin();
	      while (lit!=ringSet.end() && zPos>(*lit)->surfaceRepresentation().center().z()) lit++;
	      ringSet.insert(lit,ring);  
	    }   
	  }
	} 
        // rings ordered in z : resolve overlap
	mergedLayers.push_back(checkZoverlap(ringSet));
      }	  
      
      for (int idset = 0; idset < int(mergedLayers.size()); idset++){
	// always keep the boundaries in mind for the radial extend
	double crmin = idset ? mergedRmax[idset-1]+m_layerEnvelopeCover : innerRadius;
	double crmax = mergedRmax[idset]+m_layerEnvelopeCover;
	if(idset==int(mergedLayers.size())-1 && !doAdjustOuterRadius) crmax = outerRadius; 
	// now create the sub volume
	std::string ringVolumeName = volumeName+"Ring"+boost::lexical_cast<std::string>(idset);
	const Trk::TrackingVolume* ringVolume = m_trackingVolumeCreator->createTrackingVolume(mergedLayers[idset],
											      *m_materialProperties,
											      crmin,crmax,
											      zMin,zMax,
											      ringVolumeName,
											      binningType);
	// push back into the 
	ringVolumes.push_back(ringVolume);
      }

      // set the outer radius
      if(doAdjustOuterRadius) outerRadius = ringRmaxa[ringRmaxa.size()-1]+m_layerEnvelopeCover;
      //
      ATH_MSG_DEBUG("      -> adjusting the outer radius to the last ring at " << outerRadius );
      ATH_MSG_DEBUG("      -> created " << ringVolumes.size() << " ring volumes for Volume '" << volumeName << "'.");
      
      // create the container
      return (ringVolumes.size()>1 ? m_trackingVolumeCreator->createContainerTrackingVolume(ringVolumes,
	         										   *m_materialProperties,
											           volumeName,
											           m_buildBoundaryLayers,
											           m_replaceJointBoundaries) : ringVolumes[0] );
      
        
    } else 
      return m_trackingVolumeCreator->createTrackingVolume(layers,
                                                             *m_materialProperties,
                                                             innerRadius,outerRadius,
                                                             zMin,zMax,
                                                             volumeName,
                                                             binningType);
}     
 
                             
/** Private helper method to flush the cache into the id volumes - return volume is the one to be provided */
const Trk::TrackingVolume* InDet::StagedTrackingGeometryBuilder::createFlushVolume(std::vector<InDet::LayerSetup>& layerSetupCache,
                                                                                   double innerRadius, double& outerRadius, double extendZ) const
{
  // the return volume 
  const Trk::TrackingVolume* flushVolume = 0;
  // 
  if (layerSetupCache.size() == 1 ){
    ATH_MSG_VERBOSE("       -> single sector setup - synchronising from inner (" << innerRadius << ") to outer (" << outerRadius << ") radius.");
    ATH_MSG_VERBOSE("       -> setup identification : " << layerSetupCache[0].identification );
    // create the new tracking volume - either as a triple or as a single
        flushVolume = layerSetupCache[0].buildEndcap ? 
        packVolumeTriple(layerSetupCache[0],
                         innerRadius, outerRadius,
                         extendZ,layerSetupCache[0].zSector) :
        m_trackingVolumeCreator->createTrackingVolume(layerSetupCache[0].centralLayers,
                                                      *m_materialProperties,
                                                      innerRadius,outerRadius,
                                                      -extendZ,extendZ,
                                                      layerSetupCache[0].identification,
                                                      (Trk::BinningType)layerSetupCache[0].binningCenter);    
    
   } else {
       ATH_MSG_VERBOSE("       -> setup with " << layerSetupCache.size() << " entries - synchronising from inner (" << innerRadius << ") to outer (" << outerRadius << ") radius.");
       // prepare the volume vectors & name identification
       std::vector<const Trk::TrackingVolume*> negVolumes;
       std::vector<const Trk::TrackingVolume*> centralVolumes;
       std::vector<const Trk::TrackingVolume*> posVolumes;
       std::string combinedName;
       for (size_t ilS = 0; ilS < layerSetupCache.size(); ++ilS){
           // take the given inner radius for the first one - median otherwise
           double irE = ilS ? 0.5*(layerSetupCache[ilS].minRadiusEndcap+layerSetupCache[ilS-1].maxRadiusEndcap) : innerRadius;
           double irC = ilS ? 0.5*(layerSetupCache[ilS].minRadiusCenter+layerSetupCache[ilS-1].maxRadiusCenter) : innerRadius;
           // take the given outer radius for the last one - median otherwise
           double orE = ((ilS+1)==layerSetupCache.size()) ? outerRadius : 0.5*(layerSetupCache[ilS+1].minRadiusEndcap+layerSetupCache[ilS].maxRadiusEndcap);
           double orC = ((ilS+1)==layerSetupCache.size()) ? outerRadius : 0.5*(layerSetupCache[ilS+1].minRadiusCenter+layerSetupCache[ilS].maxRadiusCenter);
	   // Adjust last volumes in R to the same maximal radial extends!
	   if(ilS==layerSetupCache.size()-1) {
	     ATH_MSG_VERBOSE("Processing last volume");
	     ATH_MSG_VERBOSE("  --> adjust volumes to same extends: orE=" << orE << " orC=" << orC);
	     if(orE>orC) orC=orE; else orE=orC;
	   }
           // create the three volumes
           const Trk::TrackingVolume* nVolume = createTrackingVolume(layerSetupCache[ilS].negativeLayers,
                                                                     irE,orE,
                                                                     -extendZ,-layerSetupCache[ilS].zSector,
                                                                     layerSetupCache[ilS].identification+"::NegativeEndcap",
                                                                     (Trk::BinningType)layerSetupCache[ilS].binningEndcap,false);               
           const Trk::TrackingVolume* cVolume = m_trackingVolumeCreator->createTrackingVolume(layerSetupCache[ilS].centralLayers,
                                                                                  *m_materialProperties,
                                                                                  irC,orC,
                                                                                  -layerSetupCache[ilS].zSector,layerSetupCache[ilS].zSector,
                                                                                  layerSetupCache[ilS].identification+"::Barrel",
                                                                                  (Trk::BinningType)layerSetupCache[ilS].binningCenter);
           const Trk::TrackingVolume* pVolume = createTrackingVolume(layerSetupCache[ilS].positiveLayers,
                                                                     irE,orE,
                                                                     layerSetupCache[ilS].zSector,extendZ,
                                                                     layerSetupCache[ilS].identification+"::PositiveEndcap",
                                                                     (Trk::BinningType)layerSetupCache[ilS].binningEndcap,false);
           // register the right color code
           nVolume->registerColorCode(layerSetupCache[ilS].colorCode);
           cVolume->registerColorCode(layerSetupCache[ilS].colorCode);
           pVolume->registerColorCode(layerSetupCache[ilS].colorCode);
           // push them into the volume containers 
           negVolumes.push_back(nVolume);
           centralVolumes.push_back(cVolume);
           posVolumes.push_back(pVolume);
           // combined name
           combinedName += "_"+layerSetupCache[ilS].identification;
       }
       ATH_MSG_VERBOSE("       -> setup identification : " << combinedName );
       flushVolume  = packVolumeTriple(negVolumes,centralVolumes,posVolumes,combinedName);
   }
   // clear the cache
   layerSetupCache.clear();      
   // return the volume               
   return flushVolume;
    
}  

const Trk::TrackingVolume* InDet::StagedTrackingGeometryBuilder::packVolumeTriple(
                                     const std::vector<const Trk::TrackingVolume*>& negVolumes,
                                     const std::vector<const Trk::TrackingVolume*>& centralVolumes,
                                     const std::vector<const Trk::TrackingVolume*>& posVolumes,
                                     const std::string& baseName) const
{
  ATH_MSG_VERBOSE( '\t' << '\t'<< "Pack provided Volumes from '" << baseName << "' triple into a container volume. " );

  unsigned int negVolSize = negVolumes.size();
  unsigned int cenVolSize = centralVolumes.size();
  unsigned int posVolSize = posVolumes.size();
  
  
  
    // create the strings
  std::string volumeBase = m_namespace+"Containers::"+baseName;
  
  const Trk::TrackingVolume* negativeVolume = (negVolSize > 1) ?
       m_trackingVolumeCreator->createContainerTrackingVolume(negVolumes,
                                                       *m_materialProperties,
                                                       volumeBase+"::NegativeSector",
                                                       m_buildBoundaryLayers,
                                                       m_replaceJointBoundaries) : 
                                             (negVolSize ? negVolumes[0] : 0);
  const Trk::TrackingVolume* centralVolume = (cenVolSize > 1) ?
         m_trackingVolumeCreator->createContainerTrackingVolume(centralVolumes,
                                                       *m_materialProperties,
                                                       volumeBase+"::CentralSector",
                                                       m_buildBoundaryLayers,
                                                       m_replaceJointBoundaries) :
                                              (cenVolSize ? centralVolumes[0] : 0) ;
                                              
   const Trk::TrackingVolume* positiveVolume = ( posVolSize > 1) ?
         m_trackingVolumeCreator->createContainerTrackingVolume(posVolumes,
                                                       *m_materialProperties,
                                                       volumeBase+"::PositiveSector",
                                                       m_buildBoundaryLayers,
                                                       m_replaceJointBoundaries) : 
                                               (posVolSize ? posVolumes[0] : 0);
   
   if (!negativeVolume && !positiveVolume){
       ATH_MSG_DEBUG( "No negative/positive sector given - no packing needed, returning central container!" );
       return centralVolume;
   }
   // pack them together
   std::vector<const Trk::TrackingVolume*> tripleVolumes;
   if (negativeVolume) tripleVolumes.push_back(negativeVolume);
   if (centralVolume) tripleVolumes.push_back(centralVolume);
   if (positiveVolume) tripleVolumes.push_back(positiveVolume);
   // create the triple container
   const Trk::TrackingVolume* tripleContainer = 
         m_trackingVolumeCreator->createContainerTrackingVolume(tripleVolumes,
                                                                *m_materialProperties,
                                                                volumeBase,
                                                                m_buildBoundaryLayers,
                                                                m_replaceJointBoundaries);
   return tripleContainer;
}


std::vector<const Trk::Layer*> InDet::StagedTrackingGeometryBuilder::checkZoverlap(std::vector<const Trk::Layer*>& lays) const 
{
    // check disc layer overlaps in z, merge if appropriate
    std::vector<const Trk::Layer*> mergedDiscLayers;
    std::vector<const Trk::Layer*> toMerge;
    double zlast = 0.; bool overlaps = false;
    for (auto lay : lays) {
        float zpos= lay->surfaceRepresentation().center().z();
        float thick = 0.5*lay->thickness();
        if (lay==lays.front()) toMerge.push_back(lay);
        else
	  { if ( zpos - thick<zlast) {
	     toMerge.push_back(lay);
	     overlaps = true;   
	  } else {
	     if ( toMerge.size()==1 ) mergedDiscLayers.push_back(toMerge[0]);
	     else if (toMerge.size()>1) {
	       const Trk::Layer* nd = mergeDiscLayers(toMerge);
	       if (nd) mergedDiscLayers.push_back(nd); 
	       else {
		 ATH_MSG_DEBUG("radial merge of rings failed, return the input layer set");
		 return lays;
	       }
	     }
	     toMerge.clear(); toMerge.push_back(lay);
	  }
	}
	zlast = zpos+thick;
    }
    if (toMerge.size()==1) mergedDiscLayers.push_back(toMerge[0]);
    else if (toMerge.size()>1) {
      const Trk::Layer* nd = mergeDiscLayers(toMerge);
      if (nd) mergedDiscLayers.push_back(nd); 
      else {
	ATH_MSG_DEBUG("radial merge of rings failed, return the input layer set");
	return lays;
      }
    }

    if (overlaps) return mergedDiscLayers;

    return lays;
}

const Trk::Layer* InDet::StagedTrackingGeometryBuilder::mergeDiscLayers(std::vector<const Trk::Layer*>& inputDiscs) const {
 
  // on the input, disc layers overlapping in thickness : merge to a new DiscLayer
  std::pair<float,float> zb(1.e5,-1.e5);
  // order discs in radius
  std::vector< std::pair<float,float> > rbounds; std::vector<size_t> discOrder;
  size_t id=0;
  for ( auto  lay : inputDiscs ) {
    zb.first = fmin( zb.first, lay->surfaceRepresentation().center().z()-0.5*lay->thickness());
    zb.second = fmax( zb.second, lay->surfaceRepresentation().center().z()+0.5*lay->thickness());
    const Trk::DiscBounds* db = dynamic_cast<const Trk::DiscBounds*>(&(lay->surfaceRepresentation().bounds()));
    if (!db) {
      ATH_MSG_WARNING("attempt to merge non-disc layers, bailing out");
      return 0;    
    }
    float r = db->rMin();
    if (!rbounds.size() ||  r>rbounds.back().first) {
      rbounds.push_back(std::pair<float,float> (r,db->rMax()));  
      discOrder.push_back(id);
    } else {
      int ir=rbounds.size()-1;
      while (ir>=0) {
	if ( r>rbounds[ir].first ) break; 
	ir--;
      }
      rbounds.insert(rbounds.begin()+ir+1,std::pair<float,float> (r,db->rMax()));  
      discOrder.insert(discOrder.begin()+ir+1,id);           
    }
    id++;
  }
  
  std::vector<float> rsteps; std::vector<const Trk::Surface*> surfs; 
  std::vector<Trk::BinUtility*>* binUtils=new std::vector<Trk::BinUtility*>(); 
  rsteps.push_back(rbounds[0].first);
  for (unsigned int id=0; id<discOrder.size(); id++) {
    unsigned int index=discOrder[id];
    const Trk::SurfaceArray* surfArray = inputDiscs[index]->surfaceArray();    
    if (surfArray) {
      if (surfArray->binUtility()->binningValue()!=Trk::binPhi) {
	ATH_MSG_WARNING("attempt to merge 2D disc arrays, bailing out");
        return 0;
      }
      binUtils->push_back(surfArray->binUtility()->clone());
      if (id+1<discOrder.size()) rsteps.push_back( 0.5*(rbounds[id].second+rbounds[id+1].first));
      const std::vector<const Trk::Surface*> ringSurf =surfArray->arrayObjects();
      surfs.insert(surfs.end(),ringSurf.begin(),ringSurf.end());
    }  
  }
  rsteps.push_back(rbounds.back().second);

  std::vector< std::pair< Trk::SharedObject<const Trk::Surface>, Amg::Vector3D >  > surfaces;
  for ( auto  sf : surfs ) {
    Trk::SharedObject<const Trk::Surface> sharedSurface(sf,true);
    std::pair< Trk::SharedObject<const Trk::Surface>, Amg::Vector3D >  surfaceOrder(sharedSurface, sf->center());
    surfaces.push_back(surfaceOrder);
  }

  // create merged binned array
  Trk::BinnedArray<Trk::Surface>* mergeBA = new Trk::BinnedArray1D1D<Trk::Surface>(surfaces,new Trk::BinUtility(rsteps,Trk::open,Trk::binR),binUtils);

  // prepare the overlap descriptor       
  std::vector<Trk::BinUtility*>* clonedBinUtils = new std::vector<Trk::BinUtility*>;
  for (auto bu : *binUtils) clonedBinUtils->push_back(bu->clone());
  Trk::OverlapDescriptor* olDescriptor = new InDet::DiscOverlapDescriptor(true,mergeBA,clonedBinUtils);
    
  // position & bounds of the disc layer
  double disc_thickness = std::fabs(zb.second-zb.first);
  double disc_pos = (zb.first+zb.second)*0.5;

  Amg::Transform3D*  transf = new Amg::Transform3D();
  (*transf) = Amg::Translation3D(0.,0.,disc_pos);

  // get the layer material from the first merged layer
  const Trk::LayerMaterialProperties* disc_material = inputDiscs[0]->layerMaterialProperties()->clone();

  // create disc layer
  Trk::DiscLayer* layer = new Trk::DiscLayer(transf,
					     new Trk::DiscBounds(rsteps.front(),rsteps.back()),
					     mergeBA,
					     *disc_material,
					     disc_thickness,
					     olDescriptor); 
  
   // register the layer to the surfaces 
   const std::vector<const Trk::Surface*>& layerSurfaces     = mergeBA->arrayObjects();
   for (auto sf : layerSurfaces) {
      const std::vector<const Trk::Surface*>& allSurfacesVector = (sf->associatedDetectorElement())->surfaces();
      for (auto subsf : allSurfacesVector)  subsf->associateLayer(*layer);
   }
   
   for (auto disc : inputDiscs)   delete disc;      // cleanup

   return layer; 

}

void InDet::StagedTrackingGeometryBuilder::addGMmaterial(const Trk::TrackingVolume*& enclosedDetector, float bpRadius ) const {
 
  bool fixedVolume = false;
  
  // retrieve the ITk material and process
  std::vector<Trk::MaterialElement> itk_material;
  Trk::GeoMaterialConverter geoMatCnv;
  Trk::MaterialLayerHelper  mlHelper;
  double massBeamPipe = 0.;
  double massPixel = 0.;
  double massSCT = 0.;
  unsigned int nbp = 0;
  // get BeamPipe DetectorManager
  const BeamPipeDetectorManager* beamPipeMgr;    
  if (detStore()->retrieve(beamPipeMgr,"BeamPipe").isFailure()){
    ATH_MSG_ERROR("Could not retrieve BeamPipe detector manager!" );
  } else ATH_MSG_DEBUG ("BeamPipe Detector Manager found!");
 
  if (beamPipeMgr){
    // loop over GM tree
    for (unsigned int i=0; i<beamPipeMgr->getNumTreeTops(); i++) {
      //geoMatCnv.printInfo( beamPipeMgr->getTreeTop(i));
      std::vector<Trk::GeoObject*> geoContent = geoMatCnv.decodeGMtree(beamPipeMgr->getTreeTop(i));
      //mass estimate
      for (auto geo : geoContent) {
	double mass = geo->material->getDensity()/CLHEP::g*CLHEP::mm3 * geo->volumeSize;      
	if (mass==0) continue;
	if (geo->material->getName()=="Air") continue;
	if (geo->material->getName()=="Vacuum") continue;
	massBeamPipe += mass * geo->transform.size();  // assembly volumes discarded already
	if (mass>0) geoMatCnv.convertGeoObject(geo,itk_material,fixedVolume);
      }
    }
    nbp = itk_material.size();
    // double check the collected mass
    double bpmass = 0.;     
    for (unsigned int ie=0; ie<itk_material.size(); ie++) {
      bpmass += itk_material[ie].mass();
    } 
    ATH_MSG_DEBUG("Material collection finds "<<itk_material.size()<<"  beam pipe objects with total mass "<<  massBeamPipe<<", TG-converted mass is "<< bpmass);
  }
  // get PixelDetectorManager
  const InDetDD::PixelDetectorManager* pixelMgr;
  if (detStore()->retrieve(pixelMgr, "Pixel").isFailure()) {
    ATH_MSG_ERROR("Could not get PixelDetectorManager!");
  } else ATH_MSG_DEBUG ("Pixel Detector Manager found!");
  
  unsigned int npix = 0;
  if (pixelMgr) {
    // loop over GM tree
    for (unsigned int i=0; i<pixelMgr->getNumTreeTops(); i++) {
      //geoMatCnv.printInfo( pixelMgr->getTreeTop(i));
      std::vector<Trk::GeoObject*> geoContent = geoMatCnv.decodeGMtree(pixelMgr->getTreeTop(i));
      // pixel mass estimate
      for (auto geo : geoContent) {
	double mass = geo->material->getDensity()/CLHEP::g*CLHEP::mm3 * geo->volumeSize;      
	massPixel += mass * geo->transform.size();  // assembly volumes discarded already
	if (mass>0) geoMatCnv.convertGeoObject(geo,itk_material,fixedVolume);
      }
    }
    // double check the collected mass
    double pixmass = 0.;     
    for (unsigned int ie=nbp; ie<itk_material.size(); ie++) {
      pixmass += itk_material[ie].mass();
    } 
    ATH_MSG_DEBUG("Material collection finds "<<itk_material.size()-nbp<<"  pixel objects with total mass "<<  massPixel<<", TG-converted mass is "<< pixmass);
  }

  npix = itk_material.size();
 
  // get SCTDetectorManager
  const InDetDD::SCT_DetectorManager* sctMgr;
  if (detStore()->retrieve(sctMgr, "SCT").isFailure()) {
    ATH_MSG_ERROR("Could not get SCT_DetectorManager!");
  } else ATH_MSG_DEBUG ("SCT Detector Manager found!");
   
  if (sctMgr) {
    // lop over GM tree
    for (unsigned int i=0; i<sctMgr->getNumTreeTops(); i++) {
      //geoMatCnv.printInfo( sctMgr->getTreeTop(i));
      std::vector<Trk::GeoObject*> geoContent = geoMatCnv.decodeGMtree(sctMgr->getTreeTop(i));
      // total mass estimate
      for (auto geo : geoContent) {
	double mass = geo->material->getDensity()/CLHEP::g*CLHEP::mm3 * geo->volumeSize;      
	massSCT += mass * geo->transform.size();  // assembly volumes discarded already
	if (mass>0) geoMatCnv.convertGeoObject(geo,itk_material,fixedVolume);
      }
    }
    // double check the collected mass
    double sctmass = 0.;     
    for (unsigned int ie=npix; ie<itk_material.size(); ie++) {
      sctmass += itk_material[ie].mass();
    } 
    ATH_MSG_DEBUG("Material collection finds "<<itk_material.size()-npix<<"  pixel objects with total mass "<<  massSCT<<", TG-converted mass is "<< sctmass);
  }
    
   // retrieve predefined material layers & assign material
  mlHelper.processMaterial(enclosedDetector,itk_material,bpRadius);
   ATH_MSG_DEBUG("GM material processed");
 
   return;
}

/*
   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
 */

#include  "EMExtrapolationTools.h"
#include  "TrkEventPrimitives/PropDirection.h"
#include  "TrkParameters/TrackParameters.h"

//extrapolation
#include "TrkExInterfaces/IExtrapolator.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TrkNeutralParameters/NeutralParameters.h"
//
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"
//
#include "TrkTrack/Track.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
//
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"
#include "TrkParametersIdentificationHelpers/TrackParametersIdentificationHelper.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "FourMomUtils/P4Helpers.h"
#include "GaudiKernel/EventContext.h"
//
#include <tuple>

namespace{
const CaloExtensionHelpers::LayersToSelect barrelLayers={CaloSampling::PreSamplerB, 
    CaloSampling::EMB1, CaloSampling::EMB2, CaloSampling::EMB3};

const CaloExtensionHelpers::LayersToSelect endCapLayers={CaloSampling::PreSamplerE, 
      CaloSampling::EME1, CaloSampling::EME2, CaloSampling::EME3}; 
}

EMExtrapolationTools::EMExtrapolationTools(const std::string& type, 
                                           const std::string& name,
                                           const IInterface* parent) :
  AthAlgTool(type, name, parent),
  m_trtId(0)
{

  declareInterface<IEMExtrapolationTools>(this);
}

EMExtrapolationTools::~EMExtrapolationTools() {}

StatusCode EMExtrapolationTools::initialize() 
{

  ATH_MSG_DEBUG("Initializing " << name() << "...");
  // Retrieve tools
  ATH_CHECK(m_lastParticleCaloExtensionTool.retrieve() ) ;
  ATH_CHECK(m_perigeeParticleCaloExtensionTool.retrieve() );
  ATH_CHECK(m_extrapolator.retrieve());

  // retrieve TRT-ID helper
  if (detStore()->contains<TRT_ID>("TRT_ID")) {
    StatusCode sc = detStore()->retrieve(m_trtId, "TRT_ID");
    if (sc.isFailure() || !m_trtId->is_valid()){
      // TRT is not present for sLHC
      ATH_MSG_DEBUG("Could not get TRT_ID helper !");
      m_trtId = 0;
    }
    ATH_MSG_DEBUG("m_trtId initialization successful");
  } else {
    ATH_MSG_DEBUG("Could not get TRT_ID helper !");
  }

  //Retrieve input where applicable
  ATH_CHECK(m_GSFPerigeeCacheKey.initialize(m_useCaching));
  ATH_CHECK(m_GSFLastCacheKey.initialize(m_useCaching));

  return StatusCode::SUCCESS;
}

StatusCode EMExtrapolationTools::finalize() 
{
  return StatusCode::SUCCESS;
}

/*
 * This is the method that does the heavy lifting for the 
 * electrons extrapolations. Handles multipe extrapolation modes.
 */
StatusCode
EMExtrapolationTools::getMatchAtCalo (const EventContext&           ctx,
                                      const xAOD::CaloCluster*      cluster, 
                                      const xAOD::TrackParticle*    trkPB,
                                      Trk::PropDirection            direction,
                                      std::vector<double>&          eta,
                                      std::vector<double>&          phi,
                                      std::vector<double>&          deltaEta,
                                      std::vector<double>&          deltaPhi,
                                      unsigned int                  extrapFrom,
                                      Cache*                        cache) const
{
  /* Extrapolate track to calo and return 
   * the extrapolated eta/phi and
   * the deta/dphi between cluster and track
   * We allow different ways to extrapolate:
   * 1) from the last measurement  track parameters (this is always the case for TRT standalone)  
   * 2) from the perigee track parameters
   * 3) from the perigee with the track momentum rescaled by the cluster energy
   */
  ATH_MSG_DEBUG("getMatchAtCalo");
  if(deltaEta.size() < 4  || deltaPhi.size()<4 || eta.size()<4 || phi.size()<4 ){
    ATH_MSG_WARNING("deltaEta, deltaPhi, eta , phi size should be at least 4");
    return StatusCode::SUCCESS;
  }
  bool didExtension=false;
  CaloExtensionHelpers::EtaPhiPerLayerVector intersections;
  /* 
   * Rescaled Perigee is "easy"
   * It will never have a cache
   */ 
  double atPerigeePhi(-999);
  double PerigeeTrkParPhi(-999); 
  if(fromPerigeeRescaled == extrapFrom){
    std::unique_ptr<const Trk::TrackParameters> trkPar = getRescaledPerigee(trkPB, cluster);    
    if(!trkPar){
      ATH_MSG_ERROR("getMatchAtCalo: Cannot access track parameters"); 
      return StatusCode::FAILURE; 
    }  
    Amg::Vector3D atPerigee(trkPar->position().x(), trkPar->position().y(), trkPar->position().z()); 
    atPerigeePhi=atPerigee.phi(); ;
    PerigeeTrkParPhi=trkPar->momentum().phi();
    
    std::unique_ptr<Trk::CaloExtension> extension = m_perigeeParticleCaloExtensionTool->caloExtension( *trkPar, 
                                                                                                       direction, 
                                                                                                       Trk::muon);
    didExtension=extension.get()!=nullptr;
    if(didExtension){
      intersections=getIntersections(*extension,cluster); 
    }
  }
  /* For the other cases
   * See if there is a collection cache
   * else if there is an in algorithm cache
   * else do it without a cache
   */
  else if(fromPerigee == extrapFrom){
    if (m_useCaching){
      SG::ReadHandle<CaloExtensionCollection>  PerigeeCache(m_GSFPerigeeCacheKey,ctx);
      if (!PerigeeCache.isValid()) {
        ATH_MSG_ERROR("Could not retrieve Perigee Cache " << PerigeeCache.key());
        return StatusCode::FAILURE;
      }
      const Trk::CaloExtension* extension = m_perigeeParticleCaloExtensionTool->caloExtension(*trkPB,*PerigeeCache);
      didExtension=extension!=nullptr; 
      if(didExtension){
        intersections=getIntersections(*extension,cluster);
      }
    }
    else if(cache){
      const Trk::CaloExtension* extension=m_perigeeParticleCaloExtensionTool->caloExtension(*trkPB,*cache);
      didExtension=extension!=nullptr; 
      if(didExtension){ 
        intersections=getIntersections(*extension,cluster);
      }
    } 
    else{
      std::unique_ptr<Trk::CaloExtension> extension = m_perigeeParticleCaloExtensionTool->caloExtension(*trkPB);
      didExtension=extension!=nullptr;
      if(didExtension){
        intersections=getIntersections(*extension,cluster); 
      }
    }
  }
  else if(fromLastMeasurement == extrapFrom){
    if (m_useCaching){
      SG::ReadHandle<CaloExtensionCollection>  LastCache(m_GSFLastCacheKey,ctx);
      if (!LastCache.isValid()) {
        ATH_MSG_ERROR("Could not retrieve Last Cache " << LastCache.key());
        return StatusCode::FAILURE;
      }
      const Trk::CaloExtension* extension=m_lastParticleCaloExtensionTool->caloExtension(*trkPB,*LastCache);
      didExtension=extension!=nullptr; 
      if(didExtension){ 
        intersections=getIntersections(*extension,cluster);
      }
    }
    else if(cache){
      const Trk::CaloExtension* extension =m_lastParticleCaloExtensionTool->caloExtension(*trkPB,*cache);
      didExtension=extension!=nullptr; 
      if(didExtension){ 
        intersections=getIntersections(*extension,cluster);
      }
    } 
    else{
      std::unique_ptr<Trk::CaloExtension> extension = m_lastParticleCaloExtensionTool->caloExtension(*trkPB);
      didExtension=extension!=nullptr;
      if(didExtension){
        intersections=getIntersections(*extension,cluster); 
      }
    }
  }
  /*
   * Given the extension calculate the deta/dphi for the layers
   */
  if(!didExtension){
    ATH_MSG_INFO("Could not create an extension from " << extrapFrom 
                 <<  " for a track with : "
                 << " Track Pt " <<trkPB->pt()
                 << " Track Eta " <<trkPB->eta()
                 << " Track Phi " <<trkPB->phi()
                 <<" Track Fitter " << trkPB->trackFitter() );
    return StatusCode::FAILURE; 
  }
  // Negative tracks bend to the positive direction.
  // flip sign for positive ones
  const bool flipSign = trkPB->charge() > 0 ? true :false; 

  for( const auto& p : intersections ){    
    int  i(0); 
    CaloSampling::CaloSample sample = std::get<0>(p);
    if (sample == CaloSampling::PreSamplerE || sample == CaloSampling::PreSamplerB  ){
      i = 0;
    } else if (sample == CaloSampling::EME1 || sample == CaloSampling::EMB1 ){
      i = 1;
    } else if (sample == CaloSampling::EME2 || sample == CaloSampling::EMB2 ){
      i = 2;
    } else if (sample == CaloSampling::EME3 || sample == CaloSampling::EMB3) {
      i = 3;
    } else {
      continue;
    }
    eta[i]      = std::get<1>(p); 
    phi[i]      = std::get<2>(p); 
    deltaEta[i] = cluster->etaSample(sample) - std::get<1>(p); 
    deltaPhi[i] = P4Helpers::deltaPhi(cluster->phiSample(sample),std::get<2>(p)); 
    // Should we flip the sign for deltaPhi? 
    if(flipSign)  {
      deltaPhi[i] = -deltaPhi[i];
    }
    ATH_MSG_DEBUG("getMatchAtCalo: i, eta, phi, deta, dphi: "  
                  << i << " " << eta[i] << " " << phi[i] << " " 
                  << deltaEta[i] << " " << deltaPhi[i]);     
    if (fromPerigeeRescaled == extrapFrom) {
      if ( i == 2 && deltaPhi.size() > 4) { 
        /* For rescaled perigee when at sampling 2, save the the  rotation in phi
         * This is defined as: 
         * ((phi of point at sampling 2 - phi of point at track vertex/perigee) - 
         * phi of direction of track at perigee) 
         */  
        const double perToSamp2 = std::get<2>(p)  - atPerigeePhi; 
        deltaPhi[4] = fabs(P4Helpers::deltaPhi(perToSamp2, PerigeeTrkParPhi)); 
        ATH_MSG_DEBUG("getMatchAtCalo: phi-rot: " << deltaPhi[4]); 
      }
    }
  }
  return StatusCode::SUCCESS;   
}

/*
 * Photon/vertex/Neutral track parameters 
 * Related methods 
 * needed for  Extrapolating/matching conversions
 */
bool EMExtrapolationTools::matchesAtCalo(const xAOD::CaloCluster* cluster,
                                         const xAOD::Vertex *vertex,
                                         float etaAtCalo,
                                         float phiAtCalo) const
{
  if (!cluster || !vertex) return false;  
  float deltaEta = fabs(etaAtCalo - cluster->etaBE(2));
  float deltaPhi = fabs(P4Helpers::deltaPhi(cluster->phi(), phiAtCalo));

  int TRTsection = 0;
  if (xAOD::EgammaHelpers::numberOfSiTracks(vertex) == 0)
    TRTsection = getTRTsection(vertex->trackParticle(0));

  // First pass on TRT tracks, skip barrel tracks matching endcap clusters and vice-versa
  if((TRTsection==2 && (cluster->eta()<=0.6  || cluster->eta()>=2.4)   ) ||
     (TRTsection==-2 && (cluster->eta()>=-0.6 || cluster->eta()<=-2.4)  ) ||
     (TRTsection==1 && (cluster->eta()<=-0.1 || cluster->eta()>=1.3)   ) ||
     (TRTsection==-1 && (cluster->eta()>=0.1  || cluster->eta()<=-1.3)  )
    ) {
    return false;
  }

  // The maximum deltaEta/deltaPhi for Si, TRT barrel, TRT endcap
  static const std::vector<double> dEtaV{m_narrowDeltaEta, m_TRTbarrelDeltaEta, m_TRTendcapDeltaEta};
  static const std::vector<double> dPhiV{m_narrowDeltaPhi, m_narrowDeltaPhiTRTbarrel, m_narrowDeltaPhiTRTendcap};

  return (deltaEta < dEtaV[abs(TRTsection)] && deltaPhi < dPhiV[abs(TRTsection)]);
}
/*
 * The following two are the heavy lifting methods.
 * Start from vertex/Track Parameters 
 * and then calculate the eta/phi at calo
 */
bool EMExtrapolationTools::getEtaPhiAtCalo (const xAOD::Vertex* vertex, 
                                            float *etaAtCalo,
                                            float *phiAtCalo) const
{
  if (!vertex) return false;
  Amg::Vector3D momentum = getMomentumAtVertex(*vertex);
  if (momentum.mag() < 1e-5){
    ATH_MSG_DEBUG("Intersection failed");
    return false;
  }
  Trk::PerigeeSurface surface (vertex->position());
  /*
   * Create high pt track parameters to mimic a neutral particle.
   * This in principle is an approximation
   */ 
  const Trk::TrackParameters* trkPar =  surface.createTrackParameters( vertex->position(),
                                                                       momentum.unit() *1.e10, 
                                                                       +1, 
                                                                       0);
  bool success = getEtaPhiAtCalo( trkPar, etaAtCalo, phiAtCalo);
  delete trkPar;
  return success;   
}
/*  The actual calculation happens here*/
bool EMExtrapolationTools::getEtaPhiAtCalo (const Trk::TrackParameters* trkPar, 
                                            float *etaAtCalo,
                                            float *phiAtCalo) const{
  if (!trkPar) return false;
  CaloExtensionHelpers::LayersToSelect layersToSelect;    
  if ( fabs(trkPar->eta()) < 1.425) {
    // Barrel
    layersToSelect.insert(CaloSampling::EMB2 );  
  }else {
    // Endcap
    layersToSelect.insert(CaloSampling::EME2 );  
  }

  std::unique_ptr<Trk::CaloExtension> extension = nullptr;      
  extension = m_perigeeParticleCaloExtensionTool->caloExtension( *trkPar, Trk::alongMomentum, Trk::muon );
  if(!extension){
    ATH_MSG_WARNING("Could not create an extension from geEtaPhiAtCalo ");
    return false;
  } 
  CaloExtensionHelpers::EtaPhiPerLayerVector intersections;
  CaloExtensionHelpers::midPointEtaPhiPerLayerVector( *extension, intersections, &layersToSelect );
  bool hitEM2(false);
  for( const auto& p : intersections ){    
    int  i(0); 
    auto sample = std::get<0>(p);
    if ( ( sample == CaloSampling::EME2 || sample == CaloSampling::EMB2 )){
      *etaAtCalo = std::get<1>(p); 
      *phiAtCalo = std::get<2>(p); 
      hitEM2=true;
      ++i;
      ATH_MSG_DEBUG("getMatchAtCalo: i, eta, phi : "  
                    << i << " " << std::get<1>(p) << " " << std::get<2>(p) ); 
    }
  }
  return hitEM2;   
}

/* Methods to get the momemtum at the conversion vertex*/
Amg::Vector3D EMExtrapolationTools::getMomentumAtVertex(const xAOD::Vertex& vertex, unsigned int index) const
{
  Amg::Vector3D momentum(0., 0., 0.);
  if (vertex.nTrackParticles() <= index){
    ATH_MSG_WARNING("Invalid track index");
  }
  else if (vertex.vxTrackAtVertexAvailable() && vertex.vxTrackAtVertex().size()){
    // Use the parameters at the vertex 
    // (the tracks should be parallel but we will do the sum anyway)
    ATH_MSG_DEBUG("getMomentumAtVertex : getting from vxTrackAtVertex");
    const auto& trkAtVertex = vertex.vxTrackAtVertex()[index];
    const Trk::TrackParameters* paramAtVertex = trkAtVertex.perigeeAtVertex();
    if (!paramAtVertex){
      ATH_MSG_WARNING("VxTrackAtVertex does not have perigee at vertex");
    }else{
      return paramAtVertex->momentum();
    }
  }
  else if (vertex.nTrackParticles() == 1){
    // Use the first measurement
    ATH_MSG_DEBUG("getMomentumAtVertex : 1 track only, getting from first measurement");
    const xAOD::TrackParticle *tp = vertex.trackParticle(0);
    unsigned int paramindex(0);
    if (!tp || !tp->indexOfParameterAtPosition(paramindex, xAOD::FirstMeasurement)){
      ATH_MSG_WARNING("No TrackParticle or no have first measurement");
    }
    else{
      momentum += tp->curvilinearParameters(paramindex).momentum();
    }
  }
  else{
    // Extrapolate track particle to vertex
    ATH_MSG_DEBUG("getMomentumAtVertex : extrapolating to perigee surface");
    const xAOD::TrackParticle* tp = vertex.trackParticle( index );
    if (!tp) {ATH_MSG_WARNING("NULL pointer to TrackParticle in vertex");}
    else{
      const Trk::PerigeeSurface *surface = new Trk::PerigeeSurface(vertex.position());
      const Trk::TrackParameters* params = m_extrapolator->extrapolate(*tp, *surface, Trk::alongMomentum);
      delete surface;
      if (!params) ATH_MSG_DEBUG("Extrapolation to vertex (perigee) failed");
      else momentum += params->momentum();
      delete params;
    }
  }
  return momentum;  
}

Amg::Vector3D EMExtrapolationTools::getMomentumAtVertex(const xAOD::Vertex& vertex, bool reuse /* = true */) const
{
  Amg::Vector3D momentum(0., 0., 0.);
  const static SG::AuxElement::Accessor<float> accPx("px");
  const static SG::AuxElement::Accessor<float> accPy("py");
  const static SG::AuxElement::Accessor<float> accPz("pz");
  if (vertex.nTrackParticles() == 0){
    ATH_MSG_WARNING("getMomentumAtVertex : vertex has no track particles!");
    return momentum;
  } 
  if (reuse &&
      accPx.isAvailable(vertex) && 
      accPy.isAvailable(vertex) && 
      accPz.isAvailable(vertex) 
     ){
    // Already decorated with parameters at vertex
    ATH_MSG_DEBUG("getMomentumAtVertex : getting from auxdata");
    return Amg::Vector3D(accPx(vertex), 
                         accPy(vertex),  
                         accPz(vertex)); 
  }
  else{
    for (unsigned int i = 0; i < vertex.nTrackParticles(); ++i){
      momentum += getMomentumAtVertex(vertex, i);
    }
  }
  return momentum;	
}
/* 
 * Create Rescaled Perigee Parametrs
 */
std::unique_ptr<const Trk::TrackParameters> 
EMExtrapolationTools::getRescaledPerigee(const xAOD::TrackParticle* trkPB, const xAOD::CaloCluster* cluster) const {
  const Trk::TrackParameters* oldPerigee = &trkPB->perigeeParameters();
  if (!oldPerigee) return 0;
  Amg::Vector3D mom(oldPerigee->momentum().x(),oldPerigee->momentum().y(),oldPerigee->momentum().z());
  const double mag = mom.mag();
  if (mag == 0 || cluster->e() == 0) {
    // should this be a warning or debug? For now I assume that it should not happen
    ATH_MSG_WARNING("Trying to rescale perigee but cluster e = " << cluster->e() << " and perigee mag = " << mag);
    return 0;
  }
  //Same eta/phi
  double phi=mom.phi();
  double theta= mom.theta();
  // Rescale the momentum
  double charge = oldPerigee->charge();
  double qoverp = charge/cluster->e();
  // Create new perigee
  Trk::PerigeeSurface surface (oldPerigee->position());
  //The surface has the correct offset in x and y 
  return std::unique_ptr<const Trk::TrackParameters> (surface.createParameters<5,Trk::Charged>(0,
                                                                                              0,
                                                                                              phi,
                                                                                              theta,
                                                                                              qoverp));
}
/*
 * Helper to get the Eta/Phi intersections per Layer
 */
CaloExtensionHelpers::EtaPhiPerLayerVector 
EMExtrapolationTools::getIntersections (const Trk::CaloExtension& extension,
                                        const xAOD::CaloCluster* cluster) const
{
  //Layers to calculate intersections
  CaloExtensionHelpers::EtaPhiPerLayerVector intersections;
  if ( xAOD::EgammaHelpers::isBarrel( cluster )  ) {
    CaloExtensionHelpers::midPointEtaPhiPerLayerVector(extension, intersections, &barrelLayers );
  } else {
   CaloExtensionHelpers::midPointEtaPhiPerLayerVector(extension, intersections, &endCapLayers );
  }
  return intersections;
}
/*
 * Helper to identify the TRT section
 */
int EMExtrapolationTools::getTRTsection(const xAOD::TrackParticle* trkPB) const{
  if (!trkPB){
    ATH_MSG_DEBUG("Null pointer to TrackParticle");
    return 0;
  } 
  if (!m_trtId){
    ATH_MSG_DEBUG("No trt ID guessing TRT section based on eta: " << trkPB->eta());
    return (trkPB->eta() > 0 ? 1 : -1) * (fabs(trkPB->eta()) < 0.6 ? 1 : 2);
  }
  const Trk::MeasurementBase* trkPar =0;  
  if( trkPB->trackLink().isValid() && trkPB->track() != 0 ) {
    ATH_MSG_DEBUG("Will get TrackParameters from Trk::Track");
    const DataVector<const Trk::TrackStateOnSurface> *trackStates = trkPB->track()->trackStateOnSurfaces();
    if (!trackStates) {
      ATH_MSG_WARNING("NULL pointer to trackStateOnSurfaces");
      return 0;
    }   
    //Loop over the TrkStateOnSurfaces search last valid TSOS first
    for ( DataVector<const Trk::TrackStateOnSurface>::const_reverse_iterator rItTSoS = trackStates->rbegin(); 
          rItTSoS != trackStates->rend(); ++rItTSoS){
      if ( (*rItTSoS)->type(Trk::TrackStateOnSurface::Measurement) && 
           !((*rItTSoS)->type(Trk::TrackStateOnSurface::Outlier))  && 
           (*rItTSoS)->measurementOnTrack()!=nullptr && 
           !((*rItTSoS)->measurementOnTrack()->type(Trk::MeasurementBaseType::PseudoMeasurementOnTrack))){
        trkPar = (*rItTSoS)->measurementOnTrack();
        break;
      }
    }
  }
  else {
    ATH_MSG_WARNING("Track particle without Trk::Track");
  }
  if (!trkPar) {
    return 0;
  }
  const Trk::Surface& sf = trkPar->associatedSurface();
  const Identifier tid = sf.associatedDetectorElementIdentifier();
  return m_trtId->barrel_ec(tid);
}

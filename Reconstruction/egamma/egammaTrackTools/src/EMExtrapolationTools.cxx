/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include  "EMExtrapolationTools.h"
#include  "TrkEventPrimitives/PropDirection.h"
#include  "TrkParameters/TrackParameters.h"

//extrapolation      
#include "RecoToolInterfaces/IExtrapolateToCaloTool.h"
#include "CaloDetDescr/CaloDepthTool.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TrkVertexFitterInterfaces/INeutralParticleParameterCalculator.h"
#include "TrkNeutralParameters/NeutralParameters.h"
// #include "xAODTracking/TrackingPrimitives.h"

#include "xAODCaloEvent/CaloCluster.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"

#include "TrkTrack/Track.h"
#include "TrkPseudoMeasurementOnTrack/TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"

#include "xAODEgamma/EgammaxAODHelpers.h"



EMExtrapolationTools::EMExtrapolationTools(const std::string& type, 
                                           const std::string& name,
                                           const IInterface* parent) :
  AthAlgTool(type, name, parent),
  m_convUtils("Trk::NeutralParticleParameterCalculator")
{

  declareProperty( "ExtrapolateToCaloTool", m_extrapolateToCalo);

  // Name of the utility for conversion
  declareProperty("ConversionUtils", m_convUtils, "Handle of the utility for conversion");
  
  declareProperty( "BroadDeltaEta",         m_broadDeltaEta      = 0.05);
  declareProperty( "BroadDeltaPhi",         m_broadDeltaPhi      = 0.10);
  declareProperty( "NarrowDeltaEta",        m_narrowDeltaEta     = 0.05);
  declareProperty( "NarrowDeltaPhi",        m_narrowDeltaPhi     = 0.05);
  declareProperty( "NarrowDeltaPhiBrem",    m_narrowDeltaPhiBrem = 0.10);
  declareProperty( "NarrowDeltaPhiTRTbarrel",     m_narrowDeltaPhiTRTbarrel  = 0.02);
  declareProperty( "NarrowDeltaPhiBremTRTbarrel", m_narrowDeltaPhiBremTRTbarrel = 0.03);  
  declareProperty( "NarrowDeltaPhiTRTendcap",     m_narrowDeltaPhiTRTendcap  = 0.02);
  declareProperty( "NarrowDeltaPhiBremTRTendcap", m_narrowDeltaPhiBremTRTendcap = 0.03);  
  declareProperty( "TRTbarrelDeltaEta",     m_TRTbarrelDeltaEta  = 0.35);
  declareProperty( "TRTendcapDeltaEta",     m_TRTendcapDeltaEta  = 0.2);
  
  declareProperty("useTrkTrackForTRT", m_useTrkTrackForTRT = true, "Use Trk::Track instead of TrackParticle to determine TRT section");
  declareProperty("guessTRTsection", m_guessTRTsection = false, "Guess TRT section from eta, instead of using track parameters");

  declareInterface<IEMExtrapolationTools>(this);
}

// =========================================================
EMExtrapolationTools::~EMExtrapolationTools() {}

// =========================================================
StatusCode EMExtrapolationTools::initialize() 
{

  ATH_MSG_DEBUG("Initializing " << name() << "..."); 
  
  // Retrieve TrackToCalo tool for extrapolation to calorimeter
  if ( m_extrapolateToCalo.retrieve().isFailure() ) {
    ATH_MSG_ERROR("Cannot retrieve extrapolateToCaloTool " 
      << m_extrapolateToCalo);
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG("Retrieved track-to-calo tool " 
      << m_extrapolateToCalo);
  }
  
  // retrieved via the Extrapolator 
  m_calodepth = m_extrapolateToCalo->getCaloDepth();
  if (!m_calodepth) {
    ATH_MSG_ERROR("Cannot get CaloDepthTool");
    
    return StatusCode::FAILURE; 
  } else 
    ATH_MSG_DEBUG("Get CaloDepthTool");

  // retrieve TRT-ID helper
  m_trtId = 0;
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
  if (!m_trtId) m_guessTRTsection = true;

  if (m_convUtils.retrieve().isFailure()) {
    ATH_MSG_ERROR("Could not find ConvUtils tool.");
    return StatusCode::FAILURE;
  } 
  else ATH_MSG_DEBUG("ConvUtils retrieved");
  
  return StatusCode::SUCCESS;
}

// =====================================================================
StatusCode EMExtrapolationTools::finalize() 
{
  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//* Extrapolate TrackParticle to electron CaloCluster
//////////////////////////////////////////////////////////////////////////////
bool
EMExtrapolationTools::matchesAtCalo(const xAOD::CaloCluster*      cluster,       
                                    const xAOD::TrackParticle*    trkPB, 
                                    bool                          isTRT,
                                    unsigned int                  extrapFrom)
{

  // Forward to move complex extrapolation
  std::vector<bool>    doSample(4, false);
  std::vector<double>  eta(4, -999.0);
  std::vector<double>  phi(4, -999.0);
  std::vector<double>  deltaEta(4, -999.0);
  std::vector<double>  deltaPhi(4, -999.0);
  // extrapolate only to sampling 2
  doSample[2] = true;
  return (matchesAtCalo(cluster, 
                        trkPB, 
                        isTRT, 
                        Trk::alongMomentum,
                        doSample,
                        eta,
                        phi,
                        deltaEta,
                        deltaPhi,
                        extrapFrom) );
}

///////////////////////////////////////////////////////////////////////////////
//* Extrapolate double-track conversion with the neutral perigee
///////////////////////////////////////////////////////////////////////////////
bool
EMExtrapolationTools::matchesAtCalo(const xAOD::CaloCluster*           cluster, 
                                    const Trk::NeutralParameters*      phPerigee, 
                                    bool                               isTRT,
                                    double&                            deltaEta,
                                    double&                            deltaPhi) 
{

  double          tmpDeltaEta   = -999.;
  double          tmpDeltaPhi   = -999.;
  int             trapped    = 99;

  const Trk::NeutralParameters* result   = 0;
  double offset = 0.;
  CaloCell_ID::CaloSample sample;

     
  if ( xAOD::EgammaHelpers::isBarrel( cluster ) ) {
    sample  = CaloCell_ID::EMB2;
    result = m_extrapolateToCalo->extrapolate((*phPerigee), sample, offset, Trk::photon, Trk::alongMomentum);
    if (result) {
      trapped  = 0;
      tmpDeltaEta = cluster->etaSample(sample) - result->position().eta();
      tmpDeltaPhi = m_phiHelper.diff(cluster->phiSample(sample),result->position().phi());
    }
  }else {
    sample = CaloCell_ID::EME2;
    result = m_extrapolateToCalo->extrapolate((*phPerigee), sample, offset, Trk::photon, Trk::alongMomentum);
    if (result) {
      trapped = 0;
      tmpDeltaEta = cluster->etaSample(sample) - result->position().eta();
      tmpDeltaPhi = m_phiHelper.diff(cluster->phiSample(sample),result->position().phi());
    }
  }
  if(result) delete result;
  
  deltaEta = tmpDeltaEta;
  deltaPhi = tmpDeltaPhi;

  // Match in a broad eta / phi window. For TRT-only tracks we don't know if they are in the 
  // barrel or in the endcap. Apply the loosest cut (the code below would be the correct one)
  //       || (abs(isTRTB)==1 && fabs(deltaEta) > m_TRTbarrelDeltaEta) ||
  //       || (abs(isTRTB)==2 && fabs(deltaEta) > m_TRTendcapDeltaEta) ||

  if (trapped != 0
      || (!isTRT && (fabs(deltaEta) > m_broadDeltaEta) )
      || (isTRT && (fabs(deltaEta) > std::max(m_TRTbarrelDeltaEta, m_TRTendcapDeltaEta)) )         
           || fabs(deltaPhi) > m_broadDeltaPhi) return false;

  if ((isTRT && fabs(deltaPhi) < std::max(m_narrowDeltaPhiTRTbarrel, m_narrowDeltaPhiTRTendcap)) || 
      (!isTRT && fabs(deltaEta)< m_narrowDeltaEta && fabs(deltaPhi) < m_narrowDeltaPhi) )
    return true;
  
  return false;
}


//////////////////////////////////////////////////////////////////////////////
//* Extrapolate TrackParticle to electron CaloCluster
//////////////////////////////////////////////////////////////////////////////
bool
EMExtrapolationTools::matchesAtCalo(const xAOD::CaloCluster*      cluster, 
                                    const xAOD::TrackParticle*    trkPB, 
                                    bool                          isTRT, 
                                    Trk::PropDirection            direction,
                                    const std::vector<bool>&      doSample,
                                    std::vector<double>&          eta,
                                    std::vector<double>&          phi,
                                    std::vector<double>&          deltaEta,
                                    std::vector<double>&          deltaPhi,
                                    unsigned int                  extrapFrom) const  
{
  //  Extrapolate track to calo (sampling 2) and check for eta/phi
  //   match with cluster Return true for a match, and as well the
  //   values for eta/phi and deta/dphi for the samplings requested
  //     
  //   We allow different ways to extrapolate:
  // 
  //     1) from the last measurement point track parameters  
  //     2) from the perigee 
  //     3) from the perigee with the track momentum rescaled by the cluster energy
  //     4) from the cluster to the perigee - fast simple extrapolation
  //

  // Checks
  //   require input vectors to be of the same size:
  unsigned int vsz = doSample.size();
  if ((vsz < 3) || (vsz != deltaEta.size()) || (vsz != deltaPhi.size() && (vsz + 1) != deltaPhi.size()) || 
      (vsz != eta.size()) || (vsz != phi.size())) {
    ATH_MSG_ERROR("matchesAtCalo: input vectors must be the same size and >= 3, sizes - doSample: " 
      << doSample.size() << " deltaEta: " << deltaEta.size() 
      << " deltaPhi: " << deltaPhi.size());
    return false;
  }

  // local variables
  int                        trapped    = 99;
  // Sample for testing match
  unsigned int               iSampling  = 2;  
  // result of extrapolation
  const Trk::TrackParameters* result     = 0;
  // offset for extrapolation
  double                     offset     = 0.;


  const Trk::TrackParameters* trkPar =0;
  Trk::CurvilinearParameters temp; 
  if(isTRT || fromLastMeasurement == extrapFrom )  {
    //For TRT it is always the last
    int index(-1);
    for(unsigned int i(0); i< trkPB->numberOfParameters() ; ++i ){
      if( xAOD::LastMeasurement == trkPB->parameterPosition(i) ){
        index = i;
        break;
      }
    }
    if(index!=-1){
      temp = trkPB->curvilinearParameters(index);
      trkPar = &temp;
    } else {
      ATH_MSG_WARNING("Track Particle does not contain Last Measurement track parameters");
    }
  } 
  else if (fromPerigee == extrapFrom) {
    // From perigee
    trkPar = &trkPB->perigeeParameters();
  }
  else if (fromPerigeeRescaled == extrapFrom) {
    // From rescaled perigee
    trkPar = getRescaledPerigee(trkPB, cluster);    
  }
  
  if(!trkPar){
    ATH_MSG_WARNING("Track Particle does not have the requested track parameters --  extrapolation not possible");
    return false;
  }


  // Should we flip the sign for deltaPhi?
  bool flipSign = false;
  if(trkPar->charge() > 0) flipSign = true;

  // In case of standalone TRT tracks get an idea where the last measurement is
  int isTRTB = 0; // barrel or endcap TRT
  if(isTRT){
    if (!m_trtId) {
      ATH_MSG_WARNING("Should have m_trtId defined for isTRT");
      return false;
    }
    // Get last measurement for TRT check
    const Trk::TrackParameters*  trkParTRT = trkPar;
    const Trk::Surface& sf = trkParTRT->associatedSurface();
    const Identifier tid = sf.associatedDetectorElementIdentifier();
    isTRTB = m_trtId->barrel_ec(tid);
    // First pass on TRT tracks, skip barrel tracks matching endcap clusters and vice-versa
    if((isTRTB==2 && (cluster->eta()<=0.6  || cluster->eta()>=2.4)   ) ||
       (isTRTB==-2 && (cluster->eta()>=-0.6 || cluster->eta()<=-2.4)  ) ||
       (isTRTB==1 && (cluster->eta()<=-0.1 || cluster->eta()>=1.3)   ) ||
       (isTRTB==-1 && (cluster->eta()>=0.1  || cluster->eta()<=-1.3)  )
       ) {
      return false;
    }
  }

  // Identifier barrel or endcap to identify correct calo sampling
  int bec = 0;
  if ( xAOD::EgammaHelpers::isBarrel( cluster )  ) { 
    // Barrel
    bec = 0;
  } else {
    // Endcap
    bec = 1;
  }
  
  // Setup array for iteration over calo samplings
  static CaloCell_ID::CaloSample samples[2][4] = 
    { { CaloCell_ID::PreSamplerB,
        CaloCell_ID::EMB1,
        CaloCell_ID::EMB2,
        CaloCell_ID::EMB3 },
      { CaloCell_ID::PreSamplerE,
        CaloCell_ID::EME1,
        CaloCell_ID::EME2,
        CaloCell_ID::EME3 } };


  static CaloSampling::CaloSample xAODsamples[2][4] = 
    { { CaloSampling::PreSamplerB,
        CaloSampling::EMB1,
        CaloSampling::EMB2,
        CaloSampling::EMB3 },
      { CaloSampling::PreSamplerE,
        CaloSampling::EME1,
        CaloSampling::EME2,
        CaloSampling::EME3 } };
  
  if (  msgLvl (MSG::DEBUG) )
  {
    ATH_MSG_DEBUG("Parameters for extrapolation:");
    trkPar->dump( msg() );
  }

  // Extrapolate to test sample
  result = m_extrapolateToCalo->extrapolate ((*trkPar), samples[bec][iSampling], offset,
                                             Trk::nonInteracting, direction);
  // Save impacts for cleanup
  std::vector<const Trk::TrackParameters*> impacts;

  if(result) {
    // save deltaEta/deltaPhi
    trapped  = 0;
    deltaEta[iSampling] = cluster->etaSample(  xAODsamples[bec][iSampling] ) - result->position().eta();
    deltaPhi[iSampling] = m_phiHelper.diff(cluster->phiSample(xAODsamples[bec][iSampling]),result->position().phi());
    if(flipSign)  deltaPhi[iSampling] = -deltaPhi[iSampling];
    impacts.push_back(result);
  }

  
  // For rescaled perigee, we must delete the trkPar which were
  // created on the heap
  if (fromPerigeeRescaled == extrapFrom) {
    delete trkPar;
  }

  // Selection in broad eta/phi window
  if (trapped != 0 
      || (isTRT && abs(isTRTB)==1 && fabs(deltaEta[iSampling]) > m_TRTbarrelDeltaEta)
      || (isTRT && abs(isTRTB)==2 && fabs(deltaEta[iSampling]) > m_TRTendcapDeltaEta)
      || (!isTRT && (fabs(deltaEta[iSampling]) > m_broadDeltaEta)) 
      || fabs(deltaPhi[iSampling]) > m_broadDeltaPhi) {
    // cleanup
    for (unsigned int i = 0; i < impacts.size(); i++) {
      delete impacts[i];
    }
    return false;
  }
  
  // Selection in narrow eta/phi window
  if((isTRT && 
      ((abs(isTRTB)==1 && deltaPhi[iSampling] < m_narrowDeltaPhiTRTbarrel && 
        deltaPhi[iSampling] > -m_narrowDeltaPhiBremTRTbarrel) || 
       (abs(isTRTB)==2 && deltaPhi[iSampling] < m_narrowDeltaPhiTRTendcap && 
        deltaPhi[iSampling] > -m_narrowDeltaPhiBremTRTendcap)))
      || (!isTRT && fabs(deltaEta[iSampling]) < m_narrowDeltaEta && 
        deltaPhi[iSampling] < m_narrowDeltaPhi && 
        deltaPhi[iSampling] > -m_narrowDeltaPhiBrem) ) {

    // successful match
    eta[iSampling]      = result->position().eta();
    phi[iSampling]      = result->position().phi();

    // Now do other requested extrapolations - use previous
    // extrapolation endpoint for efficiency

    // Save last impact for the next starting point for extrapolation
    const Trk::TrackParameters* lastImpact = result;
    // For the first extrapolation, we must check whether or not we
    // must invert the extrapolation direction
    bool firstExtrap = true;
    for (unsigned int i = 0; i < doSample.size(); i++) {
      if (i == iSampling || !doSample[i]) continue;  // skip unrequested samples
      Trk::PropDirection dir = direction;
      if (firstExtrap) {
          // invert if we go to an earlier sampling
          if (i < iSampling) dir = (dir == Trk::alongMomentum) ? 
                                 Trk::oppositeMomentum : Trk::alongMomentum;
          firstExtrap = false;
      }
      const Trk::TrackParameters* impact = 
        m_extrapolateToCalo->extrapolate ((*lastImpact), samples[bec][i], offset,
                                          Trk::nonInteracting, dir);
      if (impact) {
        eta[i]      = impact->position().eta();
        phi[i]      = impact->position().phi();
        deltaEta[i] = cluster->etaSample( xAODsamples[bec][i]) - impact->position().eta();
        deltaPhi[i] = m_phiHelper.diff(cluster->phiSample( xAODsamples[bec][i]), impact->position().phi());
        if(flipSign)  deltaPhi[i] = -deltaPhi[i];
        lastImpact  = impact;
        impacts.push_back(impact);
      }
    }
    // cleanup
    for (unsigned int i = 0; i < impacts.size(); i++) {
      delete impacts[i];
    }
    return true;
  }

  // cleanup
  for (unsigned int i = 0; i < impacts.size(); i++) {
    delete impacts[i];
  }
  return false;
}

// =================================================================
  /** Vertex-to-cluster match **/
bool EMExtrapolationTools::matchesAtCalo(const xAOD::CaloCluster* cluster,
                                         const xAOD::Vertex *vertex,
                                         float etaAtCalo,
                                         float phiAtCalo) const
{
  if (!cluster || !vertex) return false;  
  float deltaEta = fabs(etaAtCalo - cluster->etaBE(2));
  float deltaPhi = fabs(m_phiHelper.diff(cluster->phi(), phiAtCalo));
  
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
  std::vector<double> dEtaV{m_narrowDeltaEta, m_TRTbarrelDeltaEta, m_TRTendcapDeltaEta};
  std::vector<double> dPhiV{m_narrowDeltaPhi, m_narrowDeltaPhiTRTbarrel, m_narrowDeltaPhiTRTendcap};

  return (deltaEta < dEtaV[abs(TRTsection)] && deltaPhi < dPhiV[abs(TRTsection)]);
}


/////////////////////////////////////////////////////////////////////////////////////////////
//* Extrapolate to calorimeter
/////////////////////////////////////////////////////////////////////////////////////////////
StatusCode
EMExtrapolationTools::getMatchAtCalo (const xAOD::CaloCluster*      cluster, 
                                      const xAOD::TrackParticle*    trkPB,
                                      Trk::PropDirection            direction,
                                      const std::vector<bool>&      doSample,
                                      std::vector<double>&          eta,
                                      std::vector<double>&          phi,
                                      std::vector<double>&          deltaEta,
                                      std::vector<double>&          deltaPhi,
                                      unsigned int                  extrapFrom) const
{

  // Extrapolate track to calo and return the extrapolated eta/phi and
  // the deta/dphi between cluster and track

  //   We allow different ways to extrapolate:
  // 
  //     1) from the last measurement point track parameters  
  //     2) from the perigee 
  //     3) from the perigee with the track momentum rescaled by the cluster energy
  //

  // Checks
  //   require input vectors to be of the same size:

  ATH_MSG_DEBUG("getMatchAtCalo");

  unsigned int vsz = doSample.size(); 
  if ((vsz != deltaEta.size()) || (vsz != deltaPhi.size() && (vsz + 1) != deltaPhi.size()) ||  
      (vsz != eta.size()) || (vsz != phi.size())) { 
    ATH_MSG_ERROR("getMatchAtCalo: input vectors not the same size - doSample: "  
                  << doSample.size() << " deltaEta: " << deltaEta.size()  
                  << " deltaPhi: " << deltaPhi.size()); 
    return StatusCode::FAILURE; 
  } 
 
 
  const Trk::TrackParameters* trkPar = 0;  
  Trk::CurvilinearParameters temp; 
  if( fromLastMeasurement == extrapFrom )  {
    //For TRT it is always the last
    int index(-1);
    for(unsigned int i(0); i< trkPB->numberOfParameters() ; ++i ){
      if( xAOD::LastMeasurement == trkPB->parameterPosition(i) ){
        index = i;
        break;
      }
    }
    if(index!=-1){
      temp = trkPB->curvilinearParameters(index);
      trkPar = &temp;
    } else {
      ATH_MSG_WARNING("Track Particle does not contain Last Measurement track parameters");
    }
  } 
  else if (fromPerigee == extrapFrom) {
    // From perigee
    trkPar = &trkPB->perigeeParameters();
  }
  else if (fromPerigeeRescaled == extrapFrom) {
    // From rescaled perigee
    trkPar = getRescaledPerigee(trkPB, cluster);    
  }
  
  if(!trkPar){
    ATH_MSG_ERROR("getMatchAtCalo: Cannot access track parameters"); 
    return StatusCode::FAILURE; 
  }
 
  // Should we flip the sign for deltaPhi? 
  bool flipSign = false; 
  if(trkPar->charge() > 0) flipSign = true; 
   
  // Identifier barrel or endcap to identify correct calo sampling
  int bec = 0;
  if ( xAOD::EgammaHelpers::isBarrel( cluster )  ) { 
    // Barrel
    bec = 0;
  } else {
    // Endcap
    bec = 1;
  }
  
  // Setup array for iteration over calo samplings
  static CaloCell_ID::CaloSample samples[2][4] = 
    { { CaloCell_ID::PreSamplerB,
        CaloCell_ID::EMB1,
        CaloCell_ID::EMB2,
        CaloCell_ID::EMB3 },
      { CaloCell_ID::PreSamplerE,
        CaloCell_ID::EME1,
        CaloCell_ID::EME2,
        CaloCell_ID::EME3 } };
 

  double offset = 0.; 
  // Save impacts for cleanup 
  std::vector<const Trk::TrackParameters*> impacts; 
  impacts.reserve(4); // expect max of 4 
  // Save last impact for the next starting point for extrapolation 
  const Trk::TrackParameters* lastImpact = trkPar; 
  for (unsigned int i = 0; i < doSample.size(); i++) { 
 
      ATH_MSG_DEBUG("getMatchAtCalo: doSample " << i << " "  << doSample[i]); 
 
 
    if (!doSample[i]) continue;  // skip unrequested samples 
    const Trk::TrackParameters* impact =  
      m_extrapolateToCalo->extrapolate ((*lastImpact), samples[bec][i], offset, 
                                        Trk::nonInteracting, direction); 
    if (impact  && impact != lastImpact) { 
      eta[i]      = impact->position().eta(); 
      phi[i]      = impact->position().phi(); 
      deltaEta[i] = cluster->etaBE(i) - impact->position().eta(); 
      deltaPhi[i] = m_phiHelper.diff(cluster->phiBE(i),impact->position().phi()); 
      if(flipSign)  deltaPhi[i] = -deltaPhi[i]; 
      lastImpact = impact; 
      impacts.push_back(impact); 
 
      if (fromPerigeeRescaled == extrapFrom && i == 2 && deltaPhi.size() > 4) { 
        // For rescaled perigee when at sampling 2, save the phi   
        // rotation of the track. This is defined as: ((phi of point at 
        // sampling 2 - phi of point at track vertex/perigee) - phi of 
        // direction of track at perigee) 
 
        Amg::Vector3D atSamp2  (impact->position().x(), impact->position().y(), impact->position().z()); 
        Amg::Vector3D atPerigee(trkPar->position().x(), trkPar->position().y(), trkPar->position().z()); 
        Amg::Vector3D perToSamp2 = atSamp2 - atPerigee; 
        deltaPhi[4] = fabs(m_phiHelper.diff(perToSamp2.phi(), trkPar->momentum().phi())); 
        ATH_MSG_DEBUG("getMatchAtCalo: phi-rot: " << deltaPhi[4]); 
      } 
           
      ATH_MSG_DEBUG("getMatchAtCalo: i, eta, phi, deta, dphi: "  
                    << i << " " << eta[i] << " " << phi[i] << " " 
                    << deltaEta[i] << " " << deltaPhi[i]); 
    } 
  } 
  // cleanup 
  for (unsigned int i = 0; i < impacts.size(); i++) { 
    delete impacts[i]; 
  } 
  // For rescaled perigee, we must delete the trkPar which were 
  // created on the heap 
  if (fromPerigeeRescaled == extrapFrom) { 
    delete trkPar; 
  } 
 
  return StatusCode::SUCCESS; 

}


  /**   get eta, phi at the four calorimeter layers given the
   *    Trk::TrackParameters.  doSample indicates whether or not to
   *    extrapolate to each calo sample. Also return whether the
   *    extrapolation is in barrel or not.
   */
StatusCode
EMExtrapolationTools::getEtaPhiAtCalo (const xAOD::TrackParticle*   trkPB,
                                      Trk::PropDirection            direction,
                                      const std::vector<bool>&      doSample,
                                      std::vector<double>&          eta,
                                      std::vector<double>&          phi,
                                      std::vector<bool>&            isBarrel,
                                      unsigned int                  extrapFrom) const
{
  // Extrapolate track to calo and return the extrapolated eta/phi 

  //   We allow different ways to extrapolate:
  // 
  //     1) from the last measurement point track parameters  
  //     2) from the perigee 
  //     3) from the perigee with the track momentum rescaled by the cluster energy
  //

  // Checks
  //   require input vectors to be of the same size:
  unsigned int vsz = doSample.size();
  if ((vsz != eta.size()) || (vsz != phi.size())) {
    ATH_MSG_ERROR("getMatchAtCalo: input vectors not the same size - doSample: " 
      << doSample.size() << " eta: " << eta.size() 
      << " phi: " << phi.size());
    return StatusCode::FAILURE;
  }
    
  // Get track parameters from either last point or perigee
  const Trk::TrackParameters* trkPar = 0;
  Trk::CurvilinearParameters temp; 

  if (fromLastMeasurement == extrapFrom) {
    // Use existing track parameters from last hit
    int index(-1);
    for(unsigned int i(0); i< trkPB->numberOfParameters() ; ++i ){
      if( xAOD::LastMeasurement == trkPB->parameterPosition(i) ){
        index = i;
        break;
      }
    }
    if(index!=-1){
      temp = trkPB->curvilinearParameters(index);
      trkPar = &temp;
    } else {
      ATH_MSG_WARNING("Track Particle does not contain Last Measurement track parameters");
    }
  }
  else if (fromPerigee == extrapFrom) {
    trkPar = &trkPB->perigeeParameters();
  }
  // return if track parameters not found!
  if(!trkPar) {
    ATH_MSG_ERROR("getMatchAtCalo: Cannot access track parameters");
    return StatusCode::FAILURE;
  }
  
  // Now extrapolate to the samplings.
  static CaloCell_ID::CaloSample samples[2][4] = 
    { { CaloCell_ID::PreSamplerB,
        CaloCell_ID::EMB1,
        CaloCell_ID::EMB2,
        CaloCell_ID::EMB3 },
      { CaloCell_ID::PreSamplerE,
        CaloCell_ID::EME1,
        CaloCell_ID::EME2,
        CaloCell_ID::EME3 } };

  double offset = 0.;
  int    bec    = 0;
  const Trk::TrackParameters* lastImpact = trkPar;
  const Trk::TrackParameters* dummy(0);
  std::vector<const Trk::TrackParameters*> impacts(4, dummy);
  for (unsigned int i = 0; i < doSample.size(); i++) {
    if (!doSample[i]) continue;  // skip unrequested samples

    // Must choose between barrel and endcap
    double trketa  = lastImpact->position().eta();
    double distbar = m_calodepth->deta(samples[0][i], trketa);
    double distec  = m_calodepth->deta(samples[1][i], trketa);
    if(msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << " getEtaPhiAtCalo> sample " << i << " : eta= " 
                                           << trketa << " dist to Barrel =" 
                                           << distbar << " to endcap =" << distec << endreq;
    if (distbar < 0 )           bec = 0;
    else if (distec < 0 )       bec = 1;
    else if ( distbar < distec) bec = 0;
    else                        bec = 1;
    const Trk::TrackParameters* impact = 
      m_extrapolateToCalo->extrapolate ((*lastImpact), samples[bec][i], offset,
                                        Trk::nonInteracting, direction);
    if (impact) {
      eta[i]      = impact->position().eta();
      phi[i]      = impact->position().phi();
      isBarrel[i] = (bec == 0);
      lastImpact  = impact;
      impacts.push_back(impact);
    }
  }
  // cleanup extrapolation generated objects
  for (unsigned int i = 0; i < impacts.size(); i++) {
    delete impacts[i];
  }
  // For rescaled perigee, we must delete the trkPar which were
  // created on the heap
  // Can't pass this condition, since this case wasn't considered above.
  // Comment out to avoid coverity warning.
  //if (fromPerigeeRescaled == extrapFrom) {
  //  delete trkPar;
  //}

  return StatusCode::SUCCESS;
}

// =================================================================
bool EMExtrapolationTools::getEtaPhiAtCalo (const xAOD::Vertex* vertex, 
                                            float *etaAtCalo,
                                            float *phiAtCalo) const
{
  if (!vertex) return false;
  
  // Get the momentum at the vertex
  Amg::Vector3D momentum = m_extrapolateToCalo->getMomentumAtVertex(*vertex);
  
  // Protection against momentum = 0
  if (momentum.mag() < 1e-5)
  {
    ATH_MSG_DEBUG("Intersection failed");
    return false;
  }
  
  // Decide if we extrapolate to barrel or endcap
  const CaloCell_ID::CaloSample sample = fabs(momentum.eta()) < 1.425 ? CaloCell_ID::EMB2 : CaloCell_ID::EME2;
  
  // Do a straight line intersection with the surface at the middle of EM2
  Trk::SurfaceIntersection intAtCalo = m_extrapolateToCalo->getIntersectionInCalo(vertex->position(), momentum, sample);

  if (!intAtCalo.valid)
  {
    ATH_MSG_DEBUG("Intersection failed");
    return false;
  }
  *etaAtCalo = intAtCalo.intersection.eta();
  *phiAtCalo = intAtCalo.intersection.phi();
  return true;
}

// =================================================================
Amg::Vector3D EMExtrapolationTools::getMomentumAtVertex(const xAOD::Vertex& vertex, bool reuse /* = true */) const
{
  return m_extrapolateToCalo->getMomentumAtVertex(vertex, reuse);
}
// =================================================================


const Trk::TrackParameters* 
EMExtrapolationTools::getRescaledPerigee(const xAOD::TrackParticle* trkPB, const xAOD::CaloCluster* cluster) const {
    const Trk::TrackParameters* oldPerigee = &trkPB->perigeeParameters();
    if (!oldPerigee) return 0;
    Amg::Vector3D  mom(oldPerigee->momentum().x(),oldPerigee->momentum().y(),oldPerigee->momentum().z());
    const double mag = mom.mag();
    if (mag == 0 || cluster->e() == 0) {
      // should this be a warning or debug? For now I assume that it should not happen
      ATH_MSG_WARNING("Trying to rescale perigee but cluster e = " << cluster->e() << " and perigee mag = " << mag);
      return 0;
    }
    
    // rescale the momentum
    mom = mom * Eigen::Scaling(cluster->e()/mom.mag());

    // Create new perigee
    double charge = oldPerigee->charge();
    //Construct the photon neutral parameters as neutral perigee

    Amg::Vector3D pos(oldPerigee->position().x(),oldPerigee->position().y(),oldPerigee->position().z()); 
    Trk::PerigeeSurface surface (pos); 
    AmgSymMatrix(5) *covariance = new AmgSymMatrix(5)(*(oldPerigee->covariance()));
    const Trk::TrackParameters* result 
           = surface.createParameters<5,Trk::Charged>(oldPerigee->parameters()[0],
                                                      oldPerigee->parameters()[1],
                                                      mom.phi(),
                                                      mom.theta(),
                                                      charge/mom.mag(),
                                                      covariance);
    return (result);
}

// =================================================================
int EMExtrapolationTools::getTRTsection(const xAOD::TrackParticle* trkPB) const
{
  if (!trkPB)
  {
    ATH_MSG_DEBUG("Null pointer to TrackParticle");
    return 0;
  }
  
  if (!m_trtId || m_guessTRTsection)
  {
    ATH_MSG_DEBUG("Guessing TRT section based on eta: " << trkPB->eta());
    return (trkPB->eta() > 0 ? 1 : -1) * (abs(trkPB->eta()) < 0.6 ? 1 : 2);
  }
  
  const Trk::TrackParameters* trkPar =0;
  if (!m_useTrkTrackForTRT)
  {
    Trk::CurvilinearParameters temp; 
    int index(-1);
    for(unsigned int i(0); i< trkPB->numberOfParameters() ; ++i ){
      if( xAOD::LastMeasurement == trkPB->parameterPosition(i) ){
        index = i;
        break;
      }
    }
    if(index!=-1){
      temp = trkPB->curvilinearParameters(index);
      trkPar = &temp;
    } else {
      ATH_MSG_WARNING("Track Particle does not contain Last Measurement track parameters");
    }
  
    if(!trkPar){
      ATH_MSG_WARNING("Track Particle does not have the requested track parameters");
      return 0;
    }
  }
  else if( trkPB->trackLink().isValid() && trkPB->track() != 0 ) 
  {
    ATH_MSG_DEBUG("Will get TrackParameters from Trk::Track");
    const DataVector<const Trk::TrackStateOnSurface> *trackStates = trkPB->track()->trackStateOnSurfaces();
    if (!trackStates) 
    {
      ATH_MSG_WARNING("NULL pointer to trackStateOnSurfaces");
      return 0;
    }
    
    //Loop over the TrkStateOnSurfaces
    // search last valid TSOS first
    for ( DataVector<const Trk::TrackStateOnSurface>::const_reverse_iterator rItTSoS = trackStates->rbegin(); rItTSoS != trackStates->rend(); ++rItTSoS)
    {
      if ( (*rItTSoS)->type(Trk::TrackStateOnSurface::Measurement) && (*rItTSoS)->trackParameters()!=0 && (*rItTSoS)->measurementOnTrack()!=0 && !dynamic_cast<const Trk::PseudoMeasurementOnTrack*>((*rItTSoS)->measurementOnTrack()))
      {
        trkPar = (*rItTSoS)->trackParameters();
        break;
      }
    }
  }
  else ATH_MSG_WARNING("Track particle without Trk::Track");
  
  if (!trkPar) return 0;
  const Trk::Surface& sf = trkPar->associatedSurface();
  const Identifier tid = sf.associatedDetectorElementIdentifier();
  return m_trtId->barrel_ec(tid);
}

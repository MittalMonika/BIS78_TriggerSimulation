/*
   Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
 */

// INCLUDE HEADER FILES:

#include "EMTrackMatchBuilder.h"

#include "egammaRecEvent/egammaRec.h"
#include "egammaRecEvent/egammaRecContainer.h"


#include "xAODCaloEvent/CaloCluster.h"
#include "xAODTracking/TrackParticle.h" 
#include "xAODTracking/TrackParticleContainer.h" 
#include "xAODEgamma/EgammaxAODHelpers.h"

#include "egammaUtils/CandidateMatchHelpers.h"
#include "CaloUtils/CaloCellList.h"
#include "FourMomUtils/P4Helpers.h"

#include "SGTools/CurrentEventStore.h"
#include "StoreGate/ReadHandle.h"
#include "GaudiKernel/EventContext.h"

#include <cmath>

//  END OF HEADER FILES INCLUDE

EMTrackMatchBuilder::EMTrackMatchBuilder(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent) : 
  AthAlgTool(type, name, parent)
{
  // declare interface
  declareInterface<IEMTrackMatchBuilder>(this);
}

EMTrackMatchBuilder::~EMTrackMatchBuilder() { 
}

StatusCode EMTrackMatchBuilder::initialize()
{
  ATH_MSG_DEBUG("Initializing EMTrackMatchBuilder");
  ATH_CHECK(m_TrackParticlesKey.initialize());
  
  // the extrapolation tool
  ATH_CHECK(m_extrapolationTool.retrieve());

  // set things up for the sorting
  m_sorter = TrackMatchSorter(m_distanceForScore);
  m_deltaEtaWeight = 1.0/m_deltaEtaResolution;
  m_deltaPhiWeight = 1.0/m_deltaPhiResolution;
  m_deltaPhiRescaleWeight = 1.0/m_deltaPhiRescaleResolution;

  return StatusCode::SUCCESS;
}

StatusCode EMTrackMatchBuilder::executeRec(const EventContext& ctx, 
                                           EgammaRecContainer* egammas) const
{

  ATH_MSG_DEBUG("Executing EMTrackMatchBuilder");

  // protection against bad pointers
  if (egammas==0 ) {
    return StatusCode::SUCCESS;
  }
  // retrieve the trackparticle container
  SG::ReadHandle<xAOD::TrackParticleContainer> trackPC(m_TrackParticlesKey, ctx);

  // check is only used for serial running; remove when MT scheduler used
  if( !trackPC.isValid() ) {
    ATH_MSG_ERROR("Couldn't retrieve TrackParticle container with key: " << m_TrackParticlesKey.key());
    return StatusCode::FAILURE;
  }
  //Loop over calling the trackExecute method
  for (egammaRec* eg: *egammas){ 
    // retrieve the cluster
    ATH_CHECK(trackExecute(ctx,eg,trackPC.cptr()));
  }
  return StatusCode::SUCCESS;
}

// ===============================================================
StatusCode EMTrackMatchBuilder::trackExecute(const EventContext& ctx, egammaRec* eg, 
                                             const xAOD::TrackParticleContainer* trackPC) const
{
  if (!eg || !trackPC){
    ATH_MSG_WARNING("trackExecute: NULL pointer to egammaRec or TrackParticleContainer");
    return StatusCode::SUCCESS;
  }
  // retrieve corresponding cluster
  const xAOD::CaloCluster* cluster = eg->caloCluster();
  // check if the cluster is sane
  if (cluster && cluster->e() ==0.0) {
    ATH_MSG_WARNING("trackExecute: cluster energy is 0.0! Ignoring cluster.");
    return StatusCode::SUCCESS;
  }

  // Loop over tracks and fill TrackMatch vector
  std::vector<TrackMatch> trkMatches; 
  xAOD::TrackParticleContainer::const_iterator trkIt = trackPC->begin();
  for (unsigned int trackNumber = 0; trkIt != trackPC->end(); ++trkIt, ++trackNumber) 
  {
    //Avoid TRT alone
    if(xAOD::EgammaHelpers::numberOfSiHits(*trkIt) < 4){
      continue;
    }
    /* 
     * Try with normal directions. 
     * For cosmics allow a retry with inverted direction.
     */
    if (isCandidateMatch(cluster, (*trkIt), false)){
      inBroadWindow(ctx, trkMatches, *cluster, trackNumber, 
                    (**trkIt), Trk::alongMomentum);
    }
    else if (m_isCosmics && isCandidateMatch(cluster, (*trkIt), true)){
      inBroadWindow(ctx, trkMatches, *cluster,  trackNumber, 
                    (**trkIt), Trk::oppositeMomentum);
    }
  }

  if(trkMatches.size()>0){
    //sort the track matches
    std::sort(trkMatches.begin(), trkMatches.end(), m_sorter);
    //set the matching values
    TrackMatch bestTrkMatch=trkMatches.at(0);
    for(int i=0; i<4 ;++i){
      eg->setDeltaEta (i, bestTrkMatch.deltaEta[i]); 
      eg->setDeltaPhi (i, bestTrkMatch.deltaPhi[i]); 
      eg->setDeltaPhiRescaled (i, bestTrkMatch.deltaPhiRescaled[i]); 
    }
    eg->setDeltaPhiLast(bestTrkMatch.deltaPhiLast);

    //set the element Links
    typedef ElementLink<xAOD::TrackParticleContainer> EL;
    std::vector<EL> trackParticleLinks;
    trackParticleLinks.reserve (trkMatches.size());
    std::string key = EL(*trackPC, 0).dataID();
    IProxyDict* sg = SG::CurrentEventStore::store();
    for (const TrackMatch& m : trkMatches) {
      ATH_MSG_DEBUG("Match  dR: "<< m.dR
                    <<" second  dR: "<< m.seconddR
                    <<" hasPix: "<< m.hasPix
                    <<" hitsScore: " << m.hitsScore); 
      if (key.empty())
        trackParticleLinks.emplace_back (*trackPC, m.trackNumber, sg);
      else
        trackParticleLinks.emplace_back (key, m.trackNumber, sg);
    }
    eg->setTrackParticles(trackParticleLinks);
  }

  return StatusCode::SUCCESS;
}

bool
EMTrackMatchBuilder::inBroadWindow(const EventContext& ctx,
                                   std::vector<TrackMatch>& trackMatches,
                                   const xAOD::CaloCluster&   cluster, 
                                   int                        trackNumber,
                                   const xAOD::TrackParticle& trkPB,
                                   const Trk::PropDirection dir) const
{

  IEMExtrapolationTools::TrkExtrapDef extrapFrom = IEMExtrapolationTools::fromPerigee;

  ATH_MSG_DEBUG("inBroadWindow: extrapolation method From (0 Last, 1 perigee , 2 Rescale) " << extrapFrom);

  // Now get the delta eta/phi and eta correction at the calorimeter
  std::vector<double>  eta(4, -999.0);
  std::vector<double>  phi(4, -999.0);
  // final arrays that we will write
  // Save the value of deltaPhiRescale. If we do not use rescaled
  // perigee, we recalculate deltaPhi using rescaled momentum. This
  // will be saved in EMTrackMatch
  double deltaPhiRescale  = -999.;
  double deltaPhiLast     = -999.;
  // temporary arrays
  std::vector<double>  deltaEta(4, -999.0);
  std::vector<double>  deltaPhi(4, -999.0);    
  std::vector<double>  deltaEtaRes(4, -999.0);
  std::vector<double>  deltaPhiRes(4, -999.0); 
  /*
   * Try both from perigee
   * and from perigee Rescale.
   *
   * We need anyhow both to be there at the end.
   */
  if (m_extrapolationTool->getMatchAtCalo (ctx,
                                           &cluster, 
                                           &trkPB, 
                                           dir, 
                                           eta,
                                           phi,
                                           deltaEta, 
                                           deltaPhi, 
                                           extrapFrom).isFailure()) 
  {
    return false;
  }

  IEMExtrapolationTools::TrkExtrapDef extrapFromRes = IEMExtrapolationTools::fromPerigeeRescaled;
  std::vector<double>  etaRes(4, -999.0);
  std::vector<double>  phiRes(4, -999.0);
  if (m_extrapolationTool->getMatchAtCalo (ctx, 
                                           &cluster, 
                                           &trkPB, 
                                           dir, 
                                           etaRes,
                                           phiRes,
                                           deltaEtaRes, 
                                           deltaPhiRes, 
                                           extrapFromRes).isFailure())
  {
    return false;
  }

  deltaPhiRescale = deltaPhiRes[2];
  /*
   * Sanity check for very far away matches 
   * The assumption is when we rescale we should be in the 
   * correct neighborhood for a valid track-cluster pair.
   */
  if(fabs(deltaPhiRes[2]) > m_MaxDeltaPhiRescale){
    ATH_MSG_DEBUG("DeltaPhiRescaled above maximum: " << deltaPhiRes[2] << 
                  " (max: " << m_MaxDeltaPhiRescale << ")" );
    return false;
  }
  /*
   * Try to match : First standard way.
   * If this fails and the cluster Et is larger than the track Pt
   * it might get matched only under the rescaled assumption that
   * should be less sensitive to radiative losses.
   */ 
  if(fabs(deltaEta[2]) < m_narrowDeltaEta &&
     deltaPhi[2] < m_narrowDeltaPhi && 
     deltaPhi[2] > -m_narrowDeltaPhiBrem){
    ATH_MSG_DEBUG("Matched with Perigee") ;
  }
  else if(m_SecondPassRescale && 
          cluster.et() > trkPB.pt() &&
          fabs(deltaEtaRes[2]) < m_narrowDeltaEta &&
          deltaPhiRes[2] < m_narrowDeltaPhiRescale && 
          deltaPhiRes[2] > -m_narrowDeltaPhiRescaleBrem){
    ATH_MSG_DEBUG("Not Perigee but matched with Rescale") ;
  }
  else{
    ATH_MSG_DEBUG("Normal matched Failed deltaPhi/deltaEta " << deltaPhi[2] <<" / "<< deltaEta[2]);
    ATH_MSG_DEBUG("Rescaled matched Failed deltaPhi/deltaEta " << deltaPhiRes[2] <<" / "<< deltaEtaRes[2] );
    return false;
  }

  //Always the deltaPhiLast will be from the last measurement
  IEMExtrapolationTools::TrkExtrapDef extrapFrom1 = IEMExtrapolationTools::fromLastMeasurement;
  std::vector<double>  eta1(4, -999.0);
  std::vector<double>  phi1(4, -999.0);
  std::vector<double>  deltaEta1(4, -999.0);
  std::vector<double>  deltaPhi1(4, -999.0);
  if (m_extrapolationTool->getMatchAtCalo (ctx,
                                           &cluster, 
                                           &trkPB, 
                                           dir, 
                                           eta1,
                                           phi1,
                                           deltaEta1, 
                                           deltaPhi1, 
                                           extrapFrom1).isFailure())
  {
    ATH_MSG_DEBUG("Extrapolation from last measurement failed");
    return false;
  }
  deltaPhiLast = deltaPhi1[2];
  ATH_MSG_DEBUG("Rescale dPhi " << deltaPhiRescale);
  ATH_MSG_DEBUG("dPhi Last measurement " << deltaPhiLast);      
  
  /*
   * Done with extrapolation
   * Lets do the matching logic
   */
  TrackMatch trkmatch;
  //Add the matching variable to the TrackMAtch
  for(int i=0; i<4 ;++i){
    trkmatch.deltaEta[i]=deltaEta.at(i); 
    trkmatch.deltaPhi[i]=deltaPhi.at(i); 
    trkmatch.deltaPhiRescaled[i]=deltaPhiRes.at(i); 
  }  
  trkmatch.deltaPhiLast=deltaPhiLast; 

  //Variables used for the sorting. Note both dPhi's will be used.
  trkmatch.trackNumber=trackNumber;
  if(m_useRescaleMetric){
    trkmatch.dR = sqrt(std::pow(m_deltaEtaWeight*deltaEta[2], 2) +
                       std::pow(m_deltaPhiRescaleWeight*deltaPhiRescale, 2));
    trkmatch.seconddR = sqrt(std::pow(m_deltaEtaWeight*deltaEta[2], 2) +
                             std::pow(m_deltaPhiWeight*deltaPhi[2], 2));
    }
  else{
    trkmatch.dR = sqrt(std::pow(m_deltaEtaWeight*deltaEta[2], 2) +
                       std::pow(m_deltaPhiWeight*deltaPhi[2], 2));
    trkmatch.seconddR = sqrt(std::pow(m_deltaEtaWeight*deltaEta[2], 2) +
                             std::pow(m_deltaPhiRescaleWeight*deltaPhiRescale, 2));
  }
  ATH_MSG_DEBUG(" DR " << trkmatch.dR <<" deltaPhi " 
                << deltaPhi[2] <<" deltaEta "<< deltaEta[2]);   
  /*
   * The first thing to check in 
   * finding the best track match
   * Prefer pixel over SCT only 
   */
  int nPixel=0;
  uint8_t uint8_value=0;
  //Check number of pixel hits
  if (trkPB.summaryValue(uint8_value,  xAOD::numberOfPixelDeadSensors)){
    nPixel+=uint8_value;
  }
  if (trkPB.summaryValue(uint8_value,  xAOD::numberOfPixelHits)){
    nPixel+=uint8_value;
  }
  trkmatch.hasPix = (nPixel > 0);
  /*
   * Seconday score based on hits to be used 
   * for track that are very close
   * to each other at the calo, 
   * pick the longest possible one
   */
  trkmatch.hitsScore=0;
  if(m_useScoring){
    //Check the 2 innermost layers
    int nInnerMost =0;
    if (trkPB.summaryValue(uint8_value,  xAOD::numberOfInnermostPixelLayerHits)){
      nInnerMost+=uint8_value;
    }
    int expectInnermostPixelLayerHit = 0; 
    if (trkPB.summaryValue(uint8_value,  xAOD::expectInnermostPixelLayerHit)){
      expectInnermostPixelLayerHit+=uint8_value;
    }
    int nNextToInnerMost =0;
    if (trkPB.summaryValue(uint8_value,  xAOD::numberOfNextToInnermostPixelLayerHits)){
      nNextToInnerMost+=uint8_value;
    }
    int expectNextToInnermostPixelLayerHit = 0; 
    if (trkPB.summaryValue(uint8_value,  xAOD::expectNextToInnermostPixelLayerHit)){
      expectNextToInnermostPixelLayerHit+=uint8_value;
    }
    //Secondary score , find the longest track possible, 
    //i.e the one with the most inner hists  in the pixel 
    //npixel*5 
    trkmatch.hitsScore+=(nPixel*5);
    //Extra points for NextToInnermost
    if(!expectNextToInnermostPixelLayerHit ||  nNextToInnerMost>0){
      trkmatch.hitsScore+=5;
    }
    //Extra points for Innermost
    if(!expectInnermostPixelLayerHit ||  nInnerMost>0){
      trkmatch.hitsScore+=10;
    }
  }  
  ATH_MSG_DEBUG("hasPix : " <<trkmatch.hasPix <<" hitsScore : " <<trkmatch.hitsScore);

  trackMatches.push_back(trkmatch);
  return true;
}

// =================================================================
bool
EMTrackMatchBuilder::isCandidateMatch(const xAOD::CaloCluster*  cluster,
                                      const xAOD::TrackParticle* track,
                                      bool                      flip) const
{
  // loose cluster-track matching
  if ( !m_useCandidateMatch ) {
    return true;
  }
  ATH_MSG_DEBUG("EMTrackMatch builder , broad window");
 
  //Tracking
  const Trk::Perigee candidatePerigee =track->perigeeParameters();
  //Decide whether to try the opposite direction (cosmics)
  const double trkPhi = (!flip) ? candidatePerigee.parameters()[Trk::phi] : -candidatePerigee.parameters()[Trk::phi];
  const double trkEta = (!flip) ? candidatePerigee.eta() : -candidatePerigee.eta();
  const double r_first=candidatePerigee.position().perp();
  const double z_first=candidatePerigee.position().z();
  
  //Cluster variables
  const double clusterEta=cluster->etaBE(2);
  const bool isEndCap= cluster->inEndcap();
  const double Et= cluster->e()/cosh(trkEta);
  const double clusterPhi=cluster->phiBE(2);

  //Avoid clusters with |eta| > 10 or Et less than 10 MeV
  if(fabs(clusterEta)>10.0 || Et <10){
    return false;
  }
 
  const double etaclus_corrected = CandidateMatchHelpers::CorrectedEta(clusterEta,z_first,isEndCap);
  const double phiRot = CandidateMatchHelpers::PhiROT(Et,trkEta, track->charge(),r_first ,isEndCap)  ;
  const double phiRotTrack = CandidateMatchHelpers::PhiROT(track->pt(),trkEta, track->charge(),r_first ,isEndCap)  ;
    
  const double deltaPhiStd = P4Helpers::deltaPhi(clusterPhi, trkPhi);
  const double trkPhiCorr = P4Helpers::deltaPhi(trkPhi, phiRot);
  const double deltaPhi2 = P4Helpers::deltaPhi(clusterPhi, trkPhiCorr);
  const double trkPhiCorrTrack = P4Helpers::deltaPhi(trkPhi, phiRotTrack);
  const double deltaPhi2Track = P4Helpers::deltaPhi(clusterPhi, trkPhiCorrTrack);
    
  //check eta match . Both metrics need to fail in order to disgard the track
  if ( (fabs(clusterEta - trkEta) > 2.*m_broadDeltaEta) && 
       (fabs( etaclus_corrected- trkEta) > 2.*m_broadDeltaEta)){
    ATH_MSG_DEBUG(" Fails broad window eta match (track eta, cluster eta, cluster eta corrected): ( " 
                  << trkEta << ", " << clusterEta <<", "<<etaclus_corrected<<")" );
    return false;
  }
  //It has to fail all phi metrics in order to be disgarded
  if ( (fabs(deltaPhi2) > 2.*m_broadDeltaPhi) && 
       (fabs(deltaPhi2Track) > 2.*m_broadDeltaPhi) && 
       (fabs(deltaPhiStd) > 2.*m_broadDeltaPhi) ){
    ATH_MSG_DEBUG(" Fails broad window eta match (track eta, cluster eta, cluster eta corrected): ( " 
                  << trkEta << ", " << cluster->etaBE(2) <<", "<<etaclus_corrected<<")" );
    return false;
  }
  //if not false returned we end up here
  return true;
}

bool EMTrackMatchBuilder::TrackMatchSorter::operator()(const EMTrackMatchBuilder::TrackMatch& match1,
						       const EMTrackMatchBuilder::TrackMatch& match2)
{
  if(match1.hasPix != match2.hasPix) {// prefer pixels first
    return match1.hasPix;
  }
  //sqrt(0.025**2)*sqrt(2)/sqrt(12) ~ 0.01
  if(fabs(match1.dR-match2.dR) < m_distance) {

    if(fabs(match1.seconddR-match2.seconddR) > m_distance ){ //Can the second distance separate them?
      return match1.seconddR < match2.seconddR	;
    }
    if((match1.hitsScore!= match2.hitsScore)){ //use the one with more pixel
      return match1.hitsScore>match2.hitsScore;
    }
  }
  //closest DR
  return match1.dR < match2.dR;
}

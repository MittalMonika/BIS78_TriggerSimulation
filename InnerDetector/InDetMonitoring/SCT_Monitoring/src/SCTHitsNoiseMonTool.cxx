/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/**    @file SCTHitsNoiseMonTool.cxx
 *
 *     (merging SCTHitsMonTool.cxx and SCTNoiseMonTool.cxx in one Tool)
 *
 *
 *    @author Sofia Chouridou based on code from Martin White, Luca Fiorini, Shaun Roe, Manuel Diaz & Rob McPherson
 *    'The end of all of our exploring...' (Shaun, March 2008)
 */

#include "SCTHitsNoiseMonTool.h"

#include "SCT_NameFormatter.h"

#include "AthContainers/DataVector.h"
#include "InDetRawData/SCT3_RawData.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetRIO_OnTrack/SiClusterOnTrack.h"
#include "LWHists/TH1F_LW.h"
#include "LWHists/TH2F_LW.h"
#include "LWHists/TProfile_LW.h"
#include "SCT_ConditionsTools/ISCT_ConfigurationConditionsTool.h"
#include "StoreGate/ReadHandle.h"
#include "TrkEventUtils/RoT_Extractor.h"
#include "TrkSpacePoint/SpacePoint.h"
#include "TrkSpacePoint/SpacePointCollection.h"
#include "TrkSpacePoint/SpacePointOverlapCollection.h"
#include "TrkSpacePoint/SpacePointCLASS_DEF.h"
#include "TrkTrack/Track.h"

#include "TMath.h"
#include "TH1F.h"
#include "TProfile2D.h"

#include <algorithm>
#include <fstream> // for writing an xml summary (debugging)
#include <iostream>

using namespace std;
using namespace SCT_Monitoring;
/**
 *  Utility functions to use in this file only
 **/
namespace { // use anonymous namespace to restrict scope to this file, equivalent to 'static' keyword here
  // Possible timing patterns, IXX is 1XX etc.
  enum
  Pattern {
    IXX=0, XIX, XXI
  };

  // string names for the detector parts
  static const string names[N_REGIONS] = {
    "Endcap C", "Barrel", "Endcap A"
  };
  // abbreviations for detector parts
  static const string abbreviations[N_REGIONS] = {
    "ECm", "", "ECp"
  };
  static const string streamDelimiter{"_"};

  // is the timebin in the desired pattern?
  bool
  timeBinInPattern(const int tbin, const Pattern xxx) {
    switch (xxx) {
    case IXX:
      return (tbin > 3);

      break;

    case XIX:
      return ((tbin == 2) or (tbin == 3) or (tbin == 6) or (tbin == 7));

      break;

    case XXI:
      return ((tbin == 1) or (tbin == 3) or (tbin == 5) or (tbin == 7));

      break;

    default:
      // cout<<"A strange timepattern was found; this should never happen"<<endl;
      return false;

      break;
    }
  }
}// end of anonymous namespace

// ====================================================================================================
/** Constructor, calls base class constructor with parameters
 *
 *  several properties are "declared" here, allowing selection
 *  of the filepath for histograms, the first and second plane
 *  numbers to be used, and the timebin.
 */
// ====================================================================================================
SCTHitsNoiseMonTool::SCTHitsNoiseMonTool(const string& type,
                                         const string& name,
                                         const IInterface* parent) :
  ManagedMonitorToolBase(type, name, parent) {
}

// ====================================================================================================
// ====================================================================================================
StatusCode SCTHitsNoiseMonTool::initialize() {
  ATH_CHECK(m_SCTSPContainerName.initialize());
  ATH_CHECK(m_dataObjectName.initialize());
  ATH_CHECK(m_clusContainerKey.initialize());
  ATH_CHECK(m_tracksName.initialize());

  ATH_CHECK(m_SCTDetEleCollKey.initialize());

  // Get the helper:
  ATH_CHECK(detStore()->retrieve(m_pSCTHelper, "SCT_ID"));
  ATH_CHECK(m_ConfigurationTool.retrieve());

  return ManagedMonitorToolBase::initialize();
}

// ====================================================================================================
//                           SCTHitsNoiseMonTool :: bookHistograms
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::bookHistograms() {
  ATH_MSG_DEBUG("initialize being called");
  if (newRunFlag()) {
    m_numberOfEvents = 0;
  }

  const bool doSystem[N_REGIONS] = {
    m_doNegativeEndcap, true, m_doPositiveEndcap
  };
  if (m_doTrackHits and bookGeneralTrackTimeHistos().isFailure()) {
    ATH_MSG_WARNING("Error in bookGeneralTrackTimeHistos()");
  }
  for (unsigned int det{0}; det < N_REGIONS; ++det) {
    if (doSystem[det]) {
      if (m_doTrackHits and bookGeneralTrackHits(det).isFailure()) {
        ATH_MSG_WARNING("Error in bookGeneralTrackHits()");
      }
      if (bookGeneralNoiseOccupancyMaps(det).isFailure()) {
        ATH_MSG_WARNING("Error in bookGeneralNoiseOccupancyMaps()");
      }
      if (bookGeneralHitOccupancyMaps(det).isFailure()) {
        ATH_MSG_WARNING("Error in bookGeneralHitOccupancyMaps()");
      }
      if (bookGeneralHits(det).isFailure()) {
        ATH_MSG_WARNING("Error in bookGeneralHits()");
      }
    }
  }

  if (bookNoiseDistributions().isFailure()) {
    ATH_MSG_WARNING("Error in bookNoiseDistributions()");
  }

  if (m_boolhitmaps) {
    if (book1DHitHists().isFailure()) {
      ATH_MSG_WARNING("Error in barrel book1DHitHists()");
    }
    // Only want these histograms when running the code offline
  }

  if (bookClusterSize().isFailure()) {
    ATH_MSG_WARNING("Error in bookClusterSize()");
  }

  if (newRunFlag()) {
    // book hits histogram
    MonGroup clu{this, "SCT/GENERAL/hits/summary", ManagedMonitorToolBase::run, ATTRIB_UNMANAGED};
    m_ncluHisto = new TH1F{"sct_hits", "Total SCT Hits", N_HIT_BINS * 4, FIRST_HIT_BIN, LAST_HIT_BIN * 150.};
    m_ncluHisto->GetXaxis()->SetTitle("Total SCT Hits");
    m_ncluHisto->GetYaxis()->SetTitle("Entries");
    if (clu.regHist(m_ncluHisto).isFailure()) {
      ATH_MSG_WARNING("Cannot book Histogram: SCT/GENERAL/hits/summary/sct_hits");
    }
    if (initializeNoiseMaps().isFailure()) {
      ATH_MSG_WARNING("Failed to initalize noise occupancy maps!");
    }
  }

  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                           SCTHitsNoiseMonTool :: bookHistograms
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::bookHistogramsRecurrent() {
  ATH_MSG_DEBUG("bookHistogramsRecurrent being called");
  m_numberOfEvents = 0;

  const bool doSystem[N_REGIONS] = {
    m_doNegativeEndcap, true, m_doPositiveEndcap
  }; 
  if (m_doTrackHits and bookGeneralTrackTimeHistos().isFailure()) {
    ATH_MSG_WARNING("Error in bookGeneralTrackTimeHistos()");
  }
  for (unsigned int det{0}; det < N_REGIONS; ++det) {
    if (doSystem[det]) {
      if (m_doTrackHits and bookGeneralTrackHits(det).isFailure()) {
        ATH_MSG_WARNING("Error in bookGeneralTrackHits()");
      }
      if (bookGeneralNoiseOccupancyMaps(det).isFailure()) {
        ATH_MSG_WARNING("Error in bookGeneralNoiseOccupancyMaps()");
      }
      if (bookGeneralHitOccupancyMaps(det).isFailure()) {
        ATH_MSG_WARNING("Error in bookGeneralHitOccupancyMaps()");
      }
      if (bookGeneralHits(det).isFailure()) {
        ATH_MSG_WARNING("Error in bookGeneralHits()");
      }
    }
  }

  if (bookNoiseDistributions().isFailure()) {
    ATH_MSG_WARNING("Error in bookNoiseDistributions()");
  }

  if (m_boolhitmaps) {
    if (book1DHitHists().isFailure()) {
      ATH_MSG_WARNING("Error in barrel book1DHitHists()");
    }
    // Only want these histograms when running the code offline
  }

  if (bookClusterSize().isFailure()) {
    ATH_MSG_WARNING("Error in bookClusterSize()");
  }

  // book hits histogram
  MonGroup clu{this, "SCT/GENERAL/hits/summary", ManagedMonitorToolBase::run, ATTRIB_UNMANAGED};
  m_ncluHisto = new TH1F("sct_hits", "Total SCT Hits", N_HIT_BINS * 4, FIRST_HIT_BIN, LAST_HIT_BIN * 150);
  m_ncluHisto->GetXaxis()->SetTitle("Event Number");
  m_ncluHisto->GetYaxis()->SetTitle("Num of SCT Hits");
  if (clu.regHist(m_ncluHisto).isFailure()) {
    ATH_MSG_WARNING("Cannot book Histogram: SCT/GENERAL/hits/summary/sct_hits");
  }
  if (initializeNoiseMaps().isFailure()) {
    ATH_MSG_WARNING("Failed to initalize noise occupancy maps!");
  }

  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                       SCTHitsNoiseMonTool :: fillHistograms
// This is the real workhorse, called for each event. It retrieves the data each time
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::fillHistograms() {
  ++m_numberOfEvents;
  ++m_numberOfEventsRecent;
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const EventIDBase& pEvent{ctx.eventID()};
  int tmp_lb{static_cast<int>(pEvent.lumi_block())};
  if ((tmp_lb > m_current_lb) and (m_current_lb<=NBINS_LBs)) {
    for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
      m_occ_lb[jReg][m_current_lb] = 0;
      m_occTrigger_lb[jReg][m_current_lb] = 0;
      m_hitocc_lb[jReg][m_current_lb] = 0;
      m_hitoccTrigger_lb[jReg][m_current_lb] = 0;
    }
    int nlinks[N_REGIONS_INC_GENERAL]{0, 0, 0, 0};
    SCT_ID::const_id_iterator planeIterator{m_pSCTHelper->wafer_begin()};
    SCT_ID::const_id_iterator planeEnd{m_pSCTHelper->wafer_end()};
    for (; planeIterator not_eq planeEnd; ++planeIterator) {
      Identifier planeId{*planeIterator};
      const int bec{m_pSCTHelper->barrel_ec(planeId)};
      const unsigned int systemIndex{bec2Index(bec)};
      // Don't initialize a value for disabled  modules
      if (not m_ConfigurationTool->isGood(*planeIterator, InDetConditions::SCT_SIDE)) {
        continue;
      }
      if (m_events_lb > 0) {
        for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
          m_occ_lb[jReg][m_current_lb] += m_occSumUnbiased_lb[jReg][*planeIterator] / m_events_lb;
          m_hitocc_lb[jReg][m_current_lb] += m_hitoccSumUnbiased_lb[jReg][*planeIterator] / m_events_lb;
        }
      }
      if (m_eventsTrigger_lb > 0) {
        for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
          m_occTrigger_lb[jReg][m_current_lb] += (1E5) * m_occSumUnbiasedTrigger_lb[jReg][*planeIterator] / m_eventsTrigger_lb;
          m_hitoccTrigger_lb[jReg][m_current_lb] += (1E5) * m_hitoccSumUnbiasedTrigger_lb[jReg][*planeIterator] / m_eventsTrigger_lb;
        }
      }
      for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
        m_occSumUnbiased_lb[jReg][*planeIterator] = 0;
        m_occSumUnbiasedTrigger_lb[jReg][*planeIterator] = 0;
        m_hitoccSumUnbiased_lb[jReg][*planeIterator] = 0;
        m_hitoccSumUnbiasedTrigger_lb[jReg][*planeIterator] = 0;
      }
      nlinks[systemIndex]++;
      nlinks[GENERAL_INDEX]++;
    }
    for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
      if (nlinks[jReg]>0) {
        m_occ_lb[jReg][m_current_lb] /= nlinks[jReg];
        m_occTrigger_lb[jReg][m_current_lb] /= nlinks[jReg];
        m_hitocc_lb[jReg][m_current_lb] /= nlinks[jReg];
        m_hitoccTrigger_lb[jReg][m_current_lb] /= nlinks[jReg];
      }
    }
    m_events_lb = 0;
    m_eventsTrigger_lb = 0;
  }
  m_current_lb = pEvent.lumi_block();
  // If track hits are selected, make the vector of track rdo identifiers
  if (m_doTrackHits) {
    if (makeVectorOfTrackRDOIdentifiers().isFailure()) {
      ATH_MSG_WARNING("Couldn't make vector of track RDO identifiers");
    }
  }
  if (generalHistsandNoise().isFailure()) {
    ATH_MSG_WARNING("Error in generalHists");
  }

  if (m_environment == AthenaMonManager::online) {
    if ((m_numberOfEvents % m_checkrate) == 0) {
      if (checkNoiseMaps().isFailure()) {
        ATH_MSG_WARNING("Error in checkNoiseMaps()");
      }
    }
    if ((m_current_lb % m_checkrecent == 0) and (m_current_lb > m_last_reset_lb)) {
      if (checkNoiseMaps().isFailure()) {
        ATH_MSG_WARNING("Error in checkNoiseMaps()");
      }
      if (resetNoiseMapsRecent().isFailure()) {
        ATH_MSG_WARNING("Error in resetNoiseMapsRecent");
      }
      if (resetHitMapHists().isFailure()) {
        ATH_MSG_WARNING("Error in resetHitMapsRecent");
      }
      m_last_reset_lb = m_current_lb;
      m_numberOfEventsRecent = 0;
    }
  }

  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                      SCTHitsNoiseMonTool :: procHistograms
// PJB 08/08/2005
// Here we have the statistics by plane which can be written to the
// overall statistics object (the map) and then sent to the DB
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::procHistograms() {
  bool endOfEventsBlock{endOfLumiBlockFlag()};
  if (endOfEventsBlock) {
    if (checkNoiseMaps().isFailure()) {
      ATH_MSG_WARNING("Error in checkNoiseMaps()");
    }
  }    
  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                                   SCTHitsNoiseMonTool :: checkHists
// DD 08/10/2010
// Inherited method from base class that gets called by monitoring finalize after procHists - not used
// at this time. Previously was and unnecessary middle function between procHistograms and
// checkNoiseMaps which caused checkNoiseMaps to be called twice float-filling NO histograms
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::checkHists(bool /*fromFinalize*/) {
  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                           SCTHitsNoiseMonTool :: generalHistsandNoise
//
// This method adds the spBasedNoise algorithm to the main RDO loop in an attempt to speed things up
// Once this works, it'll be the default- the old methods will be left as backups because we know they definitely work!
// This code was added by Martin White in June 2008
// Note that this is the only function with the online IPC functionality implemented- if you revert to the old methods
// you'll lose it
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::generalHistsandNoise() {
  SG::ReadHandle<SCT_RDO_Container> p_rdocontainer{m_dataObjectName};
  if (not p_rdocontainer.isValid()) {
    return StatusCode::FAILURE;
  }
  // Get the space point container
  SG::ReadHandle<SpacePointContainer> sctContainer{m_SCTSPContainerName};
  if (not sctContainer.isValid()) {
    return StatusCode::FAILURE;
  }
  Identifier SCT_Identifier;
  // Use new IDC
  int local_tothits{0};
  int totalmodules{0};
  int maxrodhits{0};

  bool isSelectedTrigger{false};
  // EDAVIES - have now changed back to using L1_RD0_EMPTY
  if (m_doTrigger) {
    if (AthenaMonManager::dataType() != AthenaMonManager::cosmics) {
      if (m_trigDecTool->isPassed(m_NOTriggerItem)) {
        isSelectedTrigger = true;
        ++m_numberOfEventsTrigger;
      }
    }
  }
  m_events_lb++;
  if (isSelectedTrigger) {
    m_eventsTrigger_lb++;
  }

  vector<int> barrel_local_nhitslayer(N_BARRELSx2, 0);
  vector<int> ECp_local_nhitslayer(N_DISKSx2, 0);
  vector<int> ECm_local_nhitslayer(N_DISKSx2, 0);
  vector<int>* hitsInLayer[N_REGIONS] = {
    &ECm_local_nhitslayer, &barrel_local_nhitslayer, &ECp_local_nhitslayer
  };
  const bool doThisSubsystem[N_REGIONS] = {
    m_doNegativeEndcap, true, m_doPositiveEndcap
  };
  // Outer Loop on RDO Collection
  for (const InDetRawDataCollection<SCT_RDORawData>* SCT_Collection: *p_rdocontainer) {
    if (SCT_Collection==nullptr) {
      continue;  // select only SCT RDOs
    }
    // MJW new code- try getting the ID of the collection using the identify() method
    Identifier tempID{SCT_Collection->identify()};
    Identifier theWaferIdentifierOfTheRDOCollection{tempID};
    Identifier theModuleIdentifierOfTheRDOCollection{m_pSCTHelper->module_id(tempID)};
    IdentifierHash theModuleHash0{m_pSCTHelper->wafer_hash(theModuleIdentifierOfTheRDOCollection)};
    IdentifierHash theModuleHash1;
    m_pSCTHelper->get_other_side(theModuleHash0, theModuleHash1);
    int Bec{m_pSCTHelper->barrel_ec(tempID)};
    
    int numberOfHitsFromSPs{0};
    int numberOfHitsFromAllRDOs{0};
    // Now we want the space point container for this module
    // We have to compare module IDs- the SP collection is defined for the 'normal' (i.e. no stereo) module side
    // Define a set of spIDs
    set<Identifier> mySetOfSPIds;
    for (unsigned int side{0}; side<N_SIDES; side++) {
      SpacePointContainer::const_iterator spContainerIterator{sctContainer->indexFind(side==0 ? theModuleHash0 : theModuleHash1)};
      if (spContainerIterator==sctContainer->end()) continue;
      Identifier tempSPID{(*spContainerIterator)->identify()};
      Identifier theModuleIdentifierOfTheSPCollection{m_pSCTHelper->module_id(tempSPID)};
      if (theModuleIdentifierOfTheRDOCollection == theModuleIdentifierOfTheSPCollection) {
        for (const Trk::SpacePoint* sp: **spContainerIterator) {
          // the following is nasty; the 'normal' sides (where the sp is defined) swap from layer to layer. To be safe,
          // we get both sides
          const vector<Identifier>& rdoList0{sp->clusterList().first->rdoList()};
          const vector<Identifier>& rdoList1{sp->clusterList().second->rdoList()};
          // copy to mySetOfSPIds. Use inserter(set, iterator_hint) for a set, or back_inserter(vec) for vector...
          copy(rdoList0.begin(), rdoList0.end(), inserter(mySetOfSPIds, mySetOfSPIds.end()));
          copy(rdoList1.begin(), rdoList1.end(), inserter(mySetOfSPIds, mySetOfSPIds.end()));
        }
      } else {
        ATH_MSG_ERROR("Module identifiers are different. indexFind gives a wrong collection??");
      }
    }
    totalmodules++;
    maxrodhits = 0;
    // Now we loop over the RDOs in the RDO collection, and add to the NO vector any that are in the mySetOfSPIds
    for (const SCT_RDORawData* rdo: *SCT_Collection) {
      const SCT3_RawData* rdo3{dynamic_cast<const SCT3_RawData*>(rdo)};
      int tbin{3};
      if (rdo3) {
        tbin = rdo3->getTimeBin();
      }
      SCT_Identifier = rdo->identify();
      const int firstStrip{m_pSCTHelper->strip(SCT_Identifier)};
      const int numberOfStrips{rdo->getGroupSize()};
      int thisBec{m_pSCTHelper->barrel_ec(SCT_Identifier)};
      int thisLayerDisk{m_pSCTHelper->layer_disk(SCT_Identifier)};
      int thisPhi{m_pSCTHelper->phi_module(SCT_Identifier)};
      int thisEta{m_pSCTHelper->eta_module(SCT_Identifier)};
      int thisSide{m_pSCTHelper->side(SCT_Identifier)};
      int thisElement{(N_SIDES * thisLayerDisk) + thisSide};
      // CAM adds mod id needed for map
      Identifier wafer_id{m_pSCTHelper->wafer_id(SCT_Identifier)};
      int chan{firstStrip};
      int limit{chan + numberOfStrips};
      unsigned int systemIndex{bec2Index(thisBec)};
      // CAM wafer id was here
      unsigned int module_hash{m_pSCTHelper->wafer_hash(wafer_id)};
      //
      if (doThisSubsystem[systemIndex]) {
        TH2F_LW* histogram{m_phitsHistoVector[systemIndex][thisElement]};
        TH2F_LW* histogram_recent{nullptr};
        if (m_environment == AthenaMonManager::online) {
          histogram_recent = m_phitsHistoVectorRecent[systemIndex][thisElement];
        }
        if (find(m_RDOsOnTracks.begin(), m_RDOsOnTracks.end(), SCT_Identifier) != m_RDOsOnTracks.end()) {
          for (int ichan{chan}; ichan < limit; ++ichan) {
            m_ptrackhitsHistoVector[systemIndex][thisElement]->Fill(thisEta, thisPhi);
            if (m_environment == AthenaMonManager::online) {
              m_ptrackhitsHistoVectorRecent[systemIndex][thisElement]->Fill(thisEta, thisPhi);
            }
            // Fill the track hits histogram
            double dTbin{static_cast<double>((tbin == 2) or (tbin == 3))};
            m_tbinfracall->Fill(thisBec+0.5, dTbin);
          }
        }
        histogram->Fill(thisEta, thisPhi, numberOfStrips);
        if (m_environment == AthenaMonManager::online) {
          histogram_recent->Fill(thisEta, thisPhi, numberOfStrips);
        }
        numberOfHitsFromAllRDOs += numberOfStrips;
        // Record number of hits in space points if timebin filtering is on hits not in bin X1X are counted as in space
        // points
        if (mySetOfSPIds.find(SCT_Identifier) != mySetOfSPIds.end()) {
          numberOfHitsFromSPs += numberOfStrips;
        } else if (m_doTimeBinFilteringForNoise and (not timeBinInPattern(tbin, XIX))) {
          numberOfHitsFromSPs += numberOfStrips;
        }
      }
      (*hitsInLayer[systemIndex])[thisElement] += numberOfStrips;
      local_tothits += numberOfStrips;
      maxrodhits += numberOfStrips;

      if (m_boolhitmaps) {
        for (int ichan{chan}; ichan < limit; ++ichan) {
          (m_phitmapHistoVector[module_hash])->Fill(ichan, 1.0);  // increment channel hit for this plane
        }
      }
    } // End of Loop on SCT_Collection, so end of loop over the RDOs in the RDO container
      // We can now do the NO calculation for this wafer
      // For the Time Dependent plots

    if (numberOfHitsFromAllRDOs > 0) {
      unsigned int systemIndex{bec2Index(Bec)};
      int diff{numberOfHitsFromAllRDOs - numberOfHitsFromSPs};
      int num{diff};
      int den{N_STRIPS - numberOfHitsFromSPs};
      float sumocc{0.};
      if (diff < 0) {
        num = 0;
        ATH_MSG_WARNING("Too many reconstructed space points for number of real hits");
      }
      if (den > 0) {
        sumocc = num / static_cast<float> (den);
        m_occSumUnbiased[theWaferIdentifierOfTheRDOCollection] += sumocc;
        if (m_environment == AthenaMonManager::online) {
          m_occSumUnbiasedRecent[theWaferIdentifierOfTheRDOCollection] += sumocc;
        }
        if (isSelectedTrigger) {
          m_occSumUnbiasedTrigger[theWaferIdentifierOfTheRDOCollection] += sumocc;
        }
        m_occSumUnbiased_lb[systemIndex][theWaferIdentifierOfTheRDOCollection] += sumocc;
        m_occSumUnbiased_lb[GENERAL_INDEX][theWaferIdentifierOfTheRDOCollection] += sumocc;
        if (isSelectedTrigger) {
          m_occSumUnbiasedTrigger_lb[systemIndex][theWaferIdentifierOfTheRDOCollection] += sumocc;
          m_occSumUnbiasedTrigger_lb[GENERAL_INDEX][theWaferIdentifierOfTheRDOCollection] += sumocc;
        }
      }

      { // hit occupancy
        float sumhitocc{static_cast<float> (numberOfHitsFromAllRDOs) / static_cast<float> (N_STRIPS)};
        m_hitoccSumUnbiased[theWaferIdentifierOfTheRDOCollection] += sumhitocc;
        if (m_environment == AthenaMonManager::online) {
          m_hitoccSumUnbiasedRecent[theWaferIdentifierOfTheRDOCollection] += sumhitocc;
        }
        if (isSelectedTrigger) {
          m_hitoccSumUnbiasedTrigger[theWaferIdentifierOfTheRDOCollection] += sumhitocc;
        }
        m_HallHits_vsLB[systemIndex]->Fill(m_current_lb, numberOfHitsFromAllRDOs);
        m_HSPHits_vsLB[systemIndex]->Fill(m_current_lb, numberOfHitsFromSPs);
        m_hitoccSumUnbiased_lb[systemIndex][theWaferIdentifierOfTheRDOCollection] += sumhitocc;
        m_hitoccSumUnbiased_lb[GENERAL_INDEX][theWaferIdentifierOfTheRDOCollection] += sumhitocc;
        if (isSelectedTrigger) {
          m_HallHitsTrigger_vsLB[systemIndex]->Fill(m_current_lb, numberOfHitsFromAllRDOs);
          m_HSPHitsTrigger_vsLB[systemIndex]->Fill(m_current_lb, numberOfHitsFromSPs);
          m_hitoccSumUnbiasedTrigger_lb[systemIndex][theWaferIdentifierOfTheRDOCollection] += sumhitocc;
          m_hitoccSumUnbiasedTrigger_lb[GENERAL_INDEX][theWaferIdentifierOfTheRDOCollection] += sumhitocc;
        }
      }// end of hit occupancy*/
    }
  }// End of Loop on RDO container


  m_ncluHisto->Fill(local_tothits, 1);

  if (m_boolhitmaps) {
    for (unsigned int iReg{0}; iReg<N_REGIONS; iReg++) {
      if ((iReg==BARREL_INDEX) or
          ((iReg==ENDCAP_A_INDEX) and m_doPositiveEndcap) or
          ((iReg==ENDCAP_C_INDEX) and m_doNegativeEndcap)) {
        for (int i{0}; i < ((iReg==BARREL_INDEX) ? N_BARRELSx2 : N_DISKSx2); ++i) {
          m_ncluHistoVector[iReg][i]->Fill(barrel_local_nhitslayer[i], 1.);
        }
      }
    }
  }

  // Fill Cluster size histogram
  SG::ReadHandle<InDet::SCT_ClusterContainer> p_clucontainer{m_clusContainerKey};
  if (not p_clucontainer.isValid()) {
    ATH_MSG_WARNING("Couldn't retrieve clusters");
  }
  for (const InDet::SCT_ClusterCollection* SCT_Collection: *p_clucontainer) {
    for (const InDet::SCT_Cluster* cluster: *SCT_Collection) {
      Identifier cluId{cluster->identify()};
      long unsigned int GroupSize{cluster->rdoList().size()};
      // Fill  Cluster Size histogram
      int elementIndex{N_SIDES * m_pSCTHelper->layer_disk(cluId) + m_pSCTHelper->side(cluId)};
      int thisPhi{m_pSCTHelper->phi_module(cluId)};
      int thisEta{m_pSCTHelper->eta_module(cluId)};
      int thisBec{m_pSCTHelper->barrel_ec(cluId)};
      unsigned int systemIndex{bec2Index(thisBec)};
      m_clusize->Fill(GroupSize, 1.);

      if ((thisBec==BARREL) or
          ((thisBec==ENDCAP_A) and m_doPositiveEndcap) or
          ((thisBec==ENDCAP_C) and m_doNegativeEndcap)) {
        m_clusizedist[systemIndex][elementIndex]->Fill(thisEta, thisPhi, GroupSize);
      }

      if (m_environment == AthenaMonManager::online) {
        m_clusizeRecent->Fill(GroupSize, 1.);
      }
    }
  }
  // } // Uncomment this line as well to turn off cluster hists in online

  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                          SCTHitsNoiseMonTool :: book1DHitHists
// Book HitMaps histograms for all present SCT modules
// using ToolRootHistSvc()
// The histograms go to the subdirectory ../hitmaps/
// to improve readability
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::book1DHitHists() {
  if (newRunFlag()) {
    m_phitmapHistoVector.clear();
    MonGroup HitHists[N_REGIONS]{{this, "SCT/SCTEC/hitmaps", ManagedMonitorToolBase::run, ATTRIB_UNMANAGED},
                                 {this, "SCT/SCTB/hitmaps",  ManagedMonitorToolBase::run, ATTRIB_UNMANAGED},
                                 {this, "SCT/SCTEA/hitmaps", ManagedMonitorToolBase::run, ATTRIB_UNMANAGED}};
    //
    SCT_ID::const_id_iterator planeIterator{m_pSCTHelper->wafer_begin()};
    SCT_ID::const_id_iterator planeEnd{m_pSCTHelper->wafer_end()};
    for (; planeIterator not_eq planeEnd; ++planeIterator) {
      Identifier planeId{*planeIterator};
      const int bec{m_pSCTHelper->barrel_ec(planeId)};
      const unsigned int systemIndex{bec2Index(bec)};
      const string formattedPosition{positionString(*planeIterator)};
      string histotitle{string("SCT ") + names[systemIndex] + " Hitmap: plane " + formattedPosition};
      h1Factory(formattedPosition, histotitle, HitHists[systemIndex], m_phitmapHistoVector,
                FIRST_STRIP - 0.5, LAST_STRIP + 0.5, N_BINS);
      if (bec == BARREL) ATH_MSG_DEBUG("Have registered the barrel hists");
      ATH_MSG_DEBUG("Have registered the hists");
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode
SCTHitsNoiseMonTool::bookGeneralHits(const unsigned int systemIndex) {
  const Bec bec{index2Bec(systemIndex)};

  if (bec == INVALID_SYSTEM) {
    ATH_MSG_FATAL("Invalid subsystem index, should be 0-2, was " << systemIndex);
    return StatusCode::FAILURE;
  }
  static const string paths[N_REGIONS] = {
    "SCT/SCTEC/hits", "SCT/SCTB/hits", "SCT/SCTEA/hits"
  };
  static const unsigned int limits[N_REGIONS] = {
    N_DISKSx2, N_BARRELSx2, N_DISKSx2
  };
  static const string titles[N_REGIONS]{"Endcap C", "barrel", "Endcap A"};
  //
  if (newRunFlag()) {
    m_phitsHistoVector[systemIndex].clear();
    m_phitsHistoVectorRecent[systemIndex].clear();
    m_ncluHistoVector[systemIndex].clear();
    MonGroup hitHists{this, paths[systemIndex], run, ATTRIB_UNMANAGED};
    for (unsigned int i{0}; i < limits[systemIndex]; ++i) {
      LayerSideFormatter layerSide{i, systemIndex};
      string streamhitmap{"hitsmap" + abbreviations[systemIndex] + "_" + layerSide.name()};
      string streamhitmaprecent{"hitsmaprecent" + abbreviations[systemIndex] + "_" + layerSide.name()};
      string streamhits{"hits" + abbreviations[systemIndex] + "_" + layerSide.name()};
      string histotitle{"SCT Hitmap for " + names[systemIndex] + ": " + layerSide.title()};
      string histotitlerecent{"SCT Hitmap from recent events for " + names[systemIndex] + ": " +
          layerSide.title()};
      h2Factory(streamhitmap, histotitle, bec, hitHists, m_phitsHistoVector[systemIndex]);
      if (m_environment == AthenaMonManager::online) {
        h2Factory(streamhitmaprecent, histotitlerecent, bec, hitHists, m_phitsHistoVectorRecent[systemIndex]);
      }
      histotitle = "SCT Hits for " + names[systemIndex] + ": " + layerSide.title();
      if (m_boolhitmaps) {
        h1Factory(streamhits, histotitle, hitHists, m_ncluHistoVector[systemIndex], FIRST_HIT_BIN, LAST_HIT_BIN,
                  N_HIT_BINS);
      }
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode
SCTHitsNoiseMonTool::bookClusterSize() {
  MonGroup clusizebGroup{this, "SCT/SCTB/hits", run, ATTRIB_UNMANAGED};
  MonGroup clusizeeaGroup{this, "SCT/SCTEA/hits", run, ATTRIB_UNMANAGED};
  MonGroup clusizeecGroup{this, "SCT/SCTEC/hits", run, ATTRIB_UNMANAGED};
  string disksidenameB[N_BARRELSx2] = {
    "0_0", "0_1", "1_0", "1_1", "2_0", "2_1", "3_0", "3_1"
  };
  string disksidenameECp[N_DISKSx2] = {
    "0_0", "0_1", "1_0", "1_1", "2_0", "2_1", "3_0", "3_1", "4_0", "4_1", "5_0", "5_1", "6_0", "6_1", "7_0", "7_1",
    "8_0", "8_1"
  };
  string disksidenameECm[N_DISKSx2] = {
    "0_0", "0_1", "1_0", "1_1", "2_0", "2_1", "3_0", "3_1", "4_0", "4_1", "5_0", "5_1", "6_0", "6_1", "7_0", "7_1",
    "8_0", "8_1"
  };

  for (int i{0}; i < N_BARRELSx2; i++) {
    m_clusizedist[BARREL_INDEX][i] = prof2DFactory("clusize_dist_B_" + disksidenameB[i], "cluster size in Barrel_" + disksidenameB[i],
                                                   clusizebGroup, 13, -6.5, 6.5, 56, -0.5, 55.5);
  }
  for (int i{0}; i < N_DISKSx2; i++) {
    m_clusizedist[ENDCAP_A_INDEX][i] = prof2DFactory("clusize_dist_EA_" + disksidenameECp[i],
                                                     "cluster size in EndcapA_" + disksidenameECp[i], clusizeeaGroup, 3, -0.5, 2.5,
                                                     52, -0.5, 51.5);
    m_clusizedist[ENDCAP_C_INDEX][i] = prof2DFactory("clusize_dist_EC_" + disksidenameECm[i],
                                                     "cluster size in EndcapC_" + disksidenameECm[i], clusizeecGroup, 3, -0.5, 2.5,
                                                     52, -0.5, 51.5);
  }
  if (newRunFlag()) {
    MonGroup BarrelCluSize{this, "SCT/GENERAL/hits", run, ATTRIB_UNMANAGED};
    // book Cluster width histogram for all SCT Detector
    m_clusize = h1Factory("clu_size", "SCT Cluster size", BarrelCluSize, 0., 200., 200);
    m_clusize->GetXaxis()->SetTitle("Cluster Size");
    m_clusize->GetYaxis()->SetTitle("Num of Events");

    if (m_environment == AthenaMonManager::online) {
      m_clusizeRecent = h1Factory("clu_size_recent", "SCT Cluster size from recent events", BarrelCluSize, 0., 200., 200);
      m_clusizeRecent->GetXaxis()->SetTitle("Cluster Size");
      m_clusizeRecent->GetYaxis()->SetTitle("Num of Events");
    }
  }
  return m_clusize ? (StatusCode::SUCCESS) : (StatusCode::FAILURE);
}

StatusCode
SCTHitsNoiseMonTool::bookGeneralNoiseOccupancyMaps(const unsigned int systemIndex) { 
  static const string paths[N_REGIONS] = {
    "SCT/SCTEC/Noise", "SCT/SCTB/Noise", "SCT/SCTEA/Noise"
  };
  static const unsigned int limits[N_REGIONS] = {
    N_DISKSx2, N_BARRELSx2, N_DISKSx2
  };

 const Bec bec{index2Bec(systemIndex)};

  if (bec == INVALID_SYSTEM) {
    ATH_MSG_FATAL("Invalid subsystem index, should be 0-2, was " << systemIndex);
    return StatusCode::FAILURE;
  }
  if (newRunFlag()) {
    MonGroup noiseOccMaps{this, paths[systemIndex], run, ATTRIB_UNMANAGED};
    m_pnoiseoccupancymapHistoVectorTrigger[systemIndex].clear();
    m_pnoiseoccupancymapHistoVectorRecent[systemIndex].clear();
    // book 2D "noise" maps, containing hits that aren't associated to tracks
    for (unsigned int i{0}; i < limits[systemIndex]; ++i) {
      LayerSideFormatter layerSide{i, systemIndex};
      const string streamhitmaptrigger{"noiseoccupancymaptrigger" + abbreviations[systemIndex] + "_" +
          layerSide.name()};
      const string streamhitmaprecent{"noiseoccupancymaprecent" + abbreviations[systemIndex] + "_" + layerSide.name()};
      string histotitle{"SCT Noise Occupancy map for " + names[systemIndex] + ": " + layerSide.title()};
      string histotitletrigger{"SCT Noise Occupancy map for " + m_NOTriggerItem + " Trigger and " +
          names[systemIndex] + ": " + layerSide.title()};
      string histotitlerecent{"SCT Noise Occupancy map in recent events for " + names[systemIndex] + ": " +
          layerSide.title()};
      prof2Factory(streamhitmaptrigger, histotitletrigger, bec, noiseOccMaps, m_pnoiseoccupancymapHistoVectorTrigger[systemIndex]);
      if (m_environment == AthenaMonManager::online) {
        prof2Factory(streamhitmaprecent, histotitlerecent, bec, noiseOccMaps, m_pnoiseoccupancymapHistoVectorRecent[systemIndex]);
      }
    }

    string histNames[N_REGIONS]{"ECm", "BAR", "ECp"};
    string histTitles[N_REGIONS]{"ECm", "Barrel", "ECp"};

    string histNamesNO[N_REGIONS]{"ECC", "BAR", "ECA"};
    string histTitlesNO[N_REGIONS]{"EndCap C", "Barrel", "EndCap A"};
    m_NO_vsLB[systemIndex] = TProfile_LW::create((histNamesNO[systemIndex]+"NO_vsLB").c_str(),
                                                 ("NO vs LB for the "+histTitlesNO[systemIndex]+" (SP noise)").c_str(),
                                                 NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_NO_vsLB[systemIndex]->GetXaxis()->SetTitle("LumiBlock");
    m_NO_vsLB[systemIndex]->GetYaxis()->SetTitle("Noise Occupancy [10^{-5}]");
    if (noiseOccMaps.regHist(m_NO_vsLB[systemIndex]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book " << m_NO_vsLB[systemIndex]->GetName());
    }

    m_NOTrigger_vsLB[systemIndex] = TProfile_LW::create((histNamesNO[systemIndex]+"NOTrigger_vsLB").c_str(),
                                                        ("NO with trigger vs LB for the "+histTitlesNO[systemIndex]+" (SP noise)").c_str(),
                                                        NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_NOTrigger_vsLB[systemIndex]->GetXaxis()->SetTitle("LumiBlock");
    m_NOTrigger_vsLB[systemIndex]->GetYaxis()->SetTitle("Noise Occupancy [10^{-5}]");
    if (noiseOccMaps.regHist(m_NOTrigger_vsLB[systemIndex]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book " << m_NOTrigger_vsLB[systemIndex]->GetName());
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode
SCTHitsNoiseMonTool::bookGeneralHitOccupancyMaps(const unsigned int systemIndex) {
  static const string paths[N_REGIONS]{"SCT/SCTEC/Noise", "SCT/SCTB/Noise", "SCT/SCTEA/Noise"};
  static const unsigned int limits[N_REGIONS]{N_DISKSx2, N_BARRELSx2, N_DISKSx2};
  static const string profileNames[N_REGIONS]{"ECm", "BAR", "ECp"};
  static const string profileTitles[N_REGIONS]{"ECm", "Barrel", "ECp"};
  static const string profileNames2[N_REGIONS]{"ECC", "BAR", "ECA"};
  static const string profileTitles2[N_REGIONS]{"EndCap C", "Barrel", "EndCap A"};

  const Bec bec{index2Bec(systemIndex)};

  if (bec == INVALID_SYSTEM) {
    ATH_MSG_FATAL("Invalid subsystem index, should be 0-2, was " << systemIndex);
    return StatusCode::FAILURE;
  }
  if (newRunFlag()) {
    MonGroup hitOccMaps{this, paths[systemIndex], run, ATTRIB_UNMANAGED};

    m_phitoccupancymapHistoVector[systemIndex].clear();
    m_phitoccupancymapHistoVectorRecent[systemIndex].clear();

    // book 2D "noise" maps, containing hits that aren't associated to tracks
    for (unsigned int i{0}; i < limits[systemIndex]; ++i) {
      LayerSideFormatter layerSide{i, systemIndex};
      const string streamhitmapR{"hitoccupancymap" + abbreviations[systemIndex] + "_" + layerSide.name()};
      const string streamhitmaprecentR{"hitoccupancymaprecent" + abbreviations[systemIndex] + "_" + layerSide.name()};
      string histotitleR{"SCT Hit Occupancy map for " + names[systemIndex] + ": " + layerSide.title()};
      string histotitletriggerR{"SCT Hit Occupancy map for " + m_NOTriggerItem + " Trigger and " + names[systemIndex] + ": " + layerSide.title()};
      string histotitlerecentR{"SCT Hit Occupancy map in recent events for " + names[systemIndex] + ": " + layerSide.title()};
      prof2Factory(streamhitmapR, histotitleR, bec, hitOccMaps, m_phitoccupancymapHistoVector[systemIndex]);
      if (m_environment == AthenaMonManager::online) {
        prof2Factory(streamhitmaprecentR, histotitlerecentR, bec, hitOccMaps, m_phitoccupancymapHistoVectorRecent[systemIndex]);
      }
    }

    m_HallHits_vsLB[systemIndex] = TProfile_LW::create(("h_HallHits"+profileNames[systemIndex]+"_vsLB").c_str(),
                                                       ("Average num of all Hits in "+profileNames[systemIndex]+" vs LB").c_str(),
                                                       NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_HallHits_vsLB[systemIndex]->GetXaxis()->SetTitle("LumiBlock");
    m_HallHits_vsLB[systemIndex]->GetYaxis()->SetTitle("Average number of all Hits");
    if (hitOccMaps.regHist(m_HallHits_vsLB[systemIndex]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book " << m_HallHits_vsLB[systemIndex]->GetName());
    }

    m_HallHitsTrigger_vsLB[systemIndex] = TProfile_LW::create(("h_HallHitsTrigger"+profileNames[systemIndex]+"_vsLB").c_str(),
                                                              ("Average num of all Hits in "+profileNames[systemIndex]+" with trigger vs LB").c_str(),
                                                              NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_HallHitsTrigger_vsLB[systemIndex]->GetXaxis()->SetTitle("LumiBlock");
    m_HallHitsTrigger_vsLB[systemIndex]->GetYaxis()->SetTitle("Average number of all Hits");
    if (hitOccMaps.regHist(m_HallHitsTrigger_vsLB[systemIndex]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book " << m_HallHitsTrigger_vsLB[systemIndex]->GetName());
    }

    m_HSPHits_vsLB[systemIndex] = TProfile_LW::create(("h_HSPHits"+profileNames[systemIndex]+"_vsLB").c_str(),
                                                      ("Average num of SP Hits in "+profileNames[systemIndex]+" vs LB").c_str(),
                                                      NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_HSPHits_vsLB[systemIndex]->GetXaxis()->SetTitle("LumiBlock");
    m_HSPHits_vsLB[systemIndex]->GetYaxis()->SetTitle("Average number of SP Hits");
    if (hitOccMaps.regHist(m_HSPHits_vsLB[systemIndex]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book " << m_HSPHits_vsLB[systemIndex]->GetName());
    }

    m_HSPHitsTrigger_vsLB[systemIndex] = TProfile_LW::create(("h_HSPHitsTrigger"+profileNames[systemIndex]+"_vsLB").c_str(),
                                                             ("Average num of SP Hits in "+profileNames[systemIndex]+" with trigger vs LB").c_str(),
                                                             NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_HSPHitsTrigger_vsLB[systemIndex]->GetXaxis()->SetTitle("LumiBlock");
    m_HSPHitsTrigger_vsLB[systemIndex]->GetYaxis()->SetTitle("Average number of SP Hits");
    if (hitOccMaps.regHist(m_HSPHitsTrigger_vsLB[systemIndex]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book " << m_HSPHitsTrigger_vsLB[systemIndex]->GetName());
    }

    m_HO_vsLB[systemIndex] = TProfile_LW::create((profileNames2[systemIndex]+"HO_vsLB").c_str(),
                                                 ("HO vs LB for the "+profileTitles2[systemIndex]+" (SP noise)").c_str(),
                                                 NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_HO_vsLB[systemIndex]->GetXaxis()->SetTitle("LumiBlock");
    m_HO_vsLB[systemIndex]->GetYaxis()->SetTitle("Hit Occupancy [10^{-5}]");
    if (hitOccMaps.regHist(m_HO_vsLB[systemIndex]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book " << m_HO_vsLB[systemIndex]->GetName());
    }

    m_HOTrigger_vsLB[systemIndex] = TProfile_LW::create((profileNames2[systemIndex]+"HOTrigger_vsLB").c_str(),
                                                        ("HO with trigger vs LB for the "+profileTitles2[systemIndex]+" (SP noise)").c_str(),
                                                        NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_HOTrigger_vsLB[systemIndex]->GetXaxis()->SetTitle("LumiBlock");
    m_HOTrigger_vsLB[systemIndex]->GetYaxis()->SetTitle("Hit Occupancy [10^{-5}]");
    if (hitOccMaps.regHist(m_HOTrigger_vsLB[systemIndex]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book " << m_HOTrigger_vsLB[systemIndex]->GetName());
    }
  }
  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                                SCTHitsNoiseMonTool :: checkNoiseMaps
// LF 13/01/2006
// Here we have the check of the histograms and we can also build
// efficiency, occupancy, rms histograms, etc from the basic ones
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::checkNoiseMaps() {
  IdentifierHash next;
  vector<float> vectorOfOccupancies;

  if (m_doSpacePointBasedNoise) {
    // CAM Added code for track and SP algorithms
    ////////////////////////////////////////////////////////
    if (m_environment == AthenaMonManager::online) { // Reset histos for online so there is proper weighting of events
      if (resetNoiseMapHists().isFailure()) {
        ATH_MSG_WARNING("Failed to reset noise occupancy maps!");
      }

      if (m_occSumUnbiasedRecent.size() and m_numberOfEventsRecent) {
        for (pair<const Identifier, float>& val: m_occSumUnbiasedRecent) {
          Identifier wafer_id{val.first};
          int element{N_SIDES * m_pSCTHelper->layer_disk(wafer_id) + m_pSCTHelper->side(wafer_id)};
          float occ{(m_numberOfEventsRecent > 0) ? val.second / (m_numberOfEventsRecent) :  val.second};
          int eta{m_pSCTHelper->eta_module(wafer_id)};
          int phi{m_pSCTHelper->phi_module(wafer_id)};
          int barrel_ec{m_pSCTHelper->barrel_ec(wafer_id)};
          unsigned int systemIndex{bec2Index(barrel_ec)};
          if ((barrel_ec == BARREL) or 
              ((barrel_ec == ENDCAP_A) and m_doPositiveEndcap) or
              ((barrel_ec == ENDCAP_C) and m_doNegativeEndcap)) {
            m_pnoiseoccupancymapHistoVectorRecent[systemIndex][element]->Fill(eta, phi, occ * 1E5);
          }
        }
      }
    }
    if (m_occSumUnbiasedTrigger.size() and m_numberOfEventsTrigger) {
      for (pair<const Identifier, float>&val: m_occSumUnbiasedTrigger) {
        Identifier wafer_id{val.first};
        int element{N_SIDES * m_pSCTHelper->layer_disk(wafer_id) + m_pSCTHelper->side(wafer_id)};
        float occ{(m_numberOfEventsTrigger > 0) ? val.second / (m_numberOfEventsTrigger) :  val.second};
        int eta{m_pSCTHelper->eta_module(wafer_id)};
        int phi{m_pSCTHelper->phi_module(wafer_id)};
        int barrel_ec{m_pSCTHelper->barrel_ec(wafer_id)};
        unsigned int systemIndex{bec2Index(barrel_ec)};
        if ((barrel_ec == BARREL) or
            ((barrel_ec == ENDCAP_A) and m_doPositiveEndcap) or
            ((barrel_ec == ENDCAP_C) and m_doNegativeEndcap)) {
          m_pnoiseoccupancymapHistoVectorTrigger[systemIndex][element]->Fill(eta, phi, occ * 1E5);
        }
      }
    }

    if (m_hitoccSumUnbiased.size() and m_numberOfEvents) {
      for (pair<const Identifier, float>& val: m_hitoccSumUnbiased) {
        Identifier wafer_id{val.first};
        int element{N_SIDES * m_pSCTHelper->layer_disk(wafer_id) + m_pSCTHelper->side(wafer_id)};
        float hitocc{(m_numberOfEvents > 0) ? val.second / (m_numberOfEvents) :  val.second};
        int eta{m_pSCTHelper->eta_module(wafer_id)};
        int phi{m_pSCTHelper->phi_module(wafer_id)};
        int barrel_ec{m_pSCTHelper->barrel_ec(wafer_id)};
        unsigned int systemIndex{bec2Index(barrel_ec)};
        if ((barrel_ec==BARREL) or
            ((barrel_ec==ENDCAP_A) and m_doPositiveEndcap) or
            ((barrel_ec==ENDCAP_C) and m_doNegativeEndcap)) {
          m_phitoccupancymapHistoVector[systemIndex][element]->Fill(eta, phi, hitocc * 1E5);
        }
      }
    }
    if (m_environment == AthenaMonManager::online) {
      if (m_hitoccSumUnbiasedRecent.size() and m_numberOfEventsRecent) {
        for (pair<const Identifier, float>& val: m_hitoccSumUnbiasedRecent) {
          Identifier wafer_id{val.first};
          int element{N_SIDES * m_pSCTHelper->layer_disk(wafer_id) + m_pSCTHelper->side(wafer_id)};
          float hitocc{(m_numberOfEventsRecent > 0) ? val.second / (m_numberOfEventsRecent) :  val.second};
          int eta{m_pSCTHelper->eta_module(wafer_id)};
          int phi{m_pSCTHelper->phi_module(wafer_id)};
          int barrel_ec{m_pSCTHelper->barrel_ec(wafer_id)};
          unsigned int systemIndex{bec2Index(barrel_ec)};
          if ((barrel_ec==BARREL) or
              ((barrel_ec==ENDCAP_A) and m_doPositiveEndcap) or
              ((barrel_ec==ENDCAP_C) and m_doNegativeEndcap)) {
            m_phitoccupancymapHistoVectorRecent[systemIndex][element]->Fill(eta, phi, hitocc * 1E5);
          }
        }
      }
    }

    if (m_current_lb<=NBINS_LBs) {
      for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
        m_occ_lb[jReg][m_current_lb] = 0;
        m_occTrigger_lb[jReg][m_current_lb] = 0;
        m_hitocc_lb[jReg][m_current_lb] = 0;
        m_hitoccTrigger_lb[jReg][m_current_lb] = 0;
      }
      int nlinks[N_REGIONS_INC_GENERAL]{0, 0, 0, 0};
      SCT_ID::const_id_iterator planeIterator{m_pSCTHelper->wafer_begin()};
      SCT_ID::const_id_iterator planeEnd{m_pSCTHelper->wafer_end()};
      for (; planeIterator not_eq planeEnd; ++planeIterator) {
        Identifier planeId{*planeIterator};
        const int bec{m_pSCTHelper->barrel_ec(planeId)};
        const unsigned int systemIndex{bec2Index(bec)};
        // Don't initialize a value for disabled  modules
        if (not m_ConfigurationTool->isGood(*planeIterator, InDetConditions::SCT_SIDE)) {
          continue;
        }
        if (m_events_lb > 0) {
          for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
            m_occ_lb[jReg][m_current_lb] += (1E5) * m_occSumUnbiased_lb[jReg][*planeIterator] / m_events_lb;
            m_hitocc_lb[jReg][m_current_lb] += (1E5) * m_hitoccSumUnbiased_lb[jReg][*planeIterator] / m_events_lb;
          }
        }
        if (m_eventsTrigger_lb > 0) {
          for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
            m_occTrigger_lb[jReg][m_current_lb] += (1E5) * m_occSumUnbiasedTrigger_lb[jReg][*planeIterator] / m_eventsTrigger_lb;
            m_hitoccTrigger_lb[jReg][m_current_lb] += (1E5) * m_hitoccSumUnbiasedTrigger_lb[jReg][*planeIterator] / m_eventsTrigger_lb;
          }
        }
        nlinks[systemIndex]++;
        nlinks[GENERAL_INDEX]++;
      }
      for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
        if (nlinks[jReg]>0) {
          m_occ_lb[jReg][m_current_lb] /= nlinks[jReg];
          m_occTrigger_lb[jReg][m_current_lb] /= nlinks[jReg];
          m_hitocc_lb[jReg][m_current_lb] /= nlinks[jReg];
          m_hitoccTrigger_lb[jReg][m_current_lb] /= nlinks[jReg];
        }
      }
    }

    for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
      m_NO_vsLB[jReg]->Reset();
      m_NOTrigger_vsLB[jReg]->Reset();
      m_HO_vsLB[jReg]->Reset();
      m_HOTrigger_vsLB[jReg]->Reset();
    }
    for (int bin{1}; bin <= NBINS_LBs; bin++) {
      for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
        if (m_occ_lb[jReg][bin] != 0) {
          m_NO_vsLB[jReg]->Fill(bin, m_occ_lb[jReg][bin]);
        }
        if (m_occTrigger_lb[jReg][bin] != 0) {
          m_NOTrigger_vsLB[jReg]->Fill(bin, m_occTrigger_lb[jReg][bin]);
        }
      }
    }
    for (int bin{1}; bin <= NBINS_LBs; bin++) {
      for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
        if (m_hitocc_lb[jReg][bin] != 0) {
          m_HO_vsLB[jReg]->Fill(bin, m_hitocc_lb[jReg][bin]);
        }
        if (m_hitoccTrigger_lb[jReg][bin] != 0) {
          m_HOTrigger_vsLB[jReg]->Fill(bin, m_hitoccTrigger_lb[jReg][bin]);
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                                SCTHitsNoiseMonTool :: initializeNoiseMaps
// DD 12/06/2011
// Here we initialize the noise occupancy maps to avoid a bug that is present
// when adding TProfiles when no noise hit has occured in a module side
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::initializeNoiseMaps() {
  for (SCT_ID::const_id_iterator planeIterator{m_pSCTHelper->wafer_begin()};
       planeIterator != m_pSCTHelper->wafer_end(); ++planeIterator) {
    // Don't initialize a value for disabled  modules
    if (not m_ConfigurationTool->isGood(*planeIterator, InDetConditions::SCT_SIDE)) {
      continue;
    }
    m_occSumUnbiased[*planeIterator] = 0.;
    m_occSumUnbiasedTrigger[*planeIterator] = 0.;
    m_occSumUnbiasedRecent[*planeIterator] = 0.;
    for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
      m_occSumUnbiased_lb[jReg][*planeIterator] = 0.;
      m_occSumUnbiasedTrigger_lb[jReg][*planeIterator] = 0.;
    }
    m_hitoccSumUnbiased[*planeIterator] = 0.;
    m_hitoccSumUnbiasedTrigger[*planeIterator] = 0.;
    m_hitoccSumUnbiasedRecent[*planeIterator] = 0.;
    for (unsigned int jReg{0}; jReg<N_REGIONS_INC_GENERAL; jReg++) {
      m_hitoccSumUnbiased_lb[jReg][*planeIterator] = 0.;
      m_hitoccSumUnbiasedTrigger_lb[jReg][*planeIterator] = 0.;
    }
  }

  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                                SCTHitsNoiseMonTool :: resetNoiseMapHists
// DD 12/06/2011
// Resets entries for all the noise occupancy histograms
//
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::resetNoiseMapHists() {
  for (unsigned int iReg{0}; iReg<N_REGIONS; iReg++) {
    if (resetVecProf2(m_pnoiseoccupancymapHistoVectorTrigger[iReg]).isFailure()) {
      ATH_MSG_WARNING("Failed to reset m_pnoiseoccupancymapHistoVectorTrigger[" << iReg << "]!");
    }
    if (resetVecProf2(m_phitoccupancymapHistoVector[iReg]).isFailure()) {
      ATH_MSG_WARNING("Failed to reset m_phitoccupancymapHistoVector[" << iReg << "]!");
    }
    if (m_environment == AthenaMonManager::online) {
      if (resetVecProf2(m_pnoiseoccupancymapHistoVectorRecent[iReg]).isFailure()) {
        ATH_MSG_WARNING("Failed to reset m_pnoiseoccupancymapHistoVectorRecent[" << iReg << "]!");
      }
      if (resetVecProf2(m_phitoccupancymapHistoVectorRecent[iReg]).isFailure()) {
        ATH_MSG_WARNING("Failed to reset hitoccupancymapHistoVectorRecent[" << iReg << "]!");
      }
    }
  }

  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                                SCTHitsNoiseMonTool :: resetNoiseMapsRecent
// DD 19/07/2011
// Resets all the values in the recent noise occ maps to zero
//
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::resetNoiseMapsRecent() {
  for (pair<const Identifier, float>& val: m_occSumUnbiasedRecent) {
    val.second = 0.0;
  }
  for (pair<const Identifier, float>& val: m_hitoccSumUnbiasedRecent) {
    val.second = 0.0;
  }
  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                                SCTHitsNoiseMonTool :: resetHitMapHists
// DD 23/07/2011
// Resets all the values in the recent hit maps to zero
//
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::resetHitMapHists() {
  if (m_environment == AthenaMonManager::online) {
    for (unsigned int iReg{0}; iReg<N_REGIONS; iReg++) {
      if (resetVecH2(m_ptrackhitsHistoVectorRecent[iReg]).isFailure()) {
        ATH_MSG_WARNING("Failed to reset m_ptrackhitsHistoVectorRecent[" << iReg << "]!");
      }
      if (resetVecH2(m_phitsHistoVectorRecent[iReg]).isFailure()) {
        ATH_MSG_WARNING("Failed to reset m_phitsHistoVectorRecent[" << iReg << "]!");
      }
    }
    m_clusizeRecent->Reset();
  }

  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                                SCTHitsNoiseMonTool :: resetVecProf2
// DD 12/06/2011
// Resets entries for all histograms in a vector of TProfile2Ds
//
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::resetVecProf2(vector<TProfile2D*>& hists) const {
  for (unsigned int i{0}; i < hists.size(); ++i) {
    if (hists[i]==nullptr) {
      continue;
    }
    hists[i]->Reset();
  }
  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                                SCTHitsNoiseMonTool :: resetVecProf2
// DD 23/07/2011
// Resets entries for all histograms in a vector of TH2s
//
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::resetVecH2(vector<TH2F_LW*>& hists) const {
  for (unsigned int i{0}; i < hists.size(); ++i) {
    if (hists[i]==nullptr) {
      continue;
    }
    hists[i]->Reset();
  }
  return StatusCode::SUCCESS;
}

// ====================================================================================================
//                           SCTHitsNoiseMonTool :: book1DNoiseHists
// Book noise occupancy histograms for all present SCT modules
// using ToolRootHistSvc()
// The histograms go to the subdirectory ../Modulemaps/
// to improve readability
//
// ====================================================================================================
StatusCode
SCTHitsNoiseMonTool::bookNoiseDistributions() {
  if (newRunFlag()) {
    MonGroup NoiseDistributions{this, "SCT/GENERAL/noise", ManagedMonitorToolBase::run, ATTRIB_UNMANAGED};

    m_NO_vsLB[GENERAL_INDEX] = TProfile_LW::create("NO_vsLB", "NO vs LB for all region (SP noise)", NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_NO_vsLB[GENERAL_INDEX]->GetXaxis()->SetTitle("LumiBlock");
    m_NO_vsLB[GENERAL_INDEX]->GetYaxis()->SetTitle("Noise Occupancy [10^{-5}]");
    if (NoiseDistributions.regHist(m_NO_vsLB[GENERAL_INDEX]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book SCTNO_vsLB");
    }

    m_NOTrigger_vsLB[GENERAL_INDEX] = TProfile_LW::create("NOTrigger_vsLB", "NO with trigger vs LB for all region (SP noise)",
                                                          NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_NOTrigger_vsLB[GENERAL_INDEX]->GetXaxis()->SetTitle("LumiBlock");
    m_NOTrigger_vsLB[GENERAL_INDEX]->GetYaxis()->SetTitle("Noise Occupancy [10^{-5}]");
    if (NoiseDistributions.regHist(m_NOTrigger_vsLB[GENERAL_INDEX]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book SCTNOTrigger_vsLB");
    }

    m_HO_vsLB[GENERAL_INDEX] = TProfile_LW::create("HO_vsLB", "HO vs LB for all region (SP noise)",
                                                   NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_HO_vsLB[GENERAL_INDEX]->GetXaxis()->SetTitle("LumiBlock");
    m_HO_vsLB[GENERAL_INDEX]->GetYaxis()->SetTitle("Hit Occupancy [10^{-5}]");
    if (NoiseDistributions.regHist(m_HO_vsLB[GENERAL_INDEX]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book " << m_HO_vsLB[GENERAL_INDEX]->GetName());
    }

    m_HOTrigger_vsLB[GENERAL_INDEX] = TProfile_LW::create("HOTrigger_vsLB", "HO with trigger vs LB for all region (SP noise)",
                                                          NBINS_LBs, 0.5, NBINS_LBs + 0.5);
    m_HOTrigger_vsLB[GENERAL_INDEX]->GetXaxis()->SetTitle("LumiBlock");
    m_HOTrigger_vsLB[GENERAL_INDEX]->GetYaxis()->SetTitle("Hit Occupancy [10^{-5}]");
    if (NoiseDistributions.regHist(m_HOTrigger_vsLB[GENERAL_INDEX]).isFailure()) {
      ATH_MSG_WARNING("Couldn't book " << m_HOTrigger_vsLB[GENERAL_INDEX]->GetName());
    }
  }
  return StatusCode::SUCCESS;
}

string
SCTHitsNoiseMonTool::positionString(const Identifier& plane) const {
  ostringstream position_txt;

  position_txt << m_pSCTHelper->barrel_ec(plane) << "_" << m_pSCTHelper->layer_disk(plane) << "_" <<
    m_pSCTHelper->phi_module(plane) << "_" << m_pSCTHelper->eta_module(plane) << "_" << m_pSCTHelper->side(plane);
  return position_txt.str();
}

StatusCode
SCTHitsNoiseMonTool::makeVectorOfTrackRDOIdentifiers() {
  // Clear the RDOsOnTracks vector
  m_RDOsOnTracks.clear();
  SG::ReadHandle<SCT_RDO_Container> p_rdocontainer{m_dataObjectName};
  if (not p_rdocontainer.isValid()) {
    ATH_MSG_FATAL("Could not find the data object " << m_dataObjectName.key() << " !");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG("Data object " << m_dataObjectName.key() << " found");
  }
  
  SG::ReadHandle<TrackCollection> tracks{m_tracksName};
  if (not tracks.isValid()) {
    ATH_MSG_FATAL("No tracks for you!");
    return StatusCode::FAILURE;
  }
  // Only do for events with less than some number of tracks
  if (tracks->size() > m_maxTracks) {
    ATH_MSG_DEBUG("The event has more than " << m_maxTracks
                  << " tracks. Don't do hits-on-track-hists");
    return StatusCode::SUCCESS;
  }
  // assemble list of rdo ids associated with tracks
  for (int i{0}; i < static_cast<int>(tracks->size()); i++) {
    const Trk::Track* track{(*tracks)[i]};
    if (track == nullptr) {
      ATH_MSG_WARNING("no pointer to track!!!");
      break;
    }
    // Get pointer to track state on surfaces
    const DataVector<const Trk::TrackStateOnSurface>* trackStates{track->trackStateOnSurfaces()};
    if (trackStates == nullptr) {
      ATH_MSG_WARNING("for current track is TrackStateOnSurfaces == Null, no data will be written for this track");
    } else {// Loop over all track states on surfaces
      for (const Trk::TrackStateOnSurface* TSOS: *trackStates) {
        // Get pointer to RIO of right type
        const InDet::SiClusterOnTrack* clus{dynamic_cast<const InDet::SiClusterOnTrack*>(TSOS->measurementOnTrack())};
        if (clus) {
          // Get Pointer to prepRawDataObject
          const InDet::SiCluster* RawDataClus{dynamic_cast<const InDet::SiCluster*>(clus->prepRawData())};
          if (RawDataClus == nullptr) {
            ATH_MSG_WARNING("SiCluster WITHOUT prepRawData!!!!");
            break;
          }
          // if Cluster is in SCT ...
          if (RawDataClus->detectorElement()->isSCT()) {
            const vector<Identifier>& rdoList{RawDataClus->rdoList()};
            m_RDOsOnTracks.insert(m_RDOsOnTracks.end(), rdoList.begin(), rdoList.end());
          }
        }
      }
    }
  }
  ATH_MSG_DEBUG("JEGN!! num RDOs on tracks is " << m_RDOsOnTracks.size());
  return StatusCode::SUCCESS;
}

StatusCode
SCTHitsNoiseMonTool::bookGeneralTrackHits(const unsigned int systemIndex) {
  static const string paths[N_REGIONS] = {
    "SCT/SCTEC/hits/", "SCT/SCTB/hits/", "SCT/SCTEA/hits/"
  };
  static const unsigned int limits[N_REGIONS] = {
    N_DISKSx2, N_BARRELSx2, N_DISKSx2
  };

  const Bec bec{index2Bec(systemIndex)};

  if (bec == INVALID_SYSTEM) {
    ATH_MSG_FATAL("Invalid subsystem index, should be 0-2, was " << systemIndex);
    return StatusCode::FAILURE;
  }
  MonGroup tracksMon{this, paths[systemIndex] + "mapsOfHitsOnTracks", run, ATTRIB_UNMANAGED};
  if (newRunFlag()) {
    m_ptrackhitsHistoVector[systemIndex].clear();
    m_ptrackhitsHistoVectorRecent[systemIndex].clear();
    // book Hitmaps and hits per layer histograms
    for (unsigned int i{0}; i < limits[systemIndex]; ++i) {
      LayerSideFormatter layerSide{i, systemIndex};
      const string streamhitmap{"mapsOfHitsOnTracks" + abbreviations[systemIndex] + streamDelimiter +
          "trackhitsmap_" + layerSide.name()};
      const string streamhitmaprecent{"mapsOfHitsOnTracksRecent" + abbreviations[systemIndex] + streamDelimiter +
          "trackhitsmap_" + layerSide.name()};
      string histotitle{"SCT hits on tracks for " + names[systemIndex] + " " + layerSide.title()};
      string histotitlerecent{"SCT hits on tracks from recent events for " + names[systemIndex] + " " +
          layerSide.title()};
      h2Factory(streamhitmap, histotitle, bec, tracksMon, m_ptrackhitsHistoVector[systemIndex]);
      if (m_environment == AthenaMonManager::online) {
        h2Factory(streamhitmaprecent, histotitlerecent, bec, tracksMon, m_ptrackhitsHistoVectorRecent[systemIndex]);
      }
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode
SCTHitsNoiseMonTool::bookGeneralTrackTimeHistos() {
  if (newRunFlag()) {
    MonGroup tbinGroup{this, "SCT/GENERAL/tbin", ManagedMonitorToolBase::run, ATTRIB_UNMANAGED};
    m_tbinfracall = profFactory("TBinFracAll", "fraction of 01X for each region", tbinGroup);
  }
  return StatusCode::SUCCESS;
}

TH2F_LW*
SCTHitsNoiseMonTool::h2Factory(const string& name, const string& title, const Bec bec,
                               MonGroup& registry, vector<TH2F_LW*>& storageVector) const {
  int firstEta{FIRST_ETA_BIN}, lastEta{LAST_ETA_BIN}, nEta{N_ETA_BINS}, 
      firstPhi{FIRST_PHI_BIN}, lastPhi{LAST_PHI_BIN}, nPhi{N_PHI_BINS};

  if (bec != BARREL) {
    firstEta = FIRST_ETA_BIN_EC;
    lastEta = LAST_ETA_BIN_EC;
    firstPhi = FIRST_PHI_BIN_EC;
    lastPhi = LAST_PHI_BIN_EC;
    nEta = N_ETA_BINS_EC;
    nPhi = N_PHI_BINS_EC;
  }
  TH2F_LW* tmp{TH2F_LW::create(name.c_str(), title.c_str(), nEta, firstEta - 0.5, lastEta + 0.5, nPhi, firstPhi - 0.5, lastPhi + 0.5)};
  tmp->SetXTitle("Index in the direction of #eta");
  tmp->SetYTitle("Index in the direction of #phi");
  bool success{registry.regHist(tmp).isSuccess()};
  if (not success) {
    ATH_MSG_WARNING("Cannot book SCT histogram: " << name);
  }
  storageVector.push_back(tmp);
  return success ? tmp : nullptr;
}

TProfile2D*
SCTHitsNoiseMonTool::prof2DFactory(const string& name, const string& title, MonGroup& registry, int nbinx,
                                   float xlo, float xhi, int nbiny, float ylo, float yhi) const {
  TProfile2D* tmp{new TProfile2D{name.c_str(), title.c_str(), nbinx, xlo - 0.5, xhi + 0.5, nbiny, ylo - 0.5, yhi + 0.5}};

  tmp->SetXTitle("Index in the direction of #eta");
  tmp->SetYTitle("Index in the direction of #phi");
  bool success{registry.regHist(tmp).isSuccess()};
  if (not success) {
    ATH_MSG_WARNING("Cannot book SCT histogram: " << name);
  }
  return success ? tmp : nullptr;
}

TProfile_LW*
SCTHitsNoiseMonTool::profFactory(const string& name, const string& title, MonGroup& registry) const {
  TProfile_LW* tmp{TProfile_LW::create(name.c_str(), title.c_str(), 3, 0, 3)};

  tmp->SetYTitle("Fraction of 01X");
  tmp->GetXaxis()->SetBinLabel(1, "Endcap C");
  tmp->GetXaxis()->SetBinLabel(2, "Barrel");
  tmp->GetXaxis()->SetBinLabel(3, "Endcap A");
  bool success{registry.regHist(tmp).isSuccess()};
  if (not success) {
    ATH_MSG_WARNING("Cannot book SCT histogram: " << name);
  }
  return success ? tmp : nullptr;
}

TProfile2D*
SCTHitsNoiseMonTool::prof2Factory(const string& name, const string& title, const Bec bec,
                                  MonGroup& registry, vector<TProfile2D*>& storageVector) const {
  int firstEta{FIRST_ETA_BIN}, lastEta{LAST_ETA_BIN}, nEta{N_ETA_BINS}, 
      firstPhi{FIRST_PHI_BIN}, lastPhi{LAST_PHI_BIN}, nPhi{N_PHI_BINS};

  if (bec != BARREL) {
    firstEta = FIRST_ETA_BIN_EC;
    lastEta = LAST_ETA_BIN_EC;
    firstPhi = FIRST_PHI_BIN_EC;
    lastPhi = LAST_PHI_BIN_EC;
    nEta = N_ETA_BINS_EC;
    nPhi = N_PHI_BINS_EC;
  }
  TProfile2D* tmp{new TProfile2D{name.c_str(), title.c_str(), nEta, firstEta - 0.5, lastEta + 0.5, nPhi, firstPhi - 0.5, lastPhi + 0.5}};
  tmp->SetXTitle("Index in the direction of #eta");
  tmp->SetYTitle("Index in the direction of #phi");
  bool success{registry.regHist(tmp).isSuccess()};
  if (not success) {
    ATH_MSG_WARNING("Cannot book SCT histogram: " << name);
  }
  storageVector.push_back(tmp);
  return success ? tmp : nullptr;
}

TH1F_LW*
SCTHitsNoiseMonTool::h1Factory(const string& name, const string& title, MonGroup& registry, const float lo,
                               const float hi, const unsigned int nbins) const {
  TH1F_LW* tmp{TH1F_LW::create(name.c_str(), title.c_str(), static_cast<int>(nbins), lo, hi)};
  bool success{registry.regHist(tmp).isSuccess()};

  if (not success) {
    ATH_MSG_WARNING("Cannot book SCT histogram: " << name);
  }
  return success ? tmp : nullptr;
}

TH1F_LW*
SCTHitsNoiseMonTool::h1Factory(const string& name, const string& title, MonGroup& registry,
                               vector<TH1F_LW*>& storageVector, const float lo, const float hi, const unsigned int nbins) const {
  TH1F_LW* tmp{TH1F_LW::create(name.c_str(), title.c_str(), static_cast<int>(nbins), lo, hi)};
  bool success{registry.regHist(tmp).isSuccess()};

  if (not success) {
    ATH_MSG_WARNING("Cannot book SCT histogram: " << name);
  }
  storageVector.push_back(tmp);
  return success ? tmp : nullptr;
}

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/** Tool to measure the intrinsic single hit efficiency in the SCT
    This tool is part of the SCT_Monitoring package. Its aim is to
    measure the intrinsic single hit efficiency in the SCT using
    the holes-on-tracks method.
    @author SCT Monitoring group hn-atlas-sct-monitoring@cern.ch
*/

#ifndef SCTEFFICIENCYTOOL_H
#define SCTEFFICIENCYTOOL_H

#include "AthenaMonitoring/ManagedMonitorToolBase.h"

#include "SCT_MonitoringNumbers.h"

#include "CommissionEvent/ComTime.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "MagFieldInterfaces/IMagFieldSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrigAnalysisInterfaces/IBunchCrossingTool.h"
#include "TrkToolInterfaces/ITrackHoleSearchTool.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "TrkTrack/Track.h" 
#include "TrkTrack/TrackCollection.h"

//Gaudi
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

//ROOT
#include "TString.h"

//STL
#include <string>
#include <array>

class Identifier;
class PixelID;
class SCT_ID;
class TRT_ID;
class IInterface;
class ISCT_ConfigurationConditionsTool;
class TH1D;
class TH1F;
class TH2I;
class TH1I;
class TProfile2D;
class TProfile;
class TGraphErrors;

class IChronoStatSvc;
class StatusCode;

/**
   @class SCTHitEffMonTool
   The tool itself
*/
class SCTHitEffMonTool : public ManagedMonitorToolBase  {
  
 public:
  typedef Trk::Track Track;
  /** Constructor */
  SCTHitEffMonTool (const std::string& type, const std::string& name, const IInterface* parent);
  /** Destructor */
  virtual ~SCTHitEffMonTool () = default;

  /** Histogram booking method */
  virtual StatusCode bookHistograms ();
  virtual StatusCode bookHistogramsRecurrent ();

  /** Histogram filling method */
  virtual StatusCode fillHistograms ();
  virtual StatusCode procHistograms ();

 private:

  StatusCode initialize();

  /** Method to cut on track or hit variables and automatize DEBUG statements */
  StatusCode failCut (bool value, std::string name);

  /** Method to compute incident angle of track to wafer */
  StatusCode findAnglesToWaferSurface (const Amg::Vector3D& mom, const Identifier id,
                                       const InDetDD::SiDetectorElementCollection* elements,
                                       double& theta, double& phi);

  /** Method to find the chip just before a given hit */
  int previousChip (double xl, int side, bool swap);

  /** Computes residual of a hit to a track */
  double getResidual (const Identifier& surfaceID,
                      const Trk::TrackParameters* trkParam,
                      const InDet::SCT_ClusterContainer* p_sctclcontainer);

  /** Single histogram booking method */
  template < class T > StatusCode bookEffHisto (T*& histo, MonGroup& MG, 
                                                TString name, TString title,
                                                int nbin, double x1, double x2);

  template < class T > StatusCode bookEffHisto (T*& histo, MonGroup& MG, 
                                                TString name, TString title,
                                                int nbinx, double x1, double x2,
                                                int nbiny, double y1, double y2);

  template < class T > StatusCode bookEffHisto (T*& histo, MonGroup& MG, 
                                                TString name, TString title,
                                                int nbinx, double* xbins,
                                                int nbiny, double* ybins);

  SG::ReadHandleKey<TrackCollection> m_TrackName{this, "TrackName", "CombinedInDetTracks"};
  IChronoStatSvc* m_chrono;

  const std::map < Identifier, unsigned int >* m_badChips;
  ServiceHandle<MagField::IMagFieldSvc> m_fieldServiceHandle{this, "MagFieldSvc", "AtlasFieldSvc"};
  ToolHandle<Trig::IBunchCrossingTool> m_bunchCrossingTool{this, "BunchCrossingTool", "Trig::BunchCrossingTool/BunchCrossingTool"};


  IntegerProperty m_DetectorMode{this, "DetectorMode", 3, "Barrel = 1, endcap =2, both =3"};
  IntegerProperty m_RunningMode{this, "RunningMode", 2};

  // If all 3 of these -1, defaults depend on cosmic/not
  IntegerProperty m_minSCTHits{this, "MinSCTHits", -1};
  IntegerProperty m_minTRTHits{this, "MinTRTHits", -1};
  IntegerProperty m_minOtherHits{this, "MinOtherHits", 6};

  IntegerProperty m_minPixelHits{this, "MinPixelHits", -1};

  FloatProperty m_maxPhiAngle{this, "MaxPhiAngle", 40., "Degrees, 100 implies no cut."};
  FloatProperty m_maxChi2{this, "MaxChi2", 3.};
  FloatProperty m_maxD0{this, "Maxd0", 10., "mm of D0"};
  FloatProperty m_minPt{this, "MinPt", 1000., "minimu pt in MeV/c"};
  FloatProperty m_effdistcut{this, "effDistanceCut", 2.};
  FloatProperty m_maxZ0sinTheta{this, "MaxZ0sinTheta", 0.};
  UnsignedIntegerProperty m_maxTracks{this, "MaxTracks", 500};

  std::string m_path;
  SG::ReadHandleKey<InDet::SCT_ClusterContainer> m_sctContainerName{this, "SCT_ClusterContainer", "SCT_Clusters"};


  BooleanProperty m_insideOutOnly{this, "InsideOutOnly", false};
  BooleanProperty m_usemasks{this, "UseMasks", false};
  BooleanProperty m_detailed{this, "Detailed", false};
  BooleanProperty m_superDetailed{this, "SuperDetailed", false};
  BooleanProperty m_usedatabase{this, "LookAtDatabase", false};
  BooleanProperty m_isCosmic{this, "IsCosmic", false};
  BooleanProperty m_isSim{this, "IsSim", false};
  BooleanProperty m_useTRTPhase{this, "UseTRTPhase", false};
  BooleanProperty m_useSCTorTRT{this, "UseSCTorTRT", false};
  BooleanProperty m_requireEnclosingHits{this, "RequireEnclosingHits", false};
  BooleanProperty m_requireOtherFace{this, "RequireOtherFace", false};
  BooleanProperty m_requireGuardRing{this, "RequireGuardRing", false, "should be returned to true"};
  BooleanProperty m_vetoBadChips{this, "VetoBadChips", true};
  BooleanProperty m_chronotime{this, "ChronoTime", false};
  BooleanProperty m_useIDGlobal{this, "useIDGlobal", false};

  ToolHandle<Trk::IResidualPullCalculator> m_residualPullCalculator{this, "ResPullCalc", "Trk::ResidualPullCalculator/ResidualPullCalculator"};
  ToolHandle<Trk::IRIO_OnTrackCreator> m_rotcreator{this, "ROTCreator", "InDet::SCT_ClusterOnTrackTool/SCT_ClusterOnTrackTool"};
  ToolHandle<Trk::ITrackHoleSearchTool> m_holeSearchTool{this, "HoleSearch", "InDet::InDetTrackHoleSearchTool"};

  ToolHandle<ISCT_ConfigurationConditionsTool> m_configConditions{this, "ConfigConditions",
      "SCT_ConfigurationConditionsTool/InDetSCT_ConfigurationConditionsTool", "Tool to retrieve SCT Configuration Tool"};

  typedef std::array < TProfile*, SCT_Monitoring::N_REGIONS > TProfArray;
  typedef std::array < TH1F*, SCT_Monitoring::N_REGIONS > TH1FArray;
  typedef std::array < TH2F*, SCT_Monitoring::N_REGIONS > TH2FArray;
  typedef std::array < std::array < TH2F*, SCT_Monitoring::N_ENDCAPS >, SCT_Monitoring::N_REGIONS > TH2FArrayLayer;

  std::array < std::array < TProfile2D*, 2 >, SCT_Monitoring::N_LAYERS_TOTAL > m_effMap;
  std::array < std::array < TProfile2D*, 2 >, SCT_Monitoring::N_LAYERS_TOTAL > m_effMapFirstBCID;
  std::array < std::array < TProfile2D*, 2 >, SCT_Monitoring::N_LAYERS_TOTAL > m_ineffMap;
  std::array < std::array < TProfile*, 2 >, SCT_Monitoring::N_LAYERS_TOTAL > m_effLumiBlock;
  std::array < TProfile2D*, SCT_Monitoring::N_LAYERS_TOTAL > m_accMap;
  std::array < TProfile2D*, SCT_Monitoring::N_LAYERS_TOTAL > m_accPhysMap;
  std::array < std::array < TProfile2D*, SCT_Monitoring::N_ENDCAPSx2 >, SCT_Monitoring::N_REGIONS > m_layerResidualHistos;

  TProfile* m_Eff_Total;
  TProfile* m_Eff_TotalBCID;
  TProfile* m_Eff_hashCodeHisto;
  TProfile* m_Eff_LumiBlockHisto_Total;
  TH1F* m_effdistribution;

  TProfile2D* m_effHashLumiB;

  TProfArray m_Eff_summaryHisto;
  TProfArray m_Eff_summaryHistoFirstBCID;
  TProfArray m_Eff_summaryHisto_old;
  TProfArray m_holesPerTrackHisto;
  TProfArray m_holesDistPerTrackHisto;
  TProfArray m_Unas_summaryHisto;
  TProfArray m_Eff_etaHisto;
  TProfArray m_Eff_etaPhiCutHisto;
  TProfArray m_Eff_ptiHisto;
  TProfArray m_Eff_ptHisto;
  TProfArray m_Unas_ptiHisto;
  TProfArray m_Eff_phiHisto;
  TProfArray m_Eff_phiEtaCutHisto;
  TProfArray m_Eff_phiHistoFinal;
  TProfArray m_Unas_phiHisto;
  TProfArray m_Eff_d0Histo;
  TProfArray m_Eff_d0PrecHisto;
  TProfArray m_Eff_z0Histo;
  TProfArray m_Eff_xlocHisto, m_Eff_ylocHistos;
  TProfArray m_Unas_xlocHisto, m_Unas_ylocHisto;
  TProfArray m_Eff_nSCTHisto;
  TProfArray m_Eff_nTRTHisto;
  TProfArray m_Eff_nOtherHisto;
  TProfArray m_Eff_otherFaceHisto;
  TProfArray m_Unas_otherFaceHisto;
  TProfArray m_Eff_timecorHisto;
  TProfArray m_probEnclosedHisto;
  TProfArray m_Eff_SelectionHisto;
  TProfArray m_Eff_EventHisto;
  TProfArray m_Eff_LumiBlockHisto;
  TProfArray m_Eff_chi2Histo;
  TProfArray m_Eff_chi2HistoFinal;
  TProfArray m_chi2vPhiHisto;
  TProfArray m_badModPerSide;
  TProfArray m_Eff_summaryIncBadMod;
  TProfArray m_Eff_nTrk; 
  TProfArray m_Eff_nGoodTrk; 

  std::array < TProfile2D*, SCT_Monitoring::N_REGIONS > m_inEffStrip;
  std::array < TProfile2D*, SCT_Monitoring::N_REGIONS > m_inEffChip;

  TH1FArray m_EventHisto;
  TH1FArray m_SelectionHisto;
  TH1FArray m_ResidualHisto;
  TH1FArray m_ResidualUnasHisto;
  TH1FArray m_ResidualMissHisto;
  TH1FArray m_timecorHisto;
  TH1FArray m_nOtherHisto;
  TH1FArray m_etaTkUsedHisto;
  TH1FArray m_phiTkUsedHisto;
  TH1FArray m_ptiTkUsedHisto;
  TH1FArray m_ptTkUsedHisto;
  TH1FArray m_d0TkUsedHisto;
  TH1FArray m_d0PrecTkUsedHisto;
  TH1FArray m_nTrkUsedHisto;
  TH1FArray m_z0TkUsedHisto;
  TH1FArray m_phiLocalHisto, m_phiLocalCutHisto;
  TH1FArray m_chi2Histo;
  TH1FArray m_localHitXHisto, m_localHitYHistos; 

  TH1F* m_mNHitHisto;
  TH1F* m_barrelNHitHisto;
  TH1F* m_pNHitHisto;
  TH1F* m_SCTNHitHisto;
  TH1F* m_trtNHitHisto;
  TH1F* m_pixelNHitHisto;
  TH1F* m_PtTkHisto;
  TH1F* m_etaTkHisto;
  TH1F* m_d0TkHisto;
  TH1F* m_d0PrecTkHisto;
  TH1F* m_nTrkHisto;
  TH1F* m_nTrkParsHisto;
  TH1F* m_nTrkGoodHisto;
  TH1F* m_LumiBlock;
  TH1F* m_z0TkHisto;
  TH1F* m_hashCodeHisto;

  TH2I* m_badModFineMap;

  TH2FArray m_localHitHisto, m_localMissHisto, m_localUnasHisto;
  TH2FArray m_localHitGHisto;
  TH2FArray m_TwoSidesResiduals;
  TH2FArray m_TwoSidesResidualsIneff;
  TH2FArray m_chi2ResidualHisto;

  TH2FArrayLayer m_xlResidualHisto;
  TH2FArrayLayer m_xlResidualE0Histo;
  TH2FArrayLayer m_xlResidualE1Histo;
  TH2FArrayLayer m_xlResidualUnasHisto;

  TGraphErrors* m_badModMap;
  TGraphErrors* m_badChipMap;

  int m_countEvent;
  const PixelID* m_pixelId;
  const SCT_ID* m_sctId;
  const TRT_ID* m_trtId;

  SG::ReadHandleKey<ComTime> m_comTimeName{this, "ComTimeKey", "TRT_Phase"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

  /**Convert a layer/disk number (0-21) to a bec index (0,1,2) according to position of that layer
   * Numbering is counter-intuitive, would expect C then B then A; in fact the original ordering was A, C, B
   * I have re-ordered this!!!! so now its C,B,A
   **/ 
  SCT_Monitoring::BecIndex layerIndex2becIndex(const int index) {
    if ((index< 0) or (index>21)) return SCT_Monitoring::INVALID_INDEX;
    if (index< 9) return SCT_Monitoring::ENDCAP_C_INDEX;
    if (index< 13) return SCT_Monitoring::BARREL_INDEX;
    if (index< 22) return SCT_Monitoring::ENDCAP_A_INDEX;
    return SCT_Monitoring::INVALID_INDEX;
  }
  ///Convert a layer/disk number (0-21) to a layer number (0-8 for endcaps, 0-3 for barrel)
  int layerIndex2layer(const int index) {
    if ((index < 0) or (index > 21)) return SCT_Monitoring::INVALID_INDEX;
    if (index < 9) return index;
    if (index < 13) return index-9;
    if (index < 22) return index-13;
    return SCT_Monitoring::INVALID_INDEX;
  }
  
  int becIdxLayer2Index(const int becIdx, const int layer) {
    switch( becIdx ) {
    case SCT_Monitoring::ENDCAP_C_INDEX:
      return layer;
    case SCT_Monitoring::BARREL_INDEX:
      return layer + SCT_Monitoring::N_DISKS;
    case SCT_Monitoring::ENDCAP_A_INDEX: 
      return layer + SCT_Monitoring::N_DISKS + SCT_Monitoring::N_BARRELS;
    default:
      return -1;
    }
  }

};

#endif //SCTEFFICIENCYTOOL_H

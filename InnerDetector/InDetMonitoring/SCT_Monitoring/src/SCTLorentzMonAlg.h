// -*- C++ -*-

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCTLORENTZMONALG_H
#define SCTLORENTZMONALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TrkTrack/TrackCollection.h"

class SCT_ID;

class SCTLorentzMonAlg : public AthMonitorAlgorithm {
 public:
  SCTLorentzMonAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~SCTLorentzMonAlg() = default;
  virtual StatusCode initialize() override final;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override final;

 private:
  enum SiliconSurface { surface100, surface111, allSurfaces, nSurfaces };
  enum Sides { side0, side1, bothSides, nSidesInclBoth };

  const SCT_ID* m_pSCTHelper{nullptr};

  ToolHandle<Trk::ITrackSummaryTool> m_trackSummaryTool{this, "TrackSummaryTool", "InDetTrackSummaryTool"};

  /// Name of the Track collection to use
  SG::ReadHandleKey<TrackCollection> m_tracksName{this, "tracksName", "CombinedInDetTracks"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

  int findAnglesToWaferSurface(const float (&vec)[3],
                               const float& sinAlpha,
                               const Identifier& id, 
                               const InDetDD::SiDetectorElementCollection* elements,
                               float& theta,
                               float& phi) const;
};

#endif // SCTLORENTZMONALG_H

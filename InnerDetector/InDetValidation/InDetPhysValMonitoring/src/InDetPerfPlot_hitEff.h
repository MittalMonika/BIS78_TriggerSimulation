/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPHYSVALMONITORING_InDetPerfPlot_hitEff
#define INDETPHYSVALMONITORING_InDetPerfPlot_hitEff
/**
 * @file InDetPerfPlot_hitEff.h
 * @author nora pettersson
 **/


// std includes
#include <string>
#include "TProfile.h"

// local includes

#include "InDetPlotBase.h"
// could be fwd declared?
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"


///class holding res plots for Inner Detector RTT Validation and implementing fill methods
class InDetPerfPlot_hitEff: public InDetPlotBase {
public:
  InDetPerfPlot_hitEff(InDetPlotBase *pParent, const std::string &dirName);

  void fill(const xAOD::TrackParticle &trkprt);
  ~InDetPerfPlot_hitEff() {/** nop **/
  }

private:
  // enum copied from the hitDecorator tool in InDetPhysValMonitoring
  enum Subdetector {
    INVALID_DETECTOR=-1, IBL, PIXEL, SCT, TRT, DBM, N_SUBDETECTORS
  };
  enum Region {
    INVALID_REGION=-1, BARREL, ENDCAP, N_REGIONS
  };
private:
  void initializePlots();

  TProfile *m_eff_hit_vs_eta[N_SUBDETECTORS][N_REGIONS];
  bool hit_DEBUG;
};


#endif

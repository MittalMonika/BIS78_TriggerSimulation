/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFOCLUSTERMOMENTPLOTS_H
#define PFOCLUSTERMOMENTPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODPFlow/PFO.h"

namespace PFO {

  class PFOClusterMomentPlots : public PlotBase {

  public:

     PFOClusterMomentPlots(PlotBase *pParent, std::string sDir, std::string sPFOContainerName);

     void fill(const xAOD::PFO& PFO);

  private:

     TH1* m_PFO_LATERAL;
     TH1* m_PFO_LONGITUDINAL;
     TH1* m_PFO_SECOND_R;
     TH1* m_PFO_CENTER_LAMBDA;
     TH1* m_PFO_FIRST_ENG_DENS;
     TH1* m_PFO_ENG_FRAC_MAX;
     TH1* m_PFO_ISOLATION;
     TH1* m_PFO_ENG_BAD_CELLS;
     TH1* m_PFO_N_BAD_CELLS;
     TH1* m_PFO_BADLARQ_FRAC;
     TH1* m_PFO_ENG_POS;
     TH1* m_PFO_SIGNIFICANCE;
     TH1* m_PFO_AVG_LAR_Q;
     TH1* m_PFO_AVG_TILE_Q;

     void initializePlots();
     std::string m_sPFOContainerName;

  };

}
#endif

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// File:  Generators/FlowAfterburnber/CheckFlow.h
// Description:
//    This is a simple algorithm to histogram particle properties
//    for diagnosing of flow generation
//
//    It has a single important parameter m_rapcut 
//    to cut off particles from very forward pseudorapidity region
//
// AuthorList:
// Andrzej Olszewski: Initial Code February 2006

#ifndef CheckFlow_New_Minbias_h
#define CheckFlow_New_Minbias_h
 
#include <math.h>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "TruthHelper/GenAccessIO.h"

#include "GaudiKernel/ITHistSvc.h"
#include "TH1.h"
#include <string>



class TH1D;                    //Forward declaration
class TProfile;                    //Forward declaration


class CheckFlow_New_Minbias:public AthAlgorithm {
public:
  CheckFlow_New_Minbias(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:
  std::string     m_key;
  bool            m_produceHistogram;

  double          m_bcut_min;
  double          m_bcut_max;
  double          m_ptcut_min;
  double          m_ptcut_max;
  double          m_rapcut_min;
  double          m_rapcut_max;


  enum{
    n_ptbin=16,
    n_etabin=8,
    n_b_bins=19
  };

  //Histograms, used if m_produceHistogram is true = 1
  TH1D *m_hist_Psi_n_true    [6] [n_b_bins];
  TH1D *m_hist_Psi_n_reco    [6] [n_b_bins];
  TH1D *m_hist_psi_corr_true [36][n_b_bins];
  TH1D *m_hist_psi_corr_reco [36][n_b_bins];

  TH1D *m_hist_Psi_n_ebe     [6][n_b_bins];
  TH1D *m_hist_Psi_n_ebe_pt  [6][n_b_bins];
  TH1D *m_hist_vn_ebe        [6][n_b_bins];
  TH1D *m_hist_vn_ebe_ID1    [6][n_b_bins];
  TH1D *m_hist_vn_ebe_ID2    [6][n_b_bins];

  TProfile *m_profile_pt_dep      [6][n_b_bins][n_etabin]; 
  TProfile *m_profile_eta_dep     [6][n_b_bins][n_ptbin ]; 
  TProfile *m_profile_pt_dep_reco [6][n_b_bins][n_etabin]; 
  TProfile *m_profile_eta_dep_reco[6][n_b_bins][n_ptbin ]; 

  TProfile *m_profile_b_dep     [6][n_ptbin ][n_etabin]; 
  TProfile *m_profile_b_dep_reco[6][n_ptbin ][n_etabin]; 

  TProfile *m_profile_resolution[6];

  ITHistSvc*      m_thistSvc;
  TruthHelper::GenAccessIO*    m_tesIO;
};
#endif 

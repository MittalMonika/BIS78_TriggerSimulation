/*
Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file InDetPerfPlot_hitResidual.cxx
 * @author shaun roe
 **/

#include "InDetPerfPlot_hitResidual.h"
#include <vector>


InDetPerfPlot_hitResidual::InDetPerfPlot_hitResidual(InDetPlotBase* pParent, const std::string& sDir)  : InDetPlotBase(
    pParent, sDir),
  m_fillAdditionalITkPlots(false),
  m_residualx_1hit{},
  m_residualx_2ormorehits{},
  m_residualx{},
  m_residualy_1hit{},
  m_residualy_2ormorehits{},
  m_residualy{},
  m_residualpullx{},
  m_residualpully{},
  m_phiWidth{},
  m_etaWidth{},
  m_phiWidth_eta{},
  m_etaWidth_eta{}
  {
//
}

void
InDetPerfPlot_hitResidual::initializePlots() {
  // const bool prependDirectory(false);
  // x residuals
  book(m_residualx[L0PIXBARR][BARREL], "residualx_l0pix_barrel");
  book(m_residualx_1hit[L0PIXBARR][BARREL], "residualx_l0pix_barrel_1hit");
  book(m_residualx_2ormorehits[L0PIXBARR][BARREL], "residualx_l0pix_barrel_2ormorehits");
  //
  book(m_residualx[PIXEL][BARREL], "residualx_pixel_barrel");
  book(m_residualx_1hit[PIXEL][BARREL], "residualx_pixel_barrel_1hit");
  book(m_residualx_2ormorehits[PIXEL][BARREL], "residualx_pixel_barrel_2ormorehits");
  //
  book(m_residualx[SCT][BARREL], "residualx_sct_barrel");
  book(m_residualx_1hit[SCT][BARREL], "residualx_sct_barrel_1hit");
  book(m_residualx_2ormorehits[SCT][BARREL], "residualx_sct_barrel_2ormorehits");
  //
  book(m_residualx[TRT][BARREL], "residualx_trt_barrel");
  // ..now endcaps
  book(m_residualx[PIXEL][ENDCAP], "residualx_pixel_endcap");
  book(m_residualx_1hit[PIXEL][ENDCAP], "residualx_pixel_endcap_1hit");
  book(m_residualx_2ormorehits[PIXEL][ENDCAP], "residualx_pixel_endcap_2ormorehits");
  //
  book(m_residualx[SCT][ENDCAP], "residualx_sct_endcap");
  book(m_residualx_1hit[SCT][ENDCAP], "residualx_sct_endcap_1hit");
  book(m_residualx_2ormorehits[SCT][ENDCAP], "residualx_sct_endcap_2ormorehits");
  //
  book(m_residualx[TRT][ENDCAP], "residualx_trt_endcap");
  // DBM
  book(m_residualx[DBM][0], "residualx_dbm_neg");
  book(m_residualx[DBM][1], "residualx_dbm_pos");
  //

  // y residuals
  book(m_residualy[L0PIXBARR][BARREL], "residualy_l0pix_barrel");
  book(m_residualy_1hit[L0PIXBARR][BARREL], "residualy_l0pix_barrel_1hit");
  book(m_residualy_2ormorehits[L0PIXBARR][BARREL], "residualy_l0pix_barrel_2ormorehits");
  //
  book(m_residualy[PIXEL][BARREL], "residualy_pixel_barrel");
  book(m_residualy_1hit[PIXEL][BARREL], "residualy_pixel_barrel_1hit");
  book(m_residualy_2ormorehits[PIXEL][BARREL], "residualy_pixel_barrel_2ormorehits");
  //
  // SCT and TRT do not have y-residuals/pulls
  //
  // ..now endcaps
  book(m_residualy[PIXEL][ENDCAP], "residualy_pixel_endcap");
  book(m_residualy_1hit[PIXEL][ENDCAP], "residualy_pixel_endcap_1hit");
  book(m_residualy_2ormorehits[PIXEL][ENDCAP], "residualy_pixel_endcap_2ormorehits");
  //

  // SCT and TRT do not have y-residuals/pulls
  // DBM
  book(m_residualy[DBM][0], "residualy_dbm_neg");
  book(m_residualy[DBM][1], "residualy_dbm_pos");
  // pulls
  // barrel
  book(m_residualpullx[L0PIXBARR][BARREL], "residualpullx_l0pix_barrel");
  book(m_residualpullx[PIXEL][BARREL], "residualpullx_pixel_barrel");
  book(m_residualpullx[SCT][BARREL], "residualpullx_sct_barrel");
  book(m_residualpullx[TRT][BARREL], "residualpullx_trt_barrel");
  book(m_residualpullx[DBM][0], "residualpullx_dbm_barrel");
  //
  book(m_residualpullx[PIXEL][ENDCAP], "residualpullx_pixel_endcap");
  book(m_residualpullx[SCT][ENDCAP], "residualpullx_sct_endcap");
  book(m_residualpullx[TRT][ENDCAP], "residualpullx_trt_endcap");
  book(m_residualpullx[DBM][1], "residualpullx_dbm_endcap");
  //
  // barrel
  book(m_residualpully[L0PIXBARR][BARREL], "residualpully_l0pix_barrel");
  book(m_residualpully[PIXEL][BARREL], "residualpully_pixel_barrel");
  //
  // SCT and TRT do not have y-residuals/pulls
  book(m_residualpully[DBM][0], "residualpully_dbm_barrel");
  book(m_residualpully[PIXEL][ENDCAP], "residualpully_pixel_endcap");
  //
  ////SCT and TRT do not have y-residuals/pulls
  book(m_residualpully[DBM][1], "residualpully_dbm_endcap");
  // introduce cluster width histograms
  book(m_phiWidth[PIXEL][BARREL], "clusterPhiWidth_pixel_barrel");
  book(m_phiWidth[PIXEL][ENDCAP], "clusterPhiWidth_pixel_endcap");
  book(m_etaWidth[PIXEL][BARREL], "clusterEtaWidth_pixel_barrel");
  book(m_etaWidth[PIXEL][ENDCAP], "clusterEtaWidth_pixel_endcap");
  //
  book(m_phiWidth[SCT][BARREL], "clusterPhiWidth_sct_barrel");
  book(m_phiWidth[SCT][ENDCAP], "clusterPhiWidth_sct_endcap");
  
  // introduce cluster width histograms
  book(m_etaWidth_eta[L0PIXBARR][BARREL], "clusterEtaWidth_eta_l0pix_barrel");
  book(m_phiWidth_eta[L0PIXBARR][BARREL], "clusterPhiWidth_eta_l0pix_barrel");
  
  book(m_phiWidth_eta[PIXEL][BARREL], "clusterPhiWidth_eta_pixel_barrel");
  book(m_phiWidth_eta[PIXEL][ENDCAP], "clusterPhiWidth_eta_pixel_endcap");
  book(m_etaWidth_eta[PIXEL][BARREL], "clusterEtaWidth_eta_pixel_barrel");
  book(m_etaWidth_eta[PIXEL][ENDCAP], "clusterEtaWidth_eta_pixel_endcap");
  //
  book(m_phiWidth_eta[SCT][BARREL], "clusterPhiWidth_eta_sct_barrel");
  book(m_phiWidth_eta[SCT][ENDCAP], "clusterPhiWidth_eta_sct_endcap");
}

void InDetPerfPlot_hitResidual::FillAdditionalITkPlots(bool fill) {

  m_fillAdditionalITkPlots = fill;
  
  if (not m_fillAdditionalITkPlots) return;

  book(m_phiWidth_eta_perLayer[0][L0PIXBARR][BARREL], "clusterPhiWidth_eta_pixel_barrel_layer0");
  book(m_phiWidth_eta_perLayer[0][PIXEL][ENDCAP]    , "clusterPhiWidth_eta_pixel_endcap_layer0");
  book(m_etaWidth_eta_perLayer[0][L0PIXBARR][BARREL], "clusterEtaWidth_eta_pixel_barrel_layer0");
  book(m_etaWidth_eta_perLayer[0][PIXEL][ENDCAP]    , "clusterEtaWidth_eta_pixel_endcap_layer0");
  
  for (int layer = 1; layer < N_LAYERS; layer++) {
    book(m_phiWidth_eta_perLayer[layer][PIXEL][BARREL], "clusterPhiWidth_eta_pixel_barrel_layer"+std::to_string(layer));
    book(m_phiWidth_eta_perLayer[layer][PIXEL][ENDCAP], "clusterPhiWidth_eta_pixel_endcap_layer"+std::to_string(layer));
    book(m_etaWidth_eta_perLayer[layer][PIXEL][BARREL], "clusterEtaWidth_eta_pixel_barrel_layer"+std::to_string(layer));
    book(m_etaWidth_eta_perLayer[layer][PIXEL][ENDCAP], "clusterEtaWidth_eta_pixel_endcap_layer"+std::to_string(layer));
  }
  for (int layer = 0; layer < N_SCTLAYERS; layer++) {
    book(m_phiWidth_eta_perLayer[layer][SCT][BARREL]  , "clusterPhiWidth_eta_sct_barrel_layer"+std::to_string(layer));
    book(m_phiWidth_eta_perLayer[layer][SCT][ENDCAP]  , "clusterPhiWidth_eta_sct_endcap_layer"+std::to_string(layer));
    book(m_etaWidth_eta_perLayer[layer][SCT][BARREL]  , "clusterEtaWidth_eta_sct_barrel_layer"+std::to_string(layer));
    book(m_etaWidth_eta_perLayer[layer][SCT][ENDCAP]  , "clusterEtaWidth_eta_sct_endcap_layer"+std::to_string(layer));
  }
}

void
InDetPerfPlot_hitResidual::fill(const xAOD::TrackParticle& trkprt) {
  const static bool hitDetailsAvailable = trkprt.isAvailable<std::vector<int> >("measurement_region");
  static int warnCount(0);

  if (!hitDetailsAvailable) {
    if (warnCount++ < 10) {
      ATH_MSG_WARNING("The hit res plots dont see any data (note:only 10 warnings issued)");
    }
  } else {
    const std::vector<int>& result_det = trkprt.auxdata< std::vector<int> >("measurement_det");

    if (!result_det.empty()) {
      const std::vector<int>& result_measureType = trkprt.auxdata< std::vector<int> >("measurement_type");
      const std::vector<int>& result_region = trkprt.auxdata< std::vector<int> >("measurement_region");
      std::vector<int> result_iLayer;
      if (m_fillAdditionalITkPlots) result_iLayer = trkprt.auxdata< std::vector<int> >("measurement_iLayer");
      const std::vector<float>& result_residualLocX = trkprt.auxdata< std::vector<float> >("hitResiduals_residualLocX");
      const std::vector<float>& result_pullLocX = trkprt.auxdata< std::vector<float> >("hitResiduals_pullLocX");
      const std::vector<float>& result_residualLocY = trkprt.auxdata< std::vector<float> >("hitResiduals_residualLocY");
      const std::vector<float>& result_pullLocY = trkprt.auxdata< std::vector<float> >("hitResiduals_pullLocY");
      const std::vector<int>& result_phiWidth = trkprt.auxdata< std::vector<int> >("hitResiduals_phiWidth");
      const std::vector<int>& result_etaWidth = trkprt.auxdata< std::vector<int> >("hitResiduals_etaWidth");

      // NP: this should be fine... resiudal filled with -1 if not hit
      if (result_det.size() != result_residualLocX.size()) {
        ATH_MSG_WARNING("Vectors of results are not matched in size!");
      }
      const auto resultSize = result_region.size();
      for (unsigned int idx = 0; idx < resultSize; ++idx) {
        const int measureType = result_measureType[idx];
        if (measureType != 4) {
          continue; // NP: Only use unbiased hits for the hit residuals ;)
        }
        const int det = result_det[idx];
        const int region = result_region[idx];
        int layer = 0;
	if (m_fillAdditionalITkPlots) layer = result_iLayer[idx];
        const int width = result_phiWidth[idx];
        const int etaWidth = result_etaWidth[idx];
        const float residualLocX = result_residualLocX[idx];
        const float pullLocX = result_pullLocX[idx];
        const float residualLocY = result_residualLocY[idx];
        const float pullLocY = result_pullLocY[idx];
        if ((det == INVALID_DETECTOR)or(region == INVALID_REGION)) {
          continue;
        }
        //TRT cluster width is undefined
        
        if ((width > 0) or (det == TRT)) {
          // introduce cluster width histograms
          fillHisto(m_phiWidth[det][region], width);
          fillHisto(m_etaWidth[det][region], etaWidth);
          fillHisto(m_phiWidth_eta[det][region], trkprt.eta(), width);
          fillHisto(m_etaWidth_eta[det][region], trkprt.eta(), etaWidth);
          if (m_fillAdditionalITkPlots and (det==PIXEL or det==L0PIXBARR or (det==SCT and layer<N_SCTLAYERS))) {
            fillHisto(m_phiWidth_eta_perLayer[layer][det][region], trkprt.eta(), width);
            fillHisto(m_etaWidth_eta_perLayer[layer][det][region], trkprt.eta(), etaWidth);
          }
          fillHisto(m_residualx[det][region], residualLocX);
          const bool hasYCoordinate = (det != SCT)and(det != TRT); // SCT & TRT do not have LocY
          if (hasYCoordinate) {
            fillHisto(m_residualy[det][region], residualLocY);
          }
          fillHisto(m_residualpullx[det][region], pullLocX);
          if (hasYCoordinate) { // SCT & TRT do not have LocY
            fillHisto(m_residualpully[det][region], pullLocY);
          }
          if ((det == TRT)or(det == DBM) ) {
            continue;
          }
          if (width == 1) {
            fillHisto(m_residualx_1hit[det][region], residualLocX);
            if (hasYCoordinate) {
              fillHisto(m_residualy_1hit[det][region], residualLocY);
            }
          } else {
            fillHisto(m_residualx_2ormorehits[det][region], residualLocX);
            if (hasYCoordinate) {
              fillHisto(m_residualy_2ormorehits[det][region], residualLocY);
            }
          }
        }
      }
    }
  }
}

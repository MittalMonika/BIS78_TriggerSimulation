/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
// Source file for class SiCombinatorialTrackFinderData_xk
/////////////////////////////////////////////////////////////////////////////////

#include "SiSPSeededTrackFinderData/SiCombinatorialTrackFinderData_xk.h"

namespace InDet {

  SiCombinatorialTrackFinderData_xk::SiCombinatorialTrackFinderData_xk() {
  }

  void SiCombinatorialTrackFinderData_xk::setTools(const Trk::IPatternParametersPropagator* propTool,
                                                   const Trk::IPatternParametersUpdator* updatorTool,
                                                   const Trk::IRIO_OnTrackCreator* rioTool,
                                                   const Trk::IPRD_AssociationTool* assoTool,
                                                   MagField::IMagFieldSvc* fieldService,
                                                   const IInDetConditionsTool* pixCondTool,
                                                   const IInDetConditionsTool* sctCondTool,
                                                   const Trk::MagneticFieldProperties* fieldProp)
  {
    // Set SiTools and conditions
    //
    m_tools.setTools(propTool,
                     updatorTool,
                     rioTool,
                     assoTool,
                     fieldService);
    m_tools.setTools(pixCondTool,
                     sctCondTool);
    m_tools.setTools(fieldProp);

    // Set tool to trajectory
    //
    m_trajectory.setTools(&m_tools);
    
    m_initialized = true;
  }

  bool SiCombinatorialTrackFinderData_xk::isInitialized() const {
    return m_initialized;
  }

  SiTrajectory_xk& SiCombinatorialTrackFinderData_xk::trajectory() {
    return m_trajectory;
  }

  Trk::TrackInfo& SiCombinatorialTrackFinderData_xk::trackinfo() {
    return m_trackinfo;
  }

  InDet::SiTools_xk& SiCombinatorialTrackFinderData_xk::tools() {
    return m_tools;
  }

  std::list<Trk::Track*>& SiCombinatorialTrackFinderData_xk::tracks() {
    return m_tracks;
  }

  int& SiCombinatorialTrackFinderData_xk::nprint() {
    return m_nprint;
  }

  int& SiCombinatorialTrackFinderData_xk::inputseeds() {
    return m_inputseeds;
  }

  int& SiCombinatorialTrackFinderData_xk::goodseeds() {
    return m_goodseeds;
  }

  int& SiCombinatorialTrackFinderData_xk::findtracks() {
    return m_findtracks;
  }

  int& SiCombinatorialTrackFinderData_xk::inittracks() {
    return m_inittracks;
  }

  int& SiCombinatorialTrackFinderData_xk::roadbug() {
    return m_roadbug;
  }

  bool& SiCombinatorialTrackFinderData_xk::heavyIon() {
    return m_heavyIon;
  }

  int& SiCombinatorialTrackFinderData_xk::cosmicTrack() {
    return m_cosmicTrack;
  }

  int& SiCombinatorialTrackFinderData_xk::nclusmin() {
    return m_nclusmin;
  }

  int& SiCombinatorialTrackFinderData_xk::nclusminb() {
    return m_nclusminb;
  }

  int& SiCombinatorialTrackFinderData_xk::nwclusmin() {
    return m_nwclusmin;
  }

  int& SiCombinatorialTrackFinderData_xk::nholesmax() {
    return m_nholesmax;
  }

  int& SiCombinatorialTrackFinderData_xk::dholesmax() {
    return m_dholesmax;
  }

  bool& SiCombinatorialTrackFinderData_xk::simpleTrack() {
    return m_simpleTrack;
  }

  double& SiCombinatorialTrackFinderData_xk::pTmin() {
    return m_pTmin;
  }

  double& SiCombinatorialTrackFinderData_xk::pTminBrem() {
    return m_pTminBrem;
  }

  double& SiCombinatorialTrackFinderData_xk::xi2max() {
    return m_xi2max;
  }

  double& SiCombinatorialTrackFinderData_xk::xi2maxNoAdd() {
    return m_xi2maxNoAdd;
  }

  double& SiCombinatorialTrackFinderData_xk::xi2maxlink() {
    return m_xi2maxlink;
  }

} // end of name space

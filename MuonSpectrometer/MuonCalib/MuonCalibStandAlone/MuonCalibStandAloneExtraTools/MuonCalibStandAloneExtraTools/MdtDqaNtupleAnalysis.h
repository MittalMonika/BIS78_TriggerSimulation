/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SRC_MDTDQANTUPLEANALYSIS_H
#define SRC_MDTDQANTUPLEANALYSIS_H

#include "GaudiKernel/Algorithm.h"

#include <iostream>
#include <vector>
#include <math.h>  
#include <fstream>      
#include <string> 
#include <iomanip>  
#include <sstream> 

#include "TString.h"
#include "TH2D.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TChain.h"

#include "MuonCalibStandAloneExtraTools/StringUtil.h"
#include "MuonCalibStandAloneExtraTools/HistogramManager.h"
#include "MuonCalibStandAloneExtraTools/MdtDqaDb.h"
#include "MuonCalibStandAloneExtraTools/MDTDqaDeadElements.h"

#include "MuonCalibStandAloneBase/RegionSelectionSvc.h"

class RegionSelectionSvc;

namespace MuonCalib {

  class MuonCalibEvent;
  class MuonCalibSegment;
  class MuonRawHitCollection;

  /**
     @class NtupleAnalysis
     An example analysis class. One can use this class as a skeleton analysis. See the description of the separate methods for the possibilites of the Calib-EDM and standalone analysis (such as the use of Display and MuonCalib::T0Fitter)
     @author Zdenko.Van.Kesteren@cern.ch
  */
  class MdtDqaNtupleAnalysis {
  public:
    MdtDqaNtupleAnalysis(bool verbose, std::string outputFileName);     //!< default constructor
    virtual ~MdtDqaNtupleAnalysis() = default;   //!< destructor

    StatusCode initialize(RegionSelectionSvc *, HistogramManager *, int, float);
    StatusCode finalize();
    
    void handleEvent(const MuonCalibEvent &event, int eventnumber, const std::vector<MuonCalibSegment *> &segments, unsigned int position );  
    void histogramAnalysis(TFile * rootfile);

    int segmentStation(MuonCalibSegment *segment );
    int segmentEta(MuonCalibSegment *segment );
    int segmentPhi(MuonCalibSegment *segment );

  private:
    RegionSelectionSvc *p_reg_sel_svc;
    HistogramManager *m_histoManager;
    bool m_verbose;
    int m_DeadElementsAlgorithm;
    int m_SectorMin;
    int m_SectorMax;
    bool m_analyseBarrel;
    bool m_analyseEndcapA;
    bool m_analyseEndcapC;

    float m_ADCCUT;
    int m_evtCounter; 

    std::string m_outputFileName;
    std::vector<MdtDqaDb*> m_MdtDqaDbList;

  };

} //namespace MuonCalib

#endif // MdtDqaNtupleAnalysis_h

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MM_DIGITIZATION_MM_ELECTRONICSRESPONSESIMULATION_H
#define MM_DIGITIZATION_MM_ELECTRONICSRESPONSESIMULATION_H
/** @class MM_ElectronicsResponseSimulation

// ------------
// Authors:
//   Iakovidis George        <george.iakovidis@cern.ch>
//   Karakostas Konstantinos <konstantinos.karakostas@cern.ch>
//   Leontsinis Stefanos     <stefanos.leontsinis@cern.ch>
//   Nektarios Chr. Benekos  <nbenekos@cern.ch>
//   Jessica Metcalfe        <jessica.metcalfe@gmail.com>
//
// Major Contributions From: Verena Martinez
//                           Tomoyuki Saito
//
// Major Restructuring for r21+ From: Lawrence Lee <lawrence.lee.jr@cern.ch>
//
//////////////////////////////////////////////////////////////////////////////

*/


#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/Service.h"
#include "AthenaKernel/MsgStreamMember.h"
#include "GaudiKernel/StatusCode.h"


/// ROOT
#include <TROOT.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>
#include <TMath.h>

/// STD'S
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <utility>
#include <string>
#include <sstream>
#include <sys/stat.h>

/// Projects
#include "MM_Digitization/MM_ElectronicsToolInput.h"
#include "MM_Digitization/MM_ElectronicsToolTriggerOutput.h"
#include "MM_Digitization/MM_DigitToolOutput.h"
// #include "MM_Digitization/StripsResponseSimulation.h"

//VMM Mapping
#include "MM_Digitization/MM_StripVmmMappingTool.h"

using std::vector;
using std::cout;
using std::endl;

/// ROOT Classed
class TF1;
class TH1;
class TH1F;
class TH2F;

class MM_ElectronicsResponseSimulation {

private:
  /** power of responce function */
  float m_peakTime;
  float m_alpha;
  float m_timeWindowLowerOffset;
  float m_timeWindowUpperOffset;
  float m_electronicsThreshold;
  float m_stripDeadTime;
  float m_artDeadTime;
  float m_stripResponseQThreshold;
  float m_stripResponseDriftGapWidth;
  float m_stripResponseDriftVelocity;

  int  m_decoupleShaperFunctionParamaters;

  vector <float> m_tStripElectronicsAbThr;
  vector <float> m_qStripElectronics;
  vector <int>   m_nStripElectronics;

  TF1 *m_h_intFn;
  // StripsResponseSimulation* m_stripObject ;
  MM_ElectronicsResponseSimulation & operator=(const MM_ElectronicsResponseSimulation &right);
  MM_ElectronicsResponseSimulation(const MM_ElectronicsResponseSimulation&);
public :

  MM_ElectronicsResponseSimulation();
  virtual ~MM_ElectronicsResponseSimulation();

  void initialize();

  void clearValues ();
  void vmmPeakResponseFunction(const vector <int> & numberofStrip, const vector<vector <float>> & qStrip, const vector<vector <float>> & tStrip);
  void vmmThresholdResponseFunction(const vector <int> & numberofStrip, const vector<vector <float>> & qStrip, const vector<vector <float>> & tStrip);
  MM_DigitToolOutput getPeakResponseFrom(const MM_ElectronicsToolInput & digiInput);
  MM_DigitToolOutput getThresholdResponseFrom(const MM_ElectronicsToolInput & digiInput);

  MM_ElectronicsToolTriggerOutput getTheFastestSignalInVMM(const MM_DigitToolOutput & ElectronicThresholdOutput, const int chMax, const int stationEta);
  int getIdTheFastestSignalInVMM(float time, int VMM_id, std::vector<int> trigger_VMM_id, const std::vector<float> ElectronicsThreshold_stripTime, float timeWindowLower, float timeWindowUpper);
  void getVMMId(const std::vector< int > & ElectronicsThreshold_stripPos, const int chMax, const int stationEta, std::vector< int > & trigger_VMM_id, std::vector< int > & trigger_MMFE_VMM_id);
  MM_DigitToolOutput applyDeadTimeStrip(const MM_DigitToolOutput & ElectronicsTriggerOutput);
  MM_ElectronicsToolTriggerOutput applyDeadTimeART(const MM_ElectronicsToolTriggerOutput & ElectronicsTriggerOutput);
  MM_ElectronicsToolTriggerOutput applyARTTiming(const MM_ElectronicsToolTriggerOutput & ElectronicsTriggerOutput, float jitter, float offset);
  bool deadChannel(int id, float time, std::vector<int> & v_id, const std::vector<float> & v_time, float deadtime);

  inline void setPeakTime(float val) { m_peakTime = val;  };
  inline void setTimeWindowLowerOffset(float val) { m_timeWindowLowerOffset = val;};
  inline void setTimeWindowUpperOffset(float val) { m_timeWindowUpperOffset = val;};
  inline void setElectronicsThreshold(float val) { m_electronicsThreshold = val;};
  inline void setStripdeadtime(float val) { m_stripDeadTime = val;};
  inline void setARTdeadtime(float val)   { m_artDeadTime = val;};

  inline void setStripResponseQThreshold    (float val) { m_stripResponseQThreshold    = val; };
  inline void setStripResponseDriftVelocity (float val) { m_stripResponseDriftVelocity = val; };
  inline void setStripResponseDriftGapWidth (float val) { m_stripResponseDriftGapWidth = val; };


  float getPeakTime() const { return m_peakTime;};
  float getAlpha() const { return m_alpha;};
  float getTimeWindowLowerOffset() const { return m_timeWindowLowerOffset ;};
  float getTimeWindowUpperOffset() const { return m_timeWindowUpperOffset ;};
  float getElectronicsThreshold() const { return m_electronicsThreshold;};
  float getStripdeadtime() const { return m_stripDeadTime;};
  float getARTdeadtime() const { return m_artDeadTime;};


  vector <float>  getTStripElectronicsAbThr () const { return m_tStripElectronicsAbThr;};
  vector <float>  getQStripElectronics      () const { return m_qStripElectronics;};
  vector <int>    getNStripElectronics      () const { return m_nStripElectronics;};


};

#endif

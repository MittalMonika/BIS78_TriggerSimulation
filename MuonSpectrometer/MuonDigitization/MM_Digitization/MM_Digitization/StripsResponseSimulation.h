/* -*- C++ -*- */

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MM_DIGITIZATION_STRIPRESPONSESIMULATION_H
#define MM_DIGITIZATION_STRIPRESPONSESIMULATION_H
/** @class StripsResponse

// ------------
// Authors:
//   Iakovidis George        <george.iakovidis@cern.ch>
//   Karakostas Konstantinos <konstantinos.karakostas@cern.ch>
//   Leontsinis Stefanos     <stefanos.leontsinis@cern.ch>
//   Nektarios Chr. Benekos  <nbenekos@cern.ch>
//
// Major Contributions From: Verena Martinez
//                           Tomoyuki Saito
//
// Major Restructuring for r21+ From: Lawrence Lee <lawrence.lee.jr@cern.ch>
//
//////////////////////////////////////////////////////////////////////////////

Comments to be added here...

*/

#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/Service.h"
#include "AthenaKernel/MsgStreamMember.h"
#include "GaudiKernel/StatusCode.h"

/// ROOT
#include <TROOT.h>
#include <TFile.h>
#include <TF1.h>
#include <TH1.h>
#include <TRandom.h>
#include <TRandom3.h>
#include <TCanvas.h>
#include <TMath.h>

/// Projects
#include "MM_Digitization/MmDigitToolInput.h"
#include "MM_Digitization/MmStripToolOutput.h"
#include "MM_Digitization/MM_IonizationCluster.h"
// #include "MM_Digitization/GarfieldGas.h"

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

using std::vector;
using std::cout;
using std::endl;

/// ROOT Classed
class TF1;
class TRandom;
class TRandom3;

class MmDigitToolInput;
class MmStripToolOutput;


class StripsResponseSimulation {

private:

  /** qThreshold=2e, we accept a good strip if the charge is >=2e */
  float m_qThreshold;


  /** // 0.350/10 diffusSigma=transverse diffusion (350 μm per 1cm ) for 93:7 @ 600 V/cm, according to garfield  */
  float m_transverseDiffusionSigma;
  float m_longitudinalDiffusionSigma;
  float m_pitch;
  /** //pitch=0.500 properties of the micromegas ToDo: to be reviewed */
  /** crosstalk of neighbor strips, it's 15%  */
  float m_crossTalk1;//0.10; //
  /** // crosstalk of second neighbor strips, it's 6% */
  float m_crossTalk2;//0.03;
  /** // (degrees) Magnetic Field 0.5 T */
  // float Lorentz_Angle;

  float m_driftGapWidth;

  /** //0.050 drift velocity in [mm/ns], driftGap=5 mm +0.128 mm (the amplification gap) */
  float m_driftVelocity;


  // Avalanche gain
  float m_avalancheGain;
  int m_maxPrimaryIons;

  float m_interactionDensityMean;
  float m_interactionDensitySigma;

  /// ToDo: random number from custom functions
  TF1 *m_polyaFunction;
  // TF1 *conv_gaus;
  TF1 *m_lorentzAngleFunction;
  TF1 *m_longitudinalDiffusionFunction;
  TF1 *m_transverseDiffusionFunction;
  TF1 *m_interactionDensityFunction;

  // GarfieldGas* gas;

  StripsResponseSimulation & operator=(const StripsResponseSimulation &right);
  StripsResponseSimulation(const StripsResponseSimulation&);

  std::vector<MM_IonizationCluster> IonizationClusters;

public :


  StripsResponseSimulation();

  virtual ~StripsResponseSimulation();
  MmStripToolOutput GetResponseFrom(const MmDigitToolInput & digiInput);

  void initialize ();
  void writeHistos();
  void initHistos ();
  void clearValues ();
  void initFunctions ();
  void whichStrips(const float & hitx, const int & stripOffest, const float & thetaDegrees, const int & stripMaxID, const MmDigitToolInput & digiInput);

  inline void set_qThreshold (float val) { m_qThreshold = val; };
  inline void set_transverseDiffusionSigma (float val) { m_transverseDiffusionSigma = val; };
  inline void set_longitudinalDiffusionSigma (float val) { m_longitudinalDiffusionSigma = val; };
  inline void set_driftVelocity (float val) { m_driftVelocity = val; };
  inline void set_crossTalk1 (float val) { m_crossTalk1 = val; };
  inline void set_crossTalk2 (float val) { m_crossTalk2 = val; };
  inline void set_driftGapWidth (float val) {m_driftGapWidth = val;};

  // inline void set_outputFile( TFile& m_file ){  m_outputFile = &m_file; };

  float get_qThreshold    () const { return m_qThreshold;      };
  float get_driftGapWidth () const { return m_driftGapWidth;   };
  float get_driftVelocity () const { return m_driftVelocity;   };

  vector <float> get_tStripElectronicsAbThr() const { return tStripElectronicsAbThr;};
  vector <float> get_qStripElectronics() const { return qStripElectronics;};
  vector <float> get_finaltStripNoSlewing() const { return finaltStripNoSlewing;};
  vector < vector <float> > get_finalqStrip() const { return finalqStrip;};
  vector < vector <float> > get_finaltStrip() const { return finaltStrip;};
  vector <int>   get_nStripElectronics() const { return nStripElectronics;};
  vector <int>   get_finalNumberofStrip() const { return finalNumberofStrip;};

  vector <int>   finalNumberofStrip;
  vector <int>   nStripElectronics;
  vector < vector <float> > finalqStrip;
  vector < vector <float> > finaltStrip;
  vector <float> finaltStripNoSlewing;
  vector <float> tStripElectronicsAbThr;
  vector <float> qStripElectronics;

  vector <int> stripNumber;
  vector <int> firstq;
  vector <float> qstrip;
  vector <float> cntTimes;
  vector <float> tStrip;
  vector <float> qStrip;
  vector <float> time;  //Drift velocity [mm/ns]
  vector <int> numberofStrip;

  vector <float> clusterelectrons;
  vector <float> l;


private:

  std::map<TString, TH1F* > m_mapOfHistograms;

  TRandom3 * m_random;


  bool m_writeOutputFile;
  TFile * m_outputFile;


};
#endif

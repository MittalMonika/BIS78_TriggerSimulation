/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
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
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GaudiKernel/StatusCode.h"

/// ROOT
#include <TROOT.h>
#include <TFile.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TRandom.h>
#include <TRandom3.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TGraph.h>

/// Projects
#include "MM_Digitization/MM_DigitToolInput.h"
#include "MM_Digitization/MM_StripToolOutput.h"
#include "MM_Digitization/MM_IonizationCluster.h"

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

class MM_DigitToolInput;
class MM_StripToolOutput;

class MM_StripsResponseSimulation {

public :


  MM_StripsResponseSimulation();

  virtual ~MM_StripsResponseSimulation();
  MM_StripToolOutput GetResponseFrom(const MM_DigitToolInput & digiInput);

  void initialize ();
  void writeHistos();
  void initHistos ();
  void clearValues ();
  void initFunctions ();
  void whichStrips(const float & hitx, const int & stripOffest, const float & incidentAngleXZ, const float & incidentAngleYZ, const int & stripMinID, const int & stripMaxID, const MM_DigitToolInput & digiInput);

  inline void setQThreshold (float val) { m_qThreshold = val; };
  inline void setTransverseDiffusionSigma (float val) { m_transverseDiffusionSigma = val; };
  inline void setLongitudinalDiffusionSigma (float val) { m_longitudinalDiffusionSigma = val; };
  inline void setDriftVelocity (float val) { m_driftVelocity = val; };
  inline void setCrossTalk1 (float val) { m_crossTalk1 = val; };
  inline void setCrossTalk2 (float val) { m_crossTalk2 = val; };
  inline void setDriftGapWidth (float val) {m_driftGapWidth = val;};
  inline void setAvalancheGain(float val) {m_avalancheGain = val;}
  inline void setInteractionDensityMean(float val) {m_interactionDensityMean = val;}
  inline void setInteractionDensitySigma(float val) {m_interactionDensitySigma = val;}
  inline void setLorentzAngleFunction(TF1* f) {m_lorentzAngleFunction = f;}

  float getQThreshold    () const { return m_qThreshold;      };
  float getDriftGapWidth () const { return m_driftGapWidth;   };
  float getDriftVelocity () const { return m_driftVelocity;   };
  float getInteractionDensityMean () const { return m_interactionDensityMean;}
  float getInteractionDensitySigma () const { return m_interactionDensitySigma;}
  float getLongitudinalDiffusionSigma () const { return m_longitudinalDiffusionSigma;}
  float getTransversDiffusionSigma () const { return m_transverseDiffusionSigma;}
  TF1* getLorentzAngleFunction () const { return m_lorentzAngleFunction;}

  vector <float> getTStripElectronicsAbThr() const { return m_tStripElectronicsAbThr;};
  vector <float> getQStripElectronics() const { return m_qStripElectronics;};
  vector <float> getFinaltStripNoSlewing() const { return m_finaltStripNoSlewing;};
  vector < vector <float> > getFinalqStrip() const { return m_finalqStrip;};
  vector < vector <float> > getFinaltStrip() const { return m_finaltStrip;};
  vector <int>   getNStripElectronics() const { return m_nStripElectronics;};
  vector <int>   getFinalNumberofStrip() const { return m_finalNumberofStrip;};

  //Declaring the Message method for further use
  MsgStream& msg(const MSG::Level lvl) const { return m_msg << lvl ; }
  bool msgLvl(const MSG::Level lvl) const { return m_msg.get().level() <= lvl ; }
  void setMessageLevel(const MSG::Level lvl) const { m_msg.get().setLevel(lvl); return; }

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

  float m_driftGapWidth;

  /** //0.050 drift velocity in [mm/ns], driftGap=5 mm +0.128 mm (the amplification gap) */
  float m_driftVelocity;


  // Avalanche gain
  float m_avalancheGain;
  int m_maxPrimaryIons;

  float m_interactionDensityMean;
  float m_interactionDensitySigma;

  vector <int>   m_finalNumberofStrip;
  vector <int>   m_nStripElectronics;
  vector < vector <float> > m_finalqStrip;
  vector < vector <float> > m_finaltStrip;
  vector <float> m_finaltStripNoSlewing;
  vector <float> m_tStripElectronicsAbThr;
  vector <float> m_qStripElectronics;

  vector <int> m_stripNumber;
  vector <int> m_firstq;
  vector <float> m_qstrip;
  vector <float> m_cntTimes;
  vector <float> m_tStrip;
  vector <float> m_qStrip;
  vector <float> m_time;  //Drift velocity [mm/ns]
  vector <int> m_numberofStrip;

  vector <float> m_clusterelectrons;
  vector <float> m_l;

  /// ToDo: random number from custom functions
  TF1 *m_polyaFunction;
  TF1 *m_lorentzAngleFunction;
  TF1 *m_longitudinalDiffusionFunction;
  TF1 *m_transverseDiffusionFunction;
  TF1 *m_interactionDensityFunction;

  MM_StripsResponseSimulation & operator=(const MM_StripsResponseSimulation &right);
  MM_StripsResponseSimulation(const MM_StripsResponseSimulation&);

  std::vector<MM_IonizationCluster> m_IonizationClusters;

  std::map<TString, TH1F* > m_mapOfHistograms;
  std::map<TString, TH2F* > m_mapOf2DHistograms;

  TRandom3 * m_random;


  bool m_writeOutputFile;
  bool m_writeEventDisplays;
  TFile * m_outputFile;

 protected:
  //Declaring private message stream member.
  mutable Athena::MsgStreamMember m_msg = Athena::MsgStreamMember("MMStripResponseSimulation");

};
#endif

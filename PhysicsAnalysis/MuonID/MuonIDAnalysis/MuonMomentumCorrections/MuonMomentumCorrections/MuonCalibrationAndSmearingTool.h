/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCAST_MUONCALIBRATIONANDMEARINGTOOL_H
#define MCAST_MUONCALIBRATIONANDMEARINGTOOL_H

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "StoreGate/ReadHandleKey.h"

//EDM include(s)
#include "xAODEventInfo/EventInfo.h"

// ROOT include(s)
#include "TRandom3.h"
#include "TH3F.h"
#include "TProfile2D.h"
#include "TFile.h"

// C++ include(s)
#include <boost/unordered_map.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <map>

// Local include(s):
#include "MuonMomentumCorrections/IMuonCalibrationAndSmearingTool.h"

#define EPSILON 1.0E-6
#define DEFAULT_INIT_VAL -999
#define MCAST_MAX_PT 100000000
#define MZPDG 91.1876 

namespace CP {

namespace MCAST {

  namespace DataType { enum { Data10 = 1, Data11 = 2, Data12 = 3, Data15 = 4, Data16=5}; }
  namespace AlgoType { enum { Muid = 1, Staco = 2, Muons = 3 }; }
  namespace Release { enum { Rel16_6 = 1, Rel17 = 2, Rel17_2 = 3, Rel17_2_Repro = 4, Rel17_2_Sum13 = 5, PreRec = 6, PreRec_2015_06_22  = 7, PreRec_2015_08_06  = 8, Rec_2015_11_15 = 9, Rec_2016_01_13 = 10, Rec_2016_01_19 = 11, PreRec_2016_05_23 = 12 , Recs2016_08_07=13 , Recs2016_15_07}; }
  namespace SmearingType { enum { Pt = 1, QoverPt = 2 }; }
  namespace DetectorType { enum { MS = 1, ID = 2, CB = 3 }; }
  namespace SystVariation { enum { Default = 0, Down = -1, Up = 1 }; }
  namespace SagittaCorType { enum { CB=0, ID=1, ME=2, WEIGHTS=3, AUTO=4}; }
  namespace SagittaSysType { enum { NOMINAL=0, RHO=1, BIAS=2}; }

}

class MuonCalibrationAndSmearingTool : public virtual IMuonCalibrationAndSmearingTool, public virtual ISystematicsTool, public asg::AsgTool {

  //::: Create a proper constructor for Athena
  ASG_TOOL_CLASS2( MuonCalibrationAndSmearingTool, CP::IMuonCalibrationAndSmearingTool, CP::ISystematicsTool )


public:

  //::: Constructor
  MuonCalibrationAndSmearingTool( const std::string& name );
  
  //::: Copy constructor
  MuonCalibrationAndSmearingTool( const MuonCalibrationAndSmearingTool& tool );

  //::: Destructor
  virtual ~MuonCalibrationAndSmearingTool();

  virtual StatusCode initialize() override;

  //::: Apply the correction on a modifyable object
  virtual CorrectionCode applyCorrection( xAOD::Muon& mu ) const override;
  virtual CorrectionCode applyCorrection( xAOD::Muon& mu,
                                          std::unique_ptr<IVars>& vars) const override;
  //::: Create a corrected copy from a constant muon
  virtual CorrectionCode correctedCopy( const xAOD::Muon& input, xAOD::Muon*& output ) const override;

  //::: Is the tool affected by a specific systematic?
  virtual bool isAffectedBySystematic( const SystematicVariation& systematic ) const override;
  //::: Which systematics have an effect on the tool's behaviour?
  virtual SystematicSet affectingSystematics() const override;
  //::: Systematics to be used for physics analysis
  virtual SystematicSet recommendedSystematics() const override;
  //::: Use specific systematic
  virtual SystematicCode applySystematicVariation ( const SystematicSet& systConfig ) override;
  virtual double expectedResolution( const IVars& vars, const std::string& DetType, const xAOD::Muon& mu, const bool mc = false ) const override; //!< Expected resolution in data (or unsmeard MC if second argument is true)
  virtual double expectedResolution( const IVars& vars, const int DetType, const xAOD::Muon& mu, const bool mc = false ) const override; //!< Expected resolution in data (or unsmeard MC if second argument is true)


  double ExpectedResolution( const IVars& ivars,
                             const std::string& DetType, const xAOD::Muon& mu, const bool mc = false ) const; //!< Expected resolution in data (or unsmeard MC if second argument is true)
  double ExpectedResolution( const IVars& ivars,
                             const int DetType, const xAOD::Muon& mu, const bool mc = false ) const; //!< Expected resolution in data (or unsmeard MC if second argument is true)

 
  virtual void setUseStatCombination(bool flag) override;
  

private:
  struct Vars : public IVars
  {
    double m_ptms = 0;
    double m_ptid = 0;
    double m_ptcb = 0;
    double m_weightID = 0;
    double m_weightMS = 0;
    double m_charge = 0;
    double m_eta = 0;
    double m_smearDeltaMS = 0;
    double m_smearDeltaID = 0;
    double m_smearDeltaCB = 0;
    int m_detRegion = -1;
    double m_g0 = 0;
    double m_g1 = 0;
    double m_g2 = 0;
    double m_g3 = 0;
    double m_g4 = 0;
  };

  CorrectionCode applyStatCombination( const ElementLink< xAOD::TrackParticleContainer >& inDetTrackParticle, 
                                       const ElementLink< xAOD::TrackParticleContainer >& extrTrackParticle ,
                                       int charge,
                                       AmgVector(5)& parsCB,
                                       AmgSymMatrix(5)& covCB,
                                       double& chi2) const;
  CorrectionCode applyStatCombination( xAOD::Muon& mu ) const;

  CorrectionCode applySagittaBiasCorrectionAuto(Vars& vars,
                                                const int DetType,const xAOD::Muon& mu,bool isMC=false,const unsigned int SytCase=0) const;
 
   CorrectionCode CorrectForCharge(double p2,double& pt,int q=0, bool isMC=false) const;
  CorrectionCode applyiSagittaBiasCorrection(Vars& vars,
                                             const unsigned int SgCorrType, const xAOD::Muon& mu,unsigned int iter=0,bool stop=false,bool isMC=false) const;
 

  //::: Regions helpers 
  StatusCode   Regions( std::string inRegionFile, int doMacroRegionsFlag = 0 );
  void         PrintRegions() const;
  unsigned int GetNRegions() const;
  int          GetRegion( const double eta, const double phi ) const;
  float        GetRegionInnerEta( const int r_i ) const; //Return Eta closer to the origin
  std::string  GetRegionName( const int r_i ) const;
  std::string  GetRegionName( const double eta, const double phi ) const;
  double GetSmearing( const Vars& vars, int, const xAOD::Muon& ) const;
  double GetSystVariation( const Vars& vars, int DetType, double var ) const;
  int GetScaleRegion( const xAOD::Muon& ) const;
  void CalcCBWeights( Vars& vars, const xAOD::Muon& ) const;
  double CalculatePt( const Vars& vars,
                      const int DetType, const double inSmearID, const double inSmearMS, const double scaleVar ) const;
  StatusCode FillValues();
  void Clean();
  double ScaleApply( const int detRegion,
                     const double pt, double S = 1.0, const double S_EnLoss = 0. ) const;
  //double ScaleApply( const double pt, const double S1, const double S2, double S = 1.0, const double S_EnLoss = 0. ) const;
  void CleanScales();
  void CollectMacroRegionsSL();//Small and large regions are collected together
  void CollectMacroRegionsSL_UpDn();//Small,Large,Up,Down regions are collected together
  void CollectMacroRegionsSL_SplitBAR();//Large,Small sectors split plus Feet(12+14) and 11+15 sector split in Barrel
  void CollectSectors();

  StatusCode SetData( const std::string& );
  StatusCode SetAlgorithm( const std::string& );
  StatusCode SetRelease( const std::string& );
  StatusCode SetType( const std::string& );
  
  unsigned int setSagittaHistogramsSingle(TProfile2D *pCB=NULL,unsigned int track=0);
  double  sagitta(TProfile2D* corrM, TLorentzVector &lv) const;
  
  void ConvertToSagittaBias(TH2F *h,float mean=1) const;
  TProfile2D* GetHist(const std::string &fname="", const std::string &hname="inclusive",double GlobalScale=MZPDG) const;

  //private:
  //::: fake assignment operator missing actual implementation
  MuonCalibrationAndSmearingTool& operator=(const MuonCalibrationAndSmearingTool& )= delete;

  struct ParameterSet { 
    double SmearTypeID; 
    double SmearTypeMS; 
    double Scale; 
    double SagittaRho;
    double SagittaBias;
  };
  std::string m_year, m_algo, m_type, m_release;
  std::string m_FilesPath;
  bool m_toroidOff;
  int m_Tsmear;
  //int m_Tdet;
  int m_Tdata;
  int m_Trel;
  int m_Talgo;
  double m_useNsigmaForICombine;
  std::vector<double> m_scale_ID, m_enLoss_MS, m_scale_MS, m_scale_CB;
  //sys variations (stat error added in quadrature), one if it's simmetrized, 2 if Up != Dw.
  std::vector<double> m_scaleSyst_ID, m_enLossSyst_MS, m_scaleSyst_MS, m_scaleSyst_CB;
  std::vector<double> m_scaleSystUp_ID, m_enLossSystUp_MS, m_scaleSystUp_MS;
  std::vector<double> m_scaleSystDw_ID, m_enLossSystDw_MS, m_scaleSystDw_MS;

  std::vector<double> m_scaleBins;
  std::vector<double> m_p1_ID, m_p2_ID, m_p2_ID_TAN, m_p0_MS, m_p1_MS, m_p2_MS;
  std::vector<double> m_E_p1_ID, m_E_p2_ID, m_E_p2_ID_TAN, m_E_p0_MS, m_E_p1_MS, m_E_p2_MS;
  // syst. errors on resolution parameters corrections:
  //one if it's simmetrized, then Stat and Sys err are separate in cfg file.
  std::vector<double> m_S_p1_ID, m_S_p2_ID, m_S_p2_ID_TAN, m_S_p0_MS, m_S_p1_MS, m_S_p2_MS;
  //Two if Up != Dw, Stat and Sys err added in quadrature in cfg file.
  std::vector<double> m_SUp_p1_ID, m_SUp_p2_ID, m_SUp_p2_ID_TAN, m_SUp_p0_MS, m_SUp_p1_MS, m_SUp_p2_MS;
  std::vector<double> m_SDw_p1_ID, m_SDw_p2_ID, m_SDw_p2_ID_TAN, m_SDw_p0_MS, m_SDw_p1_MS, m_SDw_p2_MS;
  std::vector<double> m_MC_p1_ID, m_MC_p2_ID, m_MC_p2_ID_TAN, m_MC_p0_MS, m_MC_p1_MS, m_MC_p2_MS;

  std::vector<std::string> m_names;
  bool m_loadNames;
  int m_nb_regions;
  std::vector<float> m_eta_min, m_eta_max, m_phi_min, m_phi_max;
  
  bool m_doMacroRegions;
  std::map< int, int > m_MacroRegionIdxMap;
  std::vector< std::string > m_MacroRegionName;
  std::vector< double > m_MacroRegionInnerEta;

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this,"EventInfo","EventInfo","EventInfo"}; 
  
  boost::unordered_map< SystematicSet, ParameterSet > m_Parameters;
  ParameterSet *m_currentParameters;
  
  double m_StatCombPtThreshold;
  bool m_useStatComb;
  
  std::vector <TProfile2D*> *m_sagittasCB;
  std::vector <TProfile2D*> *m_sagittasID;
  std::vector <TProfile2D*> *m_sagittasME;

  bool m_doSagittaCorrection;
  std::vector <double> m_GlobalZScales;
  std::vector <unsigned int > m_SagittaIterations;
  

}; //::: class MuonCalibrationAndSmearingTool

} //::: namespace CP

#endif

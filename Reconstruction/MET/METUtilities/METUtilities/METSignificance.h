///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// METSignificance.h
// Header file for class METSignificance
// Author: P. Francavilla<francav@cern.ch>
// Author: D. Schaefer <schae@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef METUTILITIES_MET_METSIGNIFICANCE_H
#define METUTILITIES_MET_METSIGNIFICANCE_H 1

// STL includes
#include <string>
#include <tuple>
#include "TH2F.h"
#include "TFile.h"
//#include <TMatrixD.h>

// FrameWork includes
#include "AsgTools/AnaToolHandle.h"
#include "AsgTools/AsgTool.h"

// METInterface includes
#include "METInterface/IMETSignificance.h"

// EDM includes
#include "xAODJet/JetContainer.h"

// Tracking Tool
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

// Tool interfaces
class IJERTool;
namespace CP {
  class IMuonCalibrationAndSmearingTool;
  class IEgammaCalibrationAndSmearingTool;
}
class ITauToolBase;

// Forward declaration
namespace met {

  using xAOD::IParticle;

  // typedefs
  typedef ElementLink<xAOD::IParticleContainer> obj_link_t;

  class METSignificance
  : public asg::AsgTool,
  virtual public IMETSignificance

  {
    // This macro defines the constructor with the interface declaration
    ASG_TOOL_CLASS(METSignificance, IMETSignificance)

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:

    // Copy constructor:

    /// Constructor with parameters:
    METSignificance(const std::string& name);

    /// Destructor:
    virtual ~METSignificance();

    // Athena algtool's Hooks
    StatusCode  initialize();
    StatusCode  finalize();
      
    StatusCode varianceMET(xAOD::MissingETContainer* metCont, std::string jetTermName, std::string softTermName, std::string totalMETName);

    StatusCode RotateToPhi(float phi);

    ///////////////////////////////////////////////////////////////////
    // Const methods:
    ///////////////////////////////////////////////////////////////////
    double GetMETOverSqrtSumET() const { if(m_sumet>0.0)        return (m_met/sqrt(m_sumet)); return -1.0; } 
    double GetMETOverSqrtHT   () const { if(m_ht>0.0)           return (m_met/sqrt(m_ht));    return -1.0; } 
    double GetSignificance()     const { if(m_significance>0.0) return sqrt(m_significance);  return -1.0; }
    double GetSigDirectional()   const { if(m_VarL>0.0)         return m_met/sqrt(m_VarL);    return -1.0; }
    double GetRho()              const { return m_rho;  }
    double GetVarL()             const { return m_VarL; }
    double GetVarT()             const { return m_VarT; }

    ///////////////////////////////////////////////////////////////////
    // Non-const methods:
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // Private data:
    ///////////////////////////////////////////////////////////////////
  private:

    /// Default constructor:
    METSignificance();

    // tools
    asg::AnaToolHandle<IJERTool> m_jerTool;
    asg::AnaToolHandle<CP::IMuonCalibrationAndSmearingTool>   m_muonCalibrationAndSmearingTool;
    asg::AnaToolHandle<CP::IEgammaCalibrationAndSmearingTool> m_egammaCalibTool;
    asg::AnaToolHandle<ITauToolBase>                          m_tCombinedP4FromRecoTaus;

    StatusCode AddMuon    (const xAOD::IParticle* obj, float &pt_reso, float &phi_reso);
    void AddElectron(const xAOD::IParticle* obj, float &pt_reso, float &phi_reso);
    void AddPhoton  (const xAOD::IParticle* obj, float &pt_reso, float &phi_reso);
    void AddJet     (const xAOD::IParticle* obj, float &pt_reso, float &phi_reso);
    void AddTau     (const xAOD::IParticle* obj, float &pt_reso, float &phi_reso);
    void AddSoftTerm(const xAOD::MissingET* soft,  const TVector3 &met_vect, double (&particle_sum)[2][2]);

    double GetPUProb(double jet_eta, double jet_phi,double jet_pt,  double jet_jvt);
    double GetPhiUnc(double jet_eta, double jet_phi,double jet_pt);

    std::tuple<double,double,double> CovMatrixRotation(double var_x, double var_y, double cv_xy, double Phi);

    double Significance_LT(double Numerator, double var_parall, double var_perpen, double cov);


    void InvertMatrix(double (&mat)[2][2], double (&m)[2][2]);
    void AddMatrix(double (&X)[2][2],double (&Y)[2][2], double (&mat_new)[2][2]);
    void RotateXY(const double (&mat)[2][2], double (&mat_new)[2][2], double phi);

    // soft term bias
    double BiasPtSoftdir(const double PtSoft);
    double VarparPtSoftdir(const double PtSoft, const double SoftSumet);

    // pthard - parameterization
    //double BiasPtSoftdir(const double PtSoft);
    //double VarparPtSoftdir(const double PtSoft, const double SoftSumet);

    // variables
    double m_GeV;

    int    m_softTermParam;
    double m_softTermReso;
    bool   m_treatPUJets;
    bool   m_doPhiReso;
    bool   m_applyBias;

    bool m_isData;
    bool m_isAFII;

    double m_significance;
    double m_rho;
    double m_VarL;
    double m_VarT;
    double m_CvLT;

    double m_met_VarL;
    double m_met_VarT;
    double m_met_CvLT;

    double m_met;
    double m_metphi;
    double m_metsoft;
    double m_metsoftphi;
    double m_ht;
    double m_sumet;

    // Jet Uncertainties
    TFile *m_file;
    TH2F *h_phi_reso_pt20;
    TH2F *h_phi_reso_pt50;
    TH2F *h_phi_reso_pt100;

    std::string m_configPrefix;
    std::string m_configJetPhiResoFile;
  };

} //> end namespace met
#endif //> !METUTILITIES_MET_METSIGNIFICANCE_H

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGTOOLS_MV2TAG_H
#define JETTAGTOOLS_MV2TAG_H

/******************************************************
    @class MV2Tag
    Neural-net tagger combining weights of IP3D, SV1 and
    JetFitterCombNN
    @ author L. Vacavant
********************************************************/

#include "AthenaBaseComps/AthAlgTool.h"
#include "JetTagTools/IMultivariateJetTagger.h"
#include <string>
#include <map>
#include <list>
#include "MVAUtils/BDT.h"
#include "JetTagCalibration/JetTagCalibCondData.h"

namespace Analysis {

  class MV2Tag : public AthAlgTool, public IMultivariateJetTagger
  {
  public:
    MV2Tag(const std::string& name,
       const std::string& n,
       const IInterface*);

    /**
       Implementations of the methods defined in the abstract base class
    */
    virtual ~MV2Tag();
    StatusCode initialize();
    StatusCode finalize();
    void finalizeHistos() {};

    void assignProbability(xAOD::BTagging* BTag, const std::map<std::string,double>& inputs, const std::string& jetauthor);

  private:

    std::string m_taggerName;
    std::string m_taggerNameBase; // unique name for regular and flip versions
    std::string m_treeName;
    std::string m_varStrName;

    /** Key of calibration data: */
    SG::ReadCondHandleKey<JetTagCalibCondData> m_readKey{this, "HistosKey", "JetTagCalibHistosKey", "Key of input (derived) JetTag calibration data"};
    bool m_forceMV2CalibrationAlias;
    std::string m_decTagName;
    std::string m_MV2CalibAlias;
    std::string m_MV2cXX;
    std::string m_xAODBaseName;

    std::map<std::string, double > m_defaultvals;
    std::map<std::string, float* > m_local_inputvals;
    std::map<std::string, std::string > m_MVTM_name_tranlations;


    const unsigned m_nClasses=3;//b,u,c probabilities. It might be better to read from calib file for future
    //const bool m_writeRootFile=false;//Developer option
    bool m_disableAlgo;
    int  m_warnCounter;

    /** This switch is needed to indicate what to do. The algorithm can be run to produce
	reference histograms from the given MC files (m_runModus=0) or to work in analysis mode
	(m_runModus=1) where already made reference histograms are read.*/
    std::string    m_runModus;          //!< 0=Do not read histos, 1=Read referece histos (analysis mode)

    /** Storage for the primary vertex. Can be removed when JetTag provides origin(). */
    // this pointer does not need to be deleted in the destructor (because it
    // points to something in storegate)
    //const xAOD::Vertex* m_priVtx;

    /** reader to define the MVA algorithms */
    std::map<std::string, const MVAUtils::BDT*> m_egammaBDTs;
    std::list<std::string> m_undefinedReaders; // keep track of undefined readers to prevent too many warnings.


    std::string m_ip2d_infosource;
    std::string m_ip3d_infosource;
    std::string m_sv1_infosource;
    std::string m_sv0_infosource;
    std::string m_jftNN_infosource;
    std::string m_jfprob_infosource;
    std::string m_softmuon_infosource;
    std::string m_trainingConfig;

    float d0sgn_wrtJet(const TLorentzVector& jet, const TLorentzVector& trk, float d0sig);
    float z0sgn_wrtJet(float trackTheta, float trackZ0, float jetEta);
    //void setInputVariables(xAOD::Jet& jetToTag, xAOD::BTagging* BTag);//for future
    //void ClearInputs();
    //void PrintInputs();
    void CreateLocalVariables(std::map<std::string, double> var_map);
    void ReplaceNaN_andAssign(std::map<std::string, double> var_map);
    void SetVariableRefs(const std::vector<std::string> inputVars,
			  unsigned &nConfgVar, bool &badVariableFound, std::vector<float*> &inputPointers);

    std::vector<float> GetMulticlassResponse(const MVAUtils::BDT* bdt) const {
      std::vector<float> v(m_nClasses,-1);
      return (bdt->GetPointers().size() ? bdt->GetMultiResponse(bdt->GetPointers(),m_nClasses) : v);
    }
    double GetClassResponse (const MVAUtils::BDT* bdt) const { return (bdt->GetPointers().size() ? bdt->GetGradBoostMVA(bdt->GetPointers()) : -9.); }
  }; // End class


  //inline void MultivariateTagManager::setOrigin(const xAOD::Vertex* priVtx) { m_priVtx=priVtx; }

} // End namespace

#endif

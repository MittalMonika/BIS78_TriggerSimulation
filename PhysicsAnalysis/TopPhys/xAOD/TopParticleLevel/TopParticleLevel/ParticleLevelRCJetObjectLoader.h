/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//Load the reclustering tool for the collection of truth jets already selected.
//Create the collection of reclustered jets and save them in the evtStore().
//There is also the pass selection to apply the cut on eta and pt on the reclustered jets.

#ifndef ParticleLevelRCJetObjectLoader_H_
#define ParticleLevelRCJetObjectLoader_H_

#include "TopParticleLevel/ObjectSelectorBase.h"
#include "AsgTools/AsgTool.h"
#include "xAODCore/ShallowCopy.h"

#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/TActiveStore.h"

#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "JetReclustering/JetReclusteringTool.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <set>
#include <list>
#include <vector>


namespace top{
    class ParticleLevelEvent;
    class TopConfig;
}

namespace fastjet{
  class JetDefinition;
  namespace contrib{
    class Nsubjettiness;
    class EnergyCorrelator;
  }
}
namespace JetSubStructureUtils{
  class KtSplittingScale;
  class Qw;
  class EnergyCorrelatorGeneralized; 
}


class ParticleLevelRCJetObjectLoader: public asg::AsgTool {
public:
    
    
    //Default - so root can load based on a name
    ParticleLevelRCJetObjectLoader( const std::shared_ptr<top::TopConfig> & cfg ) ;
   

    //Default - so we can clean up
    ~ParticleLevelRCJetObjectLoader();

    //Run once at the start of the job
    StatusCode initialize();

    //Run for every event (in every systematic) that needs saving
    StatusCode execute(const top::ParticleLevelEvent& plEvent);


    //Clean-up remaining things
    StatusCode finalize();

    bool passSelection(const xAOD::Jet& jet) const;
    
    //Access all of the container names from the Event Saver
     std::string inputContainerName(){return m_InputJetContainer; };
     std::string rcjetContainerName(){return m_OutputJetContainer; };
    
private:

    std::string m_name;
    const std::shared_ptr<top::TopConfig> & m_config;

    bool m_VarRCjets;
    std::string m_VarRCjets_rho;
    std::string m_VarRCjets_mass_scale;

    float m_ptcut;       // in GeV
    float m_etamax;
    float m_trim;
    float m_radius;
    float m_minradius;
    float m_massscale;
  
    bool  m_useJSS;
    bool  m_useAdditionalJSS;

    std::string m_treeName;

    std::string m_InJetContainerBase;
    std::string m_OutJetContainerBase;
    std::string m_InputJetContainer;
    std::string m_OutputJetContainer;
   

    

    //Substructure tool definitions
    std::shared_ptr<fastjet::JetDefinition> m_jet_def_rebuild; 	  
    std::shared_ptr<fastjet::contrib::Nsubjettiness> m_nSub1_beta1;
    std::shared_ptr<fastjet::contrib::Nsubjettiness> m_nSub2_beta1;
    std::shared_ptr<fastjet::contrib::Nsubjettiness> m_nSub3_beta1;
    std::shared_ptr<fastjet::contrib::EnergyCorrelator> m_ECF1;
    std::shared_ptr<fastjet::contrib::EnergyCorrelator> m_ECF2;
    std::shared_ptr<fastjet::contrib::EnergyCorrelator> m_ECF3;
    std::shared_ptr<JetSubStructureUtils::KtSplittingScale> m_split12;
    std::shared_ptr<JetSubStructureUtils::KtSplittingScale> m_split23;
    std::shared_ptr<JetSubStructureUtils::Qw> m_qw;
    std::shared_ptr<JetSubStructureUtils::EnergyCorrelatorGeneralized> m_gECF332;
    std::shared_ptr<JetSubStructureUtils::EnergyCorrelatorGeneralized> m_gECF461;
    std::shared_ptr<JetSubStructureUtils::EnergyCorrelatorGeneralized> m_gECF322;
    std::shared_ptr<JetSubStructureUtils::EnergyCorrelatorGeneralized> m_gECF331;
    std::shared_ptr<JetSubStructureUtils::EnergyCorrelatorGeneralized> m_gECF422;
    std::shared_ptr<JetSubStructureUtils::EnergyCorrelatorGeneralized> m_gECF441;
    std::shared_ptr<JetSubStructureUtils::EnergyCorrelatorGeneralized> m_gECF212;
    std::shared_ptr<JetSubStructureUtils::EnergyCorrelatorGeneralized> m_gECF321;
    std::shared_ptr<JetSubStructureUtils::EnergyCorrelatorGeneralized> m_gECF311;

    std::map<std::string,float> mass_scales = {
        {"m_t",172500.},
        {"m_w",80385.},
        {"m_z",91188.},
        {"m_h",125090.}};


    //re-clustered jets
    //  -> need unordered map for systematics
    std::shared_ptr<JetReclusteringTool> m_jetReclusteringTool;
};

#endif

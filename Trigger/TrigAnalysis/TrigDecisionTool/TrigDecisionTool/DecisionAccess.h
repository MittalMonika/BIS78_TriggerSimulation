// -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGER_DECISION_TOOL_DecisionAccess_H
#define TRIGGER_DECISION_TOOL_DecisionAccess_H

/**********************************************************************************
 * @Project: TrigDecisionTool
 * @Package: TrigDecisionTool
 * @class  : DecisionAccess
 *
 * @brief access to information about the trigger decision
 *
 * @author Michael Begel   <michael.begel@cern.ch>  - Brookhaven National Laboratory
 * @author Nicolas Berger  <Nicolas.Berger@cern.ch> - LAPP Annecy
 * @author Tomasz Bold     <Tomasz.Bold@cern.ch>    - UC Irvine - AGH Krakow
 *
 ***********************************************************************************/
#include <string>
#include <boost/algorithm/string/predicate.hpp>

#include "TrigNavStructure/TriggerElement.h"
#include "TrigDecisionTool/Logger.h"
#include "TrigDecisionTool/Conditions.h"
#include "TrigDecisionTool/Combination.h"
#include "TrigDecisionTool/Feature.h"
#include "TrigDecisionTool/FeatureContainer.h"
#include "TrigDecisionTool/ChainGroup.h"

namespace HLT {
  class Chain;
}
namespace TrigConf {
  class HLTChain;
  class HLTTriggerElement;
}

class LVL1_ROI;

namespace Trig {

  class CacheGlobalMemory;
   
  class DecisionAccess : public virtual Logger {
  public:      

    DecisionAccess();
    virtual ~DecisionAccess();

    /**
     * @brief true if given group of chains passed
     * @see Trig::ChainGroup
     * @see TrigDefs::Conditions
     **/ 
    bool isPassed(const Trig::ChainGroup* chaingroup, 
                  unsigned int condition = TrigDefs::Physics) const;
    /**
     * @brief true if given chain passed
     * @see Trig::ChainGroup
     * @see TrigDefs::Conditions
     **/ 
    bool isPassed(const std::string& chain, 
                  unsigned int condition) const;

    bool isPassed(const std::string& chain) const;
    /**
     * Return expert-level information about the various trigger flags as a
     * bit mask. Note: this does not accept a conditions flag -- the
     * prescription is always enforceLogicalFlow. The mask can be comprehended
     * using the following constants from TrigDefs::
     * EF_passedRaw
     * EF_passThrough
     * EF_prescaled
     * EF_resurrected
     * L2_passedRaw
     * L2_passThrough
     * L2_prescaled
     * L2_resurrected
     * L1_isPassedAfterPrescale
     * L1_isPassedBeforePrescale
     * L1_isPassedAfterVeto
     **/
    unsigned int isPassedBits(const Trig::ChainGroup* chaingroup) const;
    unsigned int isPassedBits(const std::string& chain) const;
    
    /**
     * @brief get the bunch group code
     * bit X=1: BG X fired (X=0..7)
     * use: bool BGXfired = BGCode & (0x1<<X)
     */
    char getBGCode() const;
    
    /**
     * @brief Runs 1, 2. Returns all features related to given chain group
     **/
    FeatureContainer features(const Trig::ChainGroup* group, 
                              unsigned int condition = TrigDefs::Physics) const;

    /**
     * @brief Runs 1, 2. Returns features related to given chain
     **/
    FeatureContainer features(const std::string& chainName = "EF_.*", 
                              unsigned int condition = TrigDefs::Physics) const;

    /**
     * @brief Runs 3+. Returns all features related to given chain group
     * @param[in] group Chain group to return features for.
     * @param[in] condition Condition requirement. Only TrigDefs::Physics and TrigDefs::includeFailedDecisions are supported.
     * @param[in] container Optional requirement to return only features within the specificed container name. Not checked if not specified. 
     * @param[in] featureCollectionMode For oneFeaturePerLeg, stop exploring each route through the navigation once one matching feature has been found.
     * @param[in] featureName Optional name of element link as saved online. The "feature" link is enforced, others may have been added. 
     * @return Vector of LinkInfo, where each entry wraps an ElementLink to the feature, and the Decision object it came from.
     **/
    template<class CONTAINER>
    std::vector< TrigCompositeUtils::LinkInfo<CONTAINER> > features(const Trig::ChainGroup* group,
                                                                    const unsigned int condition = TrigDefs::Physics,
                                                                    const std::string container = "",
                                                                    const unsigned int featureCollectionMode = TrigDefs::oneFeaturePerLeg,
                                                                    const std::string featureName = "feature") const;

    /**
     * @brief Runs 3+. Returns features related to given chain
     * @param[in] group Chain group to return features for.
     * @param[in] condition Condition requirement. Only TrigDefs::Physics and TrigDefs::includeFailedDecisions are supported.
     * @param[in] container Optional requirement to return only features within the specificed container name. Not checked if not specified. 
     * @param[in] featureCollectionMode For oneFeaturePerLeg, stop exploring each route through the navigation once one matching feature has been found.
     * @param[in] featureName Optional name of element link as saved online. The "feature" link is enforced, others may have been added. 
     * @return Vector of LinkInfo, where each entry wraps an ElementLink to the feature, and the Decision object it came from.
     **/
    template<class CONTAINER>
    std::vector< TrigCompositeUtils::LinkInfo<CONTAINER> > features(const std::string& chainName = "HLT_.*",
                                                                    const unsigned int condition = TrigDefs::Physics,
                                                                    const std::string container = "",
                                                                    const unsigned int featureCollectionMode = TrigDefs::oneFeaturePerLeg,
                                                                    const std::string featureName = "feature") const;

    /**
     * @brief gives back feature matching (by seeding relation)
     * @param te - is trigger element to start with, not that thanks to conversion operators
     * Feature object can be given here as well
     **/
    template<class T> 
    const Feature<T> ancestor(const HLT::TriggerElement* te, std::string label = "") const;

    /**
     * @brief gives back features matching (by seeding relation) - will also flatten
     *        containers if necessary
     * @param te - is trigger element to start with, not that thanks to conversion operators
     * Feature object can be given here as well
     **/
    template<class T>
    const std::vector<Trig::Feature<T> > ancestors(const HLT::TriggerElement* te, std::string label = "", unsigned int condition=TrigDefs::Physics, const std::string& teName = "") const;

  protected:

    friend class Combination;    


  private:

    virtual Trig::CacheGlobalMemory* cgm() const =0; 

    //    template<class T> 
    //    const T* getL1RoI(const std::vector<T>& rois, unsigned int id) const;


    //unsigned int initRoIWord(const HLT::TriggerElement* te) const;

  };
} // End of namespace

#include "DecisionAccess.icc"

#endif

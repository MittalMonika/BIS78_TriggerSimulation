/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TrigHLTJet_JetAttrs.cxx
// PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
//
// AUTHOR: A. Steinhebel
//
// ********************************************************************

#include "GaudiKernel/StatusCode.h"
#include "TrigHLTJetHypo/TrigHLTJetHypo_JetAttrs.h"

#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/conditionsFactory2.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/ConditionsSorter.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/JetAttrsFactory.h"

#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/SingleJetGrouper.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJetAttrsValueInterpreter.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/WidthInterpreter.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/KtDRInterpreter.h"

#include <map>
//#include <utility> //for std::move
#include <memory>

TrigHLTJetHypo_JetAttrs::TrigHLTJetHypo_JetAttrs(const std::string& name,
                   ISvcLocator* pSvcLocator):
TrigHLTJetHypoBase(name, pSvcLocator) {

    declareProperty("momentstr",   m_momentstr);
    declareProperty("E",   m_E);
    declareProperty("has",   m_has);
    declareProperty("jetVars",   m_jetVars);
    declareProperty("limit_mins",   m_limitMins);
    declareProperty("limit_maxs",   m_limitMaxs);
}


TrigHLTJetHypo_JetAttrs::~TrigHLTJetHypo_JetAttrs(){
}


HLT::ErrorCode TrigHLTJetHypo_JetAttrs::hltInitialize()
{
  ATH_MSG_INFO("in initialize()");

  //initialize map (m_conversionMap)
  m_conversionMap["width"] = std::make_unique<WidthInterpreter>();
  m_conversionMap.emplace("ktdr", std::make_unique<KtDRInterpreter>());

  return TrigHLTJetHypoBase::hltInitialize();
}


Conditions TrigHLTJetHypo_JetAttrs::getConditions() const {
    
    std::string match ("yes");

    if(m_jetVars.size() == 0){    
      auto conditions = conditionsFactoryFalse(0.0,1.0);
      std::sort(conditions.begin(), conditions.end(),ConditionsSorter());
      return conditions;
    }
    else{
      for(unsigned int count=0; count<m_has.size(); count++){
          if(m_has[count].compare(match)==0){
            std::pair<double,double> limits = (*m_conversionMap.at(m_jetVars[count%2]))(m_limitMins[count], m_limitMaxs[count]);

            auto conditions = JetAttrsSort(m_jetVars[count%2], limits.first, limits.second);
            std::sort(conditions.begin(), conditions.end(), ConditionsSorter());
            return conditions;
          }
          else{
            auto conditions = conditionsFactoryFalse(0.0,1.0);
            std::sort(conditions.begin(), conditions.end(),ConditionsSorter());
            return conditions;
          }
        }
      }
    //else{
    //  int count = 0;
    //  for(auto& var : m_jetVars){
    //    std::pair<double,double> limits = (*m_conversionMap.at(var))(m_limitMins[count], m_limitMaxs[count]);

    //auto conditions = JetAttrsSort(var, limits.first, limits.second);
    //std::sort(conditions.begin(), conditions.end(), ConditionsSorter());
    //return conditions;

    //    count+=1;
    //    }
    //  }



  //Should never reach this... included for compilation
  auto conditions = conditionsFactoryFalse(0.0,1.0);
  std::sort(conditions.begin(), conditions.end(),ConditionsSorter());
  return conditions;
    
}


 
std::shared_ptr<IJetGrouper> TrigHLTJetHypo_JetAttrs::getJetGrouper() const {
  return std::make_shared<SingleJetGrouper>();
}


bool TrigHLTJetHypo_JetAttrs::checkVals() const {

  if(m_limitMaxs.size() != m_limitMins.size()) {

    ATH_MSG_ERROR(name()
      << ": mismatch between number of limits (lower and upper): "
      << m_limitMins.size() << " "
      << m_limitMaxs.size() << " "
      );
    return false;
  }
  return true;

}

 
   
std::vector<std::shared_ptr<ICleaner>> TrigHLTJetHypo_JetAttrs::getCleaners() const {
  std::vector<std::shared_ptr<ICleaner>> v;
  return v;
} 



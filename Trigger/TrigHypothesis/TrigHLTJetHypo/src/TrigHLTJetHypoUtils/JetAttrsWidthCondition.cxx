/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/JetAttrsWidthCondition.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJet.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/NonexistantVariable.h" //error handling
#include <sstream>
#include <stdexcept>
#include <iostream>


JetAttrsWidthCondition::JetAttrsWidthCondition(
				double limitMin,
				double limitMax
                              ){
  m_limitMin = limitMin;
  m_limitMax = limitMax;
}

bool JetAttrsWidthCondition::isSatisfied(const HypoJetVector& ips) const{

    //for debugging
  //std::cout << "in JetAttrsWidthCondition checking isSatisfied with passed double limit values \n";

  if(ips.size() != 1){
    std::stringstream ss;
    ss << "JetAttrs::isSatisfied must see exactly 1 particles, but received "
       << ips.size()
       << '\n';
    
    throw std::runtime_error(ss.str());
  }

  auto jet = ips[0];
  float widthVar;
  Jet jetStruct;

  //for debugging
  //std::cout << "look at one jet and pull width value \n";
  if (jet -> getAttribute("Width",widthVar)){
    jetStruct.validCondition=true;
    jetStruct.widthVal=widthVar;
    //for debugging
    //std::cout << "Width variable exists, value " << widthVar << "\n";
    }
  else{
    jetStruct.validCondition=false;
    throw NonexistantVariable("JetAttrs condition cannot retrieve variable 'width' - does not exist");
    }

  //for debugging
  //std::cout << "calling function to see if cut passed. Returning " << passWidthCut(jetStruct) << "\n";
  return (passWidthCut(jetStruct)); 

}


bool JetAttrsWidthCondition::passWidthCut(const Jet& jet) const{

    //for debugging
    //std::cout << "checking if width passes cuts \n";
    //std::cout << "jet values: width =  " << jet.widthVal << ", validCondition = "<< jet.validCondition << "\n";
    //std::cout << "limits: limitMin = " << m_limitMin << ", limitMax = "<< m_limitMax << "\n";

  bool result;
  if(not jet.validCondition){return false;}
  (jet.widthVal >= m_limitMin and jet.widthVal<= m_limitMax) ? result=true : result=false;  

  //for debugging
  //std::cout << "Returning result " << result << "\n";

  return result;
}


std::string JetAttrsWidthCondition::toString() const noexcept {
  std::stringstream ss;
  ss << "JetAttrsWidthCondition: "

     << "limit min: "
     << m_limitMin
     << " limit max: "
     << m_limitMax
     <<'\n';

  return ss.str();
}


double JetAttrsWidthCondition::orderingParameter() const noexcept {
  return m_limitMin;
}

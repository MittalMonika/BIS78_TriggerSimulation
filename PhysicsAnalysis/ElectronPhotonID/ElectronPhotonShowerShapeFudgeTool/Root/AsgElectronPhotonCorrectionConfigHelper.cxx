/*
    Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "AsgElectronPhotonCorrectionConfigHelper.h"
#include "AsgTools/AsgMessaging.h"
#include "TEnv.h"
#include <iostream>
#include <sstream>   

namespace AsgConfigHelper{

  template <typename T>
  bool strtof(const std::string& input, T& f){  
    int diff = 0 ;
    std::string tmp = input;
    std::string::size_type first(0);
    std::string::size_type last(0);
    first = ( input.find("#") ) ;
 
    //if we do not find a comment character "#" we are fine
    if (first == std::string::npos) {
      std::istringstream buffer (tmp);
      buffer>>f;
      return true;
    } 
    else {
      //if we have found comment character check if it is inlined between two "#"
      last = (input.find("#",first+1) );
      //if nor error
      if (last == std::string::npos) {
        static const asg::AsgMessaging msg("Egamma::AsgConfigHelper");
	      msg.msg(MSG::WARNING)<<" Improper comment format , inline comment should be enclosed between two #  "<<endmsg;
	      return false;
      }
      //else if between two "#" remove this part 
      diff = last - first ;
      tmp= tmp.erase(first,diff+1);
      std::istringstream buffer (tmp);
      buffer>>f;
      return true;
    }
  }

  template <typename T>  
  std::vector<T> Helper (const std::string& input,  TEnv& env){ 
    std::vector<T> CutVector;    
    std::string env_input(env.GetValue(input.c_str(), ""));
    if (env_input.size() > 0) {
      std::string::size_type end;
      do {
	      end = env_input.find(";");
	      T  myValue(0);
	      if(AsgConfigHelper::strtof(env_input.substr(0,end),myValue)){
	        CutVector.push_back(myValue);
	      }
	      if (end != std::string::npos) {
	        env_input= env_input.substr(end+1);
	      }
      } while (end != std::string::npos);     
    }
    return CutVector;
  }
}

//use the specializations 
std::vector<double> AsgConfigHelper::HelperDouble(const std::string& input,  TEnv& env){
  return AsgConfigHelper::Helper<double> ( input,env);
}
std::vector<float> AsgConfigHelper::HelperFloat(const std::string& input,  TEnv& env){
  return AsgConfigHelper::Helper<float> (input, env);  
}
std::vector<int> AsgConfigHelper::HelperInt(const std::string& input, TEnv& env){
  return AsgConfigHelper::Helper<int> (input, env);  
}
// template does not work for std::string because of the T myValue(0); declaration, so implement it again for std::string
std::vector<std::string> AsgConfigHelper::HelperString(const std::string& input, TEnv& env){
  std::vector<std::string> CutVector;
  std::string env_input(env.GetValue(input.c_str(), ""));
  if (env_input.size() > 0) {
    std::string::size_type end;
    do {
	    end = env_input.find(";");
	    std::string myValue("");
	    if(AsgConfigHelper::strtof(env_input.substr(0,end),myValue)){
	      CutVector.push_back(myValue);
	    }
	    if (end != std::string::npos) {
	      env_input= env_input.substr(end+1);
	    }
    } while (end != std::string::npos);
  }
  return CutVector;
}



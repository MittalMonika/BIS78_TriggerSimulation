/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "./nodeIDPrinter.h"

std::string nodeIDPrinter(const std::string& name,
                          int nodeID,
                          int parentID,
                          bool pass,
                          const std::string& timerStr){

  std::stringstream ss;
  ss << name <<"  node: " << nodeID
     << " parent: " << parentID
     << " pass: " << std::boolalpha << pass << " " <<timerStr << '\n';

  return ss.str();
}


/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_ITRIGJETINFOCOLLECTOR_H
#define TRIGHLTJETHYPO_ITRIGJETINFOCOLLECTOR_H
#include <string>

class ITrigJetHypoInfoCollector{
    
public:
  virtual void collect(const std::string&, const std::string&) = 0;
};
#endif

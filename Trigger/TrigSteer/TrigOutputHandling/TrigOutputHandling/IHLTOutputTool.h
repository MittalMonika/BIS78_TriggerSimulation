/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGOUTPUTHANDLING_IHLTOUTPUTTOOL_H
#define TRIGOUTPUTHANDLING_IHLTOUTPUTTOOL_H 1

#include "GaudiKernel/IAlgTool.h"

/**
 * IHLTOutputTool implementations help in creation HLT output (EDM gaps filling, BS creation, Views merging, etc.)
 * 
 **/

class IHLTOutputTool : virtual public ::IAlgTool
{ 
 public: 
  DeclareInterfaceID(IHLTOutputTool, 1, 0);

  virtual ~IHLTOutputTool(){}

  // method to be called by clients
  virtual StatusCode createOutput(const EventContext& context) const = 0;

}; 

#endif //> !TRIGOUTPUTHANDLING_IHLTOUTPUTTOOL_H

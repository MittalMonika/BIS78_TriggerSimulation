/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLKERNEL_IMETADATATOOL_H
#define ATHENAPOOLKERNEL_IMETADATATOOL_H

/** @file IMetaDataTool.h
 *  @brief This file contains the class definition for the IMetaDataTool class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  $Id: IMetaDataTool.h,v 1.2 2007-07-30 19:06:50 gemmeren Exp $
 **/

#include "GaudiKernel/IAlgTool.h"

/** @class IMetaDataTool
 *  @brief This class provides the interface for MetaDataTools.
 **/
class IMetaDataTool : virtual public IAlgTool {

public: // Non-static members

  /// Function called when a new input file is opened
  virtual StatusCode beginInputFile() = 0;

  /// Function called when the currently open input file got completely
  /// processed
  virtual StatusCode endInputFile() = 0;

  /// Function called when the tool should write out its metadata
  virtual StatusCode metaDataStop() = 0;

  /// Gaudi boilerplate
   static const InterfaceID& interfaceID();
private: // Data
};

inline const InterfaceID& IMetaDataTool::interfaceID() {
   static const InterfaceID IID("IMetaDataTool", 1, 0);
   return(IID);
}

#endif

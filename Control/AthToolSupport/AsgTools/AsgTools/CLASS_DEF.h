/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// David Adams
// January 2014
//
// Wrapper for Athena CLASS_DEF macro.
// Ignores the macro in other environments.
//
// Note that the clid command can be used to generate the class ID:
//   clid -m MyClassName

#include "AsgTools/AsgToolsConf.h"

// Set this flag to show calls to CLASS_DEF in stanadlone environments.
//#define CLASSDEF_SHOWDEFS
#ifndef XAOD_STANDALONE
  #undef CLASSDEF_SHOWDEFS
#endif

#ifdef CLASSDEF_SHOWDEFS
#include <iostream>
int announce(const char* name, int clid) {
  std::cout << "*********************** Loading clid for " << name << ": " << clid << std::endl;
  return clid;
}
#endif

#ifndef XAOD_STANDALONE
  #include "AthenaKernel/CLASS_DEF.h"
#else
  #ifdef CLASSDEF_SHOWDEFS
    #define CLASS_DEF(NAME, CLID, VER) int clid_CLASS = announce(#NAME, CLID);
  #else
    // Do not define macro if someone else has already done so.
    #ifndef CLASS_DEF
      #define CLASS_DEF(NAME, CLID, VER)
    #endif
  #endif
#endif

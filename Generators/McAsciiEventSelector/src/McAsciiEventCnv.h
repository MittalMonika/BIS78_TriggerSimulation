///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// McAsciiEventCnv.h 
// Header file for class McAsciiEventCnv
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef MCASCIIEVENTSELECTOR_MCASCIIEVENTCNV_H 
#define MCASCIIEVENTSELECTOR_MCASCIIEVENTCNV_H 

// STL includes

// Gaudi includes
#include "GaudiKernel/Converter.h"

// Forward declaration
class IOpaqueAddress;
class DataObject;
class StatusCode;
template <class CnvType> class CnvFactory;

class McAsciiEventCnv : public Converter
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /// Destructor: 
  virtual ~McAsciiEventCnv(); 

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  virtual long repSvcType() const
  {
    return i_repSvcType();
  }

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /// Storage type and class ID
  static long storageType();    
  static const CLID& classID();                 

  StatusCode createObj( IOpaqueAddress* pAddr, DataObject*& pObj ); 

  /// Constructor with parameters: 
  McAsciiEventCnv( ISvcLocator* svcLoc );
}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

#endif //> MCASCIIEVENTSELECTOR_MCASCIIEVENTCNV_H

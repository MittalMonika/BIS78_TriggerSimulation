///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// CondInputLoader.h 
// Header file for class CondInputLoader
/////////////////////////////////////////////////////////////////// 
#ifndef IOVSVC_CONDINPUTLOADER_H
#define IOVSVC_CONDINPUTLOADER_H 1

// STL includes
#include <string>


#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/ICondSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaKernel/IIOVSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadHandleKey.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "PersistentDataModel/AthenaAttributeList.h"

#include <string>
#include <map>


class CondInputLoader
  : public ::AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  CondInputLoader( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~CondInputLoader(); 

  // Assignment operator: 
  //CondInputLoader &operator=(const CondInputLoader &alg); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize();
  virtual StatusCode  execute();
  virtual StatusCode  finalize();

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  //  CondInputLoader();

  //  void loader(Property&);

  /// Containers
  DataObjIDColl m_load;
  std::vector< SG::VarHandleKey > m_vhk;

  bool m_dump;
  bool m_first { true };

  ServiceHandle<StoreGateSvc> m_condStore;
  ServiceHandle<ICondSvc> m_condSvc;
  ServiceHandle<IIOVSvc> m_IOVSvc;
  
  std::map<std::string,std::string> m_keyFolderMap;
  SG::ReadHandleKey<AthenaAttributeList> m_inputKey;
}; 


#endif //> !SGCOMPS_SGINPUTLOADER_H

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//        Root Database implementation
//--------------------------------------------------------------------
//
//        Package    : RootStorageSvc (The POOL project)
//
//        Author     : M.Frank
//====================================================================
#ifndef POOL_ROOTSTORAGESVC_ROOTOODB_H
#define POOL_ROOTSTORAGESVC_ROOTOODB_H 1

// Framework include files
#include "StorageSvc/OODatabaseImp.h"

#include "Gaudi/PluginService.h"

/*
 *  POOL namespace
 */
namespace pool  { 

  /** @class RootOODb RootOODb.h src/RootOODb.h
    *
    * Description:
    * ROOT specific implementation of the POOL database
    * structure. The object allows to create
    * - Abstract domain implementation objects
    * - Abstract database objects (ie. ROOT files)
    * - Abstract container implementations
    *
    * @author  M.Frank
    * @version 1.0
    */
  class RootOODb  : public OODatabaseImp  {
  public:
    typedef Gaudi::PluginService::Factory<IOODatabase*> Factory;

    /// Standard Constructor
    RootOODb(DbType typ=ROOT_StorageType);
    /// Standard Destructor
    virtual ~RootOODb();
    /// Label of the specific class
    static const char* catalogLabel()  {   return "ROOT_All";       }
    /// Create Root Domain object
    IDbDomain* createDomain();
    /// Create Root Database object (TFile)
    IDbDatabase* createDatabase();
    /// Create Root Container object
    IDbContainer* createContainer(const DbType& typ);
  };

  class RootOOTree : public RootOODb {
  public:
    /// Standard Constructor
    RootOOTree() : RootOODb(ROOTTREE_StorageType)   {  }
    /// Label of the specific class
    static const char* catalogLabel()  {   return "ROOT_Tree";        }
  };
   
  class RootOOKey : public RootOODb {
  public:
    /// Standard Constructor
    RootOOKey() : RootOODb(ROOTKEY_StorageType)   {   }
    /// Label of the specific class
    static const char* catalogLabel()  {   return "ROOT_Key";         }
  };
   
}       // end namespace pool

#endif  /* POOL_ROOTSTORAGESVC_ROOTOODB_H  */

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//        Root Database factories implementation
//--------------------------------------------------------------------
//
//        Package    : RootDb (The POOL project)
//
//        Author     : M.Frank
//====================================================================
#include "RootOODb.h"
#include "RootDatabase.h"
#include "RootDomain.h"
#include "RootKeyContainer.h"
#include "RootTreeContainer.h"
#include "StorageSvc/DbInstanceCount.h"

// declare the types provided by this Storage plugin
DECLARE_COMPONENT_WITH_ID(pool::RootOODb, "ROOT_All")
DECLARE_COMPONENT_WITH_ID(pool::RootOOTree, "ROOT_Tree")
DECLARE_COMPONENT_WITH_ID(pool::RootOOKey, "ROOT_Key")

using namespace pool;

/// Standard Constructor
RootOODb::RootOODb(DbType) : IOODatabase()
{
   DbInstanceCount::increment(this);
}

const std::string&  RootOODb::name () const {
   static std::string name =  { "APR.RootStorageSvc" };
   return name;
}

/// Standard Destructor
RootOODb::~RootOODb()  {
  DbInstanceCount::decrement(this);
}

/// Create Root Domain object
IDbDomain* RootOODb::createDomain()  {
  return new RootDomain();
}

/// Create Root Database object (TFile)
IDbDatabase* RootOODb::createDatabase()  {
  return new RootDatabase();
}

/// Create Root Container object
IDbContainer* RootOODb::createContainer(const DbType& typ) { 
  if ( typ.match(ROOTKEY_StorageType) )  {
    return new RootKeyContainer();
  }
  else if ( typ.match(ROOTTREE_StorageType) )    {
    return new RootTreeContainer();
  }
  else if ( typ.match(ROOT_StorageType) )    {
    return new RootTreeContainer();
  }
  return 0;
}

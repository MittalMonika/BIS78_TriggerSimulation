/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: DbSessionObj.cpp 726071 2016-02-25 09:23:05Z krasznaa $
//====================================================================
//  DbSessionObj object implementation
//--------------------------------------------------------------------
//
//  Package    : System (The POOL project)
//
//  Description: Generic data persistency
//
//  @author      M.Frank
//====================================================================
// Framework include files
#include "DbSessionObj.h"
#include "DbDomainObj.h"
#include "StorageSvc/DbPrint.h"
#include "StorageSvc/IOODatabase.h"
#include "StorageSvc/DbInstanceCount.h"

#include "Gaudi/PluginService.h"

// C++ include files
#include <memory>
#include <map>

using namespace pool;
typedef std::map<DbType, IOODatabase*> DbTypeMap;

// Standard Constructor
DbSessionObj::DbSessionObj()
: Base("DbSession", pool::READ, POOL_StorageType)
{
  m_dbTypes = new DbTypeMap();
  DbInstanceCount::increment(this);
}

// Standard Destructor
DbSessionObj::~DbSessionObj()  {
  std::auto_ptr<DbTypeMap> types((DbTypeMap*)m_dbTypes);
  clearEntries();
  if ( types.get() )  {
    for(DbTypeMap::iterator i=types->begin(); i != types->end(); ++i)  {
      if ( (*i).second ) (*i).second->release();
      (*i).second = 0;
    }
    types->clear();
    m_dbTypes = 0;
  }
  DbInstanceCount::decrement(this);
}

// Open session
DbStatus DbSessionObj::open()   {
  DbPrint log( "DbSession");
  log << DbPrintLvl::Info << "    Open     DbSession    " << DbPrint::endmsg;
  return Success;
}

// close session
DbStatus DbSessionObj::close()   {
  DbPrint log( "DbSession");
  log << DbPrintLvl::Info << "    Closed   DbSession    " << DbPrint::endmsg;
  return Success;
}

// Access different implementations
IOODatabase* DbSessionObj::db(const DbType& typ)  const   {
  DbTypeMap* types = (DbTypeMap*)m_dbTypes;
  if ( 0 == (*types)[typ] )   {
    const std::string nam = typ.storageName();
    IOODatabase* imp = Gaudi::PluginService::Factory<IOODatabase*>::create(nam);
    if ( imp )  {
      DbStatus sc = imp->initialize(nam);
      if ( sc.isSuccess() )   {
        addDb(imp);
        return imp;
      }
      imp->release();
    }
  }
  return (*types)[typ];
}

// Inject new Database implementation
DbStatus DbSessionObj::addDb(IOODatabase* db_impl)   const   {
  if ( db_impl )    {
    DbTypeMap* types = (DbTypeMap*)m_dbTypes;
    const DbType& typ = db_impl->type();
    if ( 0 != (*types)[typ] )   {
      (*types)[typ]->release();
    }
    (*types)[typ] = db_impl;
    (*types)[typ]->addRef();
    return Success;
  }
  return Error;
}

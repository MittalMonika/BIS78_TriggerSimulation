///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// AthReentrantAlgorithm.cxx 
// Implementation file for class AthReentrantAlgorithm
// Author: C. Leggett
/////////////////////////////////////////////////////////////////// 

// AthenaBaseComps includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthAlgorithmDHUpdate.h"

// Framework includes
#include "GaudiKernel/Property.h"
#ifndef REENTRANT_GAUDI
#include "GaudiKernel/ThreadLocalContext.h"
#endif

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
AthReentrantAlgorithm::AthReentrantAlgorithm( const std::string& name, 
			    ISvcLocator* pSvcLocator,
			    const std::string& version ) : 
  ::ReEntAlgorithm   ( name, pSvcLocator, version ),
  ::AthMessaging( msgSvc(), name ),
  m_evtStore    ( "StoreGateSvc/StoreGateSvc",  name ),
  m_detStore    ( "StoreGateSvc/DetectorStore", name ),
  m_userStore   ( "UserDataSvc/UserDataSvc", name ),
  m_varHandleArraysDeclared (false)
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

  auto props = getProperties();
  for( Property* prop : props ) {
    if( prop->name() != "OutputLevel" ) {
      continue;
    }
    prop->declareUpdateHandler
      (&AthReentrantAlgorithm::msg_update_handler, this);
    break;
  }

  declareProperty( "EvtStore",
                   m_evtStore = StoreGateSvc_t ("StoreGateSvc", name),
                   "Handle to a StoreGateSvc instance: it will be used to "
                   "retrieve data during the course of the job" );

  declareProperty( "DetStore",
                   m_detStore = StoreGateSvc_t ("StoreGateSvc/DetectorStore", name),
                   "Handle to a StoreGateSvc/DetectorStore instance: it will be used to "
                   "retrieve data during the course of the job" );

  declareProperty( "UserStore",
                   m_userStore = UserDataSvc_t ("UserDataSvc/UserDataSvc", name),
                   "Handle to a UserDataSvc/UserDataSvc instance: it will be used to "
                   "retrieve user data during the course of the job" );

  // Set up to run AthAlgorithmDHUpdate in sysInitialize before
  // merging depedency lists.  This extends the output dependency
  // list with any symlinks implied by inheritance relations.
  m_updateDataHandles =
    std::make_unique<AthenaBaseComps::AthAlgorithmDHUpdate>
    (m_extendedExtraObjects,
     std::move (m_updateDataHandles));
}

// Destructor
///////////////
AthReentrantAlgorithm::~AthReentrantAlgorithm()
{ 
  ATH_MSG_DEBUG ("Calling destructor");
}

/////////////////////////////////////////////////////////////////// 
// Const methods: 
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////////////// 
// Protected methods: 
/////////////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////////////// 
// Const methods: 
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

void 
AthReentrantAlgorithm::msg_update_handler( Property& outputLevel ) 
{
   // We can't just rely on the return value of msgLevel() here. Since it's
   // not well defined whether the base class gets updated with the new
   // output level first, or this class. So by default just use the property
   // itself. The fallback is only there in case Gaudi changes its property
   // type at one point, to be able to fall back on something.
   IntegerProperty* iprop = dynamic_cast< IntegerProperty* >( &outputLevel );
   if( iprop ) {
      msg().setLevel( iprop->value() );
   } else {
      msg().setLevel( msgLevel() );
   }
}


#ifndef REENTRANT_GAUDI
/**
 * @brief Standard Gaudi execute method.
 *
 * Find the event context and call @c execute_r.
 */
StatusCode AthReentrantAlgorithm::execute()
{
  return execute_r (Gaudi::Hive::currentContext());
}
#endif


/**
 * @brief Perform system initialization for an algorithm.
 *
 * We override this to declare all the elements of handle key arrays
 * at the end of initialization.
 * See comments on updateVHKA.
 */
StatusCode AthReentrantAlgorithm::sysInitialize()
{
  ATH_CHECK( ReEntAlgorithm::sysInitialize() );

  for (SG::VarHandleKeyArray* a : m_vhka) {
    for (SG::VarHandleKey* k : a->keys()) {
      this->declare (*k);
      k->setOwner(this);
    }
  }
  m_varHandleArraysDeclared = true;

  return StatusCode::SUCCESS;
}


/**
 * @brief Return this algorithm's input handles.
 *
 * We override this to include handle instances from key arrays
 * if they have not yet been declared.
 * See comments on updateVHKA.
 */
std::vector<Gaudi::DataHandle*> AthReentrantAlgorithm::inputHandles() const
{
  std::vector<Gaudi::DataHandle*> v = ReEntAlgorithm::inputHandles();

  if (!m_varHandleArraysDeclared) {
    for (SG::VarHandleKeyArray* a : m_vhka) {
      for (SG::VarHandleKey* k : a->keys()) {
        if (!(k->mode() & Gaudi::DataHandle::Reader)) break;
        v.push_back (k);
      }
    }
  }

  return v;
}


/**
 * @brief Return this algorithm's output handles.
 *
 * We override this to include handle instances from key arrays
 * if they have not yet been declared.
 * See comments on updateVHKA.
 */
std::vector<Gaudi::DataHandle*> AthReentrantAlgorithm::outputHandles() const
{
  std::vector<Gaudi::DataHandle*> v = ReEntAlgorithm::outputHandles();

  if (!m_varHandleArraysDeclared) {
    for (SG::VarHandleKeyArray* a : m_vhka) {
      for (SG::VarHandleKey* k : a->keys()) {
        if (!(k->mode() & Gaudi::DataHandle::Writer)) break;
        v.push_back (k);
      }
    }
  }

  return v;
}


/**
 * @brief Return the list of extra output dependencies.
 *
 * This list is extended to include symlinks implied by inheritance
 * relations.
 */
const DataObjIDColl& AthReentrantAlgorithm::extraOutputDeps() const
{
  // If we didn't find any symlinks to add, just return the collection
  // from the base class.  Otherwise, return the extended collection.
  if (!m_extendedExtraObjects.empty()) {
    return m_extendedExtraObjects;
  }
  return Algorithm::extraOutputDeps();
}

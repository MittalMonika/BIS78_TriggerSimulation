/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/** @file PoolCollectionConverter.cxx
 *  @brief This file contains the implementation for the PoolCollectionConverter class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "PoolCollectionConverter.h"
#include "PoolSvc/IPoolSvc.h"
#include "PersistentDataModel/Token.h"

// Pool
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"

#include "CollectionBase/ICollection.h"
#include "CollectionBase/ICollectionQuery.h"
#include "CollectionBase/ICollectionCursor.h"
#include "CollectionBase/ICollectionDescription.h"
#include "CollectionBase/CollectionRowBuffer.h"

// Gaudi
#include "GaudiKernel/StatusCode.h"

#include <assert.h>
#include <exception>

//______________________________________________________________________________
PoolCollectionConverter::PoolCollectionConverter(const std::string& collectionType,
	const std::string& inputCollection,
	const std::string& query,
	const IPoolSvc* svc) :
	m_collectionType(),
	m_connection(),
	m_inputCollection(inputCollection),
	m_query(query),
	m_poolSvc(svc),
	m_poolCollection(0),
	m_collectionQuery(0),
	m_inputContainer() {
   // Find out if the user specified a container
   std::string::size_type p_colon = collectionType.rfind(':');
   if (p_colon != std::string::npos) {
      m_inputContainer = collectionType.substr(p_colon + 1);
      m_collectionType = collectionType.substr(0, p_colon);
   } else {
      // The user didn't specify any container. Using the default one.
      m_inputContainer = "POOLContainer";
      m_collectionType = collectionType;
   }
   std::string::size_type p_slash = m_inputContainer.find('/');
   if (p_slash != std::string::npos) {
      m_inputContainer = m_inputContainer.substr(0, p_slash);
   }
}
//______________________________________________________________________________
PoolCollectionConverter::~PoolCollectionConverter() {
   if (m_poolCollection) {
      m_poolCollection->close();
      delete m_collectionQuery; m_collectionQuery = 0;
      delete m_poolCollection; m_poolCollection = 0;
   }
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::initialize() {
   std::string collectionTypeString;
   if (m_collectionType == "ExplicitROOT") {
      collectionTypeString = "RootCollection";
   } else if (m_collectionType == "ImplicitROOT") {
      collectionTypeString = "ImplicitCollection";
   } else {
      return(StatusCode::FAILURE);
   }
   if (collectionTypeString == "ImplicitCollection") {
      // Check if already prefixed
      if (m_inputCollection.find("PFN:") == 0
	      || m_inputCollection.find("LFN:") == 0
	      || m_inputCollection.find("FID:") == 0) {
         // Aready prefixed
         m_connection = m_inputCollection;
      } else {
         // Prefix with PFN:
         m_connection = "PFN:" + m_inputCollection;
      }
      try {
         m_poolCollection = m_poolSvc->createCollection("RootCollection", m_connection, m_inputCollection);
      } catch (std::exception &e) {
         m_poolCollection = 0;
      }
      if (m_poolCollection == 0) {
         // Now set where to look in the implicit file
         m_inputCollection = m_inputContainer + "(DataHeader)";
      }
   }
   try {
      if (m_poolCollection == 0) {
         m_poolCollection = m_poolSvc->createCollection(collectionTypeString, m_connection, m_inputCollection);
      }
      if (m_poolCollection == 0 && collectionTypeString == "ImplicitCollection") {
         m_inputCollection = m_inputContainer + "_DataHeader";
         m_poolCollection = m_poolSvc->createCollection(collectionTypeString, m_connection, m_inputCollection);
      }
   } catch (std::exception &e) {
      return(StatusCode::RECOVERABLE);
   }
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::disconnectDb() {
   if (m_poolCollection == 0) {
      return(StatusCode::SUCCESS);
   }
   if (m_poolCollection->description().type() == "ImplicitCollection") {
      return(m_poolSvc->disconnectDb(m_connection));
   }
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::isValid() const {
   return(m_poolCollection != 0 ? StatusCode::SUCCESS : StatusCode::FAILURE);
}
//______________________________________________________________________________
pool::ICollectionCursor& PoolCollectionConverter::selectAll() const {
   assert(m_poolCollection);
   delete m_collectionQuery; m_collectionQuery = 0;
   m_collectionQuery = m_poolCollection->newQuery();
   m_collectionQuery->setRowCacheSize(100);   //MN: FIXME - just an arbitrary number
   return(m_collectionQuery->execute());
}
//______________________________________________________________________________
pool::ICollectionCursor& PoolCollectionConverter::executeQuery() const {
   assert(m_poolCollection);
   delete m_collectionQuery; m_collectionQuery = 0;
   m_collectionQuery = m_poolCollection->newQuery();
   m_collectionQuery->selectAll();
   m_collectionQuery->setCondition(m_query);
   m_collectionQuery->setRowCacheSize(100);   //MN: FIXME - just an arbitrary number
   return(m_collectionQuery->execute());
}
//______________________________________________________________________________
std::string PoolCollectionConverter::retrieveToken(const pool::ICollectionCursor* cursor,
		const std::string& refName) const {
// Retrieve the DataHeader Token from POOL.
// If header name is non-zero, then search the attibute list for
// the corresponding ref. Otherwise, use the main token
   std::string tokenStr;
   if (!refName.empty()) {
      std::string attrName = refName + "_ref";
      try {
	 tokenStr = cursor->currentRow().tokenList()[attrName].toString();
      } catch (...) {
	 // check also in attributes, for backward compatibility
	 try {
            tokenStr = cursor->currentRow().attributeList()[attrName].data<std::string>();
	 } catch (std::exception& e) {
	    return("");
	 }
      }
   } else {
      tokenStr = cursor->eventRef().toString();
   }
   return(tokenStr);
}
//______________________________________________________________________________
pool::ICollectionMetadata* PoolCollectionConverter::retrieveMetadata() const {
   if (m_collectionType != "ExplicitROOT" && m_collectionType != "ImplicitROOT") {
      return(0);
   }
   return(&m_poolCollection->metadata());
}

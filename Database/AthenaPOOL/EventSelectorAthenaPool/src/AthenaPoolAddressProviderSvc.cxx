/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/** @file AthenaPoolAddressProviderSvc.cxx
 *  @brief This file contains the implementation for the AthenaPoolAddressProviderSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaPoolAddressProviderSvc.h"
#include "registerKeys.h"

#include "PersistentDataModel/AthenaAttributeList.h"
#include "PersistentDataModel/DataHeader.h"
#include "PersistentDataModel/TokenAddress.h"

// Framework
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/StatusCode.h"

#include "StoreGate/ActiveStoreSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "AthenaKernel/IClassIDSvc.h"

#include <vector>

//________________________________________________________________________________
AthenaPoolAddressProviderSvc::AthenaPoolAddressProviderSvc(const std::string& name, ISvcLocator* pSvcLocator) :
	::AthService(name, pSvcLocator),
	m_activeStoreSvc("ActiveStoreSvc", name),
	m_metaDataStore("MetaDataStore", name),
	m_clidSvc("ClassIDSvc", name),
	m_guid() {
   declareProperty("DataHeaderKey",       m_dataHeaderKey = "EventSelector");
   declareProperty("DataHeaderIterator",  m_dataHeaderIterator = false);
   declareProperty("AttributeListKey",    m_attrListKey = "");
}
//________________________________________________________________________________
AthenaPoolAddressProviderSvc::~AthenaPoolAddressProviderSvc() {
}
//________________________________________________________________________________
StoreGateSvc* AthenaPoolAddressProviderSvc::eventStore() const {
   if (m_activeStoreSvc == 0) {
      if (!m_activeStoreSvc.retrieve().isSuccess()) {
         ATH_MSG_ERROR("Cannot get ActiveStoreSvc");
         throw GaudiException("Cannot get ActiveStoreSvc", name(), StatusCode::FAILURE);
      }
   }
   return(m_activeStoreSvc->operator->());
}
//________________________________________________________________________________
StatusCode AthenaPoolAddressProviderSvc::initialize() {
   if (!::AthService::initialize().isSuccess()) {
      ATH_MSG_FATAL("Cannot initialize AthService base class.");
      return(StatusCode::FAILURE);
   }
   // Retrieve MetaDataStore
   if (!m_metaDataStore.retrieve().isSuccess()) {
      ATH_MSG_FATAL("Cannot get MetaDataStore.");
      return(StatusCode::FAILURE);
   }
   // Retrieve ClassIDSvc
   if (!m_clidSvc.retrieve().isSuccess()) {
      ATH_MSG_FATAL("Cannot get ClassIDSvc.");
      return(StatusCode::FAILURE);
   }
   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode AthenaPoolAddressProviderSvc::finalize() {
   // Release ClassIDSvc
   if (!m_clidSvc.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release ClassIDSvc.");
   }
   // Release MetaDataStore
   if (!m_metaDataStore.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release MetaDataStore.");
   }
   return(::AthService::finalize());
}
//________________________________________________________________________________
StatusCode AthenaPoolAddressProviderSvc::queryInterface(const InterfaceID& riid, void** ppvInterface) {
   return(::AthService::queryInterface(riid, ppvInterface));
}
//________________________________________________________________________________
StatusCode AthenaPoolAddressProviderSvc::preLoadAddresses(StoreID::type storeID,
		IAddressProvider::tadList& tads) {
   if (storeID != StoreID::DETECTOR_STORE) {
      return(StatusCode::SUCCESS);
   }
   ServiceHandle<StoreGateSvc> detectorStoreSvc("DetectorStore", name());
   if (!detectorStoreSvc.retrieve().isSuccess()) {
      ATH_MSG_ERROR("Cannot get DetectorStore.");
      return(StatusCode::FAILURE);
   }
   SG::ReadHandle<DataHeader> dataHeader("CondProxyProvider", detectorStoreSvc->name());
   if (!dataHeader.isValid()) {
      ATH_MSG_DEBUG("Cannot retrieve DataHeader from DetectorStore.");
      return(StatusCode::SUCCESS);
   }
   ATH_MSG_DEBUG("The current File contains: " << dataHeader->size() << " objects");
   for (const auto& element : *dataHeader) {
      SG::TransientAddress* tadd = element.getAddress();
      if (tadd->clID() == ClassID_traits<DataHeader>::ID()) {
         delete tadd; tadd = nullptr;
      } else {
         ATH_MSG_DEBUG("preLoadAddresses: DataObject address, clid = " << tadd->clID() << ", name = " << tadd->name());
         tads.push_back(tadd);
      }
      EventSelectorAthenaPoolUtil::registerKeys(element, &*detectorStoreSvc);
   }
   if (!detectorStoreSvc.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release DetectorStoreSvc.");
   }
   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode AthenaPoolAddressProviderSvc::loadAddresses(StoreID::type storeID,
		IAddressProvider::tadList& tads) {
   if (storeID != StoreID::EVENT_STORE && storeID != StoreID::PILEUP_STORE) {
      return(StatusCode::SUCCESS);
   }

   Guid thisFile = Guid::null();
   long int oid2 = 0L;
   const DataHeader* dataHeader = nullptr;
   if (m_dataHeaderIterator) { // Get oid2 (event file entry number) from DataHeader proxy
      const SG::DataProxy* dhProxy = eventStore()->proxy(ClassID_traits<DataHeader>::ID(), m_dataHeaderKey.value());
      if (dhProxy != nullptr && dhProxy->address() != nullptr) {
         Token token;
         token.fromString(*dhProxy->address()->par());
         thisFile = token.dbID();
         oid2 = token.oid().second;
      }
      SG::ReadHandle<DataHeader> copiedDataHeader(thisFile.toString(), m_metaDataStore->name());
      if (copiedDataHeader.isValid()) {
         dataHeader = copiedDataHeader.cptr();
      }
   }
   if (dataHeader == nullptr) { // New file (or reading DataHeader)
      SG::ReadHandle<DataHeader> eventDataHeader(m_dataHeaderKey.value(), eventStore()->name());
      if (!eventDataHeader.isValid()) {
         ATH_MSG_ERROR("Cannot retrieve DataHeader from StoreGate: " << m_dataHeaderKey);
         return(StatusCode::FAILURE);
      }
      dataHeader = eventDataHeader.cptr();
      if (m_dataHeaderIterator) {
         std::unique_ptr<DataHeader> dataHeaderCopy(new DataHeader(*eventDataHeader.cptr()));
         SG::WriteHandle<DataHeader> wh(thisFile.toString(), m_metaDataStore->name());
         if (!wh.record(std::move(dataHeaderCopy)).isSuccess()) {
            ATH_MSG_WARNING("Can't copy event DataHeader to MetaData store.");
         }
      }
   }
   // second data header
   if (m_attrListKey.value() != "") {
      const DataHandle<AthenaAttributeList> attrList;
      std::string tokenStr;
      if (eventStore()->retrieve(attrList, m_attrListKey.value()).isSuccess()) {
         try {
            tokenStr = (*attrList)["eventRef_secondary"].data<std::string>();
            ATH_MSG_DEBUG("found AthenaAttribute, name = eventRef_secondary = " << tokenStr);
         } catch (std::exception &e) {
            ATH_MSG_ERROR(e.what());
            return (StatusCode::FAILURE);
         }
      } else {
         ATH_MSG_ERROR("Cannot find AthenaAttribute, key = " << m_attrListKey.value());
         return (StatusCode::FAILURE);
      }
      IOpaqueAddress* iop = new GenericAddress(POOL_StorageType, ClassID_traits<DataHeader>::ID(), tokenStr, "SecondaryEventSelector");
      if (!eventStore()->recordAddress(iop).isSuccess()) {
         ATH_MSG_ERROR("Cannot record address to StoreGate with token string: " << tokenStr);
         return(StatusCode::FAILURE);
      }
      ATH_MSG_DEBUG("Created dataHeader SecondaryEventSelector");
   }
   ATH_MSG_DEBUG("The current Event contains: " << dataHeader->size() << " objects");
   for (const auto& element : *dataHeader) {
      if (m_dataHeaderIterator) { // Get oid2 (event file entry number) from DataHeader proxy
         const_cast<Token*>(element.getToken())->oid().second = oid2;
      }
      SG::TransientAddress* tadd = element.getAddress();
      if (tadd->clID() == ClassID_traits<DataHeader>::ID()) { // self reference
         delete tadd; tadd = nullptr;
      } else {
         ATH_MSG_DEBUG("loadAddresses: DataObject address, clid = " << tadd->clID() << ", name = " << tadd->name());
         tadd->setProvider(this, storeID);
         tads.push_back(tadd);
      }
      EventSelectorAthenaPoolUtil::registerKeys(element, eventStore());
   }
   m_guid = thisFile;
   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode AthenaPoolAddressProviderSvc::updateAddress(StoreID::type /*storeID*/,
                                                       SG::TransientAddress* /*tad*/,
                                                       const EventContext& /*ctx*/) {
   return(StatusCode::FAILURE);
}

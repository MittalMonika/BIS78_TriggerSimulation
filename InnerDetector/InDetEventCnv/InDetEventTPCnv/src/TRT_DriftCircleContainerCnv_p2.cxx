/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetPrepRawData/TRT_DriftCircle.h"
#include "InDetEventTPCnv/InDetPrepRawData/TRT_DriftCircle_p2.h"
#include "InDetEventTPCnv/TRT_DriftCircleContainer_p2.h"
#include "InDetEventTPCnv/InDetPrepRawData/InDetPRD_Collection_p2.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"

#include "Identifier/Identifier.h"
#include "InDetIdentifier/TRT_ID.h"
#include "InDetReadoutGeometry/TRT_DetectorManager.h"
#include "InDetEventTPCnv/InDetPrepRawData/TRT_DriftCircleCnv_p2.h"
#include "InDetEventTPCnv/TRT_DriftCircleContainerCnv_p2.h"
#include "DataModel/DataPool.h"

// Gaudi
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/CnvFactory.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentSvc.h"

// Athena
#include "StoreGate/StoreGateSvc.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"

// #define IDJUMP 0x400


void TRT_DriftCircleContainerCnv_p2::transToPers(const InDet::TRT_DriftCircleContainer* transCont, InDet::TRT_DriftCircleContainer_p2* persCont, MsgStream &log) 
{

    // The transient model has a container holding collections and the
    // collections hold channels.
    //
    // The persistent model flattens this so that the persistent
    // container has two vectors:
    //   1) all collections, and
    //   2) all PRD
    //
    // The persistent collections, then only maintain indexes into the
    // container's vector of all channels. 
    //
    // So here we loop over all collection and add their channels
    // to the container's vector, saving the indexes in the
    // collection. 

    typedef InDet::TRT_DriftCircleContainer TRANS;
        
    // this is the id of the latest collection read in
    // Thisstarts from the base of the TRT identifiers
    unsigned int idLast(0);

    // 
    TRT_DriftCircleCnv_p2  chanCnv;
    TRANS::const_iterator it_Coll     = transCont->begin();
    TRANS::const_iterator it_CollEnd  = transCont->end();
    unsigned int collIndex;
    unsigned int chanBegin = 0;
    unsigned int chanEnd = 0;

    persCont->m_collections.resize(transCont->numberOfCollections());

    // to avoid the inside-loop resize
    int totSize = 0; 
    //for ( ; it_Coll != it_CollEnd;  it_Coll++)  {
    for ( it_Coll=transCont->begin(); it_Coll != it_CollEnd;  it_Coll++)  {
      const InDet::TRT_DriftCircleCollection& collection = (**it_Coll);
      totSize+=collection.size();
    }
    persCont->m_rawdata.resize(totSize);
    persCont->m_prdDeltaId.resize(totSize);

    //    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << " Preparing " << persCont->m_collections.size() << "Collections" << endreq;
    //    for (collIndex = 0; it_Coll != it_CollEnd; ++collIndex, it_Coll++)  {
    for (collIndex = 0, it_Coll=transCont->begin(); it_Coll != it_CollEnd; ++collIndex, it_Coll++)  {
        // Add in new collection
        const InDet::TRT_DriftCircleCollection& collection = (**it_Coll);
        chanBegin  = chanEnd;
        chanEnd   += collection.size();
        InDet::InDetPRD_Collection_p2& pcollection = persCont->m_collections[collIndex];
	unsigned int deltaId = (collection.identifyHash()-idLast);
        // if(deltaId*IDJUMP != collection.identify().get_compact()-idLast ) 
        //   log << MSG::FATAL << "THere is a mistake in Identifiers of the collection" << endreq;
        // if(deltaId > 0xFFFF) {
        //   log << MSG::FATAL << "Fixme!!! This is too big, something needs to be done " << endreq;
        // }
        // pcollection.m_idDelta = (unsigned short) deltaId;
        // idLast = collection.identify().get_compact(); // then update the last identifier 
	//        pcollection.m_hashId = (unsigned short) collection.identifyHash();
        pcollection.m_hashId = deltaId;
	idLast = collection.identifyHash();
        pcollection.m_size = collection.size();
        // Add in channels
        //persCont->m_rawdata.resize(chanEnd);
        //persCont->m_prdDeltaId.resize(chanEnd);
	//        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "Reading collections with " <<  collection.size() << "PRDs " << endreq;
        for (unsigned int i = 0; i < collection.size(); ++i) {
            InDet::TRT_DriftCircle_p2* pchan = &(persCont->m_rawdata[i + chanBegin]);
            const InDet::TRT_DriftCircle* chan = dynamic_cast<const InDet::TRT_DriftCircle*>(collection[i]);
            chanCnv.transToPers(chan, pchan, log);
	    //persCont->m_prdDeltaId[i+chanBegin]=chan->m_clusId.get_compact()-collection.identify().get_compact(); 
	    persCont->m_prdDeltaId[i+chanBegin]=chan->identify().get_identifier32().get_compact()-collection.identify().get_identifier32().get_compact();
	}
    }
    //    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << " ***  Writing InDet::TRT_DriftCircleContainer" << endreq;
}

void  TRT_DriftCircleContainerCnv_p2::persToTrans(const InDet::TRT_DriftCircleContainer_p2* persCont, InDet::TRT_DriftCircleContainer* transCont, MsgStream &log) 
{

    // The transient model has a container holding collections and the
    // collections hold channels.
    //
    // The persistent model flattens this so that the persistent
    // container has two vectors:
    //   1) all collections, and
    //   2) all channels
    //
    // The persistent collections, then only maintain indexes into the
    // container's vector of all channels. 
    //
    // So here we loop over all collection and extract their channels
    // from the vector.


    InDet::TRT_DriftCircleCollection* coll = 0;

    TRT_DriftCircleCnv_p2  chanCnv;
    unsigned int collBegin(0);
    // this is the id of the latest collection read in
    // This starts from the base of the TRT identifiers
    unsigned int idLast(0);
    //    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << " Reading " << persCont->m_collections.size() << "Collections" << endreq;
    for (unsigned int icoll = 0; icoll < persCont->m_collections.size(); ++icoll) {

        // Create trans collection - in NOT owner of TRT_DriftCircle (SG::VIEW_ELEMENTS)
	// IDet collection don't have the Ownership policy c'tor
        const InDet::InDetPRD_Collection_p2& pcoll = persCont->m_collections[icoll];        
	//        idLast+= pcoll.m_idDelta*IDJUMP;
        // Identifier collID= Identifier(idLast);
	idLast += pcoll.m_hashId;
        IdentifierHash collIDHash=IdentifierHash((unsigned int) idLast);
        Identifier collID = m_trtId->layer_id(collIDHash);
        coll = new InDet::TRT_DriftCircleCollection(collIDHash);
        coll->setIdentifier(collID);
        unsigned int nchans           = pcoll.m_size;
        coll->resize(nchans);
	const InDetDD::TRT_BaseElement* detEl =  m_trtMgr->getElement(collIDHash);
        // Fill with channels:
        // This is used to read the vector of errMat
        // values and lenght of the value are specified in separate vectors
	//	if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Reading collection with " << nchans << "Channels " << endreq;
        for (unsigned int ichan = 0; ichan < nchans; ++ ichan) {
            const InDet::TRT_DriftCircle_p2* pchan = &(persCont->m_rawdata[ichan + collBegin]);
            Identifier clusId=Identifier(collID.get_identifier32().get_compact()+persCont->m_prdDeltaId[ichan + collBegin]);

            std::vector<Identifier> rdoList(1);
            rdoList[0]=clusId;

            InDet::TRT_DriftCircle* chan = new InDet::TRT_DriftCircle
              (chanCnv.createTRT_DriftCircle (pchan,
                                              clusId,
                                              std::move(rdoList),
                                              detEl,
                                              log));

            // DC bugfix: set hash Id of the drift circle
	    chan->setHashAndIndex(collIDHash,ichan);
            (*coll)[ichan] = chan;
        }
        collBegin += pcoll.m_size;

        // register the PRD collection in IDC with hash - faster addCollection
        StatusCode sc = transCont->addCollection(coll, collIDHash);
        if (sc.isFailure()) {
            throw std::runtime_error("Failed to add collection to ID Container");
        }
	//	if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "AthenaPoolTPCnvIDCont::persToTrans, collection, hash_id/coll id = " << (int) collIDHash << " / " << collID << ", added to Identifiable container." << endreq;
    }

    //     if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << " ***  Reading InDet::TRT_DriftCircleContainer" << endreq;
}



//================================================================
InDet::TRT_DriftCircleContainer* TRT_DriftCircleContainerCnv_p2::createTransient(const InDet::TRT_DriftCircleContainer_p2* persObj, MsgStream& log) {
  //  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "TRT_DriftCircleContainerCnv_p2::createTransient called " << endreq;
    if(!m_isInitialized) {
     if (this->initialize(log) != StatusCode::SUCCESS) {
      log << MSG::FATAL << "Could not initialize TRT_DriftCircleContainerCnv_p2 " << endreq;
     }
    }
    std::auto_ptr<InDet::TRT_DriftCircleContainer> trans(new InDet::TRT_DriftCircleContainer(m_trtId->straw_layer_hash_max()));
    persToTrans(persObj, trans.get(), log);
    return(trans.release());
}


StatusCode TRT_DriftCircleContainerCnv_p2::initialize(MsgStream &log) {


   // Do not initialize again:
   m_isInitialized=true;
   //   if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "TRT_DriftCircleContainerCnv_p2::initialize called " << endreq;
   // Get Storegate, ID helpers, and so on
   ISvcLocator* svcLocator = Gaudi::svcLocator();
   // get StoreGate service
   StatusCode sc = svcLocator->service("StoreGateSvc", m_storeGate);
   if (sc.isFailure()) {
      log << MSG::FATAL << "StoreGate service not found !" << endreq;
      return StatusCode::FAILURE;
   }

   // get DetectorStore service
   StoreGateSvc *detStore;
   sc = svcLocator->service("DetectorStore", detStore);
   if (sc.isFailure()) {
      log << MSG::FATAL << "DetectorStore service not found !" << endreq;
      return StatusCode::FAILURE;
   }
   //   else {
   //     if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Found DetectorStore." << endreq;
   //   }

   // Get the trt helper from the detector store
   sc = detStore->retrieve(m_trtId, "TRT_ID");
   if (sc.isFailure()) {
      log << MSG::FATAL << "Could not get TRT_ID helper !" << endreq;
      return StatusCode::FAILURE;
   }
   //   else {
   //     if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Found the TRT_ID helper." << endreq;
   //   }

   sc = detStore->retrieve(m_trtMgr);
   if (sc.isFailure()) {
      log << MSG::FATAL << "Could not get TRT_DetectorDescription" << endreq;
      return sc;
   }

   //   if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converter initialized." << endreq;
   return StatusCode::SUCCESS;
}




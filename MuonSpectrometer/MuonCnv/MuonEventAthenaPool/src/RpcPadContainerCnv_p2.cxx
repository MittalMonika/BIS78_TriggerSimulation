/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#define private public
#define protected public
#include "MuonRDO/RpcPad.h"
#include "MuonRDO/RpcPadContainer.h"
#include "MuonEventAthenaPool/RpcPad_p1.h"
#include "MuonEventAthenaPool/RpcPadContainer_p2.h"
#undef private
#undef protected

#include "MuonIdHelpers/RpcIdHelper.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "RpcPadCnv_p1.h"
#include "RpcPadContainerCnv_p2.h"

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

#include "DataModel/DataPool.h"

#include "RPCcablingInterface/IRPCcablingServerSvc.h"
#include "RPCcablingInterface/RpcPadIdHash.h"
#include "RPCcablingInterface/IRPCcablingSvc.h"

StatusCode RpcPadContainerCnv_p2::initialize(MsgStream &log) {
   // Do not initialize again:
    m_isInitialized=true;

   // Get Storegate, ID helpers, and so on
    ISvcLocator* svcLocator = Gaudi::svcLocator();

    // get RPC cablingSvc
    const IRPCcablingServerSvc* RpcCabGet = 0;
    StatusCode sc =  svcLocator->service("RPCcablingServerSvc", RpcCabGet);
    if (sc.isFailure()) {
        log<<MSG::FATAL << "Could not get RPCcablingServerSvc !" << endreq;
        return StatusCode::FAILURE;
    }
    else log <<MSG::VERBOSE << " RPCcablingServerSvc retrieved" << endreq;

    sc = RpcCabGet->giveCabling(m_rpcCabling);
    if (sc.isFailure()) {
        log << MSG::FATAL << "Could not get RPCcablingSvc from the Server !" << endreq;
        m_rpcCabling = 0;
        return StatusCode::FAILURE;
    }
    else {
        if (log.level() <= MSG::VERBOSE) log <<MSG::VERBOSE << " RPCcablingSvc obtained " << endreq;
    }

    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converter initialized." << endreq;
    return StatusCode::SUCCESS;
}

void RpcPadContainerCnv_p2::transToPers(const RpcPadContainer* transCont,  RpcPadContainer_p2* persCont, MsgStream &log) 
{

    typedef RpcPadContainer TRANS;

    RpcPadCnv_p1  cnv;
    TRANS::const_iterator it_Coll     = transCont->begin();
    TRANS::const_iterator it_CollEnd  = transCont->end();
    unsigned int collIndex;

    int numColl = transCont->numberOfCollections();
    persCont->m_pads.resize(numColl);    log << MSG::DEBUG  << " Preparing " << persCont->m_pads.size() << "Collections" << endreq;

    for (collIndex = 0; it_Coll != it_CollEnd; ++collIndex, it_Coll++)  {
        // Add in new collection
        //log << MSG::DEBUG  << " New collection" << endreq;
        RpcPad_p1* pcoll = &( persCont->m_pads[collIndex] );
        cnv.transToPers( &(**it_Coll), pcoll , log);
    }
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG  << " ***  Writing RpcPadContainer ***" << endreq;
}

void  RpcPadContainerCnv_p2::persToTrans(const RpcPadContainer_p2* persCont, RpcPadContainer* transCont, MsgStream &log) 
{
    RpcPadCnv_p1  cnv;

    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG  << " Reading " << persCont->m_pads.size() << "Collections" << endreq;
    for (unsigned int icoll = 0; icoll < persCont->m_pads.size(); ++icoll) {

        RpcPad* coll = new RpcPad;
        cnv.persToTrans( &( persCont->m_pads[icoll]), coll, log);
        // another check - see if already added
        RpcPadContainer::const_iterator it = transCont->indexFind(coll->identifyHash());
        if (it!=transCont->end() ) {
            log << MSG::WARNING<<"Collection with hash="<<coll->identifyHash()<<" already exists in container (with "<<(*it)->size()<<" elements). "
                << "Will therefore DISCARD this collection which has "<<coll->size()<<" elements)!"<<endreq;
            delete coll;
        } else {
            StatusCode sc = transCont->addCollection(coll, coll->identifyHash());
            if (sc.isFailure()) {
                log << MSG::WARNING<<"Could not add collection with hash="<<coll->identifyHash()
                    <<" to IDC which has hash max of "<<transCont->size()<<" (PadHashFunction gives "<<m_rpcCabling->padHashFunction()->max()<<")"<<endreq;
                throw std::runtime_error("Failed to add collection to ID Container. Hash = "+coll->identifyHash());
            }
        }
    }

    log << MSG::DEBUG  << " ***  Reading RpcPadContainer" << endreq;
}



//================================================================
RpcPadContainer* RpcPadContainerCnv_p2::createTransient(const RpcPadContainer_p2* persObj, MsgStream& log) 
{
    if(!m_isInitialized) {
        if (this->initialize(log) != StatusCode::SUCCESS) {
            log << MSG::FATAL << "Could not initialize RpcPadContainerCnv_p2 " << endreq;
            return 0;
        } 
    }
    // log<<MSG::INFO<<"creating new pad container with hashmax= "<<m_rpcCabling->padHashFunction()->max()<<endreq;
    
    // std::auto_ptr<RpcPadContainer> trans(new RpcPadContainer(m_rpcCabling->padHashFunction()->max() ));
    std::auto_ptr<RpcPadContainer> trans(new RpcPadContainer(404 ))  ; // hardcoded number from above. FIXME!
    
    persToTrans(persObj, trans.get(), log);
    return(trans.release());
}



/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#define private public
#define protected public
#include "MuonRDO/TgcRdo.h"
#undef private
#undef protected

#include "GaudiKernel/MsgStream.h"
#include "MuonEventAthenaPool/TgcRdo_p1.h"
#include "TgcRdoCnv_p1.h"
#include "TgcRawDataCnv_p1.h"

void
TgcRdoCnv_p1::transToPers(const TgcRdo* /*transColl*/, TgcRdo_p1* /*persColl*/, MsgStream &log) 
{
    log << MSG::ERROR << "TgcRdoCnv_p1::transToPers should not be called. _p2 exists!" << endreq;
}

void
TgcRdoCnv_p1::persToTrans(const TgcRdo_p1* persColl, TgcRdo* transColl, MsgStream &log) 
{
    if (log.level() <= MSG::DEBUG)
        log << MSG::DEBUG  << " ***  Reading TgcRdo P1" << endreq;
//    log << MSG::INFO  << " ***  Reading TgcRdo P1" << endreq;

    // Invoke vector converter from the base template
    TgcRdoCnv_p1_basetype::persToTrans(persColl, transColl, log);

    transColl->m_subDetectorId = persColl->subDetectorId();
    transColl->m_rodId         = persColl->rodId();
    transColl->m_id            = persColl->identify();
    transColl->m_triggerType   = persColl->triggerType();
    transColl->setBcId(persColl->bcId());
    transColl->setL1Id(persColl->l1Id());
    transColl->setVersion(0);
    
    if (log.level() <= MSG::DEBUG)
        std::cout << "After conversion " << *transColl << std::endl;
}


/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#define private public
#define protected public
#include "MuonChamberT0s/ChamberT0s.h"
#undef private
#undef protected
#include "ChamberT0sCnv.h"
#include "MuonEventTPCnv/MuonChamberT0s/ChamberT0s_p1.h"
#include "MuonEventTPCnv/MuonChamberT0s/ChamberT0sCnv_p1.h"

static ChamberT0sCnv_p1   TPconverter_p1;

ChamberT0s_PERS* ChamberT0sCnv::createPersistent(Muon::ChamberT0s* transObj) {
    using namespace Muon;
    MsgStream log(messageService(), "MuonChamberT0sConverter" );
    ChamberT0s_PERS *persObj = TPconverter_p1.createPersistent( transObj, log );
//    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "ChamberT0s write Success" << endreq;
    return persObj;
}
   
Muon::ChamberT0s* ChamberT0sCnv::createTransient() {
    using namespace Muon;
    static pool::Guid   p1_guid("67E3F1AE-6254-4B29-8D61-5F17D0C19BB2");
    if( compareClassGuid(p1_guid) ) {
        // using auto_ptr ensures deletion of the persistent object
        std::auto_ptr< ChamberT0s_p1 > col_vect( poolReadObject< ChamberT0s_p1 >() );
        MsgStream log(messageService(), "MuonChamberT0sConverter_p1" );
        //log << MSG::DEBUG << "Reading ChamberT0s_p1" << endreq;
        return TPconverter_p1.createTransient( col_vect.get(), log );
    } 
    throw std::runtime_error("Unsupported persistent version of ChamberT0s");
}
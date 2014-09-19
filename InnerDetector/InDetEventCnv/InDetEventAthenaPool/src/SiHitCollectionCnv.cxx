/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetEventTPCnv/InDetHits/SiHitCollectionCnv_p1.h"
#include "InDetEventTPCnv/InDetHits/SiHitCollectionCnv_p2.h"
#include "InDetEventTPCnv/InDetHits/SiHit_p1.h"
#include "SiHitCollectionCnv.h"


SiHitCollection_PERS* SiHitCollectionCnv::createPersistent(SiHitCollection* transCont) {
  MsgStream mlog(messageService(), "SiHitCollectionConverter" );
  SiHitCollectionCnv_PERS converter;
  SiHitCollection_PERS *persObj = converter.createPersistent( transCont, mlog );
  return persObj;
}


SiHitCollection* SiHitCollectionCnv::createTransient() {
    MsgStream mlog(messageService(), "SiHitCollectionConverter" );
    SiHitCollectionCnv_p1   converter_p1;
    SiHitCollectionCnv_p2   converter_p2; 

    static const pool::Guid   p1_guid("36D1FF8E-5734-4A93-A133-F286CF47DB72");
    static const pool::Guid   p2_guid("BD1469C5-C904-40B8-82B9-43D25888D884");
    static const pool::Guid   old_guid("1EC39DA3-14F9-4901-88C7-F6909B064574");

    SiHitCollection       *trans_cont(0);
    if( this->compareClassGuid(p1_guid)) {
      std::auto_ptr< SiHitCollection_p1 >   col_vect( this->poolReadObject< SiHitCollection_p1 >() );
      trans_cont = converter_p1.createTransient( col_vect.get(), mlog );
    }
    else if( this->compareClassGuid(p2_guid)) { // version p2
      std::auto_ptr< SiHitCollection_p2 >   col_vect( this->poolReadObject< SiHitCollection_p2 >() );
      trans_cont = converter_p2.createTransient( col_vect.get(), mlog );
    }
    else if( this->compareClassGuid(old_guid)) {
      // old version from before TP separation, just return it
      AthenaHitsVector<SiHit>* oldColl = this->poolReadObject< AthenaHitsVector<SiHit> >();
      size_t size = oldColl->size();
      trans_cont=new SiHitCollection("DefaultCollectionName",size);
      //do the copy
      AthenaHitsVector<SiHit>::const_iterator it = oldColl->begin(), itEnd=oldColl->end();
      for (;it!=itEnd;++it)  {
	trans_cont->push_back(**it);
      }
      delete oldColl;
    }  else {
      throw std::runtime_error("Unsupported persistent version of Data container");
    }
    return trans_cont;
}

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBHitContCnv_H
#define TBHitContCnv_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCnv.h"
#include "TBEvent/TBHitCont.h"

typedef T_AthenaPoolCnv<TBHitCont> TBHitContCnvBase;

class TBHitContCnv : public TBHitContCnvBase
{
  friend class CnvFactory<TBHitContCnv >;
 public:
  TBHitContCnv(ISvcLocator* svcloc);
  virtual ~TBHitContCnv();

  /// initialization
  virtual StatusCode initialize() override;

  /// Extend base-class conversion method to modify when reading in
  virtual StatusCode     PoolToDataObject(DataObject*& pObj,
                                          const Token* token,
                                          const std::string& key) override;

  /// Extend base-class conversion method for writing
  virtual StatusCode     DataObjectToPool(DataObject*  pObj,const std::string &tname) override;
};

#endif



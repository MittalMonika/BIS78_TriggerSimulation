/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigCaloEventTPCnv
 * @class  : TrigTauClusterCnv_p1
 *
 * @brief transient persistent converter for TrigTauCluster
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <f.bucci@cern.ch>          - U. Geneva
 *
 * File and Version Information:
 * $Id: TrigTauClusterCnv_p1.h,v 1.2 2009-04-01 22:01:37 salvator Exp $
 **********************************************************************************/
#ifndef TRIGCALOEVENTTPCNV_TRIGTAUCLUSTERCNV_P1_H
#define TRIGCALOEVENTTPCNV_TRIGTAUCLUSTERCNV_P1_H

#include "TrigCaloEvent/TrigTauCluster.h"
#include "TrigCaloEventTPCnv/TrigTauCluster_p1.h"

#include "TrigCaloEventTPCnv/TrigCaloClusterCnv_p1.h"

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

class MsgStream;


class TrigTauClusterCnv_p1 : public T_AthenaPoolTPCnvConstBase<TrigTauCluster, TrigTauCluster_p1>
{
 public:
  using base_class::transToPers;
  using base_class::persToTrans;


  TrigTauClusterCnv_p1() {}
  
  virtual void persToTrans( const TrigTauCluster_p1 *persObj,
			    TrigTauCluster    *transObj,
			    MsgStream            &log ) const override;
  virtual void transToPers( const TrigTauCluster    *transObj,
			    TrigTauCluster_p1 *persObj,
			    MsgStream            &log ) const override;
};


#endif

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTSTRAWCONDALG_H
#define TRTSTRAWCONDALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "TRT_ConditionsData/AliveStraws.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h"
#include "GaudiKernel/ICondSvc.h"
#include "GaudiKernel/Property.h"
namespace InDetDD {
  class TRT_DetectorManager;
}
class TRT_ID;

class TRTStrawCondAlg : public AthReentrantAlgorithm
{
 public:
  typedef TRTCond::StrawStatusMultChanContainer StrawStatusContainer ;
  TRTStrawCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TRTStrawCondAlg() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;
  virtual int findArrayTotalIndex(const int det, const int lay) const;
  virtual int findArrayLocalWheelIndex(const int det, const int lay) const;


 private:
  ServiceHandle<ICondSvc> m_condSvc;
  SG::ReadCondHandleKey<StrawStatusContainer> m_strawReadKey{this,"StrawReadKey","/TRT/Cond/Status","Straw Status in-key"};
  SG::WriteCondHandleKey<TRTCond::AliveStraws> m_strawWriteKey{this,"StrawWriteKey","AliveStraws","AliveStraws out-key"};
  const InDetDD::TRT_DetectorManager* m_detManager;
  ToolHandle<ITRT_StrawStatusSummaryTool> m_strawStatus;
  const TRT_ID *m_trtId;
  bool m_isGEANT4;

};
#endif

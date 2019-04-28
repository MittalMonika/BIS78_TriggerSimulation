/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGEGAMMAMONITORING_TRIGEGAMMAMONITORALGORITHM_H
#define TRIGEGAMMAMONITORING_TRIGEGAMMAMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoring/Monitored.h"


class TrigEgammaMonitorAlgorithm : public AthMonitorAlgorithm {
 public:
  TrigEgammaMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~TrigEgammaMonitorAlgorithm();
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
};
#endif

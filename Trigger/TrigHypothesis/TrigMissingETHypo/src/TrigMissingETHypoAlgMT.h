/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGMISSINGETHYPO_TRIGMISSINGETHYPOALGMT_H
#define TRIGMISSINGETHYPO_TRIGMISSINGETHYPOALGMT_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "DecisionHandling/HypoBase.h"
#include "AthenaMonitoring/GenericMonitoringTool.h"


#include "TrigMissingETHypo/ITrigMissingETHypoToolMT.h"
#include "xAODTrigMissingET/TrigMissingETContainer.h"

using TrigCompositeUtils::DecisionContainer;

/**
 * @class TrigMissingETHypoAlgMT
 * @brief Implements MET selection for the HLT framework
 **/
class TrigMissingETHypoAlgMT : public ::HypoBase {
 public: 

  TrigMissingETHypoAlgMT( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~TrigMissingETHypoAlgMT(); 

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute( const EventContext& context ) const override;
  virtual StatusCode  finalize() override;
 
 private:

  // TODO: Consider taking muonContainer etc in case they are needed in future.
  StatusCode decide(const xAOD::TrigMissingETContainer*,
                    TrigCompositeUtils::DecisionContainer* newDecisions,
                    const DecisionContainer* previousDecisions
        ) const;
  StatusCode fillMonitoringHistograms(const xAOD::TrigMissingETContainer*) const;

  ToolHandleArray<ITrigMissingETHypoToolMT> m_hypoTools {
    this, "HypoTools", {}, "Hypo tools"};
     
  SG::ReadHandleKey< xAOD::TrigMissingETContainer > m_metKey {
    this, "METContainerKey", "HLT_MET", "MET Container name"};

  ToolHandle<GenericMonitoringTool> m_monTool{ this, "MonTool", "", "Monitoring tool" };  

  static float toLogGeV(const float& x, const float& fallback, const float& epsilon );
  static float toLinGeV(const float& x, const float& fallback, const float& epsilon );
  
  
}; 

#endif


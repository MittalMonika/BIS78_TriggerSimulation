/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGTAUHYPO_TrigEFTauMVHypoAlgMT_H
#define TRIGTAUHYPO_TrigEFTauMVHypoAlgMT_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthViews/View.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "DecisionHandling/TrigCompositeUtils.h"
#include "DecisionHandling/HypoBase.h"
#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetContainer.h"
#include "ITrigEFTauMVHypoTool.h"

class TrigEFTauMVHypoAlgMT : public ::HypoBase {
 public: 

  TrigEFTauMVHypoAlgMT( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~TrigEFTauMVHypoAlgMT(); 

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute( const EventContext& context ) const override;
  virtual StatusCode  finalize() override;
 
 private: 
  TrigEFTauMVHypoAlgMT();
  ToolHandleArray< ITrigEFTauMVHypoTool > m_hypoTools { this, "HypoTools", {}, "Hypo tools" };
     
  SG::ReadHandleKey< xAOD::TauJetContainer > m_tauJetKey { this, "taujetcontainer", "taujetcontainer", "taujets in view" };
  

}; 

#endif //> !TRIGEGAMMAHYPO_TESTTRIGL2TAUCALOHYPOALG_H
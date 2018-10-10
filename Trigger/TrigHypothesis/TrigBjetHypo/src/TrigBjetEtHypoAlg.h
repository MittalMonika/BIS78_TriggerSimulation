/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGBJETHYPO_TRIGBJETETHYPOALG_H
#define TRIGBJETHYPO_TRIGBJETETHYPOALG_H 1

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "DecisionHandling/TrigCompositeUtils.h"

#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"

#include "TrigBjetEtHypoTool.h"
#include "DecisionHandling/HypoBase.h"

/**
 * @class Implements b-jet selection for the new HLT framework
 * @brief 
 **/

class TrigBjetEtHypoAlg
  : public ::HypoBase
{ 
 public: 

  TrigBjetEtHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~TrigBjetEtHypoAlg(); 

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute_r( const EventContext& context ) const override;
  virtual StatusCode  finalize() override;
 
 private: 
  TrigBjetEtHypoAlg();
  ToolHandleArray< TrigBjetEtHypoTool > m_hypoTools {this,"HypoTools",{},"Hypo Tools"};

 private:
  SG::ReadHandleKey< xAOD::JetContainer > m_jetsKey {this,"Jets","Jets","Input Jet Container Key"};
  SG::WriteHandleKey< xAOD::JetContainer > m_outputJetsKey {this,"OutputJets","SplitJets","Output Jet Container Key"};
}; 

#endif //> !TRIGBJETHYPO_TRIGBJETETHYPOALG_H

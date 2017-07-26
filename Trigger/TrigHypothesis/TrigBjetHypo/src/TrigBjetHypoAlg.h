/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGBJETHYPOALG_H
#define TRIGBJETHYPOALG_H 1

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
//#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
//#include "xAODTrigCalo/TrigEMClusterContainer.h"
#include "DecisionHandling/TrigCompositeUtils.h"

//#include "xAODBTagging/BTaggingAuxContainer.h"
#include "xAODBTagging/BTaggingContainer.h"
//#include "xAODBTagging/BTagging.h"


#include "TrigBjetHypoTool.h"

/**
 * @class Implements b-jet selection for the new HLT framework
 * @brief 
 **/
using namespace TrigCompositeUtils;

class TrigBjetHypoAlg
  : public ::AthReentrantAlgorithm
{ 
 public: 

  TrigBjetHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~TrigBjetHypoAlg(); 

  StatusCode  initialize() override;
  StatusCode  execute_r( const EventContext& context ) const override;
  StatusCode  finalize() override;
 
 private: 
  TrigBjetHypoAlg();
  ToolHandleArray< TrigBjetHypoTool > m_hypoTools;

  // ----->>>>>>>
  SG::ReadHandleKey< TrigRoiDescriptorCollection > m_roisKey;
  SG::ReadHandleKey< xAOD::BTaggingContainer> m_btagKey;
  SG::WriteHandleKey< DecisionContainer > m_decisionsKey;
 
}; 


#endif //> !TRIGEGAMMAHYPO_TRIGL2CALOHYPOALG_H

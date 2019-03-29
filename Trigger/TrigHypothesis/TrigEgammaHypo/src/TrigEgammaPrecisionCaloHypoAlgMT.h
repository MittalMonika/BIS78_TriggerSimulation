/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGEGAMMAHYPO_TRIGL2CALOHYPOALGMT_PRECISION_H
#define TRIGEGAMMAHYPO_TRIGL2CALOHYPOALGMT_PRECISION_H 1

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthViews/View.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "xAODTrigCalo/TrigEMClusterContainer.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "DecisionHandling/TrigCompositeUtils.h"
#include "DecisionHandling/HypoBase.h"

#include "ITrigEgammaPrecisionCaloHypoTool.h"

/**
 * @class TrigEgammaPrecisionCaloHypoAlgMT
 * @brief Implements egamma calo selection for the new HLT framework
 **/
class TrigEgammaPrecisionCaloHypoAlgMT : public ::HypoBase {
 public: 

  TrigEgammaPrecisionCaloHypoAlgMT( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~TrigEgammaPrecisionCaloHypoAlgMT(); 

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute( const EventContext& context ) const override;
  virtual StatusCode  finalize() override;
 
 private: 
  TrigEgammaPrecisionCaloHypoAlgMT();
  ToolHandleArray< ITrigEgammaPrecisionCaloHypoTool > m_hypoTools { this, "HypoTools", {}, "Hypo tools" };
     
  SG::ReadHandleKey< xAOD::CaloClusterContainer > m_clustersKey { this, "CaloClusters", "CaloClusters", "CaloClusters in roi" };  
  

}; 

#endif //> !TRIGEGAMMAHYPO_TESTTRIGL2CALOHYPOALG_H

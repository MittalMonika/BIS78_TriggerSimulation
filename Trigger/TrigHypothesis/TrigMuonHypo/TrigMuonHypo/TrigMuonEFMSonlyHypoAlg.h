/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONEFMSONLYHYPO_TRIGMUONEFMSONLYHYPOALG_H 
#define TRIGMUONEFMSONLYHYPO_TRIGMUONEFMSONLYHYPOALG_H 1 

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h" 
#include "AthViews/View.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "DecisionHandling/TrigCompositeUtils.h"

#include "TrigMuonHypo/TrigMuonEFMSonlyHypoTool.h"

class StoreGateSvc;
class TriggerElement;

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

class TrigMuonEFMSonlyHypoAlg
   : public ::AthReentrantAlgorithm
{
  public:

   TrigMuonEFMSonlyHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );   

   virtual ~TrigMuonEFMSonlyHypoAlg();

   virtual StatusCode  initialize() override;
   virtual StatusCode  execute_r( const EventContext& context ) const override;
   virtual StatusCode  finalize() override;
    
  private:
 
    TrigMuonEFMSonlyHypoAlg(); 
    ToolHandleArray<TrigMuonEFMSonlyHypoTool> m_hypoTools {this, "HypoTools", {}, "Tools to perform selection"}; 

    SG::WriteHandleKey<TrigCompositeUtils::DecisionContainer> m_decisionsKey{
	this, "Decisions", "MuonEFMSonlyHypo_Decisions", "Name of the decisions object attached by TrigMuonEFMSonlyHypo"};

    SG::ReadHandleKey<std::vector< SG::View*>> m_viewsKey{
	this, "ViewRoIs", "MUViewRoIs", "Name of the input data on Views produced by EventCreatorAlgorithms"};

    SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{
	this, "MuonDecisions", "MuonEFMSonly_MuonData", "Name of the input data on xAOD::MuonContainer produced by MuonCreatorAlg"};

    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiKey{
	this, "RoIs", "MURoIs", "Name of the input data from L1Decoder"};

    SG::ReadHandleKey<TrigCompositeUtils::DecisionContainer> m_previousDecisionsKey{
	this, "L1Decisions", "MuonEFMSonly_Decisions", "Name of the input decisions object"};
};

#endif

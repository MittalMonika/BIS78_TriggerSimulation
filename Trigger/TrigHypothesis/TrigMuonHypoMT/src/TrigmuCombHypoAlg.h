/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUCOMBHYPO_TRIGMUCOMBHYPOALG_H 
#define TRIGMUCOMBHYPO_TRIGMUCOMBHYPOALG_H 1 

#include <string>

//#include "AthenaBaseComps/AthReentrantAlgorithm.h" 
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "xAODTrigMuon/L2CombinedMuonContainer.h"
#include "DecisionHandling/TrigCompositeUtils.h"
#include "AthViews/View.h"

#include "TrigmuCombHypoTool.h"
#include "DecisionHandling/HypoBase.h"

class TriggerElement;

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

class TrigmuCombHypoAlg
   : public ::HypoBase
{
  public:

   TrigmuCombHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );   

   virtual ~TrigmuCombHypoAlg();

   virtual StatusCode  initialize() override;
   virtual StatusCode  execute( const EventContext& context ) const override;
   virtual StatusCode  finalize() override;
    
  private:
 
    TrigmuCombHypoAlg(); 
    ToolHandleArray<TrigmuCombHypoTool> m_hypoTools {this, "HypoTools", {}, "Tools to perform selection"}; 

    SG::ReadHandleKey<xAOD::L2CombinedMuonContainer> m_muCombKey {
	this, "MuonL2CBInfoFromMuCombAlg", "MuonL2CBInfo", "Name of the input data produced by muCombMT"};
};

#endif

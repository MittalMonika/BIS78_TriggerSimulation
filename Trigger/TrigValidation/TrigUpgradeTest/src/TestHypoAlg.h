/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGUPGRADETEST_TESTHYPOALG_H
#define TRIGUPGRADETEST_TESTHYPOALG_H 1


#include <string>
#include "DecisionHandling/HypoBase.h"
#include "xAODTrigger/TrigCompositeContainer.h"
#include "DecisionHandling/TrigCompositeUtils.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "ITestHypoTool.h"
namespace HLTTest {

  /**
   * @class TestHypoAlg
   * @brief Invokes HypoTools and stores the result of the decision
   **/

  using namespace TrigCompositeUtils;
  
  class TestHypoAlg : public ::HypoBase 
  
  { 
  public: 
    TestHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~TestHypoAlg(); 

    StatusCode subInitialize() override;
    virtual StatusCode execute_r( const EventContext& context ) const override;
    StatusCode finalize() override;

  private:
    typedef TrigRoiDescriptor FeatureOBJ;
    typedef TrigRoiDescriptorCollection FeatureContainer;
    // the DecisionContainer read/write handle keys are in the base class, called previousDecisions and Output
    TestHypoAlg();
    ToolHandleArray<ITestHypoTool> m_tools                       { this, "HypoTools", {}, "Hypo tools" };
    SG::ReadHandleKey<xAOD::TrigCompositeContainer> m_recoInput  { this, "Input", "Input", "Key for reco input"};
    StringProperty m_linkName {this, "LinkName", "initialRoI",  "name of the link to the features in the decision, e.g. 'feature', 'initialRoI'"};
   
  }; 

} //> end namespace HLTTest
#endif //> !TRIGUPGRADETEST_TESTHYPOALG_H

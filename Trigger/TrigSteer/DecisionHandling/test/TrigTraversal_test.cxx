/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/EventContext.h"
#include "SGTools/TestStore.h"
#include "TestTools/initGaudi.h"
#include "TestTools/expect.h"
#include "TestTools/expect_exception.h"
#include "DecisionHandling/TrigCompositeUtils.h"
#include "xAODTrigger/TrigCompositeAuxContainer.h"
#include "xAODTrigger/TrigCompositeContainer.h"
#include "CxxUtils/checker_macros.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"


using TrigCompositeUtils::DecisionContainer;

void printPaths(const std::vector<ElementLinkVector<DecisionContainer>>& paths, const std::string& name);

template<class CONTAINER>
void printFeatures(const ElementLinkVector<CONTAINER>& featureContainer, const std::string& name);

/// @brief Test to check traversal functions of a graph of interconnect TrigComposite objects
///
/// This test hard-codes a Run 3 navigation structure and tests that the correct 
///
int main ATLAS_NOT_THREAD_SAFE () {

  using namespace TrigCompositeUtils;
  xAOD::TrigComposite::s_throwOnCopyError = true;

  errorcheck::ReportMessage::hideFunctionNames (true);

  // initialize Gaudi, SG
  ISvcLocator* pSvcLoc;
  Athena_test::initGaudi(pSvcLoc); 
  StoreGateSvc* pSG(0);
  assert( pSvcLoc->service("StoreGateSvc", pSG, true).isSuccess() );

  // Create a context
  IProxyDict* xdict = &*pSG;
  xdict = pSG->hiveProxyDict();
  EventContext ctx(0,0);
  ctx.setExtension( Atlas::ExtendedEventContext(xdict) );
  Gaudi::Hive::setCurrentContext (ctx);
  std::cout << "Context: " << ctx << std::endl;

  // check current context
  const EventContext& ctx1 = Gaudi::Hive::currentContext();
  std::cout << "Current context: " << ctx1 << std::endl;

  SG::WriteHandleKey<DecisionContainer> decisionContainerKey("MyDecisionContainer");
  SG::WriteHandleKey<xAOD::ElectronContainer> electronContainerKey("MyElectronContainer");
  SG::WriteHandleKey<xAOD::MuonContainer> muonContainerKey("MyMuonContainer");

  decisionContainerKey.initialize();
  electronContainerKey.initialize();
  muonContainerKey.initialize();

  SG::WriteHandle<DecisionContainer> decisionContainer = createAndStore( decisionContainerKey);
  DecisionContainer* decisionContainerPtr = decisionContainer.ptr();

  SG::WriteHandle<xAOD::ElectronContainer> electronContainer = createAndStoreWithAux<xAOD::ElectronContainer, xAOD::ElectronAuxContainer>( electronContainerKey, ctx1 );
  xAOD::ElectronContainer* electronContainerPtr = electronContainer.ptr();

  SG::WriteHandle<xAOD::MuonContainer> muonContainer = createAndStoreWithAux<xAOD::MuonContainer, xAOD::MuonAuxContainer>( muonContainerKey, ctx1 );
  xAOD::MuonContainer* muonContainerPtr = muonContainer.ptr();

  // Create a sufficiency complicated tree structure by hand with known expected output 
  // In the following 
  //    F = Filter
  //    IM = Input Maker
  //    H = Hypo
  //    CH = CombinedHypo

  constexpr unsigned HLT_mufast_chain = 1;
  constexpr unsigned HLT_mu_chain = 2;
  constexpr unsigned HLT_mu_em_chain = 3;
  constexpr unsigned HLT_em_chain = 4;

  // Starting nodes
  Decision* MU0 = newDecisionIn(decisionContainerPtr, "MU0");
  Decision* MU1 = newDecisionIn(decisionContainerPtr, "MU1");
  Decision* EM0 = newDecisionIn(decisionContainerPtr, "EM0");

  // Terminus node
  Decision* END = newDecisionIn(decisionContainerPtr, "HLTPassRaw"); // This name is important
  ElementLink<DecisionContainer> end_link(*decisionContainerPtr, decisionContainerPtr->size() - 1, ctx1);

  // First muon ROI
  xAOD::Muon* rec_1__mu0 = new xAOD::Muon(); // Step1 muon
  muonContainerPtr->push_back(rec_1__mu0);
  rec_1__mu0->setP4(5., 0., 0.);
  ElementLink<xAOD::MuonContainer> rec_1__mu0_link(*muonContainerPtr, muonContainerPtr->size() - 1, ctx1);
  
  // No rec_2__mu0. Fails hypo
  //
  //

  // Second muon ROI
  xAOD::Muon* rec_1__mu1 = new xAOD::Muon(); // Step1 muon
  muonContainerPtr->push_back(rec_1__mu1);
  rec_1__mu1->setP4(20., 0., 0.);
  ElementLink<xAOD::MuonContainer> rec_1__mu1_link(*muonContainerPtr, muonContainerPtr->size() - 1, ctx1);

  xAOD::Muon* rec_2__mu1 = new xAOD::Muon(); // Step2 muon
  muonContainerPtr->push_back(rec_2__mu1);
  rec_2__mu1->setP4(21., 0, 0);
  ElementLink<xAOD::MuonContainer> rec_2__mu1_link(*muonContainerPtr, muonContainerPtr->size() - 1, ctx1);

  // First EM ROI
  xAOD::Electron* rec_1__em0 = new xAOD::Electron(); // Step1 electron
  electronContainerPtr->push_back(rec_1__em0);
  rec_1__em0->setP4(30., 0., 0., 0.);
  ElementLink<xAOD::ElectronContainer> rec_1__em0_link(*electronContainerPtr, electronContainerPtr->size() - 1, ctx1);

  xAOD::Electron* rec_2__em0 = new xAOD::Electron(); // Step2 electron
  electronContainerPtr->push_back(rec_2__em0);
  rec_2__em0->setP4(31., 0., 0., 0.);
  ElementLink<xAOD::ElectronContainer> rec_2__em0_link(*electronContainerPtr, electronContainerPtr->size() - 1, ctx1);

  ///
  /// Muon RoI 0, single muon chains ID:HLT_mufast_chain, ID:2. Fails both chains at first Hypo
  ///

  {
    addDecisionID(HLT_mufast_chain, MU0);
    addDecisionID(HLT_mu_chain, MU0);

    Decision* MU_F_1__MU0 = newDecisionIn(decisionContainerPtr, "MU_F_1__MU0");
    linkToPrevious(MU_F_1__MU0, MU0);
    addDecisionID(HLT_mufast_chain, MU_F_1__MU0);
    addDecisionID(HLT_mu_chain, MU_F_1__MU0);

    Decision* MU_IM_1__MU0 = newDecisionIn(decisionContainerPtr, "MU_IM_1__MU0");
    linkToPrevious(MU_IM_1__MU0, MU_F_1__MU0);
    addDecisionID(HLT_mufast_chain, MU_IM_1__MU0);
    addDecisionID(HLT_mu_chain, MU_IM_1__MU0);

    Decision* MU_H_1__MU0 = newDecisionIn(decisionContainerPtr, "MU_H_1__MU0");
    linkToPrevious(MU_H_1__MU0, MU_IM_1__MU0);
    MU_H_1__MU0->setObjectLink<xAOD::MuonContainer>("feature", rec_1__mu0_link);
    // Fails HLT_mufast_chain
    // Fails HLT_mu_chain

  }

  ///
  /// Muon RoI 0, combined muon+electron chain ID:3. Fails first hypo
  ///

  {
    addDecisionID(HLT_mu_em_chain, MU0);

    Decision* MUEM_F_1__MU0 = newDecisionIn(decisionContainerPtr, "MUEM_F_1__MU0");
    linkToPrevious(MUEM_F_1__MU0, MU0);
    addDecisionID(HLT_mu_em_chain, MUEM_F_1__MU0);

    Decision* MUEM_IM_1__MU0 = newDecisionIn(decisionContainerPtr, "MUEM_IM_1__MU0");
    linkToPrevious(MUEM_IM_1__MU0, MUEM_F_1__MU0);
    addDecisionID(HLT_mu_em_chain, MUEM_IM_1__MU0);

    Decision* MUEM_H_1__MU0 = newDecisionIn(decisionContainerPtr, "MUEM_H_1__MU0");
    linkToPrevious(MUEM_H_1__MU0, MUEM_IM_1__MU0);
    MUEM_H_1__MU0->setObjectLink<xAOD::MuonContainer>("feature", rec_1__mu0_link);
    // Fails HLT_mu_em_chain

    Decision* MUEM_CH_1__MU0 = newDecisionIn(decisionContainerPtr, "MUEM_CH_1__MU0");
    linkToPrevious(MUEM_CH_1__MU0, MUEM_H_1__MU0);
    // Note: Combo hypo does not re-link to feature.
    // Fails HLT_mu_em_chain

  }

  ///
  /// Muon RoI 1, single muon chain ID:HLT_mufast_chain passes event after first Hypo. Single muon chain ID:HLT_mu_chain passes event after second hypo
  ///

  {
    addDecisionID(HLT_mufast_chain, MU1);
    addDecisionID(HLT_mu_chain, MU1);

    Decision* MU_F_1__MU1 = newDecisionIn(decisionContainerPtr, "MU_F_1__MU1");
    linkToPrevious(MU_F_1__MU1, MU1);
    addDecisionID(HLT_mufast_chain, MU_F_1__MU1);
    addDecisionID(HLT_mu_chain, MU_F_1__MU1);

    Decision* MU_IM_1__MU1 = newDecisionIn(decisionContainerPtr, "MU_IM_1__MU1");
    linkToPrevious(MU_IM_1__MU1, MU_F_1__MU1);
    addDecisionID(HLT_mufast_chain, MU_IM_1__MU1);
    addDecisionID(HLT_mu_chain, MU_IM_1__MU1);

    Decision* MU_H_1__MU1 = newDecisionIn(decisionContainerPtr, "MU_H_1__MU1");
    linkToPrevious(MU_H_1__MU1, MU_IM_1__MU1);
    MU_H_1__MU1->setObjectLink<xAOD::MuonContainer>("feature", rec_1__mu1_link);
    addDecisionID(HLT_mufast_chain, MU_H_1__MU1);
    addDecisionID(HLT_mu_chain, MU_H_1__MU1);

    // HLT_mufast_chain passes the event
    Decision* END_MU_H_1__MU1 = newDecisionIn(decisionContainerPtr, "END_MU_H_1__MU1");
    linkToPrevious(END_MU_H_1__MU1, MU_H_1__MU1);
    addDecisionID(HLT_mufast_chain, END_MU_H_1__MU1);
    linkToPrevious(END, END_MU_H_1__MU1);

    Decision* MU_F_2__MU1 = newDecisionIn(decisionContainerPtr, "MU_F_2__MU1");
    linkToPrevious(MU_F_2__MU1, MU_H_1__MU1);
    addDecisionID(HLT_mu_chain, MU_F_2__MU1);

    Decision* MU_IM_2__MU1 = newDecisionIn(decisionContainerPtr, "MU_IM_2__MU1");
    linkToPrevious(MU_IM_2__MU1, MU_F_2__MU1);
    addDecisionID(HLT_mu_chain, MU_IM_2__MU1);

    Decision* MU_H_2__MU1 = newDecisionIn(decisionContainerPtr, "MU_H_2__MU1");
    linkToPrevious(MU_H_2__MU1, MU_IM_2__MU1);
    MU_H_2__MU1->setObjectLink<xAOD::MuonContainer>("feature", rec_2__mu1_link);
    addDecisionID(HLT_mu_chain, MU_H_2__MU1);

    // HLT_mu_chain passes the event
    Decision* END_MU_H_2__MU1 = newDecisionIn(decisionContainerPtr, "END_MU_H_2__MU1");
    linkToPrevious(END_MU_H_2__MU1, MU_H_2__MU1);
    addDecisionID(HLT_mu_chain, END_MU_H_2__MU1);
    linkToPrevious(END, END_MU_H_2__MU1);
  }

  ///
  /// Muon RoI 1, combined muon+electron chain ID:HLT_mu_em_chain. Passes first and second hypo. Passes EM leg (later)
  ///

  {
    addDecisionID(HLT_mu_em_chain, MU1);

    Decision* MUEM_F_1__MU1 = newDecisionIn(decisionContainerPtr, "MUEM_F_1__MU1");
    linkToPrevious(MUEM_F_1__MU1, MU1);
    addDecisionID(HLT_mu_em_chain, MUEM_F_1__MU1);

    Decision* MUEM_IM_1__MU1 = newDecisionIn(decisionContainerPtr, "MUEM_IM_1__MU1");
    linkToPrevious(MUEM_IM_1__MU1, MUEM_F_1__MU1);
    addDecisionID(HLT_mu_em_chain, MUEM_IM_1__MU1);

    Decision* MUEM_H_1__MU1 = newDecisionIn(decisionContainerPtr, "MUEM_H_1__MU1");
    linkToPrevious(MUEM_H_1__MU1, MUEM_IM_1__MU1);
    MUEM_H_1__MU1->setObjectLink<xAOD::MuonContainer>("feature", rec_1__mu1_link);
    addDecisionID(HLT_mu_em_chain, MUEM_H_1__MU1);

    Decision* MUEM_CH_1__MU1 = newDecisionIn(decisionContainerPtr, "MUEM_CH_1__MU1");
    linkToPrevious(MUEM_CH_1__MU1, MUEM_H_1__MU1);
    addDecisionID(HLT_mu_em_chain, MUEM_CH_1__MU1);

    Decision* MUEM_F_2__MU1 = newDecisionIn(decisionContainerPtr, "MUEM_F_2__MU1");
    linkToPrevious(MUEM_F_2__MU1, MUEM_CH_1__MU1);
    addDecisionID(HLT_mu_em_chain, MUEM_F_2__MU1);

    Decision* MUEM_IM_2__MU1 = newDecisionIn(decisionContainerPtr, "MUEM_IM_2__MU1");
    linkToPrevious(MUEM_IM_2__MU1, MUEM_F_2__MU1);
    addDecisionID(HLT_mu_em_chain, MUEM_IM_2__MU1);

    Decision* MUEM_H_2__MU1 = newDecisionIn(decisionContainerPtr, "MUEM_H_2__MU1");
    linkToPrevious(MUEM_H_2__MU1, MUEM_IM_2__MU1);
    MUEM_H_2__MU1->setObjectLink<xAOD::MuonContainer>("feature", rec_2__mu1_link);
    addDecisionID(HLT_mu_em_chain, MUEM_H_2__MU1);

    Decision* MUEM_CH_2__MU1 = newDecisionIn(decisionContainerPtr, "MUEM_CH_2__MU1");
    linkToPrevious(MUEM_CH_2__MU1, MUEM_H_2__MU1);
    addDecisionID(HLT_mu_em_chain, MUEM_CH_2__MU1);

    // HLT_mu_em_chain passes the event
    Decision* END_MUEM_CH_2__MU1 = newDecisionIn(decisionContainerPtr, "END_MUEM_CH_2__MU1");
    linkToPrevious(END_MUEM_CH_2__MU1, MUEM_CH_2__MU1);
    addDecisionID(HLT_mu_em_chain, END_MUEM_CH_2__MU1);
    linkToPrevious(END, END_MUEM_CH_2__MU1);
  }

  ///
  /// EM RoI 0, single electron chain ID:4. Passes first and second hypo. Passes EM leg (later)
  ///

  {
    addDecisionID(HLT_em_chain, EM0);

    Decision* EM_F_1__EM0 = newDecisionIn(decisionContainerPtr, "EM_F_1__EM0");
    linkToPrevious(EM_F_1__EM0, EM0);
    addDecisionID(HLT_em_chain, EM_F_1__EM0);

    Decision* EM_IM_1__EM0 = newDecisionIn(decisionContainerPtr, "EM_IM_1__EM0");
    linkToPrevious(EM_IM_1__EM0, EM_F_1__EM0);
    addDecisionID(HLT_em_chain, EM_IM_1__EM0);

    Decision* EM_H_1__EM0 = newDecisionIn(decisionContainerPtr, "EM_H_1__EM0");
    linkToPrevious(EM_H_1__EM0, EM_IM_1__EM0);
    EM_H_1__EM0->setObjectLink<xAOD::ElectronContainer>("feature", rec_1__em0_link);
    addDecisionID(HLT_em_chain, EM_H_1__EM0);

    Decision* EM_F_2__EM0 = newDecisionIn(decisionContainerPtr, "EM_F_2__EM0");
    linkToPrevious(EM_F_2__EM0, EM_H_1__EM0);
    addDecisionID(HLT_em_chain, EM_F_2__EM0);

    Decision* EM_IM_2__EM0 = newDecisionIn(decisionContainerPtr, "EM_IM_2__EM0");
    linkToPrevious(EM_IM_2__EM0, EM_F_2__EM0);
    addDecisionID(HLT_em_chain, EM_IM_2__EM0);

    Decision* EM_H_2__EM0 = newDecisionIn(decisionContainerPtr, "EM_H_2__EM0");
    linkToPrevious(EM_H_2__EM0, EM_IM_2__EM0);
    EM_H_2__EM0->setObjectLink<xAOD::ElectronContainer>("feature", rec_2__em0_link);
    addDecisionID(HLT_em_chain, EM_H_2__EM0);

    // HLT_em_chain passes the event
    Decision* END_EM_H_2__EM0 = newDecisionIn(decisionContainerPtr, "END_EM_H_2__EM0");
    linkToPrevious(END_EM_H_2__EM0, EM_H_2__EM0);
    addDecisionID(HLT_em_chain, END_EM_H_2__EM0);
    linkToPrevious(END, END_EM_H_2__EM0);
  }

  ///
  /// EM RoI 0, combined muon+electron chain ID:HLT_mu_em_chain. Passes first and second hypo. Passes MU leg (earlier)
  ///

  {
    addDecisionID(HLT_mu_em_chain, EM0);

    Decision* MUEM_F_1__EM0 = newDecisionIn(decisionContainerPtr, "MUEM_F_1__EM0");
    linkToPrevious(MUEM_F_1__EM0, EM0);
    addDecisionID(HLT_mu_em_chain, MUEM_F_1__EM0);

    Decision* MUEM_IM_1__EM0 = newDecisionIn(decisionContainerPtr, "MUEM_IM_1__EM0");
    linkToPrevious(MUEM_IM_1__EM0, MUEM_F_1__EM0);
    addDecisionID(HLT_mu_em_chain, MUEM_IM_1__EM0);

    Decision* MUEM_H_1__EM0 = newDecisionIn(decisionContainerPtr, "MUEM_H_1__EM0");
    linkToPrevious(MUEM_H_1__EM0, MUEM_IM_1__EM0);
    MUEM_H_1__EM0->setObjectLink<xAOD::ElectronContainer>("feature", rec_1__em0_link);
    addDecisionID(HLT_mu_em_chain, MUEM_H_1__EM0);

    Decision* MUEM_CH_1__EM0 = newDecisionIn(decisionContainerPtr, "MUEM_CH_1__EM0");
    linkToPrevious(MUEM_CH_1__EM0, MUEM_H_1__EM0);
    addDecisionID(HLT_mu_em_chain, MUEM_CH_1__EM0);

    Decision* MUEM_F_2__EM0 = newDecisionIn(decisionContainerPtr, "MUEM_F_2__EM0");
    linkToPrevious(MUEM_F_2__EM0, MUEM_CH_1__EM0);
    addDecisionID(HLT_mu_em_chain, MUEM_F_2__EM0);

    Decision* MUEM_IM_2__EM0 = newDecisionIn(decisionContainerPtr, "MUEM_IM_2__EM0");
    linkToPrevious(MUEM_IM_2__EM0, MUEM_F_2__EM0);
    addDecisionID(HLT_mu_em_chain, MUEM_IM_2__EM0);

    Decision* MUEM_H_2__EM0 = newDecisionIn(decisionContainerPtr, "MUEM_H_2__EM0");
    linkToPrevious(MUEM_H_2__EM0, MUEM_IM_2__EM0);
    MUEM_H_2__EM0->setObjectLink<xAOD::ElectronContainer>("feature", rec_2__em0_link);
    addDecisionID(HLT_mu_em_chain, MUEM_H_2__EM0);

    Decision* MUEM_CH_2__EM0 = newDecisionIn(decisionContainerPtr, "MUEM_CH_2__EM0");
    linkToPrevious(MUEM_CH_2__EM0, MUEM_H_2__EM0);
    addDecisionID(HLT_mu_em_chain, MUEM_CH_2__EM0);

    // HLT_mu_em_chain passes the event
    Decision* END_MUEM_CH_2__EM0 = newDecisionIn(decisionContainerPtr, "END_MUEM_CH_2__EM0");
    linkToPrevious(END_MUEM_CH_2__EM0, MUEM_CH_2__EM0);
    addDecisionID(HLT_mu_em_chain, END_MUEM_CH_2__EM0);
    linkToPrevious(END, END_MUEM_CH_2__EM0);
  }

  // Test the graph

  std::vector<ElementLinkVector<DecisionContainer>> paths_HLT_mufast_chain;
  std::vector<ElementLinkVector<DecisionContainer>> paths_HLT_mu_chain;
  std::vector<ElementLinkVector<DecisionContainer>> paths_HLT_mu_em_chain;
  std::vector<ElementLinkVector<DecisionContainer>> paths_HLT_em_chain;
  std::vector<ElementLinkVector<DecisionContainer>> paths_HLT_all;

  recursiveGetDecisions(END, paths_HLT_mufast_chain, HLT_mufast_chain);
  recursiveGetDecisions(END, paths_HLT_mu_chain, HLT_mu_chain);
  recursiveGetDecisions(END, paths_HLT_mu_em_chain, HLT_mu_em_chain);
  recursiveGetDecisions(END, paths_HLT_em_chain, HLT_em_chain);
  recursiveGetDecisions(END, paths_HLT_all);

  printPaths(paths_HLT_mufast_chain, "HLT_mufast_chain");
  printPaths(paths_HLT_mu_chain, "HLT_mu_chain");
  printPaths(paths_HLT_mu_em_chain, "HLT_mu_em_chain");
  printPaths(paths_HLT_em_chain, "HLT_em_chain");
  printPaths(paths_HLT_all, "All");

  ElementLinkVector<xAOD::IParticleContainer> features_all_HLT_mufast_chain = getFeaturesOfType<xAOD::IParticleContainer>(paths_HLT_mufast_chain, false);
  ElementLinkVector<xAOD::IParticleContainer> features_all_HLT_mu_chain     = getFeaturesOfType<xAOD::IParticleContainer>(paths_HLT_mu_chain, false);
  ElementLinkVector<xAOD::IParticleContainer> features_all_HLT_mu_em_chain  = getFeaturesOfType<xAOD::IParticleContainer>(paths_HLT_mu_em_chain, false);
  ElementLinkVector<xAOD::IParticleContainer> features_all_HLT_em_chain     = getFeaturesOfType<xAOD::IParticleContainer>(paths_HLT_em_chain, false);
  ElementLinkVector<xAOD::IParticleContainer> features_all_HLT_all          = getFeaturesOfType<xAOD::IParticleContainer>(paths_HLT_all, false);

  printFeatures(features_all_HLT_mufast_chain, "[All features] HLT_mufast_chain");
  printFeatures(features_all_HLT_mu_chain, "[All features] HLT_mu_chain");
  printFeatures(features_all_HLT_mu_em_chain, "[All features] HLT_mu_em_chain");
  printFeatures(features_all_HLT_em_chain, "[All features] HLT_em_chain");
  printFeatures(features_all_HLT_all, "[All features] All chains");

  ElementLinkVector<xAOD::IParticleContainer> features_final_HLT_mufast_chain = getFeaturesOfType<xAOD::IParticleContainer>(paths_HLT_mufast_chain, true);
  ElementLinkVector<xAOD::IParticleContainer> features_final_HLT_mu_chain     = getFeaturesOfType<xAOD::IParticleContainer>(paths_HLT_mu_chain, true);
  ElementLinkVector<xAOD::IParticleContainer> features_final_HLT_mu_em_chain  = getFeaturesOfType<xAOD::IParticleContainer>(paths_HLT_mu_em_chain, true);
  ElementLinkVector<xAOD::IParticleContainer> features_final_HLT_em_chain     = getFeaturesOfType<xAOD::IParticleContainer>(paths_HLT_em_chain, true);
  ElementLinkVector<xAOD::IParticleContainer> features_final_HLT_all          = getFeaturesOfType<xAOD::IParticleContainer>(paths_HLT_all, true);

  printFeatures(features_final_HLT_mufast_chain, "[Final feature] HLT_mufast_chain");
  printFeatures(features_final_HLT_mu_chain, "[Final feature] HLT_mu_chain");
  printFeatures(features_final_HLT_mu_em_chain, "[Final feature] HLT_mu_em_chain");
  printFeatures(features_final_HLT_em_chain, "[Final feature] HLT_em_chain");
  printFeatures(features_final_HLT_all, "[Final feature] All chains");  

  // Check typed retrieval too
  ElementLinkVector<xAOD::MuonContainer>     features_final_mu  = getFeaturesOfType<xAOD::MuonContainer>(paths_HLT_mu_em_chain);
  ElementLinkVector<xAOD::ElectronContainer> features_final_em  = getFeaturesOfType<xAOD::ElectronContainer>(paths_HLT_mu_em_chain);
  printFeatures(features_final_mu, "[Explicit Final Muon Features] HLT_mu_em_chain");
  printFeatures(features_final_em, "[Explicit Final Electron Features] HLT_mu_em_chain");  

  // Check retrieval of a link which does NOT derive from IParticle
  END->setObjectLink<DecisionContainer>("notAnIParticle", end_link);
  EXPECT_EXCEPTION (SG::ExcCLIDMismatch, END->objectLink<xAOD::IParticleContainer>("notAnIParticle"));
  
  return 0;
  
}

template<class CONTAINER>
void printFeatures(const ElementLinkVector<CONTAINER>& featureContainer, const std::string& name) {
  std::cout << name << " features size:" << featureContainer.size() << std::endl;
  size_t count = 0;
  for (ElementLink<CONTAINER> featureLink : featureContainer) {
    std::cout << " Feature " << count++ << ":" << (*featureLink)->pt() << ",";
  }
  std::cout << std::endl;
}

void printPaths(const std::vector<ElementLinkVector<DecisionContainer>>& paths, const std::string& name) {
  std::cout << paths.size() << " paths for " << name << std::endl;
  size_t count = 0;
  for (const ElementLinkVector<DecisionContainer>& path : paths) {
    std::cout << "  Path " << count++ << "[";
    bool first = true;
    for (const ElementLink<DecisionContainer>& link : path) {
      if (!first) std::cout << " -> ";
      first = false;
      std::cout << (*link)->name();
      if ((*link)->hasObjectLink("feature")) {
        ElementLink<xAOD::IParticleContainer> feature = (*link)->objectLink<xAOD::IParticleContainer>("feature");
        std::cout << " F(" << (*feature)->pt() << ")";
      }
    }
    std::cout << "]" << std::endl;
  }
}


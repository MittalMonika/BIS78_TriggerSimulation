/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <JetAnalysisAlgorithms/JvtEfficiencyAlg.h>

#include <SelectionHelpers/SelectionHelpers.h>

//
// method implementations
//

namespace CP
{
  JvtEfficiencyAlg ::
  JvtEfficiencyAlg (const std::string& name, 
                     ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
    , m_efficiencyTool ("", this)
    , m_truthJetsName("AntiKt4TruthJets")
  {
    declareProperty ("efficiencyTool", m_efficiencyTool, "the efficiency tool we apply");
    declareProperty ("selection", m_selection, "the decoration for the JVT selection");
    declareProperty ("efficiency", m_efficiency, "the decoration for the JVT efficiency");
    declareProperty ("skipBadEfficiency", m_skipBadEfficiency, "whether to skip efficiency calculation if the selection failed");
    declareProperty ("truthJetCollection", m_truthJetsName, "the truth jet collection to use for truth tagging");
  }



  StatusCode JvtEfficiencyAlg ::
  initialize ()
  {
    ANA_CHECK (m_efficiencyTool.retrieve());
    m_systematicsList.addHandle (m_jetHandle);
    ANA_CHECK (m_systematicsList.addAffectingSystematics (m_efficiencyTool->affectingSystematics()));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());

    if (!m_selection.empty())
      ANA_CHECK (makeSelectionAccessor (m_selection, m_selectionAccessor));

    if (!m_efficiency.empty())
      m_efficiencyAccessor = std::make_unique<SG::AuxElement::Accessor<float> > (m_efficiency);

    return StatusCode::SUCCESS;
  }



  StatusCode JvtEfficiencyAlg ::
  execute ()
  {
    return m_systematicsList.foreach ([&] (const CP::SystematicSet& sys) -> StatusCode {
        ANA_CHECK (m_efficiencyTool->applySystematicVariation (sys));
        xAOD::JetContainer *jets = nullptr;
        ANA_CHECK (m_jetHandle.getCopy (jets, sys));

        const xAOD::JetContainer *truthjets = nullptr;
	if(!m_truthJetsName.empty()) {
	  ANA_CHECK(evtStore()->retrieve(truthjets,m_truthJetsName));
	  ANA_CHECK(m_efficiencyTool->tagTruth(jets,truthjets));
	}

        for (xAOD::Jet *jet : *jets)
        {
          bool goodJet = true;
          if (m_selectionAccessor || m_skipBadEfficiency)
          {
            goodJet = m_efficiencyTool->passesJvtCut (*jet);
            if (m_selectionAccessor)
              m_selectionAccessor->setBool (*jet, goodJet);
          }
          if (m_efficiencyAccessor && (goodJet || !m_skipBadEfficiency))
          {
            float efficiency = 1;
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, m_efficiencyTool->getEfficiencyScaleFactor (*jet, efficiency));
            (*m_efficiencyAccessor) (*jet) = efficiency;
          }
        }
        return StatusCode::SUCCESS;
      });
  }
}

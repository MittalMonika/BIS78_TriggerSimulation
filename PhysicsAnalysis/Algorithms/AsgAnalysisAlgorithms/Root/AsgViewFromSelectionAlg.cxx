/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgAnalysisAlgorithms/AsgViewFromSelectionAlg.h>

#include <xAODEgamma/PhotonContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODTau/TauJetContainer.h>

//
// method implementations
//

namespace CP
{
  template<typename Type> StatusCode AsgViewFromSelectionAlg ::
  executeTemplate (const CP::SystematicSet& sys)
  {
    Type *input = nullptr;
    ANA_CHECK (evtStore()->retrieve (input, m_inputHandle.getName (sys)));
    auto output = std::make_unique<Type> (SG::VIEW_ELEMENTS);
    for (auto particle : *input)
    {
      bool keep = true;
      for (const auto& accessor : m_accessors)
      {
        if ((accessor.first->getBits (*particle) | accessor.second) != selectionAccept())
        {
          keep = false;
          break;
        }
      }
      if (keep)
        output->push_back (particle);
    }
    ANA_CHECK (evtStore()->record (output.release(), m_outputHandle.getName (sys)));

    return StatusCode::SUCCESS;
  }



  StatusCode AsgViewFromSelectionAlg ::
  executeFindType (const CP::SystematicSet& sys)
  {
    const xAOD::IParticleContainer *input = nullptr;
    ANA_CHECK (m_inputHandle.retrieve (input, sys));

    if (dynamic_cast<const xAOD::ElectronContainer*> (input))
      m_function = &AsgViewFromSelectionAlg::executeTemplate<xAOD::ElectronContainer>;
    else if (dynamic_cast<const xAOD::PhotonContainer*> (input))
      m_function = &AsgViewFromSelectionAlg::executeTemplate<xAOD::PhotonContainer>;
    else if (dynamic_cast<const xAOD::JetContainer*> (input))
      m_function = &AsgViewFromSelectionAlg::executeTemplate<xAOD::JetContainer>;
    else if (dynamic_cast<const xAOD::MuonContainer*> (input))
      m_function = &AsgViewFromSelectionAlg::executeTemplate<xAOD::MuonContainer>;
    else if (dynamic_cast<const xAOD::TauJetContainer*> (input))
      m_function = &AsgViewFromSelectionAlg::executeTemplate<xAOD::TauJetContainer>;
    else
    {
      ANA_MSG_ERROR ("unknown type contained in AsgViewFromSelectionAlg, please extend it");
      return StatusCode::FAILURE;
    }

    return (this->*m_function) (sys);
  }



  AsgViewFromSelectionAlg ::
  AsgViewFromSelectionAlg (const std::string& name, 
                           ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
  {
    /// \todo this would probably better be an std::map, but this
    /// isn't supported as a property type for AnaAlgorithm right now
    declareProperty ("selection", m_selection, "the list of selection decorations");
    declareProperty ("ignore", m_ignore, "the list of cuts to *ignore* for each selection");
  }



  StatusCode AsgViewFromSelectionAlg ::
  initialize ()
  {
    m_systematicsList.addHandle (m_inputHandle);
    m_systematicsList.addHandle (m_outputHandle);
    ANA_CHECK (m_systematicsList.initialize());

    if (m_ignore.size() > m_selection.size())
    {
      ANA_MSG_ERROR ("ignore property can't have more properties than selection property");
      return StatusCode::FAILURE;
    }
    for (std::size_t iter = 0, end = m_selection.size(); iter != end; ++ iter)
    {
      SelectionType ignore = 0;
      if (iter < m_ignore.size())
        ignore = m_ignore[iter];
      std::unique_ptr<ISelectionAccessor> accessor;
      ANA_CHECK (makeSelectionAccessor (m_selection[iter], accessor));
      m_accessors.push_back (std::make_pair (std::move (accessor), ignore));
    }

    return StatusCode::SUCCESS;
  }



  StatusCode AsgViewFromSelectionAlg ::
  execute ()
  {
    return m_systematicsList.foreach ([&] (const CP::SystematicSet& sys) -> StatusCode {
        return (this->*m_function) (sys);});
  }
}

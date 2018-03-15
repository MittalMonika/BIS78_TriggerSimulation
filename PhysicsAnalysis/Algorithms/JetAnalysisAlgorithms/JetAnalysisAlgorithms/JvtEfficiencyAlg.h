/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef JET_ANALYSIS_ALGORITHMS__JVT_EFFICIENCY_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JVT_EFFICIENCY_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <JetJvtEfficiency/IJetJvtEfficiency.h>
#include <SystematicsHandles/OutOfValidityHelper.h>
#include <SystematicsHandles/SelectionHelpers.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODJet/JetContainer.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IJEREfficiencyTool

  class JvtEfficiencyAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    JvtEfficiencyAlg (const std::string& name, 
                         ISvcLocator* pSvcLocator);


  public:
    StatusCode initialize () override;

  public:
    StatusCode execute () override;
    


    /// \brief the efficiency tool
  private:
    ToolHandle<CP::IJetJvtEfficiency> m_efficiencyTool;

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysCopyHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "AntiKt4EMTopoJets", "the jet collection to run on"};

    /// \brief the decoration for the JVT selection
  private:
    std::string m_selection;

    /// \brief the accessor for \ref m_selection
  private:
    std::unique_ptr<const SG::AuxElement::Accessor<SelectionType> > m_selectionAccessor;

    /// \brief the decoration for the JVT efficiency
  private:
    std::string m_efficiency;

    /// \brief the accessor for \ref m_efficiency
  private:
    std::unique_ptr<const SG::AuxElement::Accessor<float> > m_efficiencyAccessor;

    /// \brief whether to skip efficiency calculation if the selection failed
  private:
    bool m_skipBadEfficiency = false;

    /// \brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};
  };
}

#endif

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgAnalysisAlgorithms/AsgSelectionAlg.h>

#include <PATInterfaces/ISystematicsTool.h>

//
// method implementations
//

namespace CP
{
  AsgSelectionAlg ::
  AsgSelectionAlg (const std::string& name, 
                     ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
    , m_selectionTool ("", this)
  {
    declareProperty ("selectionTool", m_selectionTool, "the selection tool we apply");
    declareProperty ("selectionDecoration", m_selectionDecoration, "the decoration for the asg selection");
  }



  StatusCode AsgSelectionAlg ::
  initialize ()
  {
    if (m_selectionDecoration.empty())
    {
      ANA_MSG_ERROR ("no selection decoration name set");
      return StatusCode::FAILURE;
    }
    ANA_CHECK (makeSelectionAccessor (m_selectionDecoration, m_selectionAccessor));

    ANA_CHECK (m_selectionTool.retrieve());
    m_systematicsTool = dynamic_cast<ISystematicsTool*>(&*m_selectionTool);
    if (m_systematicsTool)
      ANA_CHECK (m_systematicsList.addAffectingSystematics (m_systematicsTool->affectingSystematics()));
      
    m_systematicsList.addHandle (m_particlesHandle);
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode AsgSelectionAlg ::
  execute ()
  {
    return m_systematicsList.foreach ([&] (const CP::SystematicSet& sys) -> StatusCode {
        if (m_systematicsTool)
          ANA_CHECK (m_systematicsTool->applySystematicVariation (sys));

        xAOD::IParticleContainer *particles = nullptr;
        ANA_CHECK (m_particlesHandle.getCopy (particles, sys));
        for (xAOD::IParticle *particle : *particles)
        {
          m_selectionAccessor->setBits
            (*particle, selectionFromAccept (m_selectionTool->accept (particle)));
        }
        return StatusCode::SUCCESS;
      });
  }
}

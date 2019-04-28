/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


//
// includes
//

#include <AsgAnalysisAlgorithms/AsgEventScaleFactorAlg.h>

// #include <SelectionHelpers/SelectionHelpers.h>

//
// method implementations
//

namespace CP
{
  AsgEventScaleFactorAlg ::
  AsgEventScaleFactorAlg (const std::string& name, 
                          ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
  {
    declareProperty ("scaleFactorDecoration", m_scaleFactorDecoration, "the decoration for the scale factor");
  }



  StatusCode AsgEventScaleFactorAlg ::
  initialize ()
  {
    if (m_scaleFactorDecoration.empty())
    {
      ANA_MSG_ERROR ("no scale factor decoration name set");
      return StatusCode::FAILURE;
    }

    m_systematicsList.addHandle (m_eventInfoHandle);
    m_systematicsList.addHandle (m_particleHandle);
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_preselection.initialize());

    m_scaleFactorAccessor = std::make_unique<SG::AuxElement::Accessor<float> > (m_scaleFactorDecoration);

    return StatusCode::SUCCESS;
  }



  StatusCode AsgEventScaleFactorAlg ::
  execute ()
  {
    return m_systematicsList.foreach ([&] (const CP::SystematicSet& sys) -> StatusCode {
      xAOD::EventInfo *eventInfo = nullptr;
      ANA_CHECK (m_eventInfoHandle.getCopy (eventInfo, sys));

      xAOD::IParticleContainer *particles = nullptr;
      ANA_CHECK (m_particleHandle.getCopy (particles, sys));

      float scaleFactor = 1;
      for (xAOD::IParticle *particle : *particles)
      {
        if (m_preselection.getBool (*particle))
        {
          scaleFactor *= (*m_scaleFactorAccessor) (*particle);
        }
      }

      (*m_scaleFactorAccessor) (*eventInfo) = scaleFactor;

      return StatusCode::SUCCESS;
    });
  }
}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackToVertexWrapper.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_TRACKTOVERTEXWRAPPER_H
#define DERIVATIONFRAMEWORK_TRACKTOVERTEXWRAPPER_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/VertexContainer.h"
#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"

namespace DerivationFramework {

  class TrackToVertexWrapper : public AthAlgTool, public IAugmentationTool {
    public: 
      TrackToVertexWrapper(const std::string& t, const std::string& n, const IInterface* p);

      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode addBranches() const;

    private:
      ToolHandle< Trk::ITrackToVertexIPEstimator > m_tool;
      std::string m_sgName;
      std::string m_containerName;
      SG::ReadHandleKey<xAOD::VertexContainer> m_vertexKey {this, "VertexContainer", "PrimaryVertices", "primary vertex container"};
  }; 
}

#endif // DERIVATIONFRAMEWORK_TRACKTOVERTEXWRAPPER_H

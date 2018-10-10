/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// JetTrackParticleThinning.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_VERTEXPARTICLETHINNING_H
#define DERIVATIONFRAMEWORK_VERTEXPARTICLETHINNING_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "GaudiKernel/ToolHandle.h"

namespace ExpressionParsing {
  class ExpressionParser;
}

class IThinningSvc;

namespace DerivationFramework {

  class VertexParticleThinning : public AthAlgTool, public IThinningTool {
    public: 
      VertexParticleThinning(const std::string& t, const std::string& n, const IInterface* p);
      ~VertexParticleThinning();
      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode doThinning() const;

    private:
      ServiceHandle<IThinningSvc> m_thinningSvc;
      mutable unsigned int m_ntot, m_npass;
      std::string m_vertexSGKey, m_inDetSGKey, m_selectionString;
      bool m_and;
      ExpressionParsing::ExpressionParser *m_parser;
  }; 
}

#endif // DERIVATIONFRAMEWORK_JETTRACKPARTICLETHINNING_H

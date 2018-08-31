/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EvenTInfoBSErrDecorator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_EVENTINFOBSERRDECORATOR_H
#define DERIVATIONFRAMEWORK_EVENTINFOBSERRDECORATOR_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthLinks/ElementLink.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"

#include "SCT_ConditionsTools/ISCT_ByteStreamErrorsTool.h"
#include "SCT_Cabling/ISCT_CablingTool.h"
#include "SCT_Cabling/SCT_OnlineId.h"

class AtlasDetectorID;
class SCT_ID;

namespace DerivationFramework {

  class EventInfoBSErrDecorator : public AthAlgTool, public IAugmentationTool {
    public: 
      EventInfoBSErrDecorator(const std::string& type, const std::string& name, const IInterface* parent);

      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode addBranches() const;

    private:
    
      std::string m_sgName;
      std::string m_containerName;
    
      const SCT_ID*          m_sctId;

      ToolHandle<ISCT_ByteStreamErrorsTool> m_byteStreamErrTool{this, "ByteStreamErrTool", "SCT_ByteStreamErrorsTool", "Tool to retrieve SCT ByteStream Errors"};
      ToolHandle<ISCT_CablingTool> m_cabling{this, "SCT_CablingTool", "SCT_CablingTool", "Tool to retrieve SCT Cabling"};

  }; 
}

#endif // DERIVATIONFRAMEWORK_EVENTINFOBSERRDECORATOR_H

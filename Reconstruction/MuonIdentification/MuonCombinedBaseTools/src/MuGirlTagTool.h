/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEDBASETOOLS_MUGIRLTAGTOOL_H
#define MUONCOMBINEDBASETOOLS_MUGIRLTAGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCombinedToolInterfaces/IMuonCombinedInDetExtensionTool.h"
#include "MuonCombinedEvent/InDetCandidateCollection.h"

#include "MuGirlInterfaces/IMuGirlRecoTool.h"


namespace MuonCombined {

  class MuGirlTagTool: public AthAlgTool, virtual public IMuonCombinedInDetExtensionTool
  {

  public:
    MuGirlTagTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~MuGirlTagTool(void); // destructor
  
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    /**IMuonCombinedInDetExtensionTool interface: extend ID candidate */    
    virtual void extend( const InDetCandidateCollection& inDetCandidates, InDetCandidateToTagMap* tagMap ) override;

    virtual void extendWithPRDs(const InDetCandidateCollection& inDetCandidates, InDetCandidateToTagMap* tagMap, 
				const Muon::MdtPrepDataContainer* mdtPRDs, const Muon::CscPrepDataContainer* cscPRDs, const Muon::RpcPrepDataContainer* rpdPRDs, 
				const Muon::TgcPrepDataContainer* tgcPRDs, const Muon::sTgcPrepDataContainer* sTGCPRDs, const Muon::MMPrepDataContainer* mmPRDs) override;


  private:
    // Tool Handles
    ToolHandle<MuGirlNS::IMuGirlRecoTool> m_pMuGirlReconstruction;       /**< The MuGirl reconstruction tool */

  };

}	// end of namespace

#endif



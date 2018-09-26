/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_RPCDATAPREPARATOR_H
#define  TRIGL2MUONSA_RPCDATAPREPARATOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "ByteStreamCnvSvcBase/ROBDataProviderSvc.h"
#include "TrigT1Interfaces/RecMuonRoI.h"
#include "MuonRDO/RpcPadContainer.h"

#include "TrigL2MuonSA/RpcData.h"
#include "TrigL2MuonSA/RpcFitResult.h"
#include "TrigL2MuonSA/RpcPatFinder.h"
#include "TrigL2MuonSA/RecMuonRoIUtils.h"
#include "RegionSelector/IRegSelSvc.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include "MuonRPC_Cabling/MuonRPC_CablingSvc.h"
#include "MuonIdHelpers/MuonIdHelperTool.h"
#include "MuonIdHelpers/RpcIdHelper.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonPrepRawData/RpcPrepDataContainer.h"
#include "MuonPrepRawData/RpcPrepDataCollection.h"

#include "MuonRecToolInterfaces/IMuonCombinePatternTool.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"


class StoreGateSvc;
class ActiveStoreSvc;
class RpcIdHelper;

namespace HLT {
  class TriggerElement;
  class Algo;
}

namespace MuonGM {
  class MuonDetectorManager;
  class RpcReadoutElement;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

namespace TrigL2MuonSA {

class RpcDataPreparator: public AthAlgTool
{
   public:
      
      static const InterfaceID& interfaceID();

   public:

      RpcDataPreparator(const std::string& type, 
			const std::string& name,
			const IInterface*  parent);
    
      ~RpcDataPreparator();
    
      virtual StatusCode initialize();
      virtual StatusCode finalize  ();
    
      StatusCode prepareData(const TrigRoiDescriptor*    p_roids,
			     unsigned int roiWord,
			     TrigL2MuonSA::RpcHits&      rpcHits,
			     ToolHandle<RpcPatFinder>*   rpcPatFinder);

      bool isFakeRoi() { return m_isFakeRoi; }

      void setRoIBasedDataAccess(bool use_RoIBasedDataAccess);

 private:
		       
      // Reference to StoreGateSvc;
      ServiceHandle<StoreGateSvc>   m_storeGateSvc;
      ServiceHandle<ActiveStoreSvc> m_activeStore;
      
      // Region Selector
      ServiceHandle<IRegSelSvc> m_regionSelector;

      // RPC cabling service
      const IRPCcablingSvc* m_rpcCabling;
      const CablingRPCBase* m_rpcCablingSvc;
      
      const MuonGM::MuonDetectorManager* m_muonMgr;
      // Muon Id Helpers
      const RpcIdHelper* m_rpcIdHelper;

      // handles to the RoI driven data access
      ToolHandle<Muon::IMuonRawDataProviderTool> m_rawDataProviderTool;
      ToolHandle<Muon::IMuonRdoToPrepDataTool> m_rpcPrepDataProvider;
      //ToolHandle<Muon::IMuonRdoToPrepDataTool> m_rpcPrepDataProvider {
      // 	this, "RpcPrepDataProvider", "Muon::RpcRdoToPrepDataTool/RpcPrepDataProviderTool", ""};
      
      ToolHandle <Muon::MuonIdHelperTool>  m_idHelperTool;  //!< Pointer to concrete tool

      SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_rpcPrepContainerKey{
	this, "RpcPrepDataContainer", "RPC_Measurements", "Neme of the RPCContainer to read in"};

      // Declare the keys used to access the data: one for reading and one
      // for writing.
      bool m_use_RoIBasedDataAccess;
      bool m_isFakeRoi;

      // Flag to decide whether or not to run BS decoding
      Gaudi::Property< bool > m_decodeBS { this, "DecodeBS", true, "Flag to decide whether or not to run BS->RDO decoding" };
};

} // namespace TrigL2MuonSA

#endif  // TRIGL2MUONSA_RPCDATAPREPARATOR_H

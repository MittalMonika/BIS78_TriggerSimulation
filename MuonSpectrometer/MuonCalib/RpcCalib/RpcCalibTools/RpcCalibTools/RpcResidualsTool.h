/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RPC_RESIDUALTOOL
#define  RPC_RESIDUALTOOL

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "AthLinks/DataLink.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkExInterfaces/INavigator.h"
#include "TrkExInterfaces/IPropagator.h"
#include "MuonTGRecTools/IMuonTGMeasTool.h"
#include "TrkParameters/TrackParameters.h"
#include "RpcCalibTools/RpcExtrapolationTool.h"

class RpcIdHelper;
class IRegSelSvc;
namespace Muon{
  class MuonIdHelperTool;
}



struct RpcResiduals{

  Identifier id;
  float residual;
  int csize;
  float pitch;
  float time;
  int hasTwin;
  const Trk::TrackParameters* params;

};

class RpcResidualsTool: public AthAlgTool{


 public:

  RpcResidualsTool(const std::string& type, const std::string& name,
			  const IInterface* parent );
  virtual ~RpcResidualsTool();

  static const InterfaceID& interfaceID( );
  
  virtual StatusCode initialize();
  virtual StatusCode finalize();

  void getRpcResiduals(TrackCollection::const_iterator theTrack, std::vector<RpcResiduals> &theResult);

  Identifier GetTwinPanel(Identifier id);
  Identifier GetTwinViewPanel(Identifier id);

 private:

  std::string m_clusCollection;
  ActiveStoreSvc* m_activeStore;

  // MuonDetectorManager from the conditions store
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey {this, "DetectorManagerKey", 
      "MuonDetectorManager", 
      "Key of input MuonDetectorManager condition data"};    
  
  ToolHandle<Muon::MuonIdHelperTool> m_muonIdHelperTool{this, "idHelper", 
    "Muon::MuonIdHelperTool/MuonIdHelperTool", "Handle to the MuonIdHelperTool"};
  RpcExtrapolationTool*           m_rpcExtrapolTool;
};


#endif

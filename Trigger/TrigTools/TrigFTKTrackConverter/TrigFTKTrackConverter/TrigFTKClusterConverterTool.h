/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

//implementation

#ifndef TRIGFTKTRACKCONVERTER_TRIGFTKCLUSTERCONVERTERTOOL_H
#define TRIGFTKTRACKCONVERTER_TRIGFTKCLUSTERCONVERTERTOOL_H

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigFTKSim/FTKTrack.h"
#include "TrigFTKToolInterfaces/ITrigFTKClusterConverterTool.h"

#include "TrkFitterInterfaces/ITrackFitter.h" 
#include "TrkFitterUtils/FitterTypes.h" 
#include "PixelConditionsServices/IPixelOfflineCalibSvc.h"
#include "InDetCondServices/ISiLorentzAngleTool.h"

class StoreGateSvc;

class AtlasDetectorID;
class PixelID;
class SCT_ID;
class Identifier;
class IdentifierHash;

namespace InDetDD {
  class PixelDetectorManager;
  class SCT_DetectorManager;
}

namespace InDet {
  class PixelCluster;
  class SCT_Cluster;
}

class PRD_MultiTruthCollection;
class McEventCollection;


class TrigFTKClusterConverterTool : public AthAlgTool, virtual public ITrigFTKClusterConverterTool
{
 public:
  TrigFTKClusterConverterTool( const std::string&, const std::string&, const IInterface* );
  virtual ~TrigFTKClusterConverterTool(){};
  virtual StatusCode initialize();
  virtual StatusCode finalize  ();

 protected:

  InDet::SCT_Cluster*  createSCT_Cluster(IdentifierHash , float , int);
  InDet::PixelCluster* createPixelCluster(IdentifierHash , float , float , float , float , float );
  StatusCode getMcTruthCollections();
  void createSCT_Truth(Identifier id, const MultiTruth& mt, PRD_MultiTruthCollection *sctTruth, const McEventCollection*  mcEventCollection, StoreGateSvc* evtStore, std::string mcTruthName);
  void createPixelTruth(Identifier id, const MultiTruth& mt, PRD_MultiTruthCollection *pxlTruth, const McEventCollection*  mcEventCollection, StoreGateSvc* evtStore, std::string mcTruthName);

  InDet::SCT_ClusterCollection*  getCollection(InDet::SCT_ClusterContainer*, IdentifierHash); 
  InDet::PixelClusterCollection*  getCollection(InDet::PixelClusterContainer*, IdentifierHash); 
  StatusCode getMcTruthCollections(  StoreGateSvc* evtStore, const McEventCollection*  mcEventCollection,  std::string ftkPixelTruthName,   std::string ftkSctTruthName, std::string mcTruthName);

private:

  ServiceHandle<IPixelOfflineCalibSvc> m_offlineCalibSvc;

  bool m_usePixelCalibSvc;

  StoreGateSvc* m_evtStore;

  const PixelID* m_pixelId;
  const SCT_ID* m_sctId;
  
  const InDetDD::PixelDetectorManager* m_pixelManager;
  const InDetDD::SCT_DetectorManager* m_SCT_Manager;

  ToolHandle<ISiLorentzAngleTool> m_pixelLorentzAngleTool{this, "PixelLorentzAngleTool", "PixelLorentzAngleTool", "Tool to retreive Lorentz angle of Pixel"};
  ToolHandle<ISiLorentzAngleTool> m_sctLorentzAngleTool{this, "SCTLorentzAngleTool", "SCTLorentzAngleTool", "Tool to retreive Lorentz angle of SCT"};
  ToolHandle<Trk::ITrackFitter> m_trackFitter;
  bool m_doFit;
  bool m_doTruth;
  std::string m_ftkPixelTruthName;
  std::string m_ftkSctTruthName;
  std::string m_mcTruthName;

  PRD_MultiTruthCollection* m_ftkPixelTruth;
  PRD_MultiTruthCollection* m_ftkSctTruth;
  const AtlasDetectorID* m_idHelper;
  bool m_collectionsReady;
};

#endif

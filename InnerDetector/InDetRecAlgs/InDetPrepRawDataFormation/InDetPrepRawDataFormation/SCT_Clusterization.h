/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**   @file SCT_Clusterization
 *   Header file for the SCT_Clusterization class (an Algorithm).
 *   @author Paul Bell, Tommaso Lari, Shaun Roe, Carl Gwilliam
 *   @date 08 July 2008
 */


#ifndef INDETPREPRAWDATAFORMATION_SCT_CLUSTERIZATION_H
#define INDETPREPRAWDATAFORMATION_SCT_CLUSTERIZATION_H
//STL
#include <string>
#include <map>
#include <set>

//Gaudi
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
// Base class
#include "AthenaBaseComps/AthAlgorithm.h"

//InDet includes
//Stored by value, cannot be fwd declared
#include "Identifier/IdentifierHash.h"
//template parameter, so should not be possible to just fwd declare
#include "Identifier/Identifier.h"
//Next contains a typedef so cannot be fwd declared
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "InDetPrepRawData/SiClusterContainer.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "SCT_ConditionsData/SCT_FlaggedCondData.h"
//tool/service handle template parameters
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "IRegionSelector/IRegSelSvc.h"
#include "InDetConditionsSummaryService/IInDetConditionsTool.h"
#include "SiClusterizationTool/ISCT_ClusteringTool.h"

class SCT_ID;
class SCT_ChannelStatusAlg;
class SiDetectorManager;
class ISvcLocator;
class StatusCode;

namespace InDetDD{
  class SiDetectorManager;
}


namespace InDet {
/**
 *    @class SCT_Clusterization
 *    @brief Form clusters from SCT Raw Data Objects
 *    The class loops over an RDO grouping strips and creating collections of clusters, subsequently recorded in StoreGate
 *    Uses SCT_ConditionsTools to determine which strips to include.
 */
class SCT_Clusterization : public AthAlgorithm {
public:
  /// Constructor with parameters:
  SCT_Clusterization(const std::string &name,ISvcLocator *pSvcLocator);
  
  /**    @name Usual algorithm methods */
  //@{
  ///Retrieve the tools used and initialize variables
  virtual StatusCode initialize() override;
  ///Form clusters and record them in StoreGate (detector store)
  virtual StatusCode execute() override;
  ///Clean up and release the collection containers
  virtual StatusCode finalize() override;
  //@}


private:
  /**    @name Disallow default instantiation, copy, assignment */
  //@{
  SCT_Clusterization() = delete;
  SCT_Clusterization(const SCT_Clusterization&) = delete;
  SCT_Clusterization &operator=(const SCT_Clusterization&) = delete;
  //@}

  ToolHandle< ISCT_ClusteringTool >        m_clusteringTool;       //!< Clustering algorithm
  std::string                              m_managerName;   //REMOVE LATER       //!< Detector manager name in StoreGate
  const SCT_ID*                            m_idHelper;

  SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey;
  ServiceHandle<IRegSelSvc>     m_regionSelector;     //!< region selector service

  SG::ReadHandleKey<SCT_RDO_Container> m_rdoContainerKey;
  bool m_roiSeeded;                                //!< detector manager name in StoreGate
  
  
  SG::WriteHandleKey<SCT_ClusterContainer> m_clusterContainerKey;
  SG::WriteHandleKey<SiClusterContainer> m_clusterContainerLinkKey;
  SG::WriteHandleKey<SCT_FlaggedCondData> m_flaggedCondDataKey;
  const InDetDD::SiDetectorManager*        m_manager;
  unsigned int                             m_maxRDOs;
  ToolHandle<IInDetConditionsTool>         m_pSummaryTool{this, "conditionsTool",
      "SCT_ConditionsSummaryTool/InDetSCT_ConditionsSummaryTool", "Tool to retrieve SCT conditions summary"};
  bool                                     m_checkBadModules;
  std::set<IdentifierHash>                 m_flaggedModules;
  unsigned int                             m_maxTotalOccupancyPercent;
  SG::UpdateHandleKey<SCT_ClusterContainerCache> m_clusterContainerCacheKey;
};

}

#endif // INDETRIOMAKER_SCT_CLUSTERIZATION_H

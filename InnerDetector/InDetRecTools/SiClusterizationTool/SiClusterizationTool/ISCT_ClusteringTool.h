/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ISCT_ClusteringTool.h
 * Header file for the ISCT_ClusteringTool clas
 * @author Paul J. Bell, 2004
 */


#ifndef SICLUSTERIZATIONTOOL_ISCT_CLUSTERINGTOOL_H
#define SICLUSTERIZATIONTOOL_ISCT_CLUSTERINGTOOL_H

// Base class
#include "GaudiKernel/IAlgTool.h"
#include "InDetRawData/InDetRawDataCollection.h"
#include "InDetRawData/SCT_RDORawData.h"

// no forward declare ... typedef
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "GaudiKernel/ToolHandle.h"

class SCT_ID;

class SCT_ChannelStatusAlg;

namespace InDet {

  ///Interface for SCT clustering algorithms, taking input from RDOs
  class ISCT_ClusteringTool : virtual public IAlgTool 
  {
  public:

    // InterfaceID
    DeclareInterfaceID(ISCT_ClusteringTool, 1, 0);

    virtual ~ISCT_ClusteringTool() = default;

    /** Deprecated method: Clusterize a collection of SCT raw data objects
     * Called from SCT_clusterization algorithm in InDetPrepRawDataFormation, 
     * once per module
     * @param[in] @c RDOs the raw data objects
     * @param[in] @c manager the detector manager
     * @param[in] @c idHelper the SCT helper class  
     * @param[in] @c status  the class providing information on the channel status (good, masked, 
     *            noisy, dead) for removal of bad channels from clustering 
     * @param[in] flag to control if bad channel removal from clusters is done
     */
    virtual SCT_ClusterCollection* clusterize(const InDetRawDataCollection<SCT_RDORawData>& RDOs,
                                              const SCT_ID& idHelper,
                                              const SCT_ChannelStatusAlg* status,
                                              const bool CTBBadChannels) const = 0;

    /** Clusterize a collection of SCT raw data objects
     * Called from SCT_clusterization algorithm in InDetPrepRawDataFormation, 
     * once per module
     * @param[in] @c RDOs the raw data objects
     * @param[in] @c manager the detector manager
     * @param[in] @c idHelper the SCT helper class  
     * @param[in] @c status  the class providing information on the channel status (good/bad) for removal of bad channels from clustering 
     */
    virtual SCT_ClusterCollection *clusterize(const InDetRawDataCollection<SCT_RDORawData>& rdoCollection,
                                              const SCT_ID& idHelper) const = 0;
  };
}

#endif // SICLUSTERIZATIONTOOL_I_SCT_CLUSTERINGALG_H

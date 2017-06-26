/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMATOOLS_EMCLUSTERTOOL_H
#define EGAMMATOOLS_EMCLUSTERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "egammaInterfaces/IEMClusterTool.h"
#include "egammaBaseTool.h"

#include "xAODCaloEvent/CaloCluster.h" // cannot use CaloClusterFwd b/c of ClusterSize
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEgamma/EgammaFwd.h"
#include "xAODEgamma/EgammaEnums.h"
#include "StoreGate/WriteHandleKey.h"


class IegammaSwTool;
class IegammaMVATool;
class StoreGateSvc;
class CaloCellDetPos;
class CaloClusterCellLink;

/**
   @class EMClusterTool
   This tool makes the EM CaloCluster used by egamma objects.
   It also defines the cluster size 
       - in barrel
         3*7 for electrons
         3*7 for converted photons with Rconv<800mm
         3*5 for converted photons with Rconv>800mm
         3*5 for unconverted photons 
       - in end-caps
         5*5 for electrons
         5*5 for converted photons with Rconv<800mm
         5*5 for converted photons with Rconv>800mm
         5*5 for unconverted photons 
    and applies the right cluster corrections depending on classification as electron, unconverted photon or converted photon
    
   @author Thomas Koffas
   @author F. Derue
   @author B. Lenzi
   @author C. Anastopoulos
*/

class EMClusterTool : public egammaBaseTool, virtual public IEMClusterTool {

 public:

  /** @brief constructor */
  EMClusterTool (const std::string& type,const std::string& name, const IInterface* parent);

  /** @brief destructor */
  virtual ~EMClusterTool();

  /** @brief initialize method */
  virtual StatusCode initialize() override;
  /** @brief execute on container */
  virtual StatusCode contExecute(xAOD::ElectronContainer *electronContainer, 
				 xAOD::PhotonContainer *photonContainer) override;
  /** @brief finalize method */
  virtual StatusCode finalize() override;
  
  void fillPositionsInCalo(xAOD::CaloCluster* cluster) const ;
 private:

  /** @brief Set new cluster to the egamma object, decorate the new cluster
    * with a link to the old one **/
  void setNewCluster(xAOD::Egamma *eg,
                     xAOD::CaloClusterContainer *outputClusterContainer,
                     xAOD::EgammaParameters::EgammaType egType);
  
  /** @brief creation of new cluster based on existing one 
    * Return a new cluster using the seed eta0, phi0 from the existing one, 
    * applying cluster corrections and MVA calibration (requires the egamma object).
    * The cluster size depends on the given EgammaType
    */
  virtual xAOD::CaloCluster* makeNewCluster(const xAOD::CaloCluster&, xAOD::Egamma *eg, 
					    xAOD::EgammaParameters::EgammaType);

  /** @brief creation of new cluster based on existing one 
    * Return a new cluster with the given size using the seed eta0, phi0 from the
    * existing cluster and applying cluster corrections. 
    * If doDecorate is true, copy the cal to the raw signal state
    * and set the raw one to the cal e,eta,phi from the existing cluster
    */
  virtual xAOD::CaloCluster* makeNewCluster(const xAOD::CaloCluster&, 
                                            const xAOD::CaloCluster::ClusterSize&);

  /** @brief creation of new super cluster based on existing one */
  xAOD::CaloCluster* makeNewSuperCluster(const xAOD::CaloCluster& cluster, xAOD::Egamma *eg);  

  /** @brief Key of the output cluster container **/
  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputClusterContainerKey;
  /** @brief Key of the output cluster container cell links: name taken from containter name **/
  SG::WriteHandleKey<CaloClusterCellLinkContainer> m_outputClusterContainerCellLinkKey;

  /** @brief Key of the output cluster container for topo-seeded clusters **/
  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputTopoSeededClusterContainerKey;
  /** @brief Key of the output cluster container cell links for topo-seeded clusters: 
    * name taken from containter name */
  SG::WriteHandleKey<CaloClusterCellLinkContainer> m_outputTopoSeededClusterContainerCellLinkKey;

  /** Handle to the MVA calibration Tool **/
  ToolHandle<IegammaMVATool>  m_MVACalibTool;  

  /** @brief Name of the input electron container **/
  std::string m_electronContainerName;

  /** @brief Name of the input photon container **/
  std::string m_photonContainerName;  
 
  /** @brief Tool to handle cluster corrections */
  ToolHandle<IegammaSwTool>   m_clusterCorrectionTool;
  
  /** @brief do super clusters **/ 
  bool m_doSuperClusters;

  /** @brief flag to protect against applying the MVA to super Clusters **/ 
  bool m_applySuperClusters;

  /** @brief Position in Calo frame**/  
  std::unique_ptr<CaloCellDetPos> m_caloCellDetPos;

  // derived variable (not set by JOs)
  bool m_doTopoSeededContainer;

};

#endif // EGAMMATOOLS_EMCLUSTERTOOL_H

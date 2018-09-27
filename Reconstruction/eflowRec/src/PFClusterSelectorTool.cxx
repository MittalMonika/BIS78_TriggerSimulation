#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "eflowRec/eflowRecCluster.h"
#include "eflowRec/PFClusterSelectorTool.h"
#include "xAODCaloEvent/CaloCluster.h"

PFClusterSelectorTool::PFClusterSelectorTool(const std::string& type,const std::string& name,const IInterface* parent):
  base_class(type, name, parent)
{
}

StatusCode PFClusterSelectorTool::initialize(){

  ATH_CHECK(m_caloClustersReadHandleKey.initialize());
  ATH_CHECK(m_caloCalClustersReadHandleKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode PFClusterSelectorTool::execute(eflowRecClusterContainer& theEFlowRecClusterContainer,xAOD::CaloClusterContainer& theCaloClusterContainer){

  SG::ReadHandle<xAOD::CaloClusterContainer> caloClustersReadHandle(m_caloClustersReadHandleKey);
  
  /* Verify the read handle has a valid pointer, and if not return */
  if (!caloClustersReadHandle.isValid()){
    ATH_MSG_WARNING(" Invalid ReadHandle for xAOD::CaloCluster with key: " <<  caloClustersReadHandle.key());
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::CaloClusterContainer> caloCalClustersReadHandle(m_caloCalClustersReadHandleKey);
  
  /* Fill the vector of eflowRecClusters */
  unsigned int nClusters = caloClustersReadHandle->size();
  for (unsigned int iCluster = 0; iCluster < nClusters; ++iCluster) {
    /* Create the eflowRecCluster and put it in the container */
    std::unique_ptr<eflowRecCluster> thisEFRecCluster  = std::make_unique<eflowRecCluster>(ElementLink<xAOD::CaloClusterContainer>(*caloClustersReadHandle, iCluster), theCaloClusterContainer);
    
    if (caloCalClustersReadHandle.isValid()){
      std::map<IdentifierHash,double> cellsWeightMap;
      retrieveLCCalCellWeight(caloClustersReadHandle->at(iCluster)->e(), iCluster, cellsWeightMap, *caloCalClustersReadHandle);

      if (msgLvl(MSG::DEBUG)) {
        //zhangr
        std::map<IdentifierHash, double>::iterator it = cellsWeightMap.begin();
        for (; it != cellsWeightMap.end(); ++it) {
	  ATH_MSG_DEBUG("zhangrui eflowPreparation " << iCluster << "/" << nClusters << ": e="
                    << caloClustersReadHandle->at(iCluster)->e() << " (" << it->first << "  "
			<< it->second << ")");
        }
      }

      thisEFRecCluster->setCellsWeight(cellsWeightMap);
    }

    thisEFRecCluster->setClusterId(iCluster);
    theEFlowRecClusterContainer.push_back(std::move(thisEFRecCluster));
    
    if (msgLvl(MSG::DEBUG)) {
      const xAOD::CaloCluster* thisCluster = caloClustersReadHandle->at(iCluster);
      ATH_MSG_DEBUG("eflowPreparation clus = " << thisCluster->eta() << " " << thisCluster->phi() << " " << thisCluster->e()/cosh(thisCluster->eta()));
    }
  }
  
  return StatusCode::SUCCESS;
}

StatusCode PFClusterSelectorTool::finalize(){
  return StatusCode::SUCCESS;
}

void PFClusterSelectorTool::retrieveLCCalCellWeight(const double& energy, const unsigned& index, std::map<IdentifierHash,double>& cellsWeight, const xAOD::CaloClusterContainer& caloCalClustersContainer) {
  
  /* match CaloCluster with CaloCalCluster to obtain cell weight */
  /* first try the position at 'index'. If we are lucky, the loop can be avoided. */
  /* Note the read handle has been tested to be valid prior to the call of this function */
  const xAOD::CaloCluster* matchedCalCluster = caloCalClustersContainer.at(index);

  if (matchedCalCluster){

    if (!(fabs(energy - matchedCalCluster->rawE()) < 0.001)) {
      matchedCalCluster = nullptr;
      for (unsigned iCalCalCluster = 0; iCalCalCluster < caloCalClustersContainer.size();
	   ++iCalCalCluster) {
	matchedCalCluster = caloCalClustersContainer.at(iCalCalCluster);
	if (fabs(energy - matchedCalCluster->rawE()) < 0.001) {
	  break;
	}
      }
      if (!matchedCalCluster) ATH_MSG_WARNING("Invalid pointer to matched cluster - failed to find cluster match");
    }
    assert(matchedCalCluster);

    /* obtain cell index and cell weight */
    const CaloDetDescrManager*   calo_dd_man  = CaloDetDescrManager::instance();
    const CaloCell_ID*               calo_id  = calo_dd_man->getCaloCell_ID();
    xAOD::CaloCluster::const_cell_iterator itCell = matchedCalCluster->cell_begin();
    xAOD::CaloCluster::const_cell_iterator endCell = matchedCalCluster->cell_end();
    for (; itCell != endCell; ++itCell) {
      const CaloCell* pCell = *itCell;
      Identifier myId = pCell->ID();
      IdentifierHash myHashId = calo_id->calo_cell_hash(myId);
      cellsWeight[myHashId] = itCell.weight();
    }
  }
  else ATH_MSG_WARNING("Invalid pointer to matched cluster - could not look up local hadron cell weights");

  return ;
}

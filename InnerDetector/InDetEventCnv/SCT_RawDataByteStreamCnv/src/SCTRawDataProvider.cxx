/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "SCTRawDataProvider.h"

#include "SCT_RawDataByteStreamCnv/ISCTRawDataProviderTool.h"
#include "IRegionSelector/IRegSelSvc.h" 
#include "InDetIdentifier/SCT_ID.h"
#include "EventContainers/IdentifiableContTemp.h"

#include <memory>

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

/// --------------------------------------------------------------------
/// Constructor

SCTRawDataProvider::SCTRawDataProvider(const std::string& name,
                                       ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_regionSelector{"RegSelSvc", name},
  m_robDataProvider{"ROBDataProviderSvc", name},
  m_sctId{nullptr},
  m_rdoContainerCacheKey{""}
{
  declareProperty("RDOCacheKey", m_rdoContainerCacheKey);
}

/// --------------------------------------------------------------------
/// Initialize

StatusCode SCTRawDataProvider::initialize() {
  /** Get ROBDataProviderSvc */
  ATH_CHECK(m_robDataProvider.retrieve());
  /** Get the SCT ID helper **/
  ATH_CHECK(detStore()->retrieve(m_sctId, "SCT_ID"));
  if (m_roiSeeded.value()) {//Don't need SCT cabling if running in RoI-seeded mode
    ATH_CHECK(m_roiCollectionKey.initialize());
    ATH_CHECK(m_regionSelector.retrieve());
    m_cabling.disable();
  } else {
    /** Retrieve Cabling service */ 
    ATH_CHECK(m_cabling.retrieve());
  }
  //Initialize 
  ATH_CHECK(m_rdoContainerKey.initialize());
  ATH_CHECK(m_lvl1CollectionKey.initialize());
  ATH_CHECK(m_bcIdCollectionKey.initialize());
  ATH_CHECK(m_bsErrContainerKey.initialize());
  ATH_CHECK(m_bsFracContainerKey.initialize());
  ATH_CHECK(m_rdoContainerCacheKey.initialize(!m_rdoContainerCacheKey.key().empty()));

  ATH_CHECK( m_rawDataTool.retrieve() );
  
  return StatusCode::SUCCESS;
}

typedef EventContainers::IdentifiableContTemp<InDetRawDataCollection<SCT_RDORawData>> dummySCTRDO_t;

/// --------------------------------------------------------------------
/// Execute
StatusCode SCTRawDataProvider::execute()
{

  m_rawDataTool->beginNewEvent();

  SG::WriteHandle<SCT_RDO_Container> rdoContainer(m_rdoContainerKey);
  bool externalCacheRDO = !m_rdoContainerCacheKey.key().empty();
  if(!externalCacheRDO){
    ATH_CHECK(rdoContainer.record (std::make_unique<SCT_RDO_Container>(m_sctId->wafer_hash_max())));
    ATH_MSG_DEBUG("Created container for " << m_sctId->wafer_hash_max());
  }else{
    SG::UpdateHandle<SCT_RDO_Cache> update(m_rdoContainerCacheKey);
    ATH_CHECK(update.isValid());
    ATH_CHECK(rdoContainer.record (std::make_unique<SCT_RDO_Container>(update.ptr())));
    ATH_MSG_DEBUG("Created container using cache for " << m_rdoContainerCacheKey.key());
  }

  
  SG::WriteHandle<InDetBSErrContainer> bsErrContainer(m_bsErrContainerKey);
  ATH_CHECK(bsErrContainer.record(std::make_unique<InDetBSErrContainer>()));

  SG::WriteHandle<SCT_ByteStreamFractionContainer> bsFracContainer(m_bsFracContainerKey);
  ATH_CHECK(bsFracContainer.record(std::make_unique<SCT_ByteStreamFractionContainer>()));

  //// do we need this??  rdoIdc->cleanup();

  /** ask ROBDataProviderSvc for the vector of ROBFragment for all SCT ROBIDs */
  std::vector<const ROBFragment*> listOfRobFrags;
  if (not m_roiSeeded.value()) {
    std::vector<uint32_t> rodList;
    m_cabling->getAllRods(rodList);
    m_robDataProvider->getROBData(rodList , listOfRobFrags);
  } else {//Only load ROBs from RoI
    std::vector<uint32_t> listOfRobs;
    SG::ReadHandle<TrigRoiDescriptorCollection> roiCollection{m_roiCollectionKey};
    ATH_CHECK(roiCollection.isValid());
    TrigRoiDescriptor superRoI;//add all RoIs to a super-RoI
    superRoI.setComposite(true);
    superRoI.manageConstituents(false);
    for (const TrigRoiDescriptor* roi: *roiCollection) {
      superRoI.push_back(roi);
    }
    m_regionSelector->DetROBIDListUint(SCT, superRoI, listOfRobs);
    m_robDataProvider->getROBData(listOfRobs, listOfRobFrags);
  }


  ATH_MSG_DEBUG("Number of ROB fragments " << listOfRobFrags.size());

  SG::WriteHandle<InDetTimeCollection> lvl1Collection{m_lvl1CollectionKey};
  lvl1Collection = std::make_unique<InDetTimeCollection>(listOfRobFrags.size()); 
  ATH_CHECK(lvl1Collection.isValid());

  SG::WriteHandle<InDetTimeCollection> bcIdCollection{m_bcIdCollectionKey};
  bcIdCollection = std::make_unique<InDetTimeCollection>(listOfRobFrags.size()); 
  ATH_CHECK(bcIdCollection.isValid());

  std::vector<const ROBFragment*>::const_iterator rob_it{listOfRobFrags.begin()};
  for (; rob_it!=listOfRobFrags.end(); ++rob_it) {
    
    uint32_t robId{(*rob_it)->rod_source_id()};
    /**
     * Store LVL1ID and BCID information in InDetTimeCollection 
     * to be stored in StoreGate at the end of the loop.
     * We want to store a pair<ROBID, LVL1ID> for each ROD, once per event.
     **/
    
    unsigned int lvl1Id{(*rob_it)->rod_lvl1_id()};
    auto lvl1Pair{std::make_unique<std::pair<uint32_t, unsigned int>>(robId, lvl1Id)};
    lvl1Collection->push_back(std::move(lvl1Pair));
    
    unsigned int bcId{(*rob_it)->rod_bc_id()};
    auto bcIdPair{std::make_unique<std::pair<uint32_t, unsigned int>>(robId, bcId)};
    bcIdCollection->push_back(std::move(bcIdPair));
    
    ATH_MSG_DEBUG("Stored LVL1ID " << lvl1Id << " and BCID " << bcId << " in InDetTimeCollections");
    
  }
  std::unique_ptr<dummySCTRDO_t> dummyrdo;
  if (externalCacheRDO) dummyrdo = std::make_unique<dummySCTRDO_t>(rdoContainer.ptr());
  ISCT_RDO_Container *rdoInterface = externalCacheRDO ? static_cast< ISCT_RDO_Container*> (dummyrdo.get()) 
                     : static_cast<ISCT_RDO_Container* >(rdoContainer.ptr());
  /** ask SCTRawDataProviderTool to decode it and to fill the IDC */
  if (m_rawDataTool->convert(listOfRobFrags, *rdoInterface, bsErrContainer.ptr(), bsFracContainer.ptr()).isFailure()) {
    ATH_MSG_WARNING("BS conversion into RDOs failed");
  }
  if (dummyrdo) dummyrdo->MergeToRealContainer(rdoContainer.ptr());
  
  return StatusCode::SUCCESS;
}

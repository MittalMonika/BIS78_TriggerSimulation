/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigL2MuonSA/MdtDataPreparator.h"

#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"

#include "MuonRDO/MdtCsmContainer.h"

#include "CLHEP/Units/PhysicalConstants.h"

#include "Identifier/IdentifierHash.h"
#include "MuonContainerManager/MuonRdoContainerAccess.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonPrepRawData/MdtPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"

#include "TrigL2MuonSA/MdtRegionDefiner.h"

#include "xAODTrigMuon/TrigMuonDefs.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrigSteeringEvent/PhiHelper.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"

#include "MuonCablingData/MdtAmtMap.h"
#include "MuonCablingData/MdtRODMap.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonCablingData/MdtSubdetectorMap.h"
#include "MuonCablingData/MdtCsmMap.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "MuonIdHelpers/MdtIdHelper.h"

#include "GeoPrimitives/CLHEPtoEigenConverter.h"

using namespace Muon;
using namespace MuonGM;

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

static const InterfaceID IID_MdtDataPreparator("IID_MdtDataPreparator", 1, 0);

const InterfaceID& TrigL2MuonSA::MdtDataPreparator::interfaceID() { return IID_MdtDataPreparator; }

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

TrigL2MuonSA::MdtDataPreparator::MdtDataPreparator(const std::string& type, 
						   const std::string& name,
						   const IInterface*  parent): 
   AthAlgTool(type,name,parent),
   m_msg(0),
   m_storeGateSvc( "StoreGateSvc", name ),
   m_mdtRawDataProvider("Muon::MDT_RawDataProviderTool"),
   m_regionSelector(0), m_robDataProvider(0), m_recMuonRoIUtils(),
   m_mdtRegionDefiner(0),
   m_mdtPrepDataProvider("Muon::MdtRdoToPrepDataTool/MdtPrepDataProviderTool"),
   m_use_mdtcsm(true)
{
   declareInterface<TrigL2MuonSA::MdtDataPreparator>(this);

   declareProperty("MDT_RawDataProvider", m_mdtRawDataProvider);
   declareProperty("MdtPrepDataProvider", m_mdtPrepDataProvider);
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

TrigL2MuonSA::MdtDataPreparator::~MdtDataPreparator() 
{
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::initialize()
{
   // Get a message stream instance
   m_msg = new MsgStream( msgSvc(), name() );
   msg() << MSG::DEBUG << "Initializing MdtDataPreparator - package version " << PACKAGE_VERSION << endreq ;
   
   StatusCode sc;
   sc = AthAlgTool::initialize();
   if (!sc.isSuccess()) {
      msg() << MSG::ERROR << "Could not initialize the AthAlgTool base class." << endreq;
      return sc;
   }
   
   // Locate the StoreGateSvc
   sc =  m_storeGateSvc.retrieve();
   if (!sc.isSuccess()) {
      msg() << MSG::ERROR << "Could not find StoreGateSvc" << endreq;
      return sc;
   }

   // Locate MDT RawDataProvider
   sc = m_mdtRawDataProvider.retrieve();
   if ( sc.isFailure() ) {
      msg() << MSG::ERROR << "Could not retrieve " << m_mdtRawDataProvider << endreq;
      return sc;
   }
   msg() << MSG::DEBUG << "Retrieved tool " << m_mdtRawDataProvider << endreq;

   //
   std::string serviceName;

   // Locate RegionSelector
   serviceName = "RegionSelector";
   sc = service("RegSelSvc", m_regionSelector);
   if(sc.isFailure()) {
      msg() << MSG::ERROR << "Could not retrieve " << serviceName << endreq;
      return sc;
   }
   msg() << MSG::DEBUG << "Retrieved service " << serviceName << endreq;

   // Locate ROBDataProvider
   serviceName = "ROBDataProvider";
   IService* svc = 0;
   sc = service("ROBDataProviderSvc", svc);
   if(sc.isFailure()) {
      msg() << MSG::ERROR << "Could not retrieve " << serviceName << endreq;
      return sc;
   }
   m_robDataProvider = dynamic_cast<ROBDataProviderSvc*> (svc);
   if( m_robDataProvider == 0 ) {
      msg() << MSG::ERROR << "Could not cast to ROBDataProviderSvc " << endreq;
      return StatusCode::FAILURE;
   }
   msg() << MSG::DEBUG << "Retrieved service " << serviceName << endreq;

   //
   m_mdtRegionDefiner = new TrigL2MuonSA::MdtRegionDefiner(m_msg);

   // initialize the NEW cabling service
   sc = service("MuonMDT_CablingSvc",m_mdtCabling);
   if (sc != StatusCode::SUCCESS) {
     msg() << MSG::ERROR << "Could not find the MuonMDT_CablingSvc" << endreq;
     return sc;
   }
   msg() << MSG::DEBUG << "Retrieved the new cabling service " << endreq;
   
   // retrieve the mdtidhelper
   StoreGateSvc* detStore(0);
   sc = serviceLocator()->service("DetectorStore", detStore);
   if (sc.isFailure()) {
     msg() << MSG::ERROR << "Could not retrieve DetectorStore." << endreq;
     return sc;
   }
   msg() << MSG::DEBUG << "Retrieved DetectorStore." << endreq;
   
   sc = detStore->retrieve( m_muonMgr,"Muon" );
   if (sc.isFailure()) return sc;
   msg() << MSG::DEBUG << "Retrieved GeoModel from DetectorStore." << endreq;
   m_mdtIdHelper = m_muonMgr->mdtIdHelper();
   
   sc = m_mdtPrepDataProvider.retrieve();
   if (sc.isSuccess()) {
     msg() << MSG::DEBUG << "Retrieved " << m_mdtPrepDataProvider << endreq;
   } else {
     msg() << MSG::FATAL << "Could not get " << m_mdtPrepDataProvider << endreq;
     return sc;
   }

   // Retrieve ActiveStore
   sc = serviceLocator()->service("ActiveStoreSvc", m_activeStore);
   if (sc.isFailure() || m_activeStore == 0) {
     msg() << MSG::ERROR << " Cannot get ActiveStoreSvc." << endreq;
     return sc ;
   }
   msg() << MSG::DEBUG << "Retrieved ActiveStoreSvc." << endreq; 
   
   // 
   return StatusCode::SUCCESS; 
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

void TrigL2MuonSA::MdtDataPreparator::setRpcGeometry(bool use_rpc)
{
  m_mdtRegionDefiner->setRpcGeometry(use_rpc);
  return;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

void TrigL2MuonSA::MdtDataPreparator::setMdtDataCollection(bool use_mdtcsm)
{
  m_use_mdtcsm = use_mdtcsm;
  return;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

void TrigL2MuonSA::MdtDataPreparator::setRoIBasedDataAccess(bool use_RoIBasedDataAccess)
{
  m_use_RoIBasedDataAccess = use_RoIBasedDataAccess;
  return;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::prepareData(const LVL1::RecMuonRoI*  p_roi,
							const TrigRoiDescriptor*    p_roids,
							const TrigL2MuonSA::RpcFitResult& rpcFitResult,
							TrigL2MuonSA::MuonRoad&  muonRoad,
							TrigL2MuonSA::MdtRegion& mdtRegion,
							TrigL2MuonSA::MdtHits&   mdtHits_normal,
							TrigL2MuonSA::MdtHits&   mdtHits_overlap)
{
  StatusCode sc;

  m_mdtRegionDefiner->setMdtGeometry(m_mdtIdHelper, m_muonMgr);

  // define regions
  sc = m_mdtRegionDefiner->getMdtRegions(p_roi, rpcFitResult, muonRoad, mdtRegion);
  if( sc!= StatusCode::SUCCESS ) {
    msg() << MSG::WARNING << "Error in getting MDT region" << endreq;
    return sc;
  }

  sc = getMdtHits(p_roi, p_roids, mdtRegion, muonRoad, mdtHits_normal, mdtHits_overlap);
  if( sc!= StatusCode::SUCCESS ) {
    msg() << MSG::WARNING << "Error in getting MDT hits" << endreq;
    return sc;
  }

  return StatusCode::SUCCESS;

}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::prepareData(const LVL1::RecMuonRoI*           p_roi,
							const TrigRoiDescriptor*          p_roids,
							const TrigL2MuonSA::TgcFitResult& tgcFitResult,
							TrigL2MuonSA::MuonRoad&           muonRoad,
							TrigL2MuonSA::MdtRegion&          mdtRegion,
							TrigL2MuonSA::MdtHits&            mdtHits_normal,
							TrigL2MuonSA::MdtHits&            mdtHits_overlap)
{
  StatusCode sc;

  m_mdtRegionDefiner->setMdtGeometry(m_mdtIdHelper, m_muonMgr);
  
  // define regions
  sc = m_mdtRegionDefiner->getMdtRegions(p_roi, tgcFitResult, muonRoad, mdtRegion);
  if( sc!= StatusCode::SUCCESS ) {
    msg() << MSG::WARNING << "Error in getting MDT region" << endreq;
    return sc;
  }

  sc = getMdtHits(p_roi, p_roids, mdtRegion, muonRoad, mdtHits_normal, mdtHits_overlap);
  if( sc!= StatusCode::SUCCESS ) {
    msg() << MSG::WARNING << "Error in getting MDT hits" << endreq;
    return sc;
  }

  return StatusCode::SUCCESS;
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::getMdtHits(const LVL1::RecMuonRoI*  p_roi,
						       const TrigRoiDescriptor* p_roids, 
						       const TrigL2MuonSA::MdtRegion& mdtRegion,
						       TrigL2MuonSA::MuonRoad& muonRoad,
						       TrigL2MuonSA::MdtHits& mdtHits_normal,
						       TrigL2MuonSA::MdtHits& mdtHits_overlap)
{
  if (m_use_mdtcsm) {

    // preload ROBs
    std::vector<uint32_t> v_robIds;
    double etaMin = p_roi->eta() - 0.2;
    double etaMax = p_roi->eta() + 0.2;
    double phi = p_roi->phi();
    double phiMin = p_roi->phi() - 0.1;
    double phiMax = p_roi->phi() + 0.1;
    if( phi < 0 ) phi += 2*CLHEP::pi;
    if( phiMin < 0 ) phiMin += 2*CLHEP::pi;
    if( phiMax < 0 ) phiMax += 2*CLHEP::pi;
    
    TrigRoiDescriptor* roi = new TrigRoiDescriptor( p_roi->eta(), etaMin, etaMax, phi, phiMin, phiMax ); 
    
    const IRoiDescriptor* iroi = (IRoiDescriptor*) roi;
    
    std::vector<IdentifierHash> mdtHashList;
    mdtHashList.clear();
    m_regionSelector->DetHashIDList(MDT, *iroi, mdtHashList);
    msg() << MSG::DEBUG << "size of the hashids in getMdtHits " << mdtHashList.size() << endreq;
    
    if (roi) delete roi;
    
    bool redundant;
    for(int hash_iter=0; hash_iter<(int)mdtHashList.size(); hash_iter++){
      redundant = false;
      
      uint32_t newROBId = m_mdtCabling->getROBId(mdtHashList[hash_iter]);
      
      for (int rob_iter=0; rob_iter<(int)v_robIds.size(); rob_iter++){
	if(newROBId == v_robIds[rob_iter])
	  redundant = true;
      }
      if(!redundant)
	v_robIds.push_back(newROBId);
    }
    
    msg() << MSG::DEBUG << "size of the rob Ids " << v_robIds.size() << endreq;
    
    mdtHits_normal.clear();
    mdtHits_overlap.clear();
    
    // get MdtCsmContainer
    const MdtCsmContainer* pMdtCsmContainer =
      Muon::MuonRdoContainerAccess::retrieveMdtCsm("MDTCSM");
    if( pMdtCsmContainer==0 ) {
      if ( msgLvl() <= MSG::DEBUG ) {
	msg() << MSG::DEBUG << "MDT CSM container not registered by "
	      << "MuonRdoContainerManager; retrieving it "
	      << "from the store! " << endreq;
      }
      StatusCode sc = m_storeGateSvc->retrieve(pMdtCsmContainer, "MDTCSM");
      if (sc.isFailure()) {
	msg() << MSG::WARNING << "Retrieval of MdtCsmContainer failed" << endreq;
	return sc;
      }
    }
    
    // get IdHashes
    std::vector<IdentifierHash> v_idHash_normal;
    std::vector<IdentifierHash> v_idHash_overlap;
    
    // get hashIdlist by using region selector
    if (muonRoad.isEndcap) getMdtIdHashesEndcap(mdtRegion, v_idHash_normal,v_idHash_overlap);
    else getMdtIdHashesBarrel(mdtRegion, v_idHash_normal,v_idHash_overlap);
    
    msg() << MSG::DEBUG << ">>> hash id size normal: " << v_idHash_normal.size() << " overlap: " << v_idHash_overlap.size() << endreq;
    
    msg() << MSG::DEBUG << "List of MDT Chambers (normal)..." << v_idHash_normal.size() << endreq;
    for (unsigned int i=0;i<v_idHash_normal.size();i++) {
      msg() << MSG::DEBUG << "MDT chamber n.  " << i << ": hash id " << v_idHash_normal[i] << endreq;
    }
    msg() << MSG::DEBUG << "List of MDT Chambers (overlap)..." << v_idHash_overlap.size() << endreq;
    for (unsigned int i=0;i<v_idHash_overlap.size();i++) {
      msg() << MSG::DEBUG << "MDT chamber n.  " << i << ": hash id " << v_idHash_overlap[i] << endreq;
    }
    
    // get MdtCsm
    std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> v_robFragments;
    m_robDataProvider->getROBData(v_robIds,v_robFragments);
    std::vector<const MdtCsm*> v_mdtCsms_normal;
    std::vector<const MdtCsm*> v_mdtCsms_overlap;
    msg() << MSG::DEBUG << "getting MdtCsm (normal)..." << endreq;
    StatusCode sc = getMdtCsm(pMdtCsmContainer, v_robFragments, v_idHash_normal, v_mdtCsms_normal);
    if( sc!= StatusCode::SUCCESS ) {
      msg() << MSG::WARNING << "Error in getting MdtCsm (normal)" << endreq;
      return sc;
    }
    msg() << MSG::DEBUG << "getting MdtCsm (overlap)..." << endreq;
    sc = getMdtCsm(pMdtCsmContainer, v_robFragments, v_idHash_overlap,v_mdtCsms_overlap);
    if( sc!= StatusCode::SUCCESS ) {
      msg() << MSG::WARNING << "Error in getting MdtCsm (overlap)" << endreq;
      return sc;
    }
    
    msg() << MSG::DEBUG << "Found csm normal : " << v_mdtCsms_normal.size() << endreq;
    msg() << MSG::DEBUG << "Found csm overlap: " << v_mdtCsms_overlap.size() << endreq;
    
    // decode 
    msg() << MSG::DEBUG << "decoding MdtCsm (normal)..." << endreq;
    bool Endcap_Mid=false;
    for(unsigned int i=0; i<v_mdtCsms_normal.size(); i++) {
      if (!decodeMdtCsm(v_mdtCsms_normal[i], mdtHits_normal, muonRoad)) return StatusCode::FAILURE;
      if(muonRoad.isEndcap){
        int midN=0;
        for(unsigned int ti=0; ti<mdtHits_normal.size(); ti++){
          if(mdtHits_normal[ti].Chamber==4)midN++;
        }
        if(midN==0)Endcap_Mid=true;
      }
    }
    msg() << MSG::DEBUG << "decoding MdtCsm (overlap)..." << endreq;
    for(unsigned int i=0; i<v_mdtCsms_overlap.size(); i++) {
      if (!decodeMdtCsm(v_mdtCsms_overlap[i],mdtHits_overlap, muonRoad)) return StatusCode::FAILURE;
    }
    
    if(Endcap_Mid || v_mdtCsms_normal.size() == 0){
      for(unsigned int i=0; i<v_mdtCsms_overlap.size(); i++) {
        if (!decodeMdtCsm(v_mdtCsms_overlap[i],mdtHits_normal, muonRoad)) return StatusCode::FAILURE;
      }
    }

  } else { // use MdtPrepData

   // preload ROBs
    std::vector<uint32_t> v_robIds;
    std::vector<IdentifierHash> mdtHashList;

    if (m_use_RoIBasedDataAccess) {
      
      msg() << MSG::DEBUG << "Use RoI based data access" << endreq;

      const IRoiDescriptor* iroi = (IRoiDescriptor*) p_roids;
      
      m_regionSelector->DetHashIDList(MDT, *iroi, mdtHashList);
      msg() << MSG::DEBUG << "mdtHashList.size()=" << mdtHashList.size() << endreq;
      
      m_regionSelector->DetROBIDListUint(MDT, *iroi, v_robIds);
      
    } else {
      
      msg() << MSG::DEBUG << "Use full data access" << endreq;
      
      m_regionSelector->DetHashIDList(MDT, mdtHashList);
      msg() << MSG::DEBUG << "mdtHashList.size()=" << mdtHashList.size() << endreq;
     
      m_regionSelector->DetROBIDListUint(MDT, v_robIds);
    }

    StatusCode sc = collectMdtHitsFromPrepData(mdtHashList, v_robIds, mdtHits_normal, muonRoad);
    if( sc!= StatusCode::SUCCESS ) {
      msg() << MSG::WARNING << "Error in getting collection of MDT hit from prep data" << endreq;
      return sc;
    }

  }
  //
  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::getMdtCsm(const MdtCsmContainer* pMdtCsmContainer,
						      const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& v_robFragments,
						      const std::vector<IdentifierHash>& v_idHash,
						      std::vector<const MdtCsm*>& v_mdtCsms)
{
  if( m_mdtRawDataProvider->convert(v_robFragments, v_idHash).isFailure() ) {
    msg() << MSG::WARNING << "Failed to convert MDT CSM hash Ids: " << endreq;
    for(unsigned int i=0; i < v_idHash.size(); i++) {
      msg() << MSG::WARNING << v_idHash[i] << endreq;
    }
    return StatusCode::FAILURE;
  }
 
  std::vector<uint32_t> v_robIds;
  bool redundant;
  // Modificaiton provided by Jochen Meyer
  unsigned int i=0;
  int processingDetEl = 1;
  bool BMEpresent = m_mdtIdHelper->stationNameIndex("BME") != -1;
  while( i < v_idHash.size() ) {

    redundant = false;
    uint32_t newROBId = v_idHash[i];
    for (int rob_iter=0; rob_iter<(int)v_robIds.size(); rob_iter++){
      if(newROBId == v_robIds[rob_iter]) redundant=true;
    }
    if(!redundant){
      v_robIds.push_back(newROBId);
    }else{
      ++i;
      continue;
    }

    IdentifierHash v_idHash_corr = v_idHash[i];
    if( BMEpresent ) {
      // if there are BMEs the RDOs are registered with the detectorElement hash
      Identifier tmp_id;
      IdContext tmp_context = m_mdtIdHelper->module_context();
       m_mdtIdHelper->get_id(v_idHash[i], tmp_id, &tmp_context);
       // for BMEs the 2 CSMs are registered with the hashes of the 2 multilayers
       Identifier ml_id = m_mdtIdHelper->multilayerID(tmp_id, processingDetEl);
       m_mdtIdHelper->get_detectorElement_hash(ml_id, v_idHash_corr);
    }
    MdtCsmContainer::const_iterator pCsmIt = pMdtCsmContainer->indexFind(v_idHash_corr);
    
    if( pCsmIt==pMdtCsmContainer->end() ) {
      if(processingDetEl == 1) ++i;
      continue;
    }
    if( BMEpresent ){
      Identifier elementId = ((*pCsmIt)->identify());
      // if there are BMEs it's also required to process there 2nd CSM
      if( m_mdtIdHelper->stationName(elementId) == 53 ) { // is BME chamber
        // do the loop once again with the SAME iterator, but for the 2nd multilayer
	if( processingDetEl == 2 ) {
          // reset to CSM/multilayer 1 and go to next chamber
	  processingDetEl = 1;
        } else {
          // the 1st CSM was done, do the 2nd in the same chamber
	  processingDetEl = 2;
	}
      }
    }

    v_mdtCsms.push_back(*pCsmIt);
    msg() << MSG::DEBUG << "MDT Collection hash " << v_idHash_corr
       << " associated to:  SubDet 0x" << MSG::hex
       << (*pCsmIt)->SubDetId() << " MRod 0x"
       << (*pCsmIt)->MrodId() << " Link 0x"
       << (*pCsmIt)->CsmId() << MSG::dec << endreq;
    msg() << MSG::DEBUG << "Number of digit in  MDT Collection "
	  << v_idHash_corr << ": " << (*pCsmIt)->size() << endreq;
    
    if(processingDetEl == 1) ++i;
  }

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

bool TrigL2MuonSA::MdtDataPreparator::decodeMdtCsm(const MdtCsm* csm,
						   TrigL2MuonSA::MdtHits& mdtHits,
						   const TrigL2MuonSA::MuonRoad& muonRoad)
{    

  //  std::cout << "In decode csm !" << std::endl;
   if( csm->empty() ) return true;

   unsigned short int SubsystemId = csm->SubDetId();
   unsigned short int MrodId      = csm->MrodId();
   unsigned short int LinkId      = csm->CsmId();

   //     std::cout << "NOW using the new cabling " << std::endl;
   MuonMDT_CablingMap* cablingMap = m_mdtCabling->getCablingMap();
   if (!cablingMap) {
     msg() << MSG::DEBUG << "Null pointer to MuonMDT_CablingMap" << endreq;
     return true;
   }

   MdtSubdetectorMap* subdetectorMap = cablingMap->getSubdetectorMap(SubsystemId);
   if (!subdetectorMap) {
     msg() << MSG::DEBUG << "Null pointer to MdtSubdetectorMap" << endreq;
     return true;
   }

   MdtRODMap* rodMap = subdetectorMap->getRODMap(MrodId);
   if (!rodMap) {
     msg() << MSG::DEBUG << "Null pointer to MdtRODMap" << endreq;
     return true;
   }

   MdtCsmMap* csmMap = rodMap->getCsmMap(LinkId);
   if (!csmMap) {
     msg() << MSG::DEBUG << "Null pointer to MdtCsmMap" << endreq;
     return true;
   }
   
   MdtCsm::const_iterator amt = csm->begin();
   while(amt != csm->end()) {
     
     if( ! (*amt)->leading() ) {
       ++amt;
       continue;
     }       
     
     unsigned short int TdcId     = (*amt)->tdcId();
     unsigned short int ChannelId = (*amt)->channelId();
     unsigned short int drift     = (*amt)->fine() | ( (*amt)->coarse() << 5);  
     
     int StationPhi;
     int StationName;
     int StationEta;
     int MultiLayer;
     int Layer;
     int Tube;
     double cXmid;
     double cYmid;
     double cAmid = 0;
     double cPhip;   
     MdtAmtMap* amtMap = csmMap->getTdcMap(TdcId);
     
     if (!amtMap) {
       msg() << MSG::WARNING << "problem getting MdtAmtMap (null pointer returned) for TdcId=" << TdcId <<  endreq;
       ++amt;
       continue;
     }
     bool offlineID = amtMap->offlineId(ChannelId, StationName, StationEta, StationPhi, MultiLayer, Layer, Tube);	   
     
     if(!offlineID) {
       msg() << MSG::WARNING << "problem getting info from amtMap" << endreq;
       ++amt;
       continue;
     }
     
     m_mdtReadout = m_muonMgr->getMdtRElement_fromIdFields(StationName, StationEta, StationPhi,MultiLayer);
     if (!m_mdtReadout) {
       ++amt;
       continue;
     }
     
     m_muonStation = m_mdtReadout->parentMuonStation();
     
     int TubeLayers = m_mdtReadout->getNLayers();
     int TubeLayer = Layer;
     if(TubeLayer > TubeLayers) TubeLayer -= TubeLayers;
     if(MultiLayer==2)
       Layer = Layer + m_mdtReadout->getNLayers();
     
     double OrtoRadialPos = m_mdtReadout->getStationS();
     std::string chamberType = m_mdtReadout->getStationType();
     char st = chamberType[1];
     
     int chamber = 0;
     if (chamberType[0]=='E') {
       /// Endcap
       if (st=='I') chamber = xAOD::L2MuonParameters::Chamber::EndcapInner;
       if (st=='M') chamber = xAOD::L2MuonParameters::Chamber::EndcapMiddle;
       if (st=='O') chamber = xAOD::L2MuonParameters::Chamber::EndcapOuter;
       if (st=='E') chamber = xAOD::L2MuonParameters::Chamber::EndcapExtra;
     } else {
       /// Barrel
       if (st=='I') chamber = xAOD::L2MuonParameters::Chamber::BarrelInner;
       if (st=='M') chamber = xAOD::L2MuonParameters::Chamber::BarrelMiddle;
       if (st=='O') chamber = xAOD::L2MuonParameters::Chamber::BarrelOuter;
       if (st=='E' && chamberType[2]=='E') chamber = xAOD::L2MuonParameters::Chamber::BEE;
       if (st=='M' && chamberType[2]=='E') chamber = xAOD::L2MuonParameters::Chamber::BME;
     }
     
     double R = m_mdtReadout->center(TubeLayer, Tube).perp();
     double Z = m_mdtReadout->center(TubeLayer, Tube).z();
     
     Amg::Transform3D trans = Amg::CLHEPTransformToEigen(*m_muonStation->getNominalAmdbLRSToGlobal());
     if(m_muonStation->endcap()==0){
       cXmid = (trans*Amg::Vector3D(0.,0.,0.)).z();
       double halfRadialThicknessOfMultilayer = m_muonStation->RsizeMdtStation()/2.;
       cYmid = ((trans*Amg::Vector3D(0.,0.,0.)).perp()+halfRadialThicknessOfMultilayer);
     }
     else{
       cXmid = (trans*Amg::Vector3D(0.,0.,0.)).perp();
       double halfZThicknessOfMultilayer = m_muonStation->ZsizeMdtStation()/2.;
       cYmid = (trans*Amg::Vector3D(0.,0.,0.)).z();
       if(cYmid>0) cYmid += halfZThicknessOfMultilayer;
       else cYmid -= halfZThicknessOfMultilayer;
     }
     cPhip = (trans*Amg::Vector3D(0.,0.,0.)).phi();
     
     double dphi  = 0;
     double cphi  = muonRoad.phi[chamber][0];
     if( cPhip*cphi>0 ) dphi = fabsf(cPhip - cphi);
     else {
       if(fabs(cphi) > CLHEP::pi/2.) {
	 double phi1 = (cPhip>0.)? cPhip-CLHEP::pi : cPhip+CLHEP::pi;
	 double phi2 = (cphi >0.)? cphi -CLHEP::pi : cphi +CLHEP::pi;
	 dphi = fabsf(phi1) + fabsf(phi2); 
       }
       else {
	 dphi = fabsf(cPhip) + fabsf(cphi);
       }
     }
     
     if(m_muonStation->endcap()==1)
       R = sqrt(R*R+R*R*tan(dphi)*tan(dphi));
     
     Amg::Vector3D OrigOfMdtInAmdbFrame = 
       Amg::Hep3VectorToEigen( m_muonStation->getBlineFixedPointInAmdbLRS() );	    
     double Rmin =(trans*OrigOfMdtInAmdbFrame).perp();	

     float cInCo = 1./cos(fabsf(atan(OrtoRadialPos/Rmin)));
     float cPhi0 = cPhip - atan(OrtoRadialPos/Rmin);
     if(cPhi0 > CLHEP::pi) cPhip -= 2*CLHEP::pi;
     if(cPhip<0. && (fabs(CLHEP::pi+cPhip) < 0.05) ) cPhip = acos(0.)*2.;
     
     uint32_t OnlineId = ChannelId | (TdcId << 5) |
       (LinkId << 10) | (get_system_id(SubsystemId) << 13) | (MrodId <<16);
     uint16_t adc        = (*amt)->width();
     uint16_t coarseTime = (*amt)->coarse();
     uint16_t fineTime   = (*amt)->fine();
     
     msg() << MSG::DEBUG << " ...MDT hit Z/R/chamber/MultiLater/TubeLayer/Tube/Layer/adc/tdc = "
	   << Z << "/" << R << "/" << chamber << "/" << MultiLayer << "/" << TubeLayer << "/" 
	   << Tube << "/" << Layer << "/" << adc << "/" << drift << endreq;
     
     
     // no residual check for the moment
     // (residual check at pattern finder)
     if(Layer!=0 && Tube !=0)
       {
	 
	 // create the new digit
	 TrigL2MuonSA::MdtHitData tmp;
	 tmp.name       = StationName;
	 tmp.StationEta = StationEta;
	 tmp.StationPhi = StationPhi;
	 tmp.Multilayer = MultiLayer;
	 tmp.Layer      = Layer - 1;
	 tmp.TubeLayer  = TubeLayer;
	 tmp.Tube       = Tube;
	 tmp.cYmid      = cYmid;
	 tmp.cXmid      = cXmid;
	 tmp.cAmid      = cAmid;
	 tmp.cPhip      = cPhip;
	 tmp.cInCo      = cInCo;
	 tmp.cPhi0      = cPhi0;
	 for(unsigned int i=0; i<4; i++) { tmp.cType[i] = chamberType[i]; }
	 tmp.Z          = Z;
	 tmp.R          = R;
	 tmp.DriftTime  = drift;
	 tmp.DriftSpace = 0.;
	 tmp.DriftSigma = 0;
	 tmp.Adc        = adc;
	 tmp.OnlineId   = OnlineId;
	 tmp.LeadingCoarseTime  = coarseTime;
	 tmp.LeadingFineTime    = fineTime;
	 tmp.TrailingCoarseTime = 0;
	 tmp.TrailingFineTime   = 0;
	 tmp.Residual  = 0;
	 tmp.isOutlier = 0;
	 tmp.Chamber = chamber;
	 
	 mdtHits.push_back(tmp);
       }
     
     //
     ++amt;
   }
   
   //
   return true;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

uint32_t TrigL2MuonSA::MdtDataPreparator::get_system_id (unsigned short int SubsystemId) const
{
   if(SubsystemId==0x61) return 0x0;
   if(SubsystemId==0x62) return 0x1;
   if(SubsystemId==0x63) return 0x2;
   if(SubsystemId==0x64) return 0x3;
    
   return 0x4;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

void TrigL2MuonSA::MdtDataPreparator::getMdtIdHashesBarrel(const TrigL2MuonSA::MdtRegion& mdtRegion,
							   std::vector<IdentifierHash>& mdtIdHashes_normal,
							   std::vector<IdentifierHash>& mdtIdHashes_overlap)
{
   std::vector<IdentifierHash> idList;

   // get hashIdlist by using region selector
   for(int i_station=0; i_station<4; i_station++) {
     int chamber=0;
     if (i_station==0) chamber = xAOD::L2MuonParameters::Chamber::BarrelInner; 
     if (i_station==1) chamber = xAOD::L2MuonParameters::Chamber::BarrelMiddle;
     if (i_station==2) chamber = xAOD::L2MuonParameters::Chamber::BarrelOuter;
     if (i_station==3) chamber = xAOD::L2MuonParameters::Chamber::BME;
     for(int i_sector=0; i_sector<2; i_sector++) {
       for(int i_type=0; i_type<2; i_type++) {
	 idList.clear();
	 msg() << MSG::DEBUG << "chamber/sector=" << chamber << "/" << i_sector << endreq;
	 msg() << MSG::DEBUG << "...etaMin/etaMax/phiMin/phiMax="
	       << mdtRegion.etaMin[chamber][i_sector] << "/"
	       << mdtRegion.etaMax[chamber][i_sector] << "/"
	       << mdtRegion.phiMin[chamber][i_sector] << "/"
	       << mdtRegion.phiMax[chamber][i_sector] << endreq;

	 TrigRoiDescriptor _roi( 0.5*(mdtRegion.etaMin[chamber][i_sector]+mdtRegion.etaMax[chamber][i_sector]),
				 mdtRegion.etaMin[chamber][i_sector],
				 mdtRegion.etaMax[chamber][i_sector],
				 HLT::phiMean(mdtRegion.phiMin[chamber][i_sector],mdtRegion.phiMax[chamber][i_sector]),
				 mdtRegion.phiMin[chamber][i_sector],
				 mdtRegion.phiMax[chamber][i_sector]);
	 
	 m_regionSelector->DetHashIDList(MDT,static_cast<TYPEID>(mdtRegion.chamberType[chamber][i_sector][i_type]),
					 _roi, idList);
	 msg() << MSG::DEBUG << "...chamberType=" << mdtRegion.chamberType[chamber][i_sector][i_type] << endreq;
	 msg() << MSG::DEBUG << "...size IDlist=" << idList.size() << endreq;
	 std::vector<IdentifierHash>::const_iterator it = idList.begin();
	 while(it != idList.end()) {
	   if (i_sector==0) {
	     mdtIdHashes_normal.push_back(*it++);
	   } else {
	     mdtIdHashes_overlap.push_back(*it++);
	   }
	 }
       }
     }
   }
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

void TrigL2MuonSA::MdtDataPreparator::getMdtIdHashesEndcap(const TrigL2MuonSA::MdtRegion& mdtRegion,
							   std::vector<IdentifierHash>& mdtIdHashes_normal,
							   std::vector<IdentifierHash>& mdtIdHashes_overlap)
{
   std::vector<IdentifierHash> idList;

   // get hashIdlist by using region selector
   for(int i_station=0; i_station<6; i_station++) {
     int chamber = 0;
     if (i_station==0) chamber = xAOD::L2MuonParameters::Chamber::EndcapInner; 
     if (i_station==1) chamber = xAOD::L2MuonParameters::Chamber::EndcapMiddle;
     if (i_station==2) chamber = xAOD::L2MuonParameters::Chamber::EndcapOuter;
     if (i_station==3) chamber = xAOD::L2MuonParameters::Chamber::EndcapExtra;
     if (i_station==4) chamber = xAOD::L2MuonParameters::Chamber::BarrelInner;
     if (i_station==5) chamber = xAOD::L2MuonParameters::Chamber::BEE;
     for(int i_sector=0; i_sector<2; i_sector++) {
       for(int i_type=0; i_type<2; i_type++) {
	 idList.clear();
	 msg() << MSG::DEBUG << "chamber/sector=" << chamber << "/" << i_sector << endreq;
	 msg() << MSG::DEBUG << "...etaMin/etaMax/phiMin/phiMax="
	       << mdtRegion.etaMin[chamber][i_sector] << "/"
	       << mdtRegion.etaMax[chamber][i_sector] << "/"
	       << mdtRegion.phiMin[chamber][i_sector] << "/"
	       << mdtRegion.phiMax[chamber][i_sector] << endreq;

	 TrigRoiDescriptor _roi( 0.5*(mdtRegion.etaMin[chamber][i_sector]+mdtRegion.etaMin[chamber][i_sector]),
				 mdtRegion.etaMin[chamber][i_sector],
				 mdtRegion.etaMax[chamber][i_sector],
				 HLT::phiMean(mdtRegion.phiMin[chamber][i_sector],mdtRegion.phiMax[chamber][i_sector]),
				 mdtRegion.phiMin[chamber][i_sector],
				 mdtRegion.phiMax[chamber][i_sector]);
	 
	 m_regionSelector->DetHashIDList(MDT,static_cast<TYPEID>(mdtRegion.chamberType[chamber][i_sector][i_type]),
					 _roi, idList);
	 msg() << MSG::DEBUG << "...chamberType=" << mdtRegion.chamberType[chamber][i_sector][i_type] << endreq;
	 msg() << MSG::DEBUG << "...size IDlist=" << idList.size() << endreq;
	 std::vector<IdentifierHash>::const_iterator it = idList.begin();
	 while(it != idList.end()) {
	   if (i_sector==0) {
	     mdtIdHashes_normal.push_back(*it++);
	   } else {
	     mdtIdHashes_overlap.push_back(*it++);
	   }
	 }
       }
     }
   }
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::collectMdtHitsFromPrepData(const std::vector<IdentifierHash>& v_idHash,
								       std::vector<uint32_t>& v_robIds,
								       TrigL2MuonSA::MdtHits& mdtHits,
								       const TrigL2MuonSA::MuonRoad& muonRoad)
{    
  if (m_mdtPrepDataProvider->decode(v_robIds).isSuccess()) {
    msg() << MSG::DEBUG << "Calling ROB based decoding with "<< v_robIds.size() << " ROB's" << endreq;
  }
  else{
    msg() << MSG::WARNING << "Error in ROB based decoding" << endreq;
    return StatusCode::FAILURE;
  }
  
  // Get MDT container                                                                                                                                    
  if (v_idHash.empty()) {
    msg() << MSG::DEBUG << "Hash list is empty" << endreq;
    return StatusCode::SUCCESS;
  }
  
  const MdtPrepDataContainer* mdtPrds = 0;
  std::string mdtKey = "MDT_DriftCircles";
  
  if (m_activeStore) {
    if((*m_activeStore)->retrieve(mdtPrds, mdtKey).isFailure()) {
      msg() << MSG::ERROR << " Cannot retrieve MDT PRD Container " << mdtKey << endreq;
      return StatusCode::FAILURE;
    } else {
      msg()<< MSG::DEBUG << " MDT PRD Container retrieved with key " << mdtKey << endreq;	 
    }
  } else {
    msg() << MSG::ERROR << "Null pointer to ActiveStore" << endreq;
    return StatusCode::FAILURE;
  }
  
  // Get MDT collections                                                                                                                                
  ///// Vectors of prep data collections
  std::vector<const Muon::MdtPrepDataCollection*> mdtCols;
  
  MdtPrepDataContainer::const_iterator MDTcoll;
  for(std::vector<IdentifierHash>::const_iterator idit = v_idHash.begin(); idit != v_idHash.end(); ++idit) {
    
    MDTcoll = mdtPrds->indexFind(*idit);
    
    if( MDTcoll == mdtPrds->end() ) {
      msg() << MSG::DEBUG << "MDT prep data collection not found in Hash ID" << (int)*idit << endreq;
      continue;
    }
    
    if( (*MDTcoll)->size() == 0 ) {
      msg() << MSG::DEBUG << "MDT prep data collection is empty in Hash ID" << (int)*idit << endreq;
      continue;
    }
    
    mdtCols.push_back(*MDTcoll);
    
    msg() << MSG::DEBUG << "Selected Mdt Collection: "
	  << m_mdtIdHelper->show_to_string((*MDTcoll)->identify())
	  << " with size " << (*MDTcoll)->size()
	  << "in Hash ID" << (int)*idit << endreq;
  }
  
  std::vector< const MdtPrepDataCollection*>::const_iterator it = mdtCols.begin();
  std::vector< const MdtPrepDataCollection*>::const_iterator it_end = mdtCols.end();
  
  for( ;it!=it_end;++it ){
    
    Muon::MdtPrepDataCollection::const_iterator cit_begin = (*it)->begin();
    Muon::MdtPrepDataCollection::const_iterator cit_end = (*it)->end();
    
    if (cit_begin == cit_end) return StatusCode::SUCCESS;
    
    Muon::MdtPrepDataCollection::const_iterator cit = cit_begin;   
    for( ; cit!=cit_end;++cit ) {
      
      const Muon::MdtPrepData* mdt = (*cit);
      
      m_mdtReadout = mdt->detectorElement();
      if (!m_mdtReadout) continue;	
      
      m_muonStation = m_mdtReadout->parentMuonStation();
      
      int StationPhi = m_mdtReadout->getStationPhi();
      int StationEta = m_mdtReadout->getStationEta();
      int MultiLayer = m_mdtReadout->getMultilayer();
      double cXmid;
      double cYmid;
      double cAmid = 0;
      double cPhip;   
      
      Identifier id = mdt->identify();
      int adc       = mdt->adc();
      int drift     = mdt->tdc();
      
      int TubeLayers = m_mdtReadout->getNLayers();
      int TubeLayer = m_mdtIdHelper->tubeLayer(id);
      if(TubeLayer > TubeLayers) TubeLayer -= TubeLayers;
      int Layer = (MultiLayer-1)*TubeLayers + TubeLayer;
      int Tube = m_mdtIdHelper->tube(id);
      
      double OrtoRadialPos = m_mdtReadout->getStationS();
      std::string chamberType = m_mdtReadout->getStationType();
      char st = chamberType[1];
      
      int chamber = 0;
      if (chamberType[0]=='E') {
	/// Endcap
	if (st=='I') chamber = xAOD::L2MuonParameters::Chamber::EndcapInner;
	if (st=='M') chamber = xAOD::L2MuonParameters::Chamber::EndcapMiddle;
	if (st=='O') chamber = xAOD::L2MuonParameters::Chamber::EndcapOuter;
	if (st=='E') chamber = xAOD::L2MuonParameters::Chamber::EndcapExtra;
      } else {
	/// Barrel
	if (st=='I') chamber = xAOD::L2MuonParameters::Chamber::BarrelInner;
	if (st=='M') chamber = xAOD::L2MuonParameters::Chamber::BarrelMiddle;
	if (st=='O') chamber = xAOD::L2MuonParameters::Chamber::BarrelOuter;
      }
      
      double R = m_mdtReadout->center(TubeLayer, Tube).perp();
      double Z = m_mdtReadout->center(TubeLayer, Tube).z();
      
      Amg::Transform3D trans = Amg::CLHEPTransformToEigen(*m_muonStation->getNominalAmdbLRSToGlobal());
      if(m_muonStation->endcap()==0){
	cXmid = (trans*Amg::Vector3D(0.,0.,0.)).z();
	double halfRadialThicknessOfMultilayer = m_muonStation->RsizeMdtStation()/2.;
	cYmid = ((trans*Amg::Vector3D(0.,0.,0.)).perp()+halfRadialThicknessOfMultilayer);
      }
      else{
	cXmid = (trans*Amg::Vector3D(0.,0.,0.)).perp();
	double halfZThicknessOfMultilayer = m_muonStation->ZsizeMdtStation()/2.;
	cYmid = (trans*Amg::Vector3D(0.,0.,0.)).z();
	if(cYmid>0) cYmid += halfZThicknessOfMultilayer;
	else cYmid -= halfZThicknessOfMultilayer;
      }
      cPhip = (trans*Amg::Vector3D(0.,0.,0.)).phi();
      
      double dphi  = 0;
      double cphi  = muonRoad.phi[chamber][0];
      if( cPhip*cphi>0 ) {
	dphi = fabsf(cPhip - cphi);
      } else {
	if(fabs(cphi) > CLHEP::pi/2.) {
	  double phi1 = (cPhip>0.)? cPhip-CLHEP::pi : cPhip+CLHEP::pi;
	  double phi2 = (cphi >0.)? cphi -CLHEP::pi : cphi +CLHEP::pi;
	  dphi = fabsf(phi1) + fabsf(phi2); 
	}
	else {
	  dphi = fabsf(cPhip) + fabsf(cphi);
	}
      }
      
      if(m_muonStation->endcap()==1)
	R = sqrt(R*R+R*R*tan(dphi)*tan(dphi));
      
      Amg::Vector3D OrigOfMdtInAmdbFrame = 
	Amg::Hep3VectorToEigen( m_muonStation->getBlineFixedPointInAmdbLRS() );      
      double Rmin =(trans*OrigOfMdtInAmdbFrame).perp();  
      
      float cInCo = 1./cos(fabsf(atan(OrtoRadialPos/Rmin)));
      float cPhi0 = cPhip - atan(OrtoRadialPos/Rmin);
      if(cPhi0 > CLHEP::pi) cPhip -= 2*CLHEP::pi;
      if(cPhip<0. && (fabs(CLHEP::pi+cPhip) < 0.05) ) cPhip = acos(0.)*2.;
      
      msg() << MSG::DEBUG << " ...MDT hit Z/R/chamber/MultiLater/TubeLayer/Tube/Layer/adc/tdc = "
	    << Z << "/" << R << "/" << chamber << "/" << MultiLayer << "/" << TubeLayer << "/" 
	    << Tube << "/" << Layer << "/" << adc << "/" << drift << endreq;
      
      // no residual check for the moment
      // (residual check at pattern finder)
      if(Layer!=0 && Tube !=0) {
	
	// create the new digit
	TrigL2MuonSA::MdtHitData tmp;
	tmp.name       = 0;
	tmp.StationEta = StationEta;
	tmp.StationPhi = StationPhi;
	tmp.Multilayer = MultiLayer;
	tmp.Layer      = Layer - 1;
	tmp.TubeLayer  = TubeLayer;
	tmp.Tube       = Tube;
	tmp.cYmid      = cYmid;
	tmp.cXmid      = cXmid;
	tmp.cAmid      = cAmid;
	tmp.cPhip      = cPhip;
	tmp.cInCo      = cInCo;
	tmp.cPhi0      = cPhi0;
	for(unsigned int i=0; i<4; i++) { tmp.cType[i] = chamberType[i]; }
	tmp.Z          = Z;
	tmp.R          = R;
	tmp.DriftTime  = drift;
	tmp.DriftSpace = 0.;
	tmp.DriftSigma = 0;
	tmp.Adc        = adc;
	tmp.OnlineId   = 0;
	tmp.LeadingCoarseTime  = 0;
	tmp.LeadingFineTime    = 0;
	tmp.TrailingCoarseTime = 0;
	tmp.TrailingFineTime   = 0;
	tmp.Residual  = 0;
	tmp.isOutlier = 0;
	tmp.Chamber = chamber;
        
	mdtHits.push_back(tmp);
      }	  
    } // end of MdtPrepDataCollection loop
  } // end of MdtPrepDataCollection vector loop

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MdtDataPreparator::finalize()
{
   msg() << MSG::DEBUG << "Finalizing MdtDataPreparator - package version " << PACKAGE_VERSION << endreq;
   
   // delete message stream
   if ( m_msg ) delete m_msg;
   if ( m_mdtRegionDefiner ) delete m_mdtRegionDefiner;
   
   StatusCode sc = AthAlgTool::finalize(); 
   return sc;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include <algorithm>

#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IssueSeverity.h"
#include "InDetIdentifier/PixelID.h"
#include "Identifier/IdentifierHash.h" 

#include "InDetReadoutGeometry/SiDetectorManager.h"

#include "InDetPrepRawData/SiClusterContainer.h"

#include <string>
#include <sstream>

#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "TrigOnlineSpacePointTool/FastPixelClusterization.h"

#include "TrigOnlineSpacePointTool/PixelClusterCacheTool.h"
#include "eformat/SourceIdentifier.h" 
using eformat::helper::SourceIdentifier;
#include "ByteStreamData/ROBData.h" 

#define MAXNUM_PIX_BS_ERRORS 8


PixelClusterCacheTool::PixelClusterCacheTool( const std::string& type, 
					    const std::string& name, 
					    const IInterface* parent )
  : AlgTool(type, name, parent), 
    m_IdMapping("PixelCablingSvc",name), 
    m_offlineDecoder("PixelRodDecoder",this), 
    m_bsErrorSvc("PixelByteStreamErrorsSvc",name),
    m_clusteringTool("InDet::MergedPixelsTool/InDetTrigMergedPixelsTool") 
{

  declareInterface< IPixelClusterCacheTool>( this );
  declareProperty( "ClusterContainerName", m_containerName = "PixelClusterCache");
  declareProperty( "PixelROD_Decoder", m_offlineDecoder,"PixelRodDecoder");
  declareProperty( "RDO_ContainerName", m_rdoContainerName = "PixelRDO_Cache");
  declareProperty( "UseOfflineClustering", m_useOfflineClustering = true);
  declareProperty( "PixelClusteringTool", m_clusteringTool,"InDet::MergedPixelsTool/InDetTrigMergedPixelsTool");
  declareProperty( "DoBS_Conversion", m_doBS = true);
}

StatusCode PixelClusterCacheTool::initialize()  {
  MsgStream log(msgSvc(), name());

  log << MSG::INFO << name() << " in initialize" << endreq;
  StatusCode sc = AlgTool::initialize(); 
  // get DetectorStore service

  StoreGateSvc* detStore; 
  sc=service("DetectorStore",detStore);
  if (sc.isFailure()) { 
    log << MSG::ERROR << name() <<" failed to get detStore" << endreq;
    return sc;
  }

  sc = service( "StoreGateSvc", m_StoreGate );
  if (sc.isFailure()) {
    log << MSG::FATAL << "Unable to retrieve StoreGate service" << endreq;
    return sc;
  }

  const InDetDD::PixelDetectorManager * mgr;
  sc=detStore->retrieve(mgr, "Pixel");
  if (sc.isFailure()) {
    log << MSG::ERROR << name() << "failed to get Pixel Manager" << endreq;
    return StatusCode::FAILURE;
  } else { 
    log << MSG::DEBUG << name() << "Got Pixel Manager" << endreq;
  }
 
  // Get SCT & pixel helpers
  if (detStore->retrieve(m_pixel_id, "PixelID").isFailure()) {
    log << MSG::FATAL << "Could not get Pixel ID helper" << endreq;
    return StatusCode::FAILURE;
  } 

  p_indet_mgr = mgr;  

  m_clusterization.setPixelID(m_pixel_id);
  m_clusterization.initializeGeometry(mgr);
  m_cntx_pix = m_pixel_id->wafer_context();

  IToolSvc* toolSvc;
  sc=service("ToolSvc",toolSvc);
  if(sc.isFailure()) {
    log << MSG::ERROR << " Can't get ToolSvc " << endreq;
    return sc; 
  } 

  if(m_doBS) {

    sc = m_IdMapping.retrieve();
    if(sc.isFailure()) {
      log << MSG::ERROR << "cannot retrieve PixelCablingService " <<m_IdMapping<<endreq;  
      return sc;
    }   

    if(StatusCode::SUCCESS !=m_offlineDecoder.retrieve()) {
      log << MSG::ERROR << "initialize(): Can't get PixelRodDecoder " << endreq;
      return StatusCode::FAILURE; 
    }

    if(StatusCode::SUCCESS !=m_bsErrorSvc.retrieve()) {
      log << MSG::ERROR << "initialize(): Can't get PixelByteStreamError service "<<m_bsErrorSvc<< endreq;
      return StatusCode::FAILURE; 
    }
  }

  if(StatusCode::SUCCESS !=m_clusteringTool.retrieve()) {
    log << MSG::ERROR << "initialize(): Can't get tool "<<m_clusteringTool<< endreq;
    return StatusCode::FAILURE; 
  }
  else log << MSG::INFO << "retrieved "<<m_clusteringTool<< endreq; 

  log << MSG::INFO << "PixelCluster_CacheName: " << m_containerName << endreq;
  m_clusterContainer = new InDet::PixelClusterContainer(m_pixel_id->wafer_hash_max());
  m_clusterContainer->addRef();

  sc = m_StoreGate->record(m_clusterContainer, m_containerName);

  if (sc.isFailure()) { 
    log << MSG::ERROR  << " Container " << m_containerName << " could not be recorded in StoreGate !" << endreq;
    return sc;
  }  
  else { 
    log << MSG::INFO << "Container " << m_containerName << " registered  in StoreGate" << endreq;   
  } 

  // symlink the container 
  const InDet::SiClusterContainer* symSiContainer(0); 
  sc = m_StoreGate->symLink(m_clusterContainer,symSiContainer); 
  if (sc.isFailure()) { 
    log << MSG::ERROR << "Pixel clusters could not be symlinked in StoreGate !"<< endreq;
    return sc;
  }  
  else log << MSG::INFO  << "Pixel clusters " << m_containerName << " symlinked in StoreGate" << endreq;

  log << MSG::INFO << "PixelRDO_CacheName: " << m_rdoContainerName << endreq;

  if(m_doBS) {
    m_rdoContainer = new PixelRDO_Container(m_pixel_id->wafer_hash_max());
    m_rdoContainer->addRef();
  }

  return sc;
}

StatusCode PixelClusterCacheTool::finalize() 
{
  m_clusterContainer->cleanup();
  m_clusterContainer->release();
  if(m_doBS) {
    m_rdoContainer->cleanup();
    m_rdoContainer->release();
  }
  StatusCode sc = AlgTool::finalize(); 
  return sc;
}

StatusCode PixelClusterCacheTool::m_convertBStoClusters(std::vector<const ROBF*>& robFrags,
                                                  const std::vector<IdentifierHash> listOfPixIds,
							std::vector<int>& errorVect, bool isFullScan)
{
  MsgStream log(msgSvc(), name()); 
  int outputLevel = msgSvc()->outputLevel( name() );

  if(!m_StoreGate->contains<InDet::PixelClusterContainer>(m_containerName))
    {
      m_clusterContainer->cleanup();
      StatusCode sc=m_StoreGate->record(m_clusterContainer,m_containerName,false);
      if(sc.isFailure())
	{
	  log << MSG::WARNING << "Pixel Cluster Container " << m_containerName 
	      <<" cannot be recorded in StoreGate !"<< endreq;
	  return StatusCode::FAILURE;
	}
      else 
	{
	  if(outputLevel <= MSG::DEBUG)
	    log << MSG::DEBUG << "Pixel Cluster Container " << m_containerName
		<< " is recorded in StoreGate" << endreq;
	}
      if(m_doBS) {
	m_rdoContainer->cleanup();
	sc=m_StoreGate->record(m_rdoContainer,m_rdoContainerName,false);
	if(sc.isFailure())
	  {
	    log << MSG::WARNING << "PixelRDO Container " << m_rdoContainerName 
		<<" cannot be recorded in StoreGate !"<< endreq;
	    return StatusCode::FAILURE;
	  }
	else 
	  {
	    if(outputLevel <= MSG::DEBUG)
	      log << MSG::DEBUG << "PixelRDO Container " << m_rdoContainerName
		  << " is recorded in StoreGate" << endreq;
	  }
      }
    }
  else 
    {    
      if(outputLevel <= MSG::DEBUG)
	log << MSG::DEBUG << "Pixel Cluster Container " <<m_containerName 
	    << " is found in StoreGate" << endreq;
    }

  if(!m_doBS) {

    //retrieve m_rdoContainer here 

    if(!m_StoreGate->contains<PixelRDO_Container>(m_rdoContainerName)) {
      log << MSG::ERROR << "Pixel RDO Container " << m_rdoContainerName
	  <<" is not found in StoreGate !"<< endreq;
      return StatusCode::FAILURE;
    }
    const PixelRDO_Container* pCont = NULL; 
    StatusCode sc = m_StoreGate->retrieve(pCont, m_rdoContainerName);
    if (sc.isFailure()) {
      log << MSG::ERROR << "retrieval of Pixel RDO Container " << m_rdoContainerName
	  <<" failed"<< endreq;
      return sc;
    }

    m_rdoContainer = const_cast<PixelRDO_Container*>(pCont);

    if(m_rdoContainer==NULL) {
      log << MSG::ERROR << "Pixel RDO Container const_cast failed " << endreq;
      return StatusCode::FAILURE;
    }
  }

  std::vector<IdentifierHash> reducedList;
  reducedList.clear();

  for(std::vector<IdentifierHash>::const_iterator it=listOfPixIds.begin();it!=listOfPixIds.end();++it) {
    if(m_clusterContainer->indexFind((*it))==m_clusterContainer->end())
      reducedList.push_back(*it);
  }

  // std::copy(reducedList.begin(),reducedList.end(),std::ostream_iterator<IdentifierHash>(std::cout, "\n"));
  bool bs_failure=false;
  int n_rob_count=0;
  int n_recov_errors=0;

  if(m_doBS) {
    if(robFrags.size() == 0) {
      if(outputLevel <= MSG::DEBUG)
	log<<MSG::DEBUG<<" There are NO ROB data " <<endreq;
      return StatusCode::SUCCESS;
    }    
    if(outputLevel <= MSG::DEBUG)
      log << MSG::DEBUG<<" There are SOME ROB data " <<robFrags.size()<<endreq ;
    
    std::vector<bool> idList(m_pixel_id->wafer_hash_max(), false);
    
    IROBDataProviderSvc::VROBFRAG::const_iterator rob_it = robFrags.begin(); 
    IROBDataProviderSvc::VROBFRAG::const_iterator rob_it_end = robFrags.end(); 
    
    m_clusterization.initialize();
    
    for(; rob_it != rob_it_end; ++rob_it) 
      { 
	uint32_t rodid = (*rob_it)->rod_source_id() ; 
	eformat::helper::SourceIdentifier sid_rob(rodid) ;
	uint32_t detid = sid_rob.subdetector_id() ;
	if(outputLevel <= MSG::DEBUG)
	  log <<MSG::DEBUG<<"  ROB source ID SID " <<std::hex<<rodid <<std::dec<<endreq ;
	
	const ROBFragment* robFrag = (*rob_it); 
	if(outputLevel <= MSG::DEBUG)
	  log<<MSG::DEBUG<<std::hex<<"Det ID 0x"<<detid<<" Rod version 0x"<<
	    robFrag->rod_version()<<", Type="<<robFrag->rod_detev_type()<<std::dec<<endreq ;
	
	m_bsErrorSvc->resetCounts();
	
	StatusCode scRod(StatusCode::SUCCESS);
	if(!isFullScan) {
	  scRod=m_offlineDecoder->fillCollection(robFrag,m_rdoContainer,&reducedList);
	}
	else {
	  scRod=m_offlineDecoder->fillCollection(robFrag,m_rdoContainer,NULL);
	}
	
	if(scRod.isRecoverable())
	  {
	    n_recov_errors++;
	    for(int idx=0;idx<MAXNUM_PIX_BS_ERRORS;idx++)
	      {
		int n_errors = m_bsErrorSvc->getNumberOfErrors(idx);
		for(int ierror = 0; ierror < n_errors; ierror++)
		  errorVect.push_back(idx);
	      }
	  }
	else if(scRod.isFailure())
	  {
	    bs_failure=true;
	    break;
	  }
	++n_rob_count;
      }
  } 
  std::vector<InDet::PixelClusterCollection*> offlineCollVector;
  offlineCollVector.clear();

  int npixels=0;
  for (std::vector<IdentifierHash>::iterator it=reducedList.begin(); it != reducedList.end(); ++it)
    {
      PixelRDO_Container::const_iterator collIt=m_rdoContainer->indexFind((*it));
      if(collIt==m_rdoContainer->end())
	continue;
      if((*collIt)->size()!=0)
	{		  
	  if(!m_useOfflineClustering)
	    {
	      Identifier coll_id=(*collIt)->identify();
	      IdentifierHash coll_hash_id=(*collIt)->identifyHash();
	      for(PixelRDO_Collection::const_iterator rdoIt=(*collIt)->begin();rdoIt!=(*collIt)->end();
		  ++rdoIt)
		{
		  Identifier pixId=(*rdoIt)->identify();
		      
		  npixels++;
		  m_clusterization.addHit(coll_id,coll_hash_id,
					  m_pixel_id->phi_index(pixId),
					  m_pixel_id->eta_index(pixId),(*rdoIt));
		}
	    }
	  else 
	    {
	      const InDetRawDataCollection<PixelRDORawData>* pRdoColl = (*collIt);
	      InDet::PixelClusterCollection* pColl=m_clusteringTool->clusterize(*pRdoColl,*p_indet_mgr,*m_pixel_id);
	      if(pColl!=NULL)
		offlineCollVector.push_back(pColl);
	    }
	}
    }
  if(!m_useOfflineClustering) {
    if(npixels!=0) m_clusterization.finishHits();
  }
  std::vector<InDet::PixelClusterCollection*>& vColl=m_clusterization.getClusterCollections();

  if(m_useOfflineClustering) vColl=offlineCollVector;

  if(outputLevel <= MSG::DEBUG)
    log<<MSG::DEBUG<< "No of collections found: " <<  vColl.size() << endreq ;
  
  // Loop over cluster collections
  std::vector<InDet::PixelClusterCollection*>::iterator iter_coll = vColl.begin();
  for(;iter_coll!=vColl.end();iter_coll++) 
    {
      IdentifierHash hashId=(*iter_coll)->identifyHash();
      StatusCode sc=m_clusterContainer->addCollection((*iter_coll),hashId);
      if(sc.isFailure())
	{
	  if(outputLevel <= MSG::INFO)
	    log<<MSG::INFO<< "IDC record failed HashId="<<hashId<< endreq;
	}
    }
  if(outputLevel <= MSG::DEBUG)
    {
      if(bs_failure) 
	log << MSG::DEBUG <<" FAILURE in PixelRodDecoder"<<endreq;
      else
	log << MSG::DEBUG <<" number of ROBs used  "<<n_rob_count<< "Total number of recoverable errors "<<n_recov_errors<<endreq;
    }
  if(bs_failure)
    {
      return StatusCode::FAILURE;
    }
  else 
    {
      if(n_recov_errors!=0)
	{
	  return StatusCode::RECOVERABLE;
	}
      else return StatusCode::SUCCESS;
    }
}


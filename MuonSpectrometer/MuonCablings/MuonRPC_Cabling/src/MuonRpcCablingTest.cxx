/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 MuonGeoModel description
 -----------------------------------------
 Copyright (C) 2004 by ATLAS Collaboration
 ***************************************************************************/

//<doc><file>	$Id: MuonRpcCablingTest.cxx,v 1.43 2009/03/28 10:59:01 stefspa Exp $
//<version>	$Name:  $

//<<<<<< INCLUDES                                                       >>>>>>

#include "MuonRPC_Cabling/MuonRpcCablingTest.h"

#include "MuonIdHelpers/RpcIdHelper.h"
#include "RPCcablingInterface/IRPCcablingServerSvc.h"
#include "RPCcablingInterface/IRPCcablingSvc.h"
#include "RPCcablingInterface/RpcPadIdHash.h"


//#include "MuonReadoutGeometry/MuonDetectorManager.h"
//#include "MuonReadoutGeometry/RpcReadoutElement.h"

#include <fstream>
#include <sstream>
typedef std::istringstream mystream;

//<<<<<< PRIVATE DEFINES                                                >>>>>>
//<<<<<< PRIVATE CONSTANTS                                              >>>>>>
//<<<<<< PRIVATE TYPES                                                  >>>>>>
//<<<<<< PRIVATE VARIABLE DEFINITIONS                                   >>>>>>
//<<<<<< PUBLIC VARIABLE DEFINITIONS                                    >>>>>>
//<<<<<< CLASS STRUCTURE INITIALIZATION                                 >>>>>>
//<<<<<< PRIVATE FUNCTION DEFINITIONS                                   >>>>>>
//<<<<<< PUBLIC FUNCTION DEFINITIONS                                    >>>>>>
//<<<<<< MEMBER FUNCTION DEFINITIONS                                    >>>>>>


MuonRpcCablingTest::MuonRpcCablingTest(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm            ( name, pSvcLocator ),
    p_MuonMgr		    ( 0 ),
    m_idHelper              ( 0 ),
    m_selSideStr            ( "" ),
    m_firstLastChannel      ( false )
{
    //    m_check_blines = 0;
    declareProperty("selectedSide",    m_selSideStr="");
    declareProperty("selectedSectors", m_selSectorVec);
    declareProperty("selectedRois",    m_selRoiVec);    
    declareProperty("selectedPads",    m_selPadVec);    
    declareProperty("testOnlyFirstLastCMChannel",m_firstLastChannel=false);
    first = true;
}


MuonRpcCablingTest::~MuonRpcCablingTest()
{ }

StatusCode
MuonRpcCablingTest::initialize()
{
    StatusCode status = StatusCode::SUCCESS;

    msg()<<MSG::INFO<<"In Initialize"<<endreq;
    first = true;

//     status = detStore()->retrieve( p_MuonMgr );
//     if ( status.isFailure() ) {
// 	msg()<< MSG::ERROR << " Cannot retrieve MuonDetectorManager " << endreq;
//     }
//     else
//     {
// 	msg()<< MSG::DEBUG << " MuonDetectorManager  is retriven " << endreq;
// 	m_idHelper = p_MuonMgr->rpcIdHelper();
// 	msg() << MSG::DEBUG << " Id Helpers are obtained from MuonDetectorManager " << endreq;	
//     }	

    status = detStore()->retrieve( m_idHelper, "RPCIDHELPER" );
    if ( status.isFailure() ) 
    {
 	msg()<< MSG::ERROR << " Cannot retrieve RpcIdHelper " << endreq;
    }

    // get RPC cablingSvc
    const IRPCcablingServerSvc* RpcCabGet = 0;
    StatusCode sc = service("RPCcablingServerSvc", RpcCabGet);
    if (sc.isFailure()) {
	msg (MSG::FATAL) << "Could not get RPCcablingServerSvc !" << endreq;
	return StatusCode::FAILURE;
    }
    else msg (MSG::VERBOSE) << " RPCcablingServerSvc retrieved" << endreq;
  
    status = RpcCabGet->giveCabling(m_cablingSvc);
    if (status.isFailure()) {
	msg (MSG::FATAL) << "Could not get RPCcablingSvc from the Server !" << endreq;
	m_cablingSvc = 0;
	return StatusCode::FAILURE;
    } 
    else {
	msg (MSG::VERBOSE) << " RPCcablingSvc obtained " << endreq;
    }

    //    // get here the pad id helper 
    //    m_padHashIdHelper = new RpcPadIdHash();

    return status;
    
}


StatusCode
MuonRpcCablingTest::execute()
{
    if (!first) return StatusCode::SUCCESS;
    first = false;
    std::ofstream fout("RPC_Mapping.dump");
    
    MsgStream exe_log(messageService(), name());
    msg() << MSG::INFO << "Executing" << endreq;

    m_padHashIdHelper = m_cablingSvc->padHashFunction();
    
    
    const CablingRPCBase* _cabling = m_cablingSvc->getRPCCabling();
    if (!_cabling) 
    {
        msg (MSG::ERROR) <<" CablingRPCBase not retrieven !!!!!!"<<endreq;
        return StatusCode::FAILURE;
    }

    IdentifierHash Idhash;
    IdContext rpcModuleContext = m_idHelper->module_context();
    //const RpcPadContainer* tmpRdoContainer = new RpcPadContainer(1000);

    // this is the right thing to do !!!!!!!
    unsigned int hash_max = m_padHashIdHelper->max();
    msg (MSG::INFO) <<"From padHashIdHelper hash_max = "<<hash_max<<endreq;
    
    
    Identifier pad_idId;
    unsigned short int pad_id;
    unsigned int       logic_sector;
    Identifier PadId, PadId_previous;

    PadId_previous = (int) 0;

    unsigned short int iSubSysMin=0;
    unsigned short int iSubSysMax=1;
    if (m_selSideStr!="") 
    {
	if  (m_selSideStr=="A")  iSubSysMin=1;
	else if (m_selSideStr!="C") 
	{

	    msg()<<MSG::WARNING<<"Unknown side chosen - selected <"<<m_selSideStr<<"> while available are A and C"<<endreq;
	}
	else  iSubSysMax = 0;	
    }
    
    std::vector<unsigned short int> iSecVec;
    if (!m_selSectorVec.empty())
    {
	msg()<<MSG::INFO<<"# of selected Sectors is "<<m_selSectorVec.size()<<endreq;
	int ic = 0;
	for (std::vector<int>::const_iterator is=m_selSectorVec.begin(); is!=m_selSectorVec.end(); ++is) 
	{
	    iSecVec.push_back(*is);
	    msg()<<MSG::INFO<<"Sector Vec content is --- "<<*is<<" at element "<<ic<<endreq;
	    ++ic;
	}
	
    }	
    else 
    {
	for (unsigned short int is=0; is < 32; ++is) 
	{
	    iSecVec.push_back(is);
	}
    }

    std::vector<unsigned short int> iRoiVec;
    if (!m_selRoiVec.empty())
    {
	msg()<<MSG::INFO<<"# of selected Roi is "<<m_selRoiVec.size()<<endreq;
	int ic=0;
	for (std::vector<int>::const_iterator is=m_selRoiVec.begin(); is!=m_selRoiVec.end(); ++is) 
	{
	    iRoiVec.push_back(*is);
	    msg()<<MSG::INFO<<"Roi Vec content is --- "<<*is<<" at element "<<ic<<endreq;
	    ++ic;
	}
	
    }	
    else 
    {
	msg()<<MSG::INFO<<"No Roi Selected "<<endreq;
	int ic=0;
	for (unsigned short int is=0; is < 32; ++is) 
	{
	    iRoiVec.push_back(is);
	    msg()<<MSG::DEBUG<<"Roi Vec content is --- "<<is<<" at element "<<ic<<endreq;
	    ++ic;
	}
    }
    unsigned short int iSubSysId=iSubSysMin;
    for (; iSubSysId<iSubSysMax+1; ++iSubSysId)
    {
	ATH_MSG_INFO("New SubSysID = " << iSubSysId);
        for (std::vector<unsigned short int>::const_iterator is=iSecVec.begin(); is!=iSecVec.end(); ++is)
        {
	    unsigned short int iSectorId = *is;
	    ATH_MSG_INFO("New SectorID = " << iSectorId);
	    
            for (std::vector<unsigned short int>::const_iterator ir=iRoiVec.begin(); ir!=iRoiVec.end(); ++ir)
            {
		unsigned short int iRoiNumber = *ir;
		ATH_MSG_DEBUG("New RoiID = " << iRoiNumber);
		
                ATH_MSG_DEBUG("SubSysID  = " << iSubSysId << " SectorID = " << iSectorId << " RoiNumber =" << iRoiNumber);
                if (_cabling->give_PAD_address(iSubSysId,iSectorId,iRoiNumber,logic_sector,pad_id,PadId))
                {
		    pad_idId = pad_id;
                    ATH_MSG_DEBUG("pad_id, PadId ------------------------ "
				 <<  (int)pad_idId.get_identifier32().get_compact() <<" "<< PadId<<" "<<m_idHelper->show_to_string(PadId));
                    
                    if (PadId_previous == PadId)
                    {
                        ATH_MSG_DEBUG("same pad as before ");
                    }
                    else
                    {
                        IdentifierHash padHash = (*m_padHashIdHelper)(PadId);
                        ATH_MSG_DEBUG("Associated pad-Hash (via the hash-function) = "<<(int)padHash);
                        if (padHash>hash_max) msg (MSG::ERROR) <<"Computed hashId > hash_max = "<<hash_max<<endreq;

                        for (unsigned short int CMAId=0; CMAId<8; ++CMAId)
                        {
                            for (unsigned short ijk=0; ijk<6; ++ijk)
                            {
				for (unsigned short Channel=0; Channel<32;)
				{	
				    std::list<Identifier> myOfflineList = 
					m_cablingSvc->give_strip_id   (iSubSysId,
                                                   iSectorId,
                                                   pad_idId.get_identifier32().get_compact(),
                                                   CMAId	,
                                                   ijk,
                                                   Channel);
                                    
				    ATH_MSG_DEBUG("ijk = "<<ijk<<" channel = "<<Channel<<" list size is "<<myOfflineList.size());				    
				    fout<<"Online "<<iSubSysId<<"."<<iSectorId<<"."<<pad_idId.get_identifier32().get_compact()<<"."<<CMAId<<"."<<ijk<<"."<<Channel
					<<" \t "<<myOfflineList.size()<<" \t ";
				    ATH_MSG_DEBUG("Online "<<iSubSysId<<"."<<iSectorId<<"."<<pad_idId.get_identifier32().get_compact()<<"."<<CMAId<<"."<<ijk<<"."<<Channel
						  <<" \t "<<myOfflineList.size()<<" \t ");
				    if (!myOfflineList.empty()) 
				    {
					for (std::list<Identifier>::const_iterator it=myOfflineList.begin(); it!=myOfflineList.end(); ++it)
					{
					    ATH_MSG_DEBUG("ijk = "<<ijk<<" channel = "<<Channel<<" id offline = "<<m_idHelper->show_to_string(*it));
					    //Identifier idp = m_idHelper->parentID(*it);
					    fout<<m_idHelper->show_to_string(*it)<<" ";
					    ATH_MSG_DEBUG(m_idHelper->show_to_string(*it)<<" ");
					}
				    }
				    fout<<std::endl;
				    if (m_firstLastChannel) Channel+=31;
				    else ++Channel;
				}				
                            }
                        }
                    }
                    PadId_previous =  PadId;                                            
                }
                else 
                    ATH_MSG_DEBUG("No pad for this side/sector/roi ");
            }
        }
    }

    fout.close();
    

    return StatusCode::SUCCESS;
}


StatusCode
MuonRpcCablingTest::finalize()
{
    StatusCode status = StatusCode::SUCCESS;

    msg()<< MSG::INFO << "Finalizing" << endreq;
     
    return status;
}



/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TBBPCMonTool.cxx
// PACKAGE:  TBMonitoring
//
// AUTHOR:   Pierre-Antoine Delsart
//
//
// ********************************************************************

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"

#include "TBMonitoring/TBBPCMonTool.h"

#include "TBEvent/TBBPCCont.h"
#include "TBEvent/TBBPCRawCont.h"

#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

#include <vector>


/*---------------------------------------------------------*/
TBBPCMonTool::TBBPCMonTool(const std::string & type, 
				 const std::string & name,
				 const IInterface* parent)
  : MonitorToolBase(type, name, parent)
/*---------------------------------------------------------*/
{
  declareInterface<IMonitorToolBase>(this);
  declareProperty("histoPathBase",m_path = "/stat");
  declareProperty("histoPath",m_histoPath = "/BeamDetectors/BPC/");

  declareProperty("SGBPCkey",m_SGkeybpc = "BPCCont");
  declareProperty("SGBPCRawkey",m_SGkeybpcraw = "BPCRawCont");


  declareProperty("MonitorBPC",m_monitor_bpc=true);
  declareProperty("MonitorBPCRaw",m_monitor_bpcraw=true);

  declareProperty("FakeDetectors",m_fake_detector=false);

  declareProperty("BookAtFirstEvent",m_bookatfirstevent = true);

  //BPC histogram properties
  declareProperty("BPCNames",m_bpc_names);
  declareProperty("BPCMaxPos",m_posmax=10);
  declareProperty("BPCMinPos",m_posmin=-10);
  declareProperty("BPCBinNum",m_posbin=100);


  declareProperty("TDCBinNum",m_tdcbin=100);
  declareProperty("TDCMax",m_tdcmax=1024);
  declareProperty("TDCMin",m_tdcmin=0);

  m_histo_bpcposX = NULL;
  m_histo_bpcposY = NULL;
  m_histo_bpcXvsY = NULL;

  m_histo_bpctdcLeft = NULL;
  m_histo_bpctdcRight=NULL;
  m_histo_bpctdcUp=NULL;
  m_histo_bpctdcDown=NULL;
  m_histo_bpcadcHorizontal=NULL;
  m_histo_bpcadcVertical=NULL;

}

/*---------------------------------------------------------*/
TBBPCMonTool::~TBBPCMonTool()
/*---------------------------------------------------------*/
{
  if ( m_histo_bpcposX )
    delete[] m_histo_bpcposX;
  if ( m_histo_bpcposY )
    delete[] m_histo_bpcposY;
  if ( m_histo_bpcXvsY )
    delete[] m_histo_bpcXvsY;

  if (m_histo_bpctdcLeft)
    delete[] m_histo_bpctdcLeft;
  if ( m_histo_bpctdcRight )
    delete[] m_histo_bpctdcRight;
  if (m_histo_bpctdcUp )
    delete[] m_histo_bpctdcUp;
  if (m_histo_bpctdcDown)
    delete[] m_histo_bpctdcDown;
  if (m_histo_bpcadcHorizontal)
    delete[] m_histo_bpcadcHorizontal;
  if (m_histo_bpcadcVertical)
    delete[] m_histo_bpcadcVertical;



}

/*---------------------------------------------------------*/
StatusCode TBBPCMonTool:: initialize()
/*---------------------------------------------------------*/
{
  MsgStream log(msgSvc(), name());
  
  StatusCode sc;



  sc = service( "StoreGateSvc", m_StoreGate);
  if( sc.isFailure() ) {
    log << MSG::FATAL << name() 
	<< ": Unable to locate Service StoreGateSvc" 
	<< endreq;
    return sc;
  }



  //set to true whitin bookHist() 
  m_isBooked = false;

  return StatusCode::SUCCESS;
}

/*---------------------------------------------------------*/
StatusCode TBBPCMonTool::bookHists()
/*---------------------------------------------------------*/
{
  // This is called by parent class in initialize().
  // We want to book histos according to 1st event so 
  // we use mybookHist()
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "in bookHists()" << endreq;
  if(!m_bookatfirstevent)
    {
      log << MSG::INFO << " Booking histos now" << endreq;
      m_bpcnum =  m_bpc_names.size();
      std::string path;
      if(m_monitor_bpc){
	m_histo_bpcposX = new IHistogram1D*[m_bpcnum];
	m_histo_bpcposY = new IHistogram1D*[m_bpcnum];
	m_histo_bpcXvsY = new IHistogram2D*[m_bpcnum];
	
	path = m_path + m_histoPath;//+ "/BeamDetectors/BPC/";
	m_bpc_map.clear();
	
	for(int j=0;j<m_bpcnum;j++){
	  m_bpc_map.push_back(j);
	  std::string hname = path; hname =hname+ m_bpc_names[j]+ "Xpos";
	  std::string htitle = m_bpc_names[j]+ " X position";
	  m_histo_bpcposX[j] = ToolHistoSvc()->book(hname,htitle,m_posbin,m_posmin,m_posmax);
	  hname = path+m_bpc_names[j]+"Ypos";
	  htitle = m_bpc_names[j]+ " Y position";
	  m_histo_bpcposY[j] = ToolHistoSvc()->book(hname,htitle,m_posbin,m_posmin,m_posmax);
	  
	  hname = path+m_bpc_names[j]+"XvsY";
	  htitle = m_bpc_names[j]+ "X vs Y";
	  m_histo_bpcXvsY[j] = ToolHistoSvc()->book(hname,htitle,m_posbin,m_posmin,m_posmax,m_posbin,m_posmin,m_posmax);
	}
      }
      
      if(m_monitor_bpc){
	m_bpc_map.clear();
	
	m_histo_bpctdcLeft  = new IHistogram1D*[m_bpcnum];
	m_histo_bpctdcRight = new IHistogram1D*[m_bpcnum];
	m_histo_bpctdcUp    = new IHistogram1D*[m_bpcnum];
	m_histo_bpctdcDown  = new IHistogram1D*[m_bpcnum];
	m_histo_bpcadcHorizontal = new IHistogram1D*[m_bpcnum]; 
	m_histo_bpcadcVertical   = new IHistogram1D*[m_bpcnum];
	
	path =m_path + m_histoPath; //"/BeamDetectors/BPC/";
       
	for(int j=0;j<m_bpcnum;j++){
	  m_bpc_map.push_back(j);
	  std::string hname = path+m_bpc_names[j]+"tdcLeft";
	  std::string htitle =  m_bpc_names[j]+ " TDC Left";
	  m_histo_bpctdcLeft[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);
	 
	  hname = path+m_bpc_names[j]+"tdcRight";
	  htitle = m_bpc_names[j]+ " TDC Right";
	  m_histo_bpctdcRight[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);

	  hname = path+m_bpc_names[j]+"tdcUp";
	  htitle = m_bpc_names[j]+ " TDC Up";
	  m_histo_bpctdcUp[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);

	  hname = path+m_bpc_names[j]+"tdcDown";
	  htitle = m_bpc_names[j]+ " TDC Down";
	  m_histo_bpctdcDown[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);

	  hname = path+m_bpc_names[j]+"adcVert";
	  htitle = m_bpc_names[j]+ " ADC Vert";
	  m_histo_bpcadcVertical[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);

	  hname = path+m_bpc_names[j]+"adcHoriz";
	  htitle = m_bpc_names[j]+ " ADC Horiz";
	  m_histo_bpcadcHorizontal[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);

	}

      }
      
      log << MSG::DEBUG << "histo path: " << path  << endreq;
      
      log << MSG::INFO << " \t Monitoring BPC \t " ;
      if(m_monitor_bpc) log << " \t : YES "<< endreq;
      else log << " \t : NO "<< endreq;

      log << MSG::INFO << " \t Monitoring raw BPC  " ;
      if(m_monitor_bpcraw) log << " \t \t : YES "<< endreq;
      else log << " \t \t: NO "<< endreq;

      
      log << MSG::INFO << " \t Fake Detectors  \t  " ;
      if(m_fake_detector) log << " \t : YES "<< endreq;
      else log << " \t : NO "<< endreq;

      SetBookStatus(true);
      return StatusCode::SUCCESS;      
    }
  return StatusCode::SUCCESS;
}

/*---------------------------------------------------------*/
StatusCode TBBPCMonTool::mybookHists()
/*---------------------------------------------------------*/
{
 
  MsgStream log(msgSvc(), name());

#ifndef NDEBUG
  log << MSG::DEBUG << "in mybookHists()" << endreq;
#endif

  StatusCode sc;
  std::string path = m_path;

 //Get Run number
  std::stringstream rn_stream;
  EventID *thisEvent;           //EventID is a part of EventInfo
  const EventInfo* thisEventInfo;
  sc=m_StoreGate->retrieve(thisEventInfo);
  if (sc!=StatusCode::SUCCESS)
    log << MSG::WARNING << "No EventInfo object found! Can't read run number!" << endreq;
  else
    {thisEvent=thisEventInfo->event_ID();
     rn_stream << "Run " << thisEvent->run_number() << " ";
    }

  const std::string runnumber=rn_stream.str();

  TBBPCCont * bpcCont;
  TBBPCRawCont *  bpcrawCont;
  //  BPC Histos -----------------------------------
   if(m_monitor_bpc){
     // retrieve bpc container.
     sc = m_StoreGate->retrieve(bpcCont, m_SGkeybpc);
     if (sc.isFailure()){
       log << MSG::INFO 
	   << "BeamDetectorMonitoring: Retrieval of BPC failed" 
	   << endreq;
       m_monitor_bpc=false;
     } else {
       
       // get number of bpc
       m_bpcnum = bpcCont->size() > m_bpc_names.size() ? m_bpc_names.size():bpcCont->size();
       // if m_bpcnum=0 don't book histo yet.
       if(m_bpcnum==0) {
	 log << MSG::INFO << " BPC Cont is empty"<< endreq;
	 return StatusCode::SUCCESS;
       }
       if(bpcCont->size() != m_bpc_names.size())
	 log << MSG::INFO 
	     << "!! Warning !! Number of BPC in SG="
	     <<  bpcCont->size()
	     << " differs from number in jobOptions=" << m_bpc_names.size()
	     << endreq;

       // Map histo number to a bpc name
       m_bpc_map.clear();
       //m_bpc_map.resize(m_bpcnum);
       int Nfound=0;
       for(unsigned j=0;j<m_bpc_names.size();j++){
	 TBBPCCont::const_iterator it_bc   = bpcCont->begin();
	 TBBPCCont::const_iterator last_bc   = bpcCont->end();
	 while(it_bc!=last_bc) {
	   if((*it_bc)->getDetectorName()==m_bpc_names[j]) break;
	   it_bc++;
	 }
	 if(it_bc!=last_bc){
	   m_bpc_map.push_back(j);
	   log << MSG::DEBUG << "histo "<<Nfound<< " --> BPC "<< m_bpc_names[m_bpc_map[Nfound]] << endreq;
	   Nfound++;
	 }
	 else {log << MSG::DEBUG << "BPC "<< m_bpc_names[j]<< " Not Found"  << endreq;}
       }
       
       // Now there are Nfound matching between BPC in Cont and the ones in the joboption list
       // BPC j in Cont is BPC m_bpc_map[j] in joboption list.

       //       m_bpc_map.resize(Nfound);
       m_bpcnum=Nfound;
 
       m_histo_bpcposX = new IHistogram1D*[m_bpcnum];
       m_histo_bpcposY = new IHistogram1D*[m_bpcnum];
       m_histo_bpcXvsY = new IHistogram2D*[m_bpcnum];

       path = m_path + m_histoPath;//"/BeamDetectors/BPC/";

       for(int j=0;j<m_bpcnum;j++){
	 std::string hname = path; hname =hname+ m_bpc_names[m_bpc_map[j]]+ "Xpos";
	 std::string htitle = runnumber+m_bpc_names[m_bpc_map[j]]+ " X position";
	 m_histo_bpcposX[j] = ToolHistoSvc()->book(hname,htitle,m_posbin,m_posmin,m_posmax);
	 hname = path+m_bpc_names[m_bpc_map[j]]+"Ypos";
	 htitle = runnumber+m_bpc_names[m_bpc_map[j]]+ " Y position";
	 m_histo_bpcposY[j] = ToolHistoSvc()->book(hname,htitle,m_posbin,m_posmin,m_posmax);

	 hname = path+m_bpc_names[m_bpc_map[j]]+"XvsY";
	 htitle = runnumber+m_bpc_names[m_bpc_map[j]]+ "X vs Y";
	 m_histo_bpcXvsY[j] = ToolHistoSvc()->book(hname,htitle,m_posbin,m_posmin,m_posmax,m_posbin,m_posmin,m_posmax);
       }
     }
     
   }
   // BPCRaw histos -------------------------------------
   if(m_monitor_bpcraw){
     sc = m_StoreGate->retrieve(bpcrawCont, m_SGkeybpcraw);
     if (sc.isFailure()){
       log << MSG::INFO << "BeamDetectorMonitoring: Retrieval of BPCRawCont failed" << endreq;
       m_monitor_bpcraw=false;
     } else {
       m_bpcnum = bpcrawCont->size() > m_bpc_names.size() ? m_bpc_names.size():bpcrawCont->size();
       if(m_bpcnum==0) {
	 log << MSG::INFO << " BPCRaw Cont is empty"<< endreq;
	 return StatusCode::SUCCESS;
       }
       if(bpcrawCont->size() != m_bpc_names.size())
	 log << MSG::INFO << "!! Warning !! Number of BPCRaw in SG="<< bpcrawCont->size()<<" differs from number in jobOptions" << endreq;
       
       // Map histo number to a bpc name
       m_bpc_map.clear();
       //m_bpc_map.resize(m_bpcnum);
       int Nfound=0;
       for(unsigned j=0;j<m_bpc_names.size();j++){
	 TBBPCRawCont::const_iterator it_bc   = bpcrawCont->begin();
	 TBBPCRawCont::const_iterator last_bc   = bpcrawCont->end();
	 while(it_bc!=last_bc) {
	   if((*it_bc)->getDetectorName()==m_bpc_names[j]) break;
	   it_bc++;
	 }
	 if(it_bc!=last_bc){
	   m_bpc_map.push_back(j);
	   log << MSG::DEBUG << "histo "<<Nfound<< " --> BPC "<< m_bpc_names[j] <<" ==" << (*it_bc)->getDetectorName()  << endreq;
	   Nfound++;
	 }
	 else {log << MSG::DEBUG << "BPC "<< m_bpc_names[j]<< " Not Found"  << endreq;}
       }
       
       // Now there are Nfound matching between BPC in Cont and the ones in the joboption list
       // BPC j in Cont is BPC m_bpc_map[j] in joboption list.

       //       m_bpc_map.resize(Nfound);
       m_bpcnum=Nfound;

       m_histo_bpctdcLeft  = new IHistogram1D*[m_bpcnum];
       m_histo_bpctdcRight = new IHistogram1D*[m_bpcnum];
       m_histo_bpctdcUp    = new IHistogram1D*[m_bpcnum];
       m_histo_bpctdcDown  = new IHistogram1D*[m_bpcnum];
       m_histo_bpcadcHorizontal = new IHistogram1D*[m_bpcnum]; 
       m_histo_bpcadcVertical   = new IHistogram1D*[m_bpcnum];

       path =m_path + m_histoPath;//"/BeamDetectors/BPC/";

       for(int j=0;j<m_bpcnum;j++){
	 std::string hname = path+m_bpc_names[m_bpc_map[j]]+"tdcLeft";
	 std::string htitle =  runnumber+m_bpc_names[m_bpc_map[j]]+ " TDC Left";
	 m_histo_bpctdcLeft[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);
	 
	 hname = path+m_bpc_names[m_bpc_map[j]]+"tdcRight";
	 htitle = runnumber+m_bpc_names[m_bpc_map[j]]+ " TDC Right";
	 m_histo_bpctdcRight[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);

	 hname = path+m_bpc_names[m_bpc_map[j]]+"tdcUp";
	 htitle = runnumber+m_bpc_names[m_bpc_map[j]]+ " TDC Up";
	 m_histo_bpctdcUp[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);

	 hname = path+m_bpc_names[m_bpc_map[j]]+"tdcDown";
	 htitle = runnumber+m_bpc_names[m_bpc_map[j]]+ " TDC Down";
	 m_histo_bpctdcDown[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);

	 hname = path+m_bpc_names[m_bpc_map[j]]+"adcVert";
	 htitle = runnumber+m_bpc_names[m_bpc_map[j]]+ " ADC Vert";
	 m_histo_bpcadcVertical[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);

	 hname = path+m_bpc_names[m_bpc_map[j]]+"adcHoriz";
	 htitle = runnumber+m_bpc_names[m_bpc_map[j]]+ " ADC Horiz";
	 m_histo_bpcadcHorizontal[j] = ToolHistoSvc()->book(hname,htitle,m_tdcbin,0,m_tdcmax);

       }
     }
   }


  log << MSG::DEBUG << "histo path: " << path  << endreq;

  log << MSG::INFO << " \t Monitoring BPC \t " ;
  if(m_monitor_bpc) log << " \t : YES "<< endreq;
  else log << " \t : NO "<< endreq;

  log << MSG::INFO << " \t Monitoring raw BPC  " ;
  if(m_monitor_bpcraw) log << " \t \t : YES "<< endreq;
  else log << " \t \t: NO "<< endreq;


  log << MSG::INFO << " \t Fake Detectors  \t  " ;
  if(m_fake_detector) log << " \t : YES "<< endreq;
  else log << " \t : NO "<< endreq;
  
  SetBookStatus(true);

  return StatusCode::SUCCESS;
}

// /*---------------------------------------------------------*/
// bool TBBPCMonTool::histsNotBooked()
// /*---------------------------------------------------------*/
// {

//   if(m_isBooked) return false;
//   else return true;
// }

/*---------------------------------------------------------*/
StatusCode TBBPCMonTool::fillHists()
/*---------------------------------------------------------*/
{
 
  MsgStream log(msgSvc(), name());

#ifndef NDEBUG
  log << MSG::DEBUG << "in fillHists()" << endreq;
#endif

  // Fill some bpc and stuff (testing) 
  if(m_fake_detector) FillRandomDetect();

  if(this->histsNotBooked()) {
    this->mybookHists();
  }

  StatusCode sc;

  if(m_bpcnum==0) {
    log << MSG::INFO << " Nothing to monitor"<< endreq;
    return StatusCode::SUCCESS;
  }
  
  // BPC & BPCRaw Monitor     ----------------------------------------------------
  if(m_monitor_bpc){
    //Pointer to a TBBPC container
    TBBPCCont *  bpcCont;
    TBBPCRawCont *  bpcrawCont;
    
    //Retrieve TBBPC collection from SG
    sc = m_StoreGate->retrieve(bpcCont, m_SGkeybpc);
    if (sc.isFailure()){
      log << MSG::DEBUG << "BeamDetectorMonitoring: Retrieval of BPC failed" << endreq;

    }else {

      TBBPCRawCont::const_iterator it_bcraw ;
      //      TBBPCRawCont::const_iterator last_bcraw ;
      if(m_monitor_bpcraw){
	sc = m_StoreGate->retrieve(bpcrawCont, m_SGkeybpcraw);
	if (sc.isFailure()){
	  log << MSG::DEBUG << "BeamDetectorMonitoring: Retrieval of BPCRaw failed" << endreq;
	  
	}
      }

      TBBPCCont::const_iterator it_bc   = bpcCont->begin();
      TBBPCCont::const_iterator last_bc   = bpcCont->end();


      // loop over list of bpc to monitor :
      for(int nameind=0; nameind<m_bpcnum;nameind++){

	// if we got out of sync, find the bpc named m_bpc_names[m_bpc_map[nameind]]
	if(it_bc==last_bc) it_bc   = bpcCont->begin();  // reinit it_bc
	if((*it_bc)->getDetectorName() != m_bpc_names[m_bpc_map[nameind]]){
	  it_bc=bpcCont->begin();
	  while(it_bc!=last_bc)
	    if ((*it_bc)->getDetectorName() != m_bpc_names[m_bpc_map[nameind]]) it_bc++;
	    else break;

	  if(it_bc==last_bc) {
	    // did not find the bpc
	    log << MSG::INFO 
		<< "BeamDetectorMonitoring: could not find bpc named "
		<< m_bpc_names[m_bpc_map[nameind]] 
		<< endreq;
	    continue;
	  }
	}
	
	// now it_bpc contains the right bpc.
	const TBBPC * bpc = (*it_bc);
	log << MSG::DEBUG << "Filling histo '" << m_bpc_names[m_bpc_map[nameind]] 
	    <<". X=" << bpc->getXPos() << " Y=" << bpc->getYPos() << endreq;
	if(! bpc->isXPosOverflow()) m_histo_bpcposX[nameind]->fill(bpc->getXPos() , 1.0);
	if(! bpc->isYPosOverflow()) m_histo_bpcposY[nameind]->fill(bpc->getYPos() , 1.0);
	
	if(!bpc->isYPosOverflow()&& !bpc->isXPosOverflow())  m_histo_bpcXvsY[nameind]->fill(bpc->getXPos(),bpc->getYPos(),1.0);

	if(m_monitor_bpcraw){
	  // get BPCRaw coresponding to BPC 
	  it_bcraw =  bpcrawCont->begin() + (it_bc - bpcCont->begin()) ;
	  const TBBPCRaw * bpcraw = (*it_bcraw);
	  if(!bpcraw->isOverflow(TBBPCRaw::tdcLeft))  m_histo_bpctdcLeft[nameind]->fill(bpcraw->getTDCLeft(),1.0);
	  if(!bpcraw->isOverflow(TBBPCRaw::tdcRight)) m_histo_bpctdcRight[nameind]->fill(bpcraw->getTDCRight(),1.0);
	  if(!bpcraw->isOverflow(TBBPCRaw::tdcUp))    m_histo_bpctdcDown[nameind]->fill(bpcraw->getTDCUp(),1.0);
	  if(!bpcraw->isOverflow(TBBPCRaw::tdcDown))  m_histo_bpctdcUp[nameind]->fill(bpcraw->getTDCDown(),1.0);
	  if(!bpcraw->isOverflow(TBBPCRaw::adcHorizontal)) m_histo_bpcadcHorizontal[nameind]->fill(bpcraw->getADCHorizontal(),1.0);
	  if(!bpcraw->isOverflow(TBBPCRaw::adcVertical))   m_histo_bpcadcVertical[nameind]->fill(bpcraw->getADCVertical(),1.0);
	}
	
	// get next bpc :
	it_bc++;

      } // nameind loop

    } // storegate
  } // monitor_bpc


  // BPCRaw only Monitor     ----------------------------------------------------
  if(m_monitor_bpcraw && !m_monitor_bpc ){
    //Pointer to a TBBPC container
    TBBPCRawCont *  bpcrawCont;
    
    //Retrieve TBBPC collection from SG
    sc = m_StoreGate->retrieve(bpcrawCont, m_SGkeybpcraw);
    if (sc.isFailure()){
      log << MSG::DEBUG 
	  << "BeamDetectorMonitoring: Retrieval of BPCRaw failed" 
	  << endreq;

    }else {

      TBBPCRawCont::const_iterator it_bc   = bpcrawCont->begin();
      TBBPCRawCont::const_iterator last_bc   = bpcrawCont->end();


      // loop over list of bpc to monitor :
      for(int nameind=0; nameind<m_bpcnum;nameind++){
	log << MSG::DEBUG << "looking for BPC : "<< m_bpc_names[m_bpc_map[nameind]] <<endreq;

	// if we got out of sync, find the bpc named m_bpc_names[m_bpc_map[nameind]]
	if(it_bc==last_bc) it_bc   = bpcrawCont->begin();  // reinit it_bc
	if((*it_bc)->getDetectorName() != m_bpc_names[m_bpc_map[nameind]]){
	  it_bc=bpcrawCont->begin();
	  while(it_bc!=last_bc){
	    if ((*it_bc)->getDetectorName() != m_bpc_names[m_bpc_map[nameind]]) it_bc++;
	    else break;
	  }
	  if(it_bc==last_bc) {
	    // did not find the bpcraw
	    log << MSG::INFO 
		<< "BeamDetectorMonitoring: could not find bpcraw named "
		<< m_bpc_names[m_bpc_map[nameind]] 
		<< endreq;
	    continue;
	  }else {}
	}
	

	// now it_bc contains the right bpc.
	const TBBPCRaw * bpcraw = (*it_bc);
	log << MSG::DEBUG << "BPC " <<bpcraw->getDetectorName() << endreq;
	log << MSG::DEBUG 
	    << "BPCRaw"
	    << nameind << "  " 
	    << bpcraw->getDetectorName() << " signals "
	    << (unsigned int) bpcraw->getTDCLeft()
	    << " " <<  (int) bpcraw->getTDCRight()
	    << " " <<  (int) bpcraw->getTDCUp() 
	    << " " <<  (int) bpcraw->getTDCDown()
	    << endreq;

	if(!bpcraw->isOverflow(TBBPCRaw::tdcLeft))  m_histo_bpctdcLeft[nameind]->fill(bpcraw->getTDCLeft(),1.0);
	if(!bpcraw->isOverflow(TBBPCRaw::tdcRight)) m_histo_bpctdcRight[nameind]->fill(bpcraw->getTDCRight(),1.0);
	if(!bpcraw->isOverflow(TBBPCRaw::tdcUp))    m_histo_bpctdcDown[nameind]->fill(bpcraw->getTDCUp(),1.0);
	if(!bpcraw->isOverflow(TBBPCRaw::tdcDown))  m_histo_bpctdcUp[nameind]->fill(bpcraw->getTDCDown(),1.0);
	if(!bpcraw->isOverflow(TBBPCRaw::adcHorizontal)) m_histo_bpcadcHorizontal[nameind]->fill(bpcraw->getADCHorizontal(),1.0);
	if(!bpcraw->isOverflow(TBBPCRaw::adcVertical))   m_histo_bpcadcVertical[nameind]->fill(bpcraw->getADCVertical(),1.0);
	
	
	it_bc++;

      }
    }
  } 

  log << MSG::DEBUG << "fillHists() ended" << endreq;
  return StatusCode::SUCCESS;
}

/*---------------------------------------------------------*/
void TBBPCMonTool::FillRandomDetect()
/*---------------------------------------------------------*/
{
  // Fake different beam detectors/data classes

  MsgStream log(msgSvc(), name());

  // BPC ---------------------------------------------------
  TBBPCCont * bpcCont = new TBBPCCont();


  TBBPC *bpc1 = new TBBPC("BPC1");
  TBBPC *bpc2 = new TBBPC("BPC2");

  bpc1->setXPos(1.0);   bpc1->setYPos(1.0); 
  bpc2->setXPos(2.0);   bpc2->setYPos(2.0); 

  bpcCont->push_back(bpc1);
  bpcCont->push_back(bpc2);


  StatusCode sc = m_StoreGate->record(bpcCont,m_SGkeybpc);
  if ( sc.isFailure( ) ) {
    log << MSG::FATAL << "Cannot record BPCCont" << endreq;
  }

  // Broken.
  // BPC Raw ---------------------------------------------------
  TBBPCRawCont * bpcrawCont = new TBBPCRawCont();
  TBBPCRaw *bpcraw1 = new TBBPCRaw("BPC1");
  TBBPCRaw *bpcraw2 = new TBBPCRaw("BPC2");
  for(int i=0;i<6;i++){
//     bpcraw1->setSignal(i,i*m_tdcmax/6);
//     bpcraw2->setSignal(i,i*m_tdcmax/6+50);
  }

  bpcrawCont->push_back(bpcraw1);
  bpcrawCont->push_back(bpcraw2);

  sc = m_StoreGate->record(bpcrawCont,m_SGkeybpcraw);
  if ( sc.isFailure( ) ) {
    log << MSG::FATAL << "Cannot record BPCRawCont" << endreq;
  }


}

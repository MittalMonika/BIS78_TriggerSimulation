/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// TBCondRunParTool.cxx
// Richard Hawkings, started 2/7/04

#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "StoreGate/StoreGateSvc.h"
#include "EventInfo/EventID.h"
#include "EventInfo/EventInfo.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "TBCondRunPar/TBCondRunParTool.h"

// magic copied from TRT_IdentifierConversionTool
static const InterfaceID 
      IID_TBCONDRUNPAR_TBCondRunParTool("TBCondRunParTool",1,0);

TBCondRunParTool::TBCondRunParTool(const std::string& type,
         const std::string&name, const IInterface* parent)
  : AlgTool(type,name,parent), m_log(msgSvc(),name),
    p_storeGate(0),p_detstore(0),m_crun(-1),m_cevent(-1),m_runpfolder("")
{
  declareInterface<TBCondRunParTool>(this);
  declareProperty("RunParamFolder",m_runpfolder);
}

TBCondRunParTool::~TBCondRunParTool()
{}

const InterfaceID& TBCondRunParTool::interfaceID()
{ return IID_TBCONDRUNPAR_TBCondRunParTool; }

StatusCode TBCondRunParTool::initialize()
{
  m_log << MSG::INFO << "In initialize of TBCondRunParTool" << endreq;
  m_log << MSG::INFO << 
    "RunParams will be read from folder: " << m_runpfolder << 
    " in COOL database" << endreq;
  // check StoreGate service available
  StatusCode sc = service("StoreGateSvc", p_storeGate);
  if (sc.isFailure())
  {
      m_log << MSG::FATAL << "StoreGate service not found" << endreq;
      return StatusCode::FAILURE;
  }
  // get detector store
  if (StatusCode::SUCCESS!=service("DetectorStore",p_detstore)) {
    m_log << MSG::FATAL << "Detector store not found" << endreq; 
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode TBCondRunParTool::finalize() {
  m_log << MSG::DEBUG << "Finalize called" << endreq;
  return StatusCode::SUCCESS;
}

bool TBCondRunParTool::checkcache() {
  // find the current run and event - check if data structure may have changed
  int run=0,event=0;
  bool update=false;
  if (StatusCode::SUCCESS==p_storeGate->retrieve(m_eventinfo)) {
    run=m_eventinfo->event_ID()->run_number();
    event=m_eventinfo->event_ID()->event_number();
  } else {
    m_log << MSG::ERROR << "Could not retrieve run/event" << endreq;
  }
  if (run!=m_crun || event!=m_cevent) {
    m_log << MSG::DEBUG << "Retrieve new data for run/event " << run << "/"
	  << event << endreq;
    m_crun=run;
    m_cevent=event;
    // access the database and update cached values
    update=true;
    m_run_number=-1;
    m_trigger_type=-1;
    m_det_mask=-1;
    m_beam_type=-1;
    m_beam_energy=-1;
    m_file_tag="";

    update=extractCool(run);
  }
  return update;
}

StatusCode TBCondRunParTool::getTable(const std::string table,const int /*irow*/,
                const void* /*tblptr*/,
		int& ncol,int& nrow, std::vector<std::string>& names,
				std::vector<std::string>& rows) const {
  // this implementation is dummy - at present it does not return anything
  //tblptr=0;
  ncol=0;
  nrow=0;
  names.clear();
  rows.clear();
  m_log << MSG::DEBUG << "getTable: " << table << endreq;
  bool iret = extractCoolTest(std::string("/TILE/DCS/TILE_LV_62/BEAM"), 0);
  // get number of columns and rows
  if(!iret) 
     m_log << MSG::DEBUG << "extractCoolTest: /TILE/DCS/TILE_LV_62/BEAM failed !" << endreq;
  return StatusCode::SUCCESS;
}		 

StatusCode TBCondRunParTool::extractVal(
   const std::vector<std::string> names, const std::vector<std::string> rows,
   const std::string name,int& ival) const {
  StatusCode found=StatusCode::FAILURE;
  for (unsigned int icol=0;icol<names.size();++icol) {
      m_log << MSG::DEBUG << "Check " << icol << " " << names[icol]
	    << rows[icol] << endreq;
      if (names[icol]==name) {
        ival=atoi(rows[icol].c_str());
	found=StatusCode::SUCCESS;
	break;
      }
  }
  return found;
}

StatusCode TBCondRunParTool::extractVal(
   const std::vector<std::string> names, const std::vector<std::string> rows,
   const std::string name,float& fval) const {
  StatusCode found=StatusCode::FAILURE;
  for (unsigned int icol=0;icol<names.size();++icol) {
      m_log << MSG::DEBUG << "Check " << icol << " " << names[icol]
	    << rows[icol] << endreq;
      if (names[icol]==name) {
        fval=atof(rows[icol].c_str());
	found=StatusCode::SUCCESS;
	break;
      }
  }
  return found;
}

StatusCode TBCondRunParTool::extractVal(
   const std::vector<std::string> names, const std::vector<std::string> rows,
   const std::string name,std::string& sval) const {
  StatusCode found=StatusCode::FAILURE;
  for (unsigned int icol=0;icol<names.size();++icol) {
      m_log << MSG::DEBUG << "Check " << icol << " " << names[icol]
	    << rows[icol] << endreq;
      if (names[icol]==name) {
        sval=rows[icol];
	found=StatusCode::SUCCESS;
	break;
      }
  }
  return found;
}

void TBCondRunParTool::printTable(const std::vector<std::string> names,
				  const std::vector<std::string> rows) const {
  for (unsigned int icol=0;icol<names.size();++icol) 
    m_log << MSG::INFO << "Column: " << icol <<  " " << names[icol] 
	  << rows[icol] << endreq;
}

StatusCode TBCondRunParTool::getVal(const std::string folder, const unsigned int chnum, float& fval){
      StatusCode found=StatusCode::FAILURE;
      const CondAttrListCollection* atrlist=0;
      if (StatusCode::SUCCESS==p_detstore->retrieve(atrlist,folder)) {
         CondAttrListCollection::const_iterator it   = atrlist->begin();
         CondAttrListCollection::const_iterator last = atrlist->end();
         for (; it != last; ++it) {
                if(chnum == (*it).first) {
//                   std::cout << "chan, attr: " << (*it).first << " / "<<(*it).second.toOutputStream(std::cout)<< std::endl;
                   fval =  (*it).second[0].data<float>();
                   found = StatusCode::SUCCESS;
                   m_log << MSG::DEBUG << "chnum: "<<chnum<<" chan, attr: " << (*it).first <<" fval: "<<fval<<endreq;
                   break;
                }
         }
      }

      return found;

}



bool TBCondRunParTool::extractCool(const int run) {
  bool update=true;
  const AthenaAttributeList* atrlist=0;
  std::string table=m_runpfolder+"/RunParams.RunParams";
  if (StatusCode::SUCCESS==p_detstore->retrieve(atrlist,table)) {
    // std::ostringstream atrs;
    // atrlist->print(atrs);
    // m_log << "Attribute list contents: " << atrs.str() << endreq;
    long ldata;
    m_run_number=(*atrlist)["run_number"].data<int>();
    m_trigger_type=(*atrlist)["trigger_type"].data<int>();
    m_det_mask=(*atrlist)["detector_mask"].data<int>();
    m_beam_type=(*atrlist)["beam_type"].data<int>();
    m_beam_energy=(*atrlist)["beam_energy"].data<int>();
    m_file_tag=(*atrlist)["filename_tag"].data<std::string>();
    // FIXME should be long
    ldata=(*atrlist)["timeSOR"].data<int>();
    std::ostringstream timeSOR;
    timeSOR << ldata;
    m_time_SOR=timeSOR.str();
    m_log << MSG::DEBUG <<"run_number: "<<m_run_number<<" m_beam_energy: "<<m_beam_energy<<endreq;
    // check consistency
    m_status=0;
    if (m_run_number!=run) m_status=1;
  } else {
    m_log << MSG::ERROR << "Cannot retrieve AttributeList for " << table <<
      endreq;
    m_status=1;
  }
  return update;
}

bool TBCondRunParTool::extractCoolTest(const std::string folder, const int /*run*/) const{
  bool update=true;
  const CondAttrListCollection* atrlist=0;
  if (StatusCode::SUCCESS==p_detstore->retrieve(atrlist,folder)) {
//   std::ostringstream atrs;
   m_log << MSG::DEBUG << "Attribute list contents: " <<endreq;
   atrlist->dump();
  }
  return update;
}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


// System
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <unistd.h>

// C/C++
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

// ROOT
#include "TFile.h"
#include "TTree.h"

// Framework
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/FileIncident.h"

// Trigger
#include "TrigNavigation/Navigation.h"
#include "TrigInterfaces/AlgoConfig.h"

// Local
#include "TrigCostMonitor/TrigNtSaveTool.h"

//---------------------------------------------------------------------------------------
Trig::TrigNtSaveTool::TrigNtSaveTool(const std::string& type,
				     const std::string& name,
				     const IInterface* parent)
  :AlgTool(type, name, parent), 
   m_log(0),
   m_histSvc("THistSvc/THistSvc", name),
   m_incSvc("IncidentSvc", name),
   m_file(0),
   m_treeConfig(0),
   m_treeEvent(0),
   m_config(0),
   m_event(0),
   m_countConfig(0),
   m_countEvent(0),
   m_filenConfig(0),
   m_filenEvent(0)
{
  declareInterface<Trig::ITrigNtTool>(this);
  
  declareProperty("writeFile",        m_writeFile        = false);
  declareProperty("writeCostOnly",    m_writeCostOnly    = false);
  declareProperty("writeRateOnly",    m_writeRateOnly    = false);
  declareProperty("printPostSummary", m_printPostSummary = true);

  declareProperty("streamConfig",     m_streamConfig  = "");
  declareProperty("streamEvent",      m_streamEvent   = "");
  declareProperty("fileName",         m_fileName      = "");
  declareProperty("filePath",         m_filePath      = "");

  declareProperty("treeNameEvent",    m_treeNameEvent  = "event");
  declareProperty("treeNameConfig",   m_treeNameConfig = "config");

  declareProperty("branchBuffer",     m_branchBuffer  = 10000000);
  declareProperty("branchSplit",      m_branchSplit   = 0);
  declareProperty("fileCompr",        m_fileCompr     = 1);
}

//---------------------------------------------------------------------------------------
Trig::TrigNtSaveTool::~TrigNtSaveTool()
{
}

//---------------------------------------------------------------------------------------
StatusCode Trig::TrigNtSaveTool::initialize()
{      
  m_log = new MsgStream(msgSvc(), name());

  log() << MSG::INFO << "Configuring output tree(s)..."   << endreq
	<< "   writeFile        = " << m_writeFile        << endreq
	<< "   fileName         = " << m_fileName         << endreq
	<< "   filePath         = " << m_filePath         << endreq
	<< "   writeCostOnly    = " << m_writeCostOnly    << endreq
	<< "   writeRateOnly    = " << m_writeRateOnly    << endreq
	<< "   printPostSummary = " << m_printPostSummary << endreq
	<< "   streamConfig     = " << m_streamConfig     << endreq
	<< "   streamEvent      = " << m_streamEvent      << endreq
	<< "   treeNameEvent    = " << m_treeNameEvent    << endreq
	<< "   treeNameConfig   = " << m_treeNameConfig   << endreq;

  if(m_writeFile) {
    //
    // Write ROOT file diirectly (without HistSvc)
    // 
    if(m_fileName.empty()) {
      if(m_incSvc.retrieve().isFailure()) {
	log() << MSG::ERROR << "Failed to retrieve IncidentSvc" << endreq;
	return StatusCode::FAILURE;
      }
      else {
	log() << MSG::DEBUG << "Retrieved incident service: " << m_incSvc << endreq;
      }
      
      m_incSvc->addListener(this, "BeginInputFile");
      m_incSvc->addListener(this, "EndInputFile");
    }
  }
  else {
    //
    // Save TTrees via THistSvc
    //
    if(m_histSvc.retrieve().isFailure()) {
      log() << MSG::ERROR << "Failed to retrieve THistSvc" << endreq;
      return StatusCode::FAILURE;
    }

    if(!m_treeConfig && !m_treeNameConfig.empty()) { 
      m_treeConfig = MakeTree(m_treeNameConfig, m_streamConfig);
    }

    if(!m_treeEvent  && !m_treeNameEvent.empty())  {
      m_treeEvent  = MakeTree(m_treeNameEvent,  m_streamEvent);
    }

    if(!m_config) m_config = new TrigMonConfig();
    if(!m_event)  m_event  = new TrigMonEvent();
    
    if(m_treeConfig) m_treeConfig->Branch("conf", "TrigMonConfig", &m_config, m_branchBuffer, m_branchSplit);
    if(m_treeEvent)  m_treeEvent ->Branch("data", "TrigMonEvent",  &m_event,  m_branchBuffer, m_branchSplit);
  }

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------------------
StatusCode Trig::TrigNtSaveTool::finalize()
{
  //
  // Fill one last TrigMonConfig
  //
  log() << MSG::INFO << "finalize()" << endreq;

  //
  // Clean up
  //
  if(m_printPostSummary) {
    log() << MSG::INFO
	  << name() << " POST_COST: # TrigMonConfig = " << m_countConfig << endreq
	  << name() << " POST_COST: # TrigMonEvent  = " << m_countEvent  << endreq;
  }

  //
  // Close ROOT file is we have opened it directly
  //
  Close();  

  if(m_config) { delete m_config; m_config = 0; }
  if(m_event)  { delete m_event;  m_event  = 0; }

  delete m_log; m_log = 0;

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
void Trig::TrigNtSaveTool::handle(const Incident& inc)
{
  if(outputLevel() <= MSG::DEBUG) {
    log() << MSG::DEBUG << "Incident source=" << inc.source() << " type=" << inc.type() << endreq;
  }

  if(!m_writeFile || !m_fileName.empty()) {
    log() << MSG::DEBUG << "Nothing in handle() - output method does not use files" << endreq;
    return;
  }

  const FileIncident *finc = dynamic_cast<const FileIncident *>(&inc);
  if(!finc) {
    log() << MSG::INFO << "Unabled to cast Incident to FileIncident" << endreq;
    return;
  }

  log() << MSG::INFO << "Fired incident: " << inc.source() << ", " << inc.type() << endreq
	<< "   file name: " << finc->fileName() << endreq;

  if(inc.type() == "BeginInputFile") {
    std::string fileName = finc->fileName();
    const std::size_t spos = fileName.find_last_of("/");

    if(spos != std::string::npos && spos+1 < fileName.size()) {
      fileName = fileName.substr(spos+1, fileName.size()-spos-1);
    }
    
    for(unsigned i = 0; i < fileName.size(); ++i) {
      if(fileName[i] == ':') fileName[i] = '_';
    }       
    
    //
    // Make output file
    //
    struct timeval currentTime; currentTime.tv_sec = 0; currentTime.tv_usec = 0;
    gettimeofday(&currentTime, 0);

    unsigned year = 0, mon = 0, mday = 0, hour = 0;
    time_t timer(currentTime.tv_sec);
    const struct tm* mytm = gmtime(&timer);
    
    if(mytm) { 
      year = mytm->tm_year % 100;
      mon  = mytm->tm_mon+1;
      mday = mytm->tm_mday;
      hour = mytm->tm_hour;
    }
    
    //
    // Make output file
    //
    std::stringstream file;
    file << "cost" 
	 << "." << std::setw(2) << std::setfill('0') << year
	 << "-" << std::setw(2) << std::setfill('0') << mon
	 << "-" << std::setw(2) << std::setfill('0') << mday
	 << "-" << std::setw(2) << std::setfill('0') << hour
	 << "." << fileName
	 << ".root";
    
    if(!TrigNtSaveTool::Open(file.str())) {
      log() << MSG::WARNING << "Failed to open file: " << file.str() << endreq;
    }
  }

  if(inc.type() == "EndInputFile") {
    TrigNtSaveTool::Close();
  }
}

//---------------------------------------------------------------------------------------
bool Trig::TrigNtSaveTool::Fill(TrigMonConfig *config)
{
  //
  // Process val trigger configuration event
  //
  if(!config) return false;
  
  //
  // Open ROOT file on a first call
  //
  Open(m_fileName);    

  //
  // Save configuration if config tree exists
  //
  if(m_treeConfig) {
    (*m_config) = (*config);
    m_treeConfig->Fill();
    
    ++m_filenConfig;
    ++m_countConfig;    
  }
  else {
    if(outputLevel() <= MSG::DEBUG) {
      log() << MSG::DEBUG << "Fill - TrigMonConfig tree is NULL pointer" << endreq;
    }
  }

  return true;
}

//---------------------------------------------------------------------------------------
bool Trig::TrigNtSaveTool::Fill(TrigMonEvent &event)
{
  //
  // Open ROOT file on a first call
  //
  Open(m_fileName);  

  //
  // Save event data to event tree
  //
  if(!m_treeEvent) {
    if(outputLevel() <= MSG::DEBUG) {
      log() << MSG::DEBUG << "Fill - TrigMonEvent tree is NULL pointer" << endreq;
    }
    return true;
  }

  //
  // Copy event data into event branch object
  //
  (*m_event) = event;   
  
  if(m_writeCostOnly) {
    //
    // Write event only if it has cost data
    //
    if(!m_event->getVec<TrigMonSeq>().empty()) {
      //
      // Fill tree
      //
      m_treeEvent->Fill();
      
      ++m_filenEvent;
      ++m_countEvent;
    }
  }
  else {
    
    if(m_writeRateOnly) {
      //
      // Erase all information except for rates
      //
      m_event->getVec<TrigMonSeq>().clear();
      m_event->getVec<TrigMonRoi>().clear();
      m_event->getVec<TrigMonROB>().clear();
      m_event->getVec<TrigMonTE> ().clear();
    }
    
    //
    // Fill tree
    //
    m_treeEvent->Fill();
    
    ++m_filenEvent;
    ++m_countEvent;
  }
  
  return true;
}

//---------------------------------------------------------------------------------------
bool Trig::TrigNtSaveTool::Open(const std::string &file)
{      
  //
  // Initialize new ROOT files
  //
  if(!m_writeFile) { 
    log() << MSG::DEBUG << "Using HistSvc for output - nothing to do in Open()" << endreq;
    return false;
  }

  //
  // Write data directly to ROOT file on local filesystem
  //    
  if(file.empty()) { 
    log() << MSG::WARNING << "ROOT file name is empty" << endreq;
    return false;
  }
  if(m_file) { 
    log() << MSG::DEBUG << "ROOT file already exists: " << m_file->GetName() << endreq;
    return false;
  }

  std::string path = m_filePath;  
  if(!m_filePath.empty() && m_filePath[m_filePath.size()-1] != '/') {
    path = path + "/";
  }
  
  path = path + file;
  
  TDirectory *dir_cur = gDirectory;
  m_file = new TFile(path.c_str(), "RECREATE", "trigger cost", m_fileCompr);
  
  if(!m_file || !m_file->IsOpen()) {
    log() << MSG::ERROR << "Failed to open ROOT file: " << path << endreq;
    gDirectory = dir_cur;
    return false;
  }
  
  log() << MSG::INFO << "Opened COST_FILE: " << path << endreq;
  
  m_filenConfig = 0;
  m_filenEvent  = 0;

  if(!m_config) m_config = new TrigMonConfig();
  if(!m_event)  m_event  = new TrigMonEvent();

  if(!m_treeNameConfig.empty()) {
    m_treeConfig = new TTree(m_treeNameConfig.c_str(), m_treeNameConfig.c_str());
    m_treeConfig -> SetDirectory(m_file);
    m_treeConfig->Branch("conf", "TrigMonConfig", &m_config, m_branchBuffer, m_branchSplit);  
  }

  if(!m_treeNameEvent.empty()) {
    m_treeEvent  = new TTree(m_treeNameEvent.c_str(),  m_treeNameEvent.c_str());
    m_treeEvent  -> SetDirectory(m_file);   
    m_treeEvent ->Branch("data", "TrigMonEvent",  &m_event,  m_branchBuffer, m_branchSplit);
  }
  
  gDirectory = dir_cur;

  return true;
}

//---------------------------------------------------------------------------------------
void Trig::TrigNtSaveTool::Close()
{      
  //
  // Write files
  //
  if(m_file) {
    const std::string filename = m_file->GetName();
    TDirectory *dir_cur = gDirectory;
    m_file -> Write();
    m_file -> Close();
    gDirectory = dir_cur;

    log() << MSG::INFO 
	  << "Closed COST_FILE: " << filename << endreq
	  << filename << " # TrigMonConfig = " << m_filenConfig << endreq
	  << filename << " # TrigMonEvent  = " << m_filenEvent  << endreq;

    m_file        = 0;
    m_treeConfig  = 0;
    m_treeEvent   = 0;

    m_filenConfig = 0;
    m_filenEvent  = 0;
  }
}

//---------------------------------------------------------------------------------------
TTree* Trig::TrigNtSaveTool::MakeTree(const std::string &tree_name, 
				      const std::string &stream_name)
{
  //
  // Make and register TTree via THistSvc
  //
  if(tree_name.empty() || stream_name.empty()) { 
    return 0;
  }

  const std::string tree_key = "/"+stream_name+"/"+tree_name;  
  TTree *tree = 0;

  if(m_histSvc->exists(tree_key) && m_histSvc->getTree(tree_key, tree).isSuccess()) {
    log() << MSG::INFO << "Retrieved TTree from HistSvc: " << tree_key << endreq;    
    return tree;
  }

  tree = new TTree(tree_name.c_str(), tree_name.c_str());
  tree -> SetDirectory(0);
  
  if(m_histSvc -> regTree(tree_key, tree).isFailure()) {
    log() << MSG::WARNING << "Could not register TTree " << tree_name << endreq;
    delete tree; tree = 0;
  }
  else {
    log() << MSG::INFO << "Registered new TTree " << tree_key << endreq;
  }

  return tree;
}

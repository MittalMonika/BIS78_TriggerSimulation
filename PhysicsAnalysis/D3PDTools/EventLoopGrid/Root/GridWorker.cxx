/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#include "EventLoopGrid/GridWorker.h"

#include <memory>
#include <iostream>
#include <exception>

#include <TFile.h>
#include <TTree.h>
#include <TCollection.h>

#include "RootCoreUtils/Assert.h"
#include "RootCoreUtils/ThrowMsg.h"
#include "SampleHandler/Sample.h"
#include "EventLoop/Algorithm.h"
#include "EventLoop/EventRange.h"
#include "EventLoop/Job.h"
#include "EventLoop/MessageCheck.h"
#include "EventLoop/OutputStream.h"
#include "EventLoop/GridReportingModule.h"

#include <iostream>
#include <string>
#include <exception>

#include <TROOT.h>
#include <TList.h>
#include <TFile.h>
#include <TObject.h>

#include "RootCoreUtils/Assert.h"
#include "RootCoreUtils/ThrowMsg.h"
#include "SampleHandler/SampleGrid.h"
#include "SampleHandler/MetaObject.h"
#include "SampleHandler/SampleHandler.h"
#include "EventLoop/Algorithm.h"
#include "EventLoop/JobConfig.h"
#include "EventLoopGrid/GridWorker.h"
#include "EventLoop/Driver.h"
#include "EventLoop/OutputStream.h"

ClassImp(EL::GridWorker)

namespace EL
{

  void GridWorker::testInvariant() const {
    RCU_INVARIANT (this != 0);
  }

  GridWorker::GridWorker ()
  {

    RCU_NEW_INVARIANT (this);
  }


  void GridWorker::run(JobConfig&& jobConfig, const TList& bigOutputs, const std::string& location) {
    using namespace msgEventLoop;
    RCU_CHANGE_INVARIANT (this);

    {//Create and register the "big" output files with base class
      TIter itr(&bigOutputs);
      TObject *obj = 0;
      while ((obj = itr())) {
	EL::OutputStream *os = dynamic_cast<EL::OutputStream*>(obj);
	if (os) {
          Detail::OutputStreamData data {
            location + "/" + os->label() + ".root", "RECREATE"};
          ANA_CHECK_THROW (addOutputStream (os->label(), std::move (data)));
	}
	else {
	  throw "ERROR: Bad input"; 
	}
      }
    }

    setJobConfig (std::move (jobConfig));

    addModule (std::make_unique<Detail::GridReportingModule> ());
    ANA_CHECK_THROW (initialize());

    std::vector<std::string> fileList; 

    std::ifstream infile("input.txt");
    while (infile) {
      std::string sLine;
      if (!getline(infile, sLine)) break;
      std::istringstream ssLine(sLine);
      while (ssLine) {
        std::string sFile;
        if (!getline(ssLine, sFile, ',')) break;
        fileList.push_back(sFile);
      }
    } 
  
    if (fileList.size() == 0) {
      //not fatal, maybe input was not expected
    }

    if (fileList.size() == 0) {
      //User was expecting input after all.
      gSystem->Exit(EC_BADINPUT);
    }

    for (std::size_t currentFile = 0;
	 currentFile != fileList.size();
	 currentFile++) {

      EventRange eventRange;
      eventRange.m_url = fileList.at(currentFile);

      try {
        if (processEvents (eventRange).isFailure()) {
          Fail(eventsProcessed(), currentFile, fileList.at(currentFile));
        }

      }
      catch (std::exception& e) {
        std::cout << "Caught exception while executing algorithm:\n"
                  << e.what() << "\n";
        Fail(eventsProcessed(), currentFile, fileList.at(currentFile));
      }
      catch (char const* e) {
        std::cout << "Caught exception while executing algorithm:\n"
                  << e << "\n";
        Fail(eventsProcessed(), currentFile, fileList.at(currentFile));
      }
      catch (...) {
        std::cout << "Caught unknown exception while executing algorithm";
        Fail(eventsProcessed(), currentFile, fileList.at(currentFile));
      }
    }    

    ANA_CHECK_THROW (finalize ());

    int nEvents = eventsProcessed();
    int nFiles = fileList.size();
    std::cout << "\nLoop finished. ";
    std::cout << "Read " << nEvents << " events in " << nFiles << " files.\n";

    NotifyJobFinished(eventsProcessed(), fileList);
  }



  void GridWorker ::
  execute (const std::string& sampleName)
  {
    using namespace msgEventLoop;

    ANA_MSG_INFO ("Running with ROOT version " << gROOT->GetVersion()
                  << " (" << gROOT->GetVersionDate() << ")");

    ANA_MSG_INFO ("Loading EventLoop grid job");


    TList bigOutputs;
    std::unique_ptr<JobConfig> jobConfig;
    SH::MetaObject *mo = 0;

    try {
      std::unique_ptr<TFile> f (TFile::Open("jobdef.root"));
      if (f && !f->IsZombie()) {
	
	mo = dynamic_cast<SH::MetaObject*>(f->Get(sampleName.c_str()));
	if (!mo) {
	  throw "Could not read in sample meta object";
	}
	
        jobConfig.reset (dynamic_cast<JobConfig*>(f->Get("jobConfig")));
        if (jobConfig == nullptr)
          RCU_THROW_MSG ("failed to read jobConfig object");

        std::unique_ptr<TList> outs ((TList*)f->Get("outputs"));
	if (outs) {  
	  TIter itr(outs.get());
	  TObject *obj = 0;
	  while ((obj = itr())) {
	    EL::OutputStream * out = dynamic_cast<EL::OutputStream*>(obj);
	    if (out) {
	      bigOutputs.Add(out);
	    }
	    else {
	      throw "Encountered unexpected entry in list of outputs"; 
	    }
	  }
	}
	else {
	  throw "Could not read list of outputs"; 
	}
      }
      else {
	throw "Could not read jobdef"; 
      }

      f->Close();
      f.reset ();
    
      const std::string location = ".";

      EL::GridWorker worker;
      worker.setMetaData (mo);
      worker.setOutputHist (location);
      worker.setSegmentName ("output");
      
      ANA_MSG_INFO ("Starting EventLoop Grid worker");

      worker.run (std::move (*jobConfig), bigOutputs, location);

      ANA_MSG_INFO ("EventLoop Grid worker finished");
      ANA_MSG_INFO ("Saving output");
    } catch (...)
    {
      Detail::report_exception ();
      ANA_MSG_ERROR ("Aborting job due to internal GridWorker error");
      ANA_MSG_ERROR ("The cause of this is probably a misconfigured job");
      gSystem->Exit(EC_ABORT);
    }
  }

  void GridWorker::NotifyJobFinished(uint64_t eventsProcessed,
                                     const std::vector<std::string>& fileList) {
    // createJobSummary
    std::ofstream summaryfile("../AthSummary.txt");
    if (summaryfile.is_open()) {
      unsigned int nFiles = fileList.size();
      summaryfile << "Files read: " << nFiles << std::endl;
      for (unsigned int i = 0; i < nFiles; i++) {
        summaryfile << "  " << fileList.at(i) << std::endl;
      }      
      summaryfile << "Events Read:    " << eventsProcessed << std::endl;
      summaryfile.close();
    }
    else {
      //cout << "Failed to write summary file.\n";
    } 
  }

  void GridWorker::Fail(uint64_t eventsProcessed, std::size_t currentFile,
                        const std::string& fileName) {
    try {
      std::cerr << "Error reported at event " << eventsProcessed
                << " in file " << currentFile << " (" 
                << fileName << ")\n"
                << "ending job with status \"failed\"\n";
    } catch (...) {}
    gSystem->Exit(EC_FAIL);
  }
}

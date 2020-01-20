/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//          
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (krumnack@iastate.edu) for bug
// reports, feature suggestions, praise and complaints.


//
// includes
//

#include <EventLoop/TEventSvc.h>

#include <memory>
#include <xAODRootAccess/TEvent.h>
#include <xAODRootAccess/tools/TFileAccessTracer.h>
#include <xAODRootAccess/TStore.h>
// #include <xAODRootAccess/D3PDPerfStats.h>
#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/ThrowMsg.h>
#include <SampleHandler/MetaObject.h>
#include <xAODCore/tools/ReadStats.h>
#include <xAODCore/tools/PerfStats.h>
#include <xAODCore/tools/IOStats.h>
#include <AsgTools/MessageCheck.h>

//
// method implementations
//

ClassImp (EL::TEventSvc)

namespace EL
{
  const std::string TEventSvc::name = "TEventSvc";



  void TEventSvc ::
  testInvariant () const
  {
    RCU_INVARIANT (this != 0);
  }



  TEventSvc ::
  TEventSvc ()
  {
    RCU_NEW_INVARIANT (this);
  }



  TEventSvc ::
  ~TEventSvc ()
  {
    RCU_DESTROY_INVARIANT (this);
  }



  xAOD::TEvent *TEventSvc ::
  event () const
  {
    RCU_DESTROY_INVARIANT (this);

    if (m_event == 0)
      RCU_THROW_MSG ("TEventSvc not yet configured");
    xAOD::TEvent *const result = m_event.get();
    RCU_PROVIDE (result != 0);
    return result;
  }



  xAOD::TStore *TEventSvc ::
  store () const
  {
    RCU_DESTROY_INVARIANT (this);

    if (m_store == 0)
      RCU_THROW_MSG ("TEventSvc not yet configured");
    xAOD::TStore *const result = m_store.get();
    RCU_PROVIDE (result != 0);
    return result;
  }



  const char *TEventSvc ::
  GetName() const
  {
    RCU_READ_INVARIANT (this);
    return name.c_str();
  }



  StatusCode TEventSvc ::
  fileExecute ()
  {
    RCU_READ_INVARIANT (this);
    RCU_ASSERT (m_event != nullptr);
    RCU_ASSERT (m_store != nullptr);
    ANA_CHECK (m_event->readFrom (wk()->inputFile()));
    return StatusCode::SUCCESS;
  }



  StatusCode TEventSvc ::
  histInitialize ()
  {
    ANA_CHECK_SET_TYPE (EL::StatusCode);

    RCU_READ_INVARIANT (this);
    RCU_ASSERT (m_event == nullptr);

    std::string modeStr = wk()->metaData()->castString
      (Job::optXaodAccessMode);
    if (!modeStr.empty())
    {
      xAOD::TEvent::EAuxMode mode = xAOD::TEvent::kClassAccess; //compiler dummy
      if (modeStr == Job::optXaodAccessMode_class)
        mode = xAOD::TEvent::kClassAccess;
      else if (modeStr == Job::optXaodAccessMode_branch)
        mode = xAOD::TEvent::kBranchAccess;
      else if (modeStr == Job::optXaodAccessMode_athena)
        mode = xAOD::TEvent::kAthenaAccess;
      else
      {
        ANA_MSG_ERROR ("unknown XAOD access mode: " << modeStr);
        return StatusCode::FAILURE;
      }
      m_event.reset (new xAOD::TEvent (mode));
    } else
    {
      m_event.reset (new xAOD::TEvent);
    }
    if (wk()->metaData()->castDouble (Job::optXAODSummaryReport, 1) == 0)
      xAOD::TFileAccessTracer::enableDataSubmission (false);

    RCU_ASSERT (m_store == nullptr);
    m_store.reset (new xAOD::TStore);

    ANA_CHECK (m_event->readFrom (wk()->inputFile()));
    return StatusCode::SUCCESS;
  }



  StatusCode TEventSvc ::
  changeInput (bool firstFile)
  {
    RCU_CHANGE_INVARIANT (this);

    if (firstFile)
    {
      m_useStats = wk()->metaData()->castBool (Job::optXAODPerfStats, false);
      if (m_useStats)
        xAOD::PerfStats::instance().start();
    }

    if (fileExecute() != StatusCode::SUCCESS)
      return StatusCode::FAILURE;
    if (m_event->getEntry (wk()->treeEntry()) < 0)
      RCU_THROW_MSG ("failed to read from xAOD");
    return StatusCode::SUCCESS;
  }



  StatusCode TEventSvc ::
  execute ()
  {
    RCU_CHANGE_INVARIANT (this);

    m_store->clear ();
    if (m_event->getEntry (wk()->treeEntry()) < 0)
      RCU_THROW_MSG ("failed to read from xAOD");
    return StatusCode::SUCCESS;
  }



  EL::StatusCode TEventSvc ::
  finalize ()
  {
    if (m_useStats)
    {
      xAOD::PerfStats::instance().stop();
      std::auto_ptr<xAOD::ReadStats> stats
	(new xAOD::ReadStats (xAOD::IOStats::instance().stats()));
      stats->SetName (Job::optXAODReadStats.c_str());
      stats->Print ();
      wk()->addOutput (stats.release());
    }
    return StatusCode::SUCCESS;
  }
}

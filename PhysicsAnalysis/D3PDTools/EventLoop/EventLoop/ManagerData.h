/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__MANAGER_DATA_H
#define EVENT_LOOP__MANAGER_DATA_H

#include <EventLoop/Global.h>

#include <EventLoop/ManagerStep.h>
#include <SampleHandler/MetaObject.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

class StatusCode;

namespace EL
{
  namespace Detail
  {
    /// \brief an internal data structure for passing data between
    /// different job routines
    ///
    /// \warn This is an **internal** data structure that users should
    /// not directly interface with.  It is not guaranteed to stay
    /// around and can change or even disappear from one EventLoop
    /// version to the next.
    ///
    /// The main reason for this design is that I have a fair amount
    /// of information that I need to prepare a job for
    /// submission/execution, and the process involves a number of
    /// steps.  By centralizing all that information I can more easily
    /// restructure the submission process and factor out utility
    /// functions.

    struct ManagerData final
    {
      /// \brief standard constructor
      ///
      /// This is mostly defined, so that this include file doesn't
      /// need to include a whole bunch of other files, just to
      /// destruct their objects
      ManagerData () noexcept;

      /// \brief standard destructor
      ///
      /// This is mostly defined, so that this include file doesn't
      /// need to include a whole bunch of other files, just to
      /// destruct their objects
      ~ManagerData () noexcept;


      /// \brief the driver we are working on/with
      const Driver *driver {nullptr};

      /// \brief the path to the submission directory
      std::string submitDir;

      /// \brief whether we are merely resubmitting this job
      bool resubmit {false};

      /// \brief if we are resubmitting, the resubmit option chosen
      std::string resubmitOption;

      /// \brief the options with which we submit this job
      SH::MetaObject options;

      /// \brief the user job we are submitting
      Job *job {nullptr};

      /// \brief whether we successfully submitted the job in this
      /// process
      bool submitted {false};

      /// \brief whether we successfully retrieved the job in this
      /// process
      bool retrieved {false};

      /// \brief whether the job completed
      bool completed {false};

      /// \brief if we run in batch, the batch job object we are using
      std::unique_ptr<BatchJob> batchJob;

      /// \brief if we run in batch, this is the list of job-indices
      /// to run
      std::vector<std::size_t> batchJobIndices;


      /// \brief the name of the submission script to use.  if this
      /// contains {JOBID} it will create one script for each job id
      ///
      /// Some batch systems are picky about names.  Others don't
      /// allow passing arguments into submission scripts.  So this
      /// allows being flexible.
    public:
      std::string batchName;

      /// \brief any additional code we need for setting up the batch
      /// job.  if multiple files are used, {JOBID} will be replaced
      /// with the index of the current file.
      ///
      /// Some batch systems need extra lines when making there
      /// submission scripts, which can either be specially formatted
      /// option lines or just some special commands.
    public:
      std::string batchInit;

      /// \brief the code needed for setting EL_JOBID
      ///
      /// Normally one can just pass the index of the job as the first
      /// argument to the execution script, but some systems instead
      /// use environment variables rationale.
      ///
      /// This is not used if we have separate execution scripts from
      /// the job.
    public:
      std::string batchJobId;

      /// \brief whether to skip the release setup
      ///
      /// Some batch drivers will setup the release in their own way,
      /// or will not need a release setup.  Setting this to true will
      /// skip the standard release setup.
      ///
      /// Besides avoiding to create the release setup commands, this
      /// will also suppress errors when the cmake-work-directory can
      /// not be located.
    public:
      bool batchSkipReleaseSetup {false};

      /// \brief the directory location for writing batch output
    public:
      std::string batchWriteLocation;

      /// \brief the directory location with batch input configuration
    public:
      std::string batchSubmitLocation;

      /// \brief whether we have/use a shared file system
    public:
      bool sharedFileSystem {true};


      /// \brief the current \ref ManagerStep
      ManagerStep step {ManagerStep::initial};

      /// \brief the next \ref ManagerStep
      ///
      /// By having this part of the manager data I am able to adjust
      /// the flow order via the managers (if desired)
      ManagerStep nextStep {ManagerStep::initial};


      /// \brief the list of managers to run
      std::map<ManagerOrder,std::unique_ptr<Manager> > managers;

      /// \brief add the given manager
      /// \par Guarantee
      ///   strong
      /// \par Failures
      ///   duplicate order entry\n
      ///   out of memory I
      void addManager (std::unique_ptr<Manager> manager);

      /// \brief run all the managers from start to finish
      /// \par Guarantee
      ///   basic
      /// \par Failures
      ///   manager errors
      ::StatusCode run ();
    };
  }
}

#endif

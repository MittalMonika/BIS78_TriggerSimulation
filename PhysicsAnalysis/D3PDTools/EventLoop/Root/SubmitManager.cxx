/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <EventLoop/SubmitManager.h>

#include <AsgTools/StatusCode.h>
#include <EventLoop/Driver.h>
#include <EventLoop/Job.h>
#include <EventLoop/ManagerData.h>
#include <EventLoop/ManagerOrder.h>
#include <EventLoop/MessageCheck.h>
#include <EventLoop/MetricsSvc.h>
#include <RootCoreUtils/Assert.h>
#include <SampleHandler/Sample.h>
#include <SampleHandler/SampleHist.h>
#include <TFile.h>
#include <TSystem.h>
#include <fstream>

using namespace EL::msgEventLoop;

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    Detail::ManagerOrder SubmitManager ::
    getManagerOrder () const noexcept
    {
      return ManagerOrder::OPERATION;
    }



    ::StatusCode SubmitManager ::
    doManagerStep (Detail::ManagerData& data) const
    {
      switch (data.step)
      {
      case Detail::ManagerStep::fillOptions:
        {
          data.options = *data.job->options();
          data.options.fetchDefaults (*data.driver->options());
        }
        break;

      case Detail::ManagerStep::addSystemAlgs:
        {
          if (data.options.castBool (Job::optDisableMetrics, false))
            if (!data.job->algsHas (MetricsSvc::name))
              data.job->algsAdd (new MetricsSvc);
        }
        break;

      case Detail::ManagerStep::createSubmitDir:
        {
          if (data.options.castBool (Job::optRemoveSubmitDir, false))
            gSystem->Exec (("rm -rf " + data.submitDir).c_str());
          if (gSystem->MakeDirectory (data.submitDir.c_str()) != 0)
          {
            ANA_MSG_ERROR ("could not create output directory " + data.submitDir);
            return ::StatusCode::FAILURE;
          }
        }
        break;

      case Detail::ManagerStep::prepareSubmitDir:
        {
          {
            std::unique_ptr<TFile> file (TFile::Open ((data.submitDir + "/driver.root").c_str(), "RECREATE"));
            file->WriteObject (data.driver, "driver");
            file->Close ();
          }
          data.job->sampleHandler().save (data.submitDir + "/input");
          {
            std::ofstream file ((data.submitDir + "/location").c_str());
            file << data.submitDir << "\n";
          }

          SH::SampleHandler sh_hist;
          for (SH::SampleHandler::iterator sample = data.job->sampleHandler().begin(),
                 end = data.job->sampleHandler().end(); sample != end; ++ sample)
          {
            const std::string histfile
              = data.submitDir + "/hist-" + (*sample)->name() + ".root";
            std::unique_ptr<SH::SampleHist> hist
              (new SH::SampleHist ((*sample)->name(), histfile));
            hist->meta()->fetch (*(*sample)->meta());
            sh_hist.add (hist.release());
          }
          sh_hist.save (data.submitDir + "/hist");
        }
        break;

      case Detail::ManagerStep::submitJob:
        {
          ANA_MSG_INFO ("submitting job in " << data.submitDir);
        }
        break;

      case Detail::ManagerStep::postSubmit:
        {
          if (!data.submitted)
          {
            ANA_MSG_FATAL ("Driver::submit not implemented in class " << typeid(*data.driver).name());
            std::abort ();
          }

          ANA_MSG_DEBUG ("make submitted");
          // this particular file can be checked to see if a job has
          // been submitted successfully.
          std::ofstream ((data.submitDir + "/submitted").c_str());
        }
        break;

      default:
        (void) true; // safe to do nothing
      }
      return ::StatusCode::SUCCESS;
    }
  }
}

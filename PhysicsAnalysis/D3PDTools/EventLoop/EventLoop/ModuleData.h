/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EVENT_LOOP__MODULE_DATA_H
#define EVENT_LOOP__MODULE_DATA_H

#include <EventLoop/Global.h>

#include <EventLoop/AlgorithmData.h>
#include <SampleHandler/Global.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class TFile;
class TList;
class TObject;
class TTree;

namespace EL
{
  namespace Detail
  {
    /// \brief the data the EventLoop core classes are sharing with
    /// the \ref Module implementation
    ///
    /// This class serves as a common argument to all Module methods,
    /// thereby allowing to share a lot of information with all
    /// modules without having to list of it explicitly in the module
    /// signature.
    ///
    /// This is purposely implemented as a simple `struct` without any
    /// accessor function, let alone a full whiteboard implementation;
    /// just because anything else feels like overkill right now.
    /// Right now (18 Jan 19) I only anticipate a limited number of
    /// module implementations, all written by EventLoop developers,
    /// meaning such abstractions and protections buy little, but cost
    /// a lot.  Should the module infrastructure ever be opened up to
    /// the user this would have to be changed.
    ///
    /// \warn Just like the \ref Module class this interface may
    /// change completely and without warning from one release to the
    /// next.

    struct ModuleData
    {
      /// \brief the list of algorithms
      std::vector<Detail::AlgorithmData> m_algs; //!

      /// \brief the input file url of the currently opened file
      std::string m_inputFileUrl;

      /// \brief the input file pointer of the currently opened filed
      std::unique_ptr<TFile> m_inputFile;

      /// \brief the (main) tree in the input file
      TTree *m_inputTree {nullptr};

      /// \brief the meta-data we use
      const SH::MetaObject *m_metaData {nullptr};

      /// \brief the number of events that have been processed
      uint64_t m_eventsProcessed {0};

      /// \brief the histogram output list
      TList *m_output {nullptr};

      /// \brief Tree saving per-job statistics information
      std::unique_ptr<TTree> m_jobStats;
    };
  }
}

#endif

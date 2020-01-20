#ifndef EVENT_LOOP_COMPS__I_FILTER_WORKER_H
#define EVENT_LOOP_COMPS__I_FILTER_WORKER_H

//        Copyright Iowa State University 2017.
//                  Author: Nils Krumnack
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (nils.erik.krumnack@cern.ch) for bug
// reports, feature suggestions, praise and complaints.


#include <EventLoopComps/Global.h>

namespace EL
{
  /// \brief the interface to the filter functions in the algorithm
  /// sequence
  ///
  /// This interface summarizes the subset of the \ref Worker
  /// interface that relates to setting the filter status.

  class IFilterWorker
  {
    /// \brief standard (virtual) destructor
    /// \par Guarantee
    ///   no-fail
  public:
    virtual ~IFilterWorker () noexcept = default;


    /// \brief whether the current algorithm passed its filter
    /// criterion for the current event
    /// \par Guarantee
    ///   no-fail
  public:
    virtual bool filterPassed () const noexcept = 0;

    /// \brief set the value of \ref filterPassed
    /// \par Guarantee
    ///   no-fail
  public:
    virtual void setFilterPassed (bool val_filterPassed) noexcept = 0;
  };
}

#endif

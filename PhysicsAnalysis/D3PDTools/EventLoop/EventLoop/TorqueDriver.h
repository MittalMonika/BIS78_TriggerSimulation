/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENT_LOOP_TORQUE_DRIVER_HH
#define EVENT_LOOP_TORQUE_DRIVER_HH

//          
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (krumnack@iastate.edu) for bug
// reports, feature suggestions, praise and complaints.


/// This module defines a driver class for running on torque batch
/// systems.  The interface provided in this class is intended for
/// experts only.  The module is considered to be in the pre-alpha
/// stage.



#include <EventLoop/Global.h>

#include <EventLoop/BatchDriver.h>
#include <SampleHandler/Global.h>

namespace EL
{
  class TorqueDriver final : public BatchDriver
  {
    //
    // public interface
    //

    /// effects: test the invariant of this object
    /// guarantee: no-fail
  public:
    void testInvariant () const;


    /// effects: standard default constructor
    /// guarantee: strong
    /// failures: low level errors I
  public:
    TorqueDriver ();



    //
    // interface inherited from BatchDriver
    //

    /// returns: the code needed for setting EL_JOBID
    /// guarantee: strong
    /// failures: out of memory II
    /// rationale: normally one can just pass the index of the job as
    ///   the first argument to the execution script, but some systems
    ///   instead use environment variables
    /// rationale: this is not used if we have separate execution
    ///   scripts from the job
  private:
    virtual std::string batchJobId () const override;


    /// effects: perform the actual torque submission with njob jobs
    /// guarantee: strong
    /// failures: submission errors
    /// rationale: the virtual part of batch submission
  private:
    virtual void
    batchSubmit (Detail::ManagerData& data) const override;



    //
    // private interface
    //

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Winconsistent-missing-override"
    ClassDef(TorqueDriver, 1);
#pragma GCC diagnostic pop
  };
}

#endif

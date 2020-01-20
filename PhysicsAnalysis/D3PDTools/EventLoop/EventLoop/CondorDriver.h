/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENT_LOOP_CONDOR_DRIVER_HH
#define EVENT_LOOP_CONDOR_DRIVER_HH

//          
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (krumnack@iastate.edu) for bug
// reports, feature suggestions, praise and complaints.


/// This module defines a driver class for running on condor batch
/// systems.  The interface provided in this class is intended for
/// experts only.  The module is considered to be in the pre-alpha
/// stage.



#include <EventLoop/Global.h>

#include <EventLoop/BatchDriver.h>
#include <SampleHandler/Global.h>

namespace EL
{
  class CondorDriver final : public BatchDriver
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
    CondorDriver ();



    //
    // interface inherited from BatchDriver
    //

  protected:
    virtual ::StatusCode
    doManagerStep (Detail::ManagerData& data) const override;

    /// effects: special initialization for condor scripts: export PATH
    /// guarantee: strong
    /// failures: none
    /// rationale: condor jobs do not export PATH, needed by RootCore setup scripts
  private:
    virtual std::string batchInit () const override;


    //
    // private interface
    //

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Winconsistent-missing-override"
    ClassDef(CondorDriver, 1);
#pragma GCC diagnostic pop
  };
}

#endif

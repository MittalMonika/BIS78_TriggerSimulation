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

#include <EventLoop/Global.h>

#include <cstdlib>
#include <EventLoop/LLDriver.h>
#include <EventLoop/UnitTest.h>

//
// main program
//

using namespace EL;

int main ()
{
  if (getenv ("ROOTCORE_AUTO_UT") != 0)
    return EXIT_SUCCESS;

  const std::string HOME = getenv ("HOME");

  LLDriver driver;
  driver.shellInit = "export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase && source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh && eval localSetupROOT --rootVersion=5.34.00-x86_64-slc5-gcc4.3 --skipConfirm";
  driver.queue = "tier3";
  UnitTest ut ("ll");
  ut.cleanup = false;
  ut.location = "$HOME/unit-test.$$";
  return ut.run (driver);
}

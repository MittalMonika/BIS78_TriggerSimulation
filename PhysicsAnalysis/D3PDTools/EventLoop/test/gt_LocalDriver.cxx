/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//        
//                  Author: Nils Krumnack
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (nils.erik.krumnack@cern.ch) for bug
// reports, feature suggestions, praise and complaints.


//
// includes
//

#include <AsgTools/MessageCheck.h>
#include <EventLoop/LocalDriver.h>
#include <EventLoop/UnitTestConfig.h>
#include <EventLoop/UnitTestFixture.h>
#include <gtest/gtest.h>

#ifdef ROOTCORE
#include <xAODRootAccess/Init.h>
#endif

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//
// method implementations
//

using namespace asg::msgUserCode;

struct MyUnitTestConfig : public EL::UnitTestConfig
{
  MyUnitTestConfig ()
  {
    static const std::shared_ptr<EL::LocalDriver>
      driver (new EL::LocalDriver);
    m_driver = driver;
  }
};

namespace EL
{
  // this has to be SLOW, since it requires the entire release to have
  // been build for it to work
  INSTANTIATE_TEST_CASE_P(SLOW_LocalDriverTest, UnitTestFixture,
			  ::testing::Values(MyUnitTestConfig()),);
}

int main (int argc, char **argv)
{
#ifdef ROOTCORE
  StatusCode::enableFailure();
  ANA_CHECK (xAOD::Init ());
#endif
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS();
}

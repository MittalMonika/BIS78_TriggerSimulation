/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgTools/AnaToolHandle.h>
#include <AsgTools/MessageCheck.h>
#include <AsgTools/IMessagePrinter.h>
#include <AsgTools/MessagePrinterMock.h>
#include <AsgTools/MessagePrinterOverlay.h>
#include <AsgTesting/UnitTest.h>
#include <cmath>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>
#include <gmock/gmock.h>

#ifdef ROOTCORE
#include <xAODRootAccess/Init.h>
#endif

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

using namespace testing;

//
// unit test
//

namespace asg
{
  template<typename T,typename T2>
  void checkTypeSingle (T2 scSuccess, T2 scTest, bool expectedSuccess,
                        std::function<bool(const T&)> successTest)
  {
    using namespace asg::msgUserCode;

    bool success = false;
    T mySC = [&] () -> T {
      ANA_CHECK_SET_TYPE (T);
      ANA_CHECK (T (scTest));
      success = true;
      return scSuccess;
    } ();
    ASSERT_TRUE (successTest (mySC) == expectedSuccess);
    ASSERT_TRUE (success == expectedSuccess);
  }

  template<typename T,typename T2>
  void checkType (T2 scSuccess, T2 scFailure1, T2 scFailure2,
                  std::function<bool(const T&)> successTest)
  {
    checkTypeSingle<T> (scSuccess, scSuccess, true, successTest);
    checkTypeSingle<T> (scSuccess, scFailure1, false, successTest);
    checkTypeSingle<T> (scSuccess, scFailure2, false, successTest);
  }

  TEST (MessageCheck, checkType)
  {
    checkType<StatusCode> (StatusCode::SUCCESS, StatusCode::FAILURE, StatusCode::FAILURE, [] (const StatusCode& sc) -> bool {return sc.isSuccess();});
#ifdef ROOTCORE
    checkType<xAOD::TReturnCode> (xAOD::TReturnCode::kSuccess, xAOD::TReturnCode::kFailure, xAOD::TReturnCode::kRecoverable, [] (const xAOD::TReturnCode& sc) -> bool {return sc.isSuccess();});
#endif
    checkType<bool> (true, false, false, [] (const bool& sc) -> bool {return sc;});
  }
}

ATLAS_GOOGLE_TEST_MAIN

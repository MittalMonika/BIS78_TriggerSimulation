/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  errorcheck_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2006
 * @brief Regression tests for errorcheck.
 */

#undef NDEBUG

#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/AthStatusCode.h"
#include "TestTools/initGaudi.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Bootstrap.h"
#include <cassert>


class Algtest
  : public Algorithm
{
public:
  Algtest()
    : Algorithm ("algname", Gaudi::svcLocator()) {}
  virtual StatusCode execute() { return StatusCode (StatusCode::SUCCESS); }
  StatusCode test1();
};


StatusCode Algtest::test1()
{
  REPORT_ERROR (StatusCode (StatusCode::FAILURE)) << "foomsg";
  REPORT_MESSAGE (MSG::INFO) << "some info";
  // Can't write it like CHECK( StatusCode (StatusCode::SUCCESS) );
  // or we get parse errors from clang.
  StatusCode sc1 (StatusCode::SUCCESS);
  CHECK( sc1 );
  StatusCode sc2 (StatusCode::FAILURE);
  CHECK( sc2 );
  return StatusCode (StatusCode::SUCCESS);
}


class Algtooltest
  : public AlgTool
{
public:
  Algtooltest (IInterface* parent)
    : AlgTool ("tooltype", "toolname", parent) {}
  StatusCode test1();
};


StatusCode Algtooltest::test1()
{
  REPORT_ERROR (StatusCode (StatusCode::FAILURE)) << "foomsg";
  REPORT_MESSAGE (MSG::INFO) << "some info";
  CHECK_CODE( StatusCode (StatusCode::SUCCESS), StatusCode(123) );
  CHECK_CODE( StatusCode (StatusCode::RECOVERABLE), StatusCode(123) );
  return StatusCode (StatusCode::SUCCESS);
}


class Servtest
  : public Service
{
public:
  Servtest()
    : Service ("servname", Gaudi::svcLocator()) {}
  StatusCode test1();
};


StatusCode Servtest::test1()
{
  REPORT_ERROR (StatusCode (StatusCode::FAILURE)) << "foomsg";
  REPORT_MESSAGE (MSG::INFO) << "some info";
  CHECK_RECOVERABLE( StatusCode (StatusCode::SUCCESS) );
  CHECK_RECOVERABLE( StatusCode (StatusCode::FAILURE) );
  return StatusCode (StatusCode::SUCCESS);
}


class Test
{
public:
  StatusCode test1();
  int test2();
};


StatusCode Test::test1()
{
  REPORT_ERROR (StatusCode (StatusCode::FAILURE)) << "foomsg";
  REPORT_MESSAGE (MSG::INFO) << "some info";
  CHECK_FATAL( StatusCode (StatusCode::SUCCESS) );
  CHECK_FATAL( StatusCode (StatusCode::RECOVERABLE) );
  return StatusCode (StatusCode::SUCCESS);
}

int Test::test2()
{
  // Can't write it like CHECK( StatusCode (StatusCode::SUCCESS) );
  // or we get parse errors from clang.
  StatusCode sc1 (StatusCode::SUCCESS);
  CHECK( sc1, 0 );
  StatusCode sc2 (StatusCode::FAILURE);
  CHECK( sc2, -1 );
  return 0;
}



StatusCode test1a()
{
  // Can't write it like CHECK( StatusCode (StatusCode::SUCCESS) );
  // or we get parse errors from clang.
  StatusCode sc1 (StatusCode::SUCCESS);
  CHECK_WITH_CONTEXT( sc1, "alg" );
  StatusCode sc2 (StatusCode::FAILURE);
  CHECK_WITH_CONTEXT( sc2, "alg" );
  return StatusCode (StatusCode::SUCCESS);
}


StatusCode test1b()
{
  // Can't write it like CHECK( StatusCode (StatusCode::SUCCESS) );
  // or we get parse errors from clang.
  StatusCode sc1 (StatusCode::SUCCESS);
  CHECK_RECOVERABLE_WITH_CONTEXT( sc1, "alg" );
  StatusCode sc2 (StatusCode::FAILURE);
  CHECK_RECOVERABLE_WITH_CONTEXT( sc2, "alg" );
  return StatusCode (StatusCode::SUCCESS);
}


StatusCode test1c()
{
  // Can't write it like CHECK( StatusCode (StatusCode::SUCCESS) );
  // or we get parse errors from clang.
  StatusCode sc1 (StatusCode::SUCCESS);
  CHECK_RECOVERABLE_WITH_CONTEXT( sc1, "alg" );
  StatusCode sc2 (StatusCode::RECOVERABLE);
  CHECK_RECOVERABLE_WITH_CONTEXT( sc2,"alg" );
  return StatusCode (StatusCode::SUCCESS);
}


StatusCode test1d()
{
  // Can't write it like CHECK( StatusCode (StatusCode::SUCCESS) );
  // or we get parse errors from clang.
  StatusCode sc1 (StatusCode::SUCCESS);
  CHECK_FATAL_WITH_CONTEXT( sc1, "alg" );
  StatusCode sc2 (StatusCode::FAILURE);
  CHECK_FATAL_WITH_CONTEXT( sc2, "alg" );
  return StatusCode (StatusCode::SUCCESS);
}


StatusCode test1e()
{
  // Can't write it like CHECK( StatusCode (StatusCode::SUCCESS) );
  // or we get parse errors from clang.
  StatusCode sc1 (StatusCode::SUCCESS);
  CHECK_FATAL_WITH_CONTEXT( sc1, "alg" );
  StatusCode sc2 (StatusCode::RECOVERABLE);
  CHECK_FATAL_WITH_CONTEXT( sc2, "alg" );
  return StatusCode (StatusCode::SUCCESS);
}


StatusCode test1f()
{
  // Can't write it like CHECK( StatusCode (StatusCode::SUCCESS) );
  // or we get parse errors from clang.
  StatusCode sc1 (StatusCode::SUCCESS);
  CHECK_CODE_WITH_CONTEXT( sc1, "alg", StatusCode(123) );
  StatusCode sc2 (StatusCode::RECOVERABLE);
  CHECK_CODE_WITH_CONTEXT( sc2, "alg", StatusCode(123) );
  return StatusCode (StatusCode::SUCCESS);
}

int test1g()
{
  // Can't write it like CHECK( StatusCode (StatusCode::SUCCESS) );
  // or we get parse errors from clang.
  StatusCode sc1 (StatusCode::SUCCESS);
  CHECK_WITH_CONTEXT( sc1, "alg", 0 );
  StatusCode sc2 (StatusCode::FAILURE);
  CHECK_WITH_CONTEXT( sc2, "alg", -1 );
  return 0;
}

StatusCode test1h()
{
  StatusCode sc1 (Athena::Status::TIMEOUT);
  CHECK_WITH_CONTEXT( sc1, "algtimeout" );
  return StatusCode (StatusCode::SUCCESS);
}

StatusCode test1()
{
  // Can't write it like CHECK( StatusCode (StatusCode::SUCCESS) );
  // or we get parse errors from clang.
  Algtest algtest;  algtest.addRef();
  Algtooltest algtooltest (&algtest);  algtooltest.addRef();
  Servtest servtest;  servtest.addRef();
  Test test;
  REPORT_ERROR_WITH_CONTEXT (StatusCode (StatusCode::FAILURE), "alg")
    << "foomsg";
  REPORT_MESSAGE_WITH_CONTEXT (MSG::INFO, "alg")
    << "some message";
  {
    errorcheck::ReportMessage msg (MSG::INFO, ERRORCHECK_ARGS, "alg");
    msg << "a... ";
    msg << "b";
  }
  REPORT_MESSAGE_WITH_CONTEXT (MSG::INFO, "alg") << "foo" << endmsg;
  assert( test1a().isFailure() );
  assert( test1b().isFailure() );
  assert( test1c().isFailure() );
  assert( test1d().isFailure() );
  assert( test1e().isFailure() );
  assert( test1f().isFailure() );
  assert( test1g()==-1 );
  assert( test1h().isFailure() );
  assert( algtest.test1().isFailure() );
  assert( algtooltest.test1().isFailure() );
  assert( servtest.test1().isFailure() );
  assert( test.test1().isFailure() );
  assert( test.test2()==-1 );

  errorcheck::ReportMessage::hideErrorLocus();
  REPORT_ERROR_WITH_CONTEXT (StatusCode (StatusCode::FAILURE), "alg")
    << "foox";
  errorcheck::ReportMessage::hideErrorLocus (false);
  REPORT_ERROR_WITH_CONTEXT (StatusCode (StatusCode::FAILURE), "alg")
    << "fooy";
  return StatusCode (StatusCode::SUCCESS);
}


namespace errorcheck {
std::string clean_allocator (std::string f);
}


void test2 (std::vector<int> = std::vector<int>(),
            const int* = 0,
            int (*)() = 0,
            int [] = 0)
{
  REPORT_MESSAGE_WITH_CONTEXT (MSG::INFO, "test2") << "test2" << endmsg;

  assert (errorcheck::clean_allocator ("void test2(std::vector<int, std::allocator<int> >, const int*, int (*)(), int*)") ==
          "void test2(std::vector<int>, const int*, int (*)(), int*)");
}


void test3 (const std::string& = "", int = 0)
{
  REPORT_MESSAGE_WITH_CONTEXT (MSG::INFO, "test3") << "test3" << endmsg;
}

void test_checking()
{
  // Test the error reporting macro
  {
    StatusCode sc1;
    StatusCode sc2;
    auto check = [&]() -> StatusCode { CHECK_FAILED(sc1, "test_checking", sc1, sc2); };
    StatusCode sc = check();
    assert( sc1.checked() );     // macro checks the StatusCode
    assert( sc2.checked() );     // sc2 copied on return, i.e. checked
    assert( not sc.checked() );  // the returned copy of sc2 is unchecked
  }
  // Test the CHECK macro
  {
    auto check = [](StatusCode& sc1, StatusCode& sc2) -> StatusCode {
                   CHECK_WITH_CONTEXT(sc1, "test_checking", sc2);
                   return StatusCode::SUCCESS;
                 };
    // Test CHECK macro on success
    {
      StatusCode sc1;
      StatusCode sc2;
      StatusCode sc = check(sc1, sc2);
      assert( sc1.checked() );     // macro checks the StatusCode
      assert( not sc2.checked() ); // on success the macro does not return sc2
      assert( not sc.checked() );  // the returned copy of sc2 is unchecked
    }
    // Test CHECK macro on failure
    {
      StatusCode sc1(StatusCode::FAILURE);
      StatusCode sc2;
      StatusCode sc = check(sc1, sc2);
      assert( sc1.checked() );     // macro checks the StatusCode
      assert( sc2.checked() );     // on failure, sc2 copied on return, i.e. checked
      assert( not sc.checked() );  // the returned copy of sc2 is unchecked
    }
  }
  // Test REPORT macro
  {
    StatusCode sc(StatusCode::FAILURE);
    REPORT_ERROR_WITH_CONTEXT(sc, "test_checking");
    assert( sc.checked() );
  }
}

int main()
{
  ISvcLocator* loc;
  assert( Athena_test::initGaudi (loc) );
  test1();
  test2();
  test3();
  test_checking();
  return 0;
}

/*
 * Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file EventCommonTPCnv/test/P4PxPyPzECnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2018
 * @brief Regression tests.
 */

#undef NDEBUG
#include "EventCommonTPCnv/P4PxPyPzECnv_p1.h"
#include "TestTools/leakcheck.h"
#include "GaudiKernel/MsgStream.h"
#include <cassert>
#include <iostream>


void compare (const P4PxPyPzE& a1,
              const P4PxPyPzE& a2)
{
  assert (a1.px() == a2.px());
  assert (a1.py() == a2.py());
  assert (a1.pz() == a2.pz());
  assert (a1.e()  == a2.e());
}


void testit (const P4PxPyPzE& trans1)
{
  MsgStream log (0, "test");
  P4PxPyPzECnv_p1 cnv;
  P4PxPyPzE_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  P4PxPyPzE trans2;
  cnv.persToTrans (&pers, &trans2, log);
  compare (trans1, trans2);
}


void test1()
{
  std::cout << "test1\n";

  Athena_test::Leakcheck check;

  P4PxPyPzE trans1 (1, 2.5, 1.5, 4);
  testit (trans1);
}


int main()
{
  std::cout << "P4PxPyPzECnv_p1_test\n";
  test1();
  return 0;
}

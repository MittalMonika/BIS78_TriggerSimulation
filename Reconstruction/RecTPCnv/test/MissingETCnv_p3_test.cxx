/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file RecTPCnv/test/MissingETCnv_p3_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2019
 * @brief Regression tests.
 */

#undef NDEBUG
#include "RecTPCnv/MissingETCnv_p3.h"
#include "MissingETEvent/MissingET.h"
#include "TestTools/leakcheck.h"
#include "GaudiKernel/MsgStream.h"
#include <cassert>
#include <iostream>


void compare (const MissingEtRegions& p1,
              const MissingEtRegions& p2)
{
  assert (p1.exRegVec() == p2.exRegVec());
  assert (p1.eyRegVec() == p2.eyRegVec());
  assert (p1.etSumRegVec() == p2.etSumRegVec());
}


void compare (const MissingET& p1,
              const MissingET& p2)
{
  compare (*p1.getRegions(), *p2.getRegions());
  assert (p1.getSource() == p2.getSource());
  assert (p1.etx() == p2.etx());
  assert (p1.ety() == p2.ety());
  assert (p1.sumet() == p2.sumet());
}


void testit (const MissingET& trans1)
{
  MsgStream log (0, "test");
  MissingETCnv_p3 cnv;
  MissingET_p3 pers;
  cnv.transToPers (&trans1, &pers, log);
  MissingET trans2;
  cnv.persToTrans (&pers, &trans2, log);
  compare (trans1, trans2);
}


void test1()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  auto regions = std::make_unique<MissingEtRegions>();
  regions->setExRegVec (std::vector<double> {20.5, 21.5, 22.5});
  regions->setEyRegVec (std::vector<double> {23.5, 24.5, 25.5});
  regions->setEtSumRegVec (std::vector<double> {26.5, 27.5, 28.5});
  MissingET trans1 (MissingET::Final,
                    std::move (regions),
                    12.5,
                    100.5,
                    110.5);
  testit (trans1);
}


int main()
{
  std::cout << "RecTPCnv/MissingETCnv_p3\n";
  test1();
  return 0;
}

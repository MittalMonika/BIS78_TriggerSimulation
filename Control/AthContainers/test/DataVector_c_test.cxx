/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file AthContainers/test/DataVector_c_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2013
 * @brief Regression tests for DataVector
 *
 * The @c DataVector regression tests are split into several pieces,
 * in order to reduce the memory required for compilation.
 */

// Disable this test in standalone mode:
#ifndef XAOD_STANDALONE

#undef NDEBUG


#include "DataVector_test.icc"


void test2_c()
{
  std::cout << "test2_c\n";
  do_test2<M, P> ();
}


int main()
{
  test2_c();
  return 0;
}

#else

int main() {
   return 0;
}

#endif // not XAOD_STANDALONE

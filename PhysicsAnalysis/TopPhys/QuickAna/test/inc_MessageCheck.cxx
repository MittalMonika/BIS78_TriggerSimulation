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

#include <QuickAna/MessageCheck.h>

using namespace ana;

//
// main program
//

int main ()
{
  using namespace msgToolHandle;

  ANA_CHECK (StatusCode ());
  ANA_MSG_ERROR ("message");
  return 0;
}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
//#include "TestTools/expect.h"
#include "DecisionHandling/TrigCompositeUtils.h"

#include "xAODTrigger/TrigCompositeAuxContainer.h"

int main() {
  using namespace TrigCompositeUtils;

  auto dc = std::make_unique<DecisionContainer>();
  auto decisionAux = std::make_unique<DecisionAuxContainer>();
  dc->setStore(decisionAux.get());  


  // try insertions
  auto d1 = newDecisionIn(dc.get());
  addDecisionID( 1, d1 );
  addDecisionID( 2, d1 );

  auto d2 = newDecisionIn(dc.get());
  addDecisionID( 1, d2 );
  addDecisionID( 3, d2 );

  DecisionIDContainer ids;
  decisionIDs(d1, ids);

  for (auto id: ids)
    std::cout << id << " from d1\n"; 

  decisionIDs(d2, ids);
  for (auto id: ids)
    std::cout << id << " from d1 and d2\n"; 

  ids.clear();
  ids.insert(3);
  ids.insert(7);
  const DecisionIDContainer& cids = ids;
  {
    bool yn = passingIDs(d1, cids);
    std::cout <<  " 3 or 7 contained in d1 " << (yn ? "YES" : "NO") << std::endl;
  }

  {
    bool yn = passingIDs(d2, cids);
    std::cout <<  " 3 or 7 contained in d2 " << (yn ? "YES" : "NO") << std::endl;
  }
  
  return 0;
  
}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DecisionHandling_TrigCompositeUtils_h
#define DecisionHandling_TrigCompositeUtils_h

#include <set>
#include <memory>
#include "GaudiKernel/EventContext.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#include "xAODTrigger/TrigCompositeContainer.h"
#include "xAODTrigger/TrigCompositeAuxContainer.h"

namespace TrigCompositeUtils {

  // alias types, for readability and to simplify future evolution
  typedef xAOD::TrigComposite Decision;
  typedef xAOD::TrigCompositeContainer DecisionContainer;
  typedef xAOD::TrigCompositeAuxContainer DecisionAuxContainer;
  typedef SG::WriteHandle<DecisionContainer> DecisionWriteHandle;
  /*
    creates and right away stores the DecisionContainer under the key
  */
  DecisionWriteHandle createAndStore(const SG::WriteHandleKey<DecisionContainer>& key, const EventContext& ctx);

  
  /*
    @brief helper method to that created the Decision objects, places it in the container and returns
    This is to make this:
    auto d = newDecisionIn(output);
    instead of:
    auto d = new Decision; 
    output->push_back(d);    
   */
  
  Decision* newDecisionIn (DecisionContainer* dc);

  // aliases for the decision IDs, in fact this are just ints
  typedef unsigned int DecisionID;
  typedef std::set<DecisionID> DecisionIDContainer;

  /*
    @brief Appends the decision (given as ID) to the decision object
   */
  void addDecisionID( DecisionID id,  Decision* d);

      
  /*
    @brief Extracts DecisionIDs stored in the Decsion object 
   */
  void decisionIDs(const Decision* d, DecisionIDContainer& id );


  /*
    @brief Checks if any of the DecisionIDs passed in arg required is availble in Decision object
  */
  bool passingIDs( const Decision* d,  const DecisionIDContainer& required);
  
}


#endif // DecisionHandling_TrigCompositeUtils_h

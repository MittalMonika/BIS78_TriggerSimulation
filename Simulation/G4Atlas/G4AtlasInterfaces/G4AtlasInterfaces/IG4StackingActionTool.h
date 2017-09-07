/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASINTERFACES__G4UA_IG4STACKINGACTIONTOOL_H
#define G4ATLASINTERFACES__G4UA_IG4STACKINGACTIONTOOL_H

// Framework includes
#include "GaudiKernel/IAlgTool.h"

// Forward declarations
class G4UserStackingAction;

namespace G4UA
{

  /// @class IG4StackingActionTool
  /// @brief Abstract interface for tools that construct G4 stacking actions.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class IG4StackingActionTool : virtual public IAlgTool
  {
    
    public:

      /// @brief Return the action for current thread.
      virtual G4UserStackingAction* getG4StackingAction() = 0;

      /// Interface declaration
      static const InterfaceID& interfaceID() {
        static const InterfaceID iid_IStepTool("G4UA::IG4StackingActionTool", 1, 0);
        return iid_IStepTool;
      }

  }; // class IG4StackingActionTool

} // namespace G4UA

#endif

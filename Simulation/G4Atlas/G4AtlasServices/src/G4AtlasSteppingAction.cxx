/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "G4AtlasSteppingAction.h"
#include "G4AtlasInterfaces/ISteppingAction.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  G4AtlasSteppingAction::G4AtlasSteppingAction()
  {
  }

  //---------------------------------------------------------------------------
  // Process one step
  //---------------------------------------------------------------------------
  void G4AtlasSteppingAction::UserSteppingAction(const G4Step* step)
  {
    // Loop over my actions and apply each one in turn
    for(ISteppingAction* action : m_actions){
      action->processStep(step);
    }
  }

  //---------------------------------------------------------------------------
  // Add one action to the action list
  //---------------------------------------------------------------------------
  void G4AtlasSteppingAction::addAction(ISteppingAction* action)
  {
    m_actions.push_back(action);
  }

} // namespace G4UA

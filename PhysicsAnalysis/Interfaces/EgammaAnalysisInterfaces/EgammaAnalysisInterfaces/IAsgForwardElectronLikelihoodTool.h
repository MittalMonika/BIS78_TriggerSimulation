/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#ifndef EGAMMAANALYSISINTERFACES__IASGFORWARDELECTRONLIKELIHOODSELECTOR__
#define EGAMMAANALYSISINTERFACES__IASGFORWARDELECTRONLIKELIHOODSELECTOR__

/**
   @class IAsgForwardElectronLikelihoodTool
   @brief Interface to tool to select electrons

   20-JUL-2018, copied from tool for central electrons

*/

// Include the interfaces
#include "PATCore/IAsgSelectionTool.h"
// Forward declarations
namespace Root{
  class TAccept;
  class TResult;
}
#include "xAODEgamma/ElectronFwd.h"
#include "xAODEgamma/EgammaFwd.h"

class IAsgForwardElectronLikelihoodTool : virtual public IAsgSelectionTool
{

  ASG_TOOL_INTERFACE(IAsgForwardElectronLikelihoodTool)

 public:

  /// @name IAsgForwardElectronLikelihoodTool  methods in Addition to the IAsgSelectionTool ones
  /// Some are there to mainly support the calls done from the online/Trigger side 
  /// @{

  /// accept with pointer to  IParticle  so as to not hide the IAsgSelectionTool one
  virtual const Root::TAccept& accept( const xAOD::IParticle* part ) const = 0;

  /// accept method with pointer to electron 
  virtual const Root::TAccept& accept( const xAOD::Electron*  part ) const = 0;

  /// accept method with pointer to electron  when mu not in EventInfo for online 
  virtual const Root::TAccept& accept( const xAOD::Electron*  part, double mu ) const = 0;

  /// accept method with pointer to  egamma 
  virtual const Root::TAccept& accept( const xAOD::Egamma*    part ) const = 0;

  /// accept method with pointer to egammma when mu not in EventInfo for online 
  virtual const Root::TAccept& accept( const xAOD::Egamma*    part, double mu ) const = 0;

  /// TResult method: for pointer to IParticle
  virtual const Root::TResult& calculate( const xAOD::IParticle* part ) const = 0;

  /// TResult method: for pointer to electron
  virtual const Root::TResult& calculate( const xAOD::Electron*  eg   ) const = 0;

  /// TResult method: for pointer to egamma 
  virtual const Root::TResult& calculate( const xAOD::Egamma*    eg   ) const = 0;

  /// TResult method: for pointer to electron  when mu not in EventInfo for online
  virtual const Root::TResult& calculate( const xAOD::Electron* eg, double mu ) const = 0;

  /// TResult method: for pointer to egamma when mu not in EventInfo for online
  virtual const Root::TResult& calculate( const xAOD::Egamma*   eg, double mu ) const = 0;

  // Just return the TResult 
  virtual const Root::TResult& getTResult( ) const = 0;

  ///Method to get the operating point 
  virtual std::string getOperatingPointName( ) const = 0;

  ///@}

  /// Virtual Destructor
  virtual ~IAsgForwardElectronLikelihoodTool() {}

}; // End: class definition


#endif


/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

#ifndef EGAMMAANALYSISINTERFACES__IASGEGAMMAISEMSELECTOR__
#define EGAMMAANALYSISINTERFACES__IASGEGAMMAISEMSELECTOR__

/**
   @class IAsgEGammaIsEMSelector
   @brief Interface to tool to select photons

   @author Jovan Mitrevski (UCSC) Karsten Koeneke (CERN)
   @date   Dec 2011 - Fab 2012

   11-MAR-2014, convert to ASGTool
*/

// Include the interfaces
#include "AsgAnalysisInterfaces/ISelectionTool.h"
#include "xAODEgamma/EgammaFwd.h"
#include "xAODEgamma/PhotonFwd.h"
#include "xAODEgamma/ElectronFwd.h"
#include "GaudiKernel/EventContext.h"

class IAsgEGammaIsEMSelector : virtual public CP::ISelectionTool
{

  ASG_TOOL_INTERFACE(IAsgEGammaIsEMSelector)

public:

  /// @name IAsgEGammaIsEMSelector methods in Addition to the ISelectionTool
  /// @{

  /// accept with pointer to  IParticle  so as to not hide the ISelectionTool one
  virtual asg::AcceptData accept( const xAOD::IParticle* part ) const = 0;
  virtual asg::AcceptData accept( const EventContext& ctx, const xAOD::IParticle* part ) const = 0;

  /// accept with pointer to Egamma object 
  virtual asg::AcceptData accept( const EventContext& ctx, const xAOD::Egamma* part) const = 0;

  /// accept with Photon pointer
  virtual asg::AcceptData accept( const EventContext& ctx, const xAOD::Photon* part ) const = 0;

  /// accept with Electron reference
  virtual asg::AcceptData accept( const EventContext& ctx, const xAOD::Electron* part ) const = 0;

  ///Add a legacy execute method - return isEM value
  virtual StatusCode execute(const EventContext& ctx, const xAOD::Egamma* eg, unsigned int& isEM) const =0;

  /// Report the current operating point
  virtual std::string getOperatingPointName( ) const =0;

  ///@}

  /// Virtual Destructor
  virtual ~IAsgEGammaIsEMSelector() {};

}; 


#endif


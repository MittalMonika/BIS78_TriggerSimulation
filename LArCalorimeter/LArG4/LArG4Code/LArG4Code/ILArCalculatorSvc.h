/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ILArCalculatorSvc.h 
// Prepared in 2016 based on LArVCalculator from Bill Seligman

#ifndef LARG4CODE_ILARCALCULATORSVC_H
#define LARG4CODE_ILARCALCULATORSVC_H

#include "GaudiKernel/IService.h"

#include "globals.hh"

#include "LArG4Code/LArG4Identifier.h"


// Forward declaractions:
class G4Step;
class StatusCode;

/// struct to pass the hit info
struct LArHitData { LArG4Identifier id; G4double time; G4double energy; };

/// TODO: NEEDS DOCUMENTATION
class ILArCalculatorSvc: virtual public IService {
public:

  ILArCalculatorSvc() {};
  static const InterfaceID& interfaceID() { 
     static const InterfaceID  IID_ILArCalculatorSvc("ILArCalculatorSvc",1,0);
     return IID_ILArCalculatorSvc; 
  }

  virtual ~ILArCalculatorSvc() {};

  /// "OOTcut" the time cut (ns) after which an energy deposit is
  /// considered to be out-of-time w.r.t. the event.
  virtual G4float OOTcut() const = 0;

  /// Check if the current hitTime is in-time
  virtual G4bool isInTime(G4double hitTime) const = 0;    // units = ns

  /// New interface, to pass the hit info directly
  virtual G4bool Process(const G4Step*, std::vector<LArHitData>&) const = 0;

};

#endif

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file IInDetConditionsTool.h
 * @author shaun.roe@cern.ch
**/
#ifndef IInDetConditionsTool_h
#define IInDetConditionsTool_h
//Gaudi Includes
#include "GaudiKernel/IInterface.h"

//Local includes
#include "InDetConditionsSummaryService/InDetHierarchy.h"

//forward declarations
class Identifier;
class IdentifierHash;
/**
 * @class IInDetConditionsTool
 * Interface class for service providing summary of status of a detector element
**/
class IInDetConditionsTool: virtual public IInterface, virtual public IAlgTool{
public:
  virtual ~IInDetConditionsTool(){
    //nop, needed for derived classes
  }
  static const InterfaceID & interfaceID(); //!< reimplemented from IInterface
 
  //@name isActive methods reveal whether the element should return data
  //@{
  virtual bool isActive(const Identifier & elementId, const InDetConditions::Hierarchy h=InDetConditions::DEFAULT) const =0;
  virtual bool isActive(const IdentifierHash & elementHash) const =0;
  virtual bool isActive(const IdentifierHash & elementHash, const Identifier & elementId) const =0;
  //@}
  ///activeFraction method gives the fraction of active elements in the range of identifiers
  virtual double activeFraction(const IdentifierHash & elementHash, const Identifier & idStart, const Identifier & idEnd) const =0;  
  //@{
  
  //@name isGood methods reveal whether the data are reliable
   //@{
  virtual bool isGood(const Identifier & elementId, const InDetConditions::Hierarchy h=InDetConditions::DEFAULT) const =0;
  virtual bool isGood(const IdentifierHash & elementHash) const =0;
  virtual bool isGood(const IdentifierHash & elementHash, const Identifier & elementId) const =0;
  //@}
  ///goodFraction method gives the fraction of good elements in the range of identifiers
  virtual double goodFraction(const IdentifierHash & elementHash, const Identifier & idStart, const Identifier & idEnd) const =0;
   
};

inline const InterfaceID & IInDetConditionsTool::interfaceID(){
  static const InterfaceID IID_IInDetConditionsTool("IInDetConditionsTool",1,0);
  return IID_IInDetConditionsTool;
}
#endif

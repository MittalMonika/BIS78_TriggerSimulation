/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ISCT_ByteStreamErrorsSvc.h
 * interface file for service that keeps track of errors in the bytestream.
 * @author nbarlow@cern.ch
 **/

#ifndef ISCT_ByteStreamErrorsSvc_h
#define ISCT_ByteStreamErrorsSvc_h

#include <set>

#include "InDetConditionsSummaryService/InDetHierarchy.h"
#include "SCT_ConditionsServices/ISCT_ConditionsSvc.h"
#include "SCT_ConditionsData/SCT_ByteStreamErrors.h"

class Identifier;
class IdentifierHash;

/**
 * @class SCT_ByteStreamErrorsSvc
 * Service that keeps track of modules that give rise to errors in the bytestram.
 **/

class ISCT_ByteStreamErrorsSvc: virtual public ISCT_ConditionsSvc {

 public:
  //@name Service methods
  //@{

  virtual ~ISCT_ByteStreamErrorsSvc() {}

  static const InterfaceID& interfaceID();

  //@}
  
  virtual const std::set<IdentifierHash>* getErrorSet(int errorType)=0;

  virtual void setRODSimulatedData()=0;

  virtual bool isRODSimulatedData()=0;

  virtual void addError(IdentifierHash id, int errorType)=0;
  virtual void addErrorCount(int errorType)=0;

  virtual void resetSets()=0;
  virtual void resetCounts()=0; /** for the counts used by HLT */
  virtual int getNumberOfErrors(int errorType)=0; /** for HLT */

  virtual void addRODHVCounter(bool HVisOn)=0;

  virtual bool isCondensedReadout()=0;

  virtual void setCondensedReadout(bool isCondensed)=0;

  virtual bool HVisOn()=0;

  /** Set first temporarily masked chip information from byte stream trailer */
  virtual void setFirstTempMaskedChip(const IdentifierHash& hashId, const unsigned int firstTempMaskedChip)=0;
  /** Get first temporarily masked chip information */
  virtual unsigned int getFirstTempMaskedChip(const IdentifierHash& hashId) const =0;
  /** Map of temporary chip status for all modules with at least one bad chip (packed as 1st 12 bits of unsigned int) */
  virtual const std::map<Identifier, unsigned int>* tempMaskedChips() const =0;
  /** Temporary status of chips for a particular module (packed as 1st 12 bits of unsigned int) */
  virtual unsigned int tempMaskedChips(const Identifier& moduleId) const =0;
  /** Status ABCD errors of chips for a particular module (packed as 1st 12 bits of unsigned int) */
  virtual unsigned int abcdErrorChips(const Identifier& moduleId) const =0;

 private:

};

inline const InterfaceID& ISCT_ByteStreamErrorsSvc::interfaceID() {
  static const InterfaceID IID_SCT_ByteStreamErrorsSvc{"SCT_ByteStreamErrorsSvc", 1, 0};
  return IID_SCT_ByteStreamErrorsSvc;
}

#endif // ISCT_ByteStreamErrorsSvc_h

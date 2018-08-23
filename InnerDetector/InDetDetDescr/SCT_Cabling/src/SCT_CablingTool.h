/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_CablingTool_h
#define SCT_CablingTool_h
/**
 * @file SCT_CablingTool.h
 * Header file for  SCT cabling service
 * @author Shaun Roe
 * @author Susumu Oda
 * @date 19 August, 2018
 **/

#include "SCT_Cabling/ISCT_CablingTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "Identifier/IdentifierHash.h"
#include "SCT_Cabling/SCT_CablingData.h"
#include "StoreGate/ReadCondHandleKey.h"

//STL includes
#include <string>

//fwd declarations
class StatusCode;
class SCT_ID;
class Identifier;

/**
 * @class SCT_CablingTool, providing mappings of online and offline identifiers and also serial numbers
 **/
class SCT_CablingTool: public extends<AthAlgTool, ISCT_CablingTool> {
 public:

  //@name Tool methods, reimplemented
  //@{
  SCT_CablingTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~SCT_CablingTool() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  //@}
  
  //@name ISCT_CablingTool methods implemented, these are visible to clients
  //@{
  /// return offline hash, given the online Id (used by decoders)
  virtual IdentifierHash getHashFromOnlineId(const SCT_OnlineId& onlineId, const bool withWarnings=true) const override;
   
  /// return the online Id, given a hash (used by simulation encoders)
  virtual SCT_OnlineId getOnlineIdFromHash(const IdentifierHash& hash) const override;
  
  /// return the online Id, given an offlineId
  virtual SCT_OnlineId getOnlineIdFromOfflineId(const Identifier& offlineId) const override;
  
  /// return the rob/rod Id, given a hash (used by simulation encoders)
  virtual std::uint32_t getRobIdFromHash(const IdentifierHash& hash) const override {
    return getOnlineIdFromHash(hash).rod();
  }
    
  /// return the rob/rod Id, given an offlineId (used by simulation encoders)
  virtual std::uint32_t getRobIdFromOfflineId(const Identifier& offlineId) const override {
    return getOnlineIdFromOfflineId(offlineId).rod();
  }

  /// size of the data structure (for the SCT should be 8176, one for each module side)
  virtual unsigned int size() const override;
    
  /// is the data structure empty?
  virtual bool empty() const override;
    
  /// get hash from a module serial number, needed in the conditions service because configurations are stored by module s/n
  virtual IdentifierHash getHashFromSerialNumber(const SCT_SerialNumber& sn) const override;

  /// get module serial number from hash, needed during filling of data structure
  virtual SCT_SerialNumber getSerialNumberFromHash(const IdentifierHash& hash) const override;

  /// fill a users vector with all the RodIds
  virtual void getAllRods(std::vector<std::uint32_t>& usersVector) const override;

  /// fill a user's vector with all the hash ids which belong to a given rod
  virtual void getHashesForRod(std::vector<IdentifierHash>& usersVector, const std::uint32_t rodId) const override;
  //@}

 private:
  SG::ReadCondHandleKey<SCT_CablingData> m_data{this, "SCT_CablingData", "SCT_CablingData", "SCT_CablingData created by SCT_CablingCondAlgFromCoraCool"};
  StringProperty m_cablingDataSource; //!< the name of the data source
  const SCT_ID* m_idHelper; //!< helper for offlineId/hash conversions
  bool m_usingDatabase;

  const SCT_CablingData* getData() const;
};

#endif // SCT_CablingTool_h

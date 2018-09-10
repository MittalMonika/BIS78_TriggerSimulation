/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_CablingToolCB_h
#define SCT_CablingToolCB_h
/**
 * @file SCT_CablingToolCB.h
 * Header file for  SCT cabling tool using call back
 * @author Shaun Roe
 * @date 20 October, 2008
 **/

#include "AthenaBaseComps/AthAlgTool.h"
#include "SCT_Cabling/ISCT_CablingTool.h"

#include "Identifier/IdentifierHash.h"
#include "SCT_Cabling/ISCT_FillCabling.h"
#include "SCT_Cabling/SCT_CablingData.h"

//Gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IIncidentListener.h"

//STL includes
#include <cstdint> //uint32 in std: namespace
#include <string>
#include <vector>

//fwd declarations
class StatusCode;
class InterfaceID;
class Incident;
class SCT_ID;
class Identifier;

/**
 * @class SCT_CablingToolCB, providing mappings of online and offline identifiers and also serial numbers
 **/
class SCT_CablingToolCB: public extends<AthAlgTool, ISCT_CablingTool, IIncidentListener> {
 public:

  ///Incident listener method
  virtual void handle(const Incident& beginRunIncident);
    
  //@name Service methods, reimplemented
  //@{
  SCT_CablingToolCB(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~SCT_CablingToolCB() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  //@}
  
  //@name ISCT_CablingToolCB methods implemented, these are visible to clients
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
  virtual void getAllRods(std::vector<std::uint32_t>& usersVector) const override { m_data.getRods(usersVector); }
    
  /// fill a user's vector with all the hash ids which belong to a given rod
  virtual void getHashesForRod(std::vector<IdentifierHash>& usersVector, const std::uint32_t rodId) const override;
  //@}

 private:
  SCT_CablingData m_data;

  ToolHandle<ISCT_FillCabling> m_cablingFillerText{"SCT_FillCablingFromText"};
  ToolHandle<ISCT_FillCabling> m_cablingFillerCoraCool{"SCT_FillCablingFromCoraCool"};
  StringProperty m_cablingDataSource; //!< the name of the data source
  const SCT_ID* m_idHelper; //!< helper for offlineId/hash conversions
  bool m_usingDatabase;
};

#endif // SCT_CablingToolCB_h

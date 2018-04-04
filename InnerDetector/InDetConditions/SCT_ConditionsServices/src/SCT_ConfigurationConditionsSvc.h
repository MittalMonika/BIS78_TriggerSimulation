/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_ConfigurationConditionsSvc.h
 * header file for service which reads SCT configuration from database
 * @author shaun.roe@cern.ch, gwilliam@mail.cern.ch
**/

#ifndef SCT_ConfigurationConditionsSvc_h
#define SCT_ConfigurationConditionsSvc_h

// STL includes
#include <string>
#include <mutex>

// Gaudi includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ContextSpecificPtr.h"

// Athena includes
#include "AthenaBaseComps/AthService.h"
#include "SCT_ConditionsServices/ISCT_ConditionsSvc.h"
#include "SCT_ConditionsServices/ISCT_ConfigurationConditionsSvc.h"
#include "SCT_ConditionsData/SCT_ConfigurationCondData.h"
#include "StoreGate/DataHandle.h"
#include "StoreGate/ReadCondHandleKey.h"

// Forward declarations
template <class TYPE> class SvcFactory;
class SCT_ID;
class StoreGateSvc;
namespace InDetDD { class SCT_DetectorManager; }

/**
 * @class SCT_ConfigurationConditionsSvc
 * Service which reads SCT_Configuration from the database
 * Deals with bad modules, bad links, strips out of the readout and bad strips
**/

class SCT_ConfigurationConditionsSvc: virtual public ISCT_ConfigurationConditionsSvc, virtual public AthService {
  friend class SvcFactory<SCT_ConfigurationConditionsSvc>;
public:

  //@name Service methods
  //@{
  SCT_ConfigurationConditionsSvc(const std::string& name, ISvcLocator* svc);
  virtual ~SCT_ConfigurationConditionsSvc() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode queryInterface(const InterfaceID& riid, void** ppvInterface) override;
  static const InterfaceID& interfaceID();
  //@}
  
  /**Can the service report about the given component? (chip, module...)*/
  virtual bool                          canReportAbout(InDetConditions::Hierarchy h) override;
  
  /**Is the detector element good?*/
  virtual bool                          isGood(const Identifier& elementId, InDetConditions::Hierarchy h=InDetConditions::DEFAULT) override;
  
  /**Is it good?, using wafer hash*/
  virtual bool                          isGood(const IdentifierHash& hashId) override;

  /**Manually get the data in the structure before proceding*/
  virtual StatusCode                    fillData() override { return StatusCode::FAILURE; }
  
  /**Fill data from an IOVDbSvc callback*/
  virtual StatusCode                    fillData(int& i, std::list<std::string>& l) override;
  
  /**Are the data available?*/
  virtual bool                          filled() const override;
  
  /**Can the data be filled during the initialize phase?*/
  virtual bool                          canFillDuringInitialize() override { return false; }

  /**List of bad modules*/
  virtual const std::set<Identifier>*   badModules() override;
  /**List of bad strips*/
  virtual void                          badStrips(std::set<Identifier>& strips, bool ignoreBadModules=false, bool ignoreBadChips=false) override;
  /**List of bad strips for a given module*/
  virtual void                          badStrips(const Identifier& moduleId, std::set<Identifier>& strips, bool ignoreBadModules=false, bool ignoreBadChips=false) override;
  /**List of bad links*/
  virtual std::pair<bool, bool>         badLinks(const Identifier& moduleId) override;
  /**Bad links for a given module*/
  virtual const std::map<Identifier, std::pair<bool, bool>>* badLinks() override;
  /**List of bad chips*/
  virtual const std::map<Identifier, unsigned int>* badChips() override;
  /**Bad chips for a given module*/
  virtual unsigned int                  badChips(const Identifier& moduleId) const override;
  /** Get the chip number containing a particular strip*/
  virtual int                           getChip(const Identifier& stripId) const override;

private:
  // Mutex to protect the contents.
  mutable std::mutex m_mutex;
  // Cache to store events for slots
  mutable std::vector<EventContext::ContextEvt_t> m_cache;
  // Pointer of SCT_MonitorConditionsCondData
  mutable Gaudi::Hive::ContextSpecificPtr<const SCT_ConfigurationCondData> m_condData;
  SG::ReadCondHandleKey<SCT_ConfigurationCondData> m_condKey;

  ServiceHandle<StoreGateSvc>           m_detStore;                 //!< Handle on the detector store
  const SCT_ID*                         m_pHelper;                  //!< ID helper for SCT
  const InDetDD::SCT_DetectorManager*   m_pManager;                 //!< SCT detector manager
  bool                                  m_checkStripsInsideModules; //!< Do we want to check if a strip is bad because it is inside a bad module

  /** Is a strip within a bad module*/
  bool                                  isStripInBadModule(const Identifier& stripId);
  /** Is a wafer in a bad module*/
  bool                                  isWaferInBadModule(const Identifier& waferId);
  
  /**Is a chip with this Identifier good?*/
  bool isGoodChip(const Identifier& stripId) const;

  /** enum for constants*/
  enum {stripsPerChip=128, lastStrip=767};

  const SCT_ConfigurationCondData* getCondData(const EventContext& ctx) const;
};

#endif // SCT_ConfigurationConditionsSvc_h

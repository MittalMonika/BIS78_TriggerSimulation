/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @file SCT_FlaggedConditionSvc.h
 * header file for service allowing one to flag modules as 'bad' with a reason
 * @author gwilliam@mail.cern.ch
 */

#ifndef SCT_FlaggedConditionSvc_h
#define SCT_FlaggedConditionSvc_h
 
// Gaudi
#include "GaudiKernel/ServiceHandle.h"

// Athena
#include "AthenaBaseComps/AthService.h"
#include "SCT_ConditionsServices/ISCT_FlaggedConditionSvc.h"
#include "InDetConditionsSummaryService/InDetHierarchy.h"
#include "StoreGate/ReadHandleKey.h"

// Forward declarations
template <class TYPE> class SvcFactory;
class ISvcLocator;

class Identifier;
class IdentifierHash;
class SCT_ID;
class StoreGateSvc;

/*
 * @class SCT_FlaggedConditionSvc
 * Service allowing one to flag detector elements as 'bad' with a reason
 */

class SCT_FlaggedConditionSvc: virtual public ISCT_FlaggedConditionSvc, virtual public AthService {
  friend class SvcFactory<SCT_FlaggedConditionSvc>;

public:
  //@name Service methods
  //@{
  SCT_FlaggedConditionSvc(const std::string& name, ISvcLocator* svc);
  virtual ~SCT_FlaggedConditionSvc() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode queryInterface( const InterfaceID& riid, void** ppvInterface ) override;
  static const InterfaceID & interfaceID();
  //@}

  /**Can the service report about the given component? (chip, module...)*/
  virtual bool canReportAbout(InDetConditions::Hierarchy h) override;
  
  /**Is the detector element good?*/
  virtual bool isGood(const Identifier& elementId, InDetConditions::Hierarchy h=InDetConditions::DEFAULT) override;
  virtual bool isGood(const IdentifierHash& hashId) override;

  /**Manually get the data*/
  virtual StatusCode fillData() override {
    return StatusCode::SUCCESS;
  }

  /** Get the data via callback*/
  virtual StatusCode fillData(int& /*i*/ , std::list<std::string>& /*l*/) override {
    return StatusCode::FAILURE;
  }
  
  /**Are the data available?*/
  virtual bool filled() const override {return true;}

  /**Can the data be filled during the initialize phase?*/
  virtual bool canFillDuringInitialize() override { return false; }

  /**Flag a wafer as bad with a reason (by Identifier)*/
  virtual bool flagAsBad(const Identifier& id, const std::string& source) override;
  /**Flag a wafer ID as bad with a reason (by IdentifierHash)*/
  virtual bool flagAsBad(const IdentifierHash& hashId, const std::string& source) override;
  
  /**Get the reason why the wafer is bad (by Identifier)*/ 
  virtual const std::string& details(const Identifier& id) const override;
  /**Get the reason why the wafer is bad (by IdentifierHash)*/ 
  virtual const std::string& details(const IdentifierHash& id) const override;

  /**Get number flagged as bad (per event)*/
  virtual int numBadIds() const override;

  /**Get IdentifierHashs ofwafers flagged as bad + reason (per event)*/
  virtual const SCT_FlaggedCondData* getBadIds() const override;

  /**Reset between events*/
  virtual void resetBadIds() override;

 private:
  SG::ReadHandleKey<SCT_FlaggedCondData> m_badIds;

  ServiceHandle<StoreGateSvc> m_detStore; //!< Handle on detector store
  const SCT_ID* m_sctID; //!< ID helper for SCT

  const SCT_FlaggedCondData* getCondData() const;
};

#endif // SCT_FlaggedConditionSvc_h

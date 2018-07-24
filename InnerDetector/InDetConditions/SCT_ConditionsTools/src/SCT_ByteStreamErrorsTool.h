/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_ByteStreamErrorsTool.h
 * header file for tool that keeps track of errors in the bytestream.
 * @author Susumu.Oda@cern.ch
**/

#ifndef SCT_ByteStreamErrorsTool_h
#define SCT_ByteStreamErrorsTool_h

///STL includes
#include <set>
#include <map>
#include <list>
#include <mutex>

///Gaudi includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ContextSpecificPtr.h"

///Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetConditionsSummaryService/InDetHierarchy.h"
#include "SCT_ConditionsTools/ISCT_ByteStreamErrorsTool.h"
#include "SCT_ConditionsTools/ISCT_ConfigurationConditionsTool.h"
#include "InDetByteStreamErrors/InDetBSErrContainer.h"
#include "InDetByteStreamErrors/SCT_ByteStreamFractionContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "Identifier/IdContext.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"

/** Read (Cond)Handle Key */
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

/** forward declarations */
template <class TYPE> class SvcFactory;
class ISvcLocator;
class SCT_ID;

/**
 * @class SCT_ByteStreamErrorsTool
 * Tool that keeps track of modules that give rise to errors in the bytestram.
**/

class SCT_ByteStreamErrorsTool: public extends<AthAlgTool, ISCT_ByteStreamErrorsTool> {

public:
  //@name Tool methods
  //@{
  SCT_ByteStreamErrorsTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~SCT_ByteStreamErrorsTool() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  //@}
  
  virtual bool canReportAbout(InDetConditions::Hierarchy h) const override;
  
  ///Is the detector element good?
  virtual bool isGood(const Identifier& elementId, InDetConditions::Hierarchy h=InDetConditions::DEFAULT) const override;
  virtual bool isGood(const IdentifierHash& elementIdHash) const override;
  
  const std::set<IdentifierHash>* getErrorSet(int errorType) const override; // Used by SCTRawDataProviderTool and others

  virtual unsigned int tempMaskedChips(const Identifier& moduleId) const override; // Internally used
  virtual unsigned int abcdErrorChips(const Identifier& moduleId) const override; // Internally used
  virtual bool isRODSimulatedData() const override; // Internally used
  virtual bool HVisOn() const override; // Internally used
  virtual bool isCondensedReadout() const override; // Not used

private:

  ToolHandle<ISCT_ConfigurationConditionsTool> m_config{this, "ConfigTool",
      "SCT_ConfigurationConditionsTool/InDetSCT_ConfigurationConditionsTool", "Tool to retrieve SCT Configuration Tool"};
  const SCT_ID* m_sct_id;
  IdContext m_cntx_sct;

  SG::ReadHandleKey<InDetBSErrContainer> m_bsErrContainerName{this, "ContainerName", "SCT_ByteStreamErrs", "Key of InDetBSErrContainer for SCT"};
  SG::ReadHandleKey<SCT_ByteStreamFractionContainer> m_bsFracContainerName{this, "FracContainerName", "SCT_ByteStreamFrac", "Key of SCT_ByteStreamFractionContainer"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

  mutable std::vector<std::set<IdentifierHash> > m_bsErrors[SCT_ByteStreamErrors::NUM_ERROR_TYPES]; // Used by getErrorSet, addError, resetSets

  mutable std::vector<std::map<Identifier, unsigned int> > m_tempMaskedChips;
  mutable std::vector<std::map<Identifier, unsigned int> > m_abcdErrorChips;

  // Mutex to protect the contents.
  mutable std::mutex m_mutex;
  // Cache to store events for slots
  mutable std::vector<EventContext::ContextEvt_t> m_cache;
  mutable std::vector<EventContext::ContextEvt_t> m_cacheElements;

  // Pointer of InDetDD::SiDetectorElementCollection
  mutable Gaudi::Hive::ContextSpecificPtr<const InDetDD::SiDetectorElementCollection> m_detectorElements;

  mutable unsigned int m_nRetrievalFailure;

  StatusCode fillData(const EventContext& ctx) const;

  void addError(IdentifierHash id, int errorType, const EventContext& ctx) const;
  void resetSets(const EventContext& ctx) const;

  bool isGoodChip(const Identifier& stripId) const;
  int getChip(const Identifier& stripId) const;

  // For isRODSimulatedData, HVisOn and isCondensedReadout
  const SCT_ByteStreamFractionContainer* getFracData() const;
  const InDetDD::SiDetectorElement* getDetectorElement(const IdentifierHash& waferHash) const;

  const std::set<IdentifierHash>& getErrorSet(SCT_ByteStreamErrors::errorTypes errorType, const EventContext& ctx) const;
  const std::map<Identifier, unsigned int>& getTempMaskedChips(const EventContext& ctx) const;
  const std::map<Identifier, unsigned int>& getAbcdErrorChips(const EventContext& ctx) const;
};

#endif // SCT_ByteStreamErrorsTool_h

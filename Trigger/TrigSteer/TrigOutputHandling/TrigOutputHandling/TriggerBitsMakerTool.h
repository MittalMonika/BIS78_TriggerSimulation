/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGOUTPUTHANDLING_TRIGGERBITSMAKERTOOL_H
#define TRIGOUTPUTHANDLING_TRIGGERBITSMAKERTOOL_H

#include <string>
#include "DecisionHandling/TrigCompositeUtils.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigOutputHandling/HLTResultMTMakerTool.h"
#include "TrigConfData/HLTMenu.h"

/**
 * @class TriggerBitsMakerTool
 * @brief fills trigger bits in the HLTResultMT object
 **/
class TriggerBitsMakerTool : public extends<AthAlgTool, HLTResultMTMakerTool> {
public:
  TriggerBitsMakerTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~TriggerBitsMakerTool() override;

  virtual StatusCode fill( HLT::HLTResultMT& resultToFill ) const override;
  StatusCode fill( std::vector<uint32_t>& place ) const;
  
  virtual StatusCode initialize() override;
  virtual StatusCode start() override;
  virtual StatusCode finalize() override;

private:
  enum BitCategory{ HLTPassRawCategory, HLTPrescaledCategory, HLTRerunCategory };

  StatusCode setBit(const TrigCompositeUtils::DecisionID chain, const BitCategory category, HLT::HLTResultMT& resultToFill) const;

  /// Check that a chain's hash in the menu JSON (via python) agrees with the C++ implementation
  ///
  StatusCode hashConsistencyCheck(const std::string& chain, const size_t hash) const;

  /// Check that no existing key maps to a given value
  ///
  StatusCode preInsertCheck(const std::string& chain, const uint32_t bit) const;

  SG::ReadHandleKey<TrigCompositeUtils::DecisionContainer> m_finalChainDecisions { this, "ChainDecisions", "HLTNav_Summary", "Container with final chain decisions"  };

  SG::ReadHandleKey<TrigConf::HLTMenu> m_HLTMenuKey{this, "HLTTriggerMenu", "DetectorStore+HLTTriggerMenu", "HLT Menu"};

  Gaudi::Property<std::map<std::string, uint32_t>> m_extraChainToBit { this, "ExtraChainToBit", {},
    "Special case and testing purposes hard-coded chain-to-bit mappings to use in addition to those from the HLT menu."};

  typedef std::map< TrigCompositeUtils::DecisionID, uint32_t> ChainToBitMap;
  ChainToBitMap m_mapping; //!< Mapping of each chain's hash ID to its chain counter

  uint32_t m_largestBit; //!< Largest chain counter hence largest bit needed to be stored in result bitmap
};

#endif // TRIGOUTPUTHANDLING_TRIGGERBITSMAKERTOOL_H

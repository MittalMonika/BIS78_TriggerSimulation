/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include "RoIsUnpackingToolBase.h"
#include "TrigConfL1Data/TriggerItem.h"

RoIsUnpackingToolBase::RoIsUnpackingToolBase(const std::string& type, 
                                             const std::string& name, 
                                             const IInterface* parent) 
  : base_class(type, name, parent)
{
}


StatusCode RoIsUnpackingToolBase::initialize() 
{
  if ( !m_monTool.empty() ) CHECK( m_monTool.retrieve() );
  CHECK( m_decisionsKey.initialize() );


  return StatusCode::SUCCESS;
}

StatusCode RoIsUnpackingToolBase::decodeMapping( std::function< bool(const TrigConf::TriggerThreshold*)> filter, const TrigConf::ItemContainer& l1Items, const IRoIsUnpackingTool::SeedingMap& seeding ) {
  for ( auto chainItemPair: seeding) {
    std::string chainName = chainItemPair.first;
    std::string itemName = chainItemPair.second;
    auto itemsIterator = l1Items.get<TrigConf::tag_name_hash>().find(itemName);
    
    if ( itemsIterator != l1Items.get<TrigConf::tag_name_hash>().end() ) {
      const TrigConf::TriggerItem* item = *itemsIterator;
      const TrigConf::TriggerItemNode* node = item->topNode();
      std::vector<TrigConf::TriggerThreshold*> itemThresholds;
      node->getAllThresholds(itemThresholds);
      for ( const TrigConf::TriggerThreshold* th: itemThresholds ) {
	if ( filter(th) ) {
	  m_thresholdToChainMapping[HLT::Identifier(th->name())].push_back( HLT::Identifier(chainName) );
	}
      }
    } else {
      // we may wish to change that to ERROR at some point
      ATH_MSG_WARNING( "Could not find item: " << itemName << " that is used in seeding");
    }
  }
  return StatusCode::SUCCESS;
}

void RoIsUnpackingToolBase::addChainsToDecision( HLT::Identifier thresholdId,
                                                 TrigCompositeUtils::Decision* d,
                                                 const HLT::IDSet& activeChains ) const {
  auto chains = m_thresholdToChainMapping.find( thresholdId );
  if ( chains == m_thresholdToChainMapping.end() ) {
    ATH_MSG_DEBUG("Threshold not known " << thresholdId);
    return;
  }
  for ( auto chainId: chains->second ) {
    if ( activeChains.find(chainId) != activeChains.end() ) {
      TrigCompositeUtils::addDecisionID( chainId.numeric(), d );
      ATH_MSG_DEBUG( "Added chain to the RoI/threshold decision " << chainId );
    }
  }
}

StatusCode RoIsUnpackingToolBase::copyThresholds( const std::vector<TrigConf::TriggerThreshold*>& src, std::vector<TrigConf::TriggerThreshold*>& dest ) const {
  for ( auto th: src ) {
    if ( th == nullptr ) {
      ATH_MSG_INFO( "Nullptr TrigConf::TriggerThreshold" ); 
    } else {
      ATH_MSG_INFO( "Found threshold in the configuration: " << th->name() << " of ID: " << HLT::Identifier( th->name() ).numeric() ); 
      dest.push_back( th );      
    }
  }

  return StatusCode::SUCCESS;
}




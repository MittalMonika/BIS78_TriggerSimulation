/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_UNIFIEDFLOWNETWORKBUILDER_H
#define TRIGHLTJETHYPO_UNIFIEDFLOWNETWORKBUILDER_H


#include "./IFlowNetworkBuilder.h"
#include "./FlowNetworkBuilderBase.h"
#include "./ConditionsDefsMT.h"
#include "./Tree.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/FlowEdge.h"

using TreeVec = std::vector<std::size_t>;
using IDJetGroup = std::map<std::size_t, HypoJetVector>;

class UnifiedFlowNetworkBuilder: 
virtual public IFlowNetworkBuilder, private FlowNetworkBuilderBase{
 public:
  UnifiedFlowNetworkBuilder(ConditionsMT,
			    const Tree&,
			    const std::vector<std::vector<int>>&);

 protected:
  virtual std::optional<std::vector<std::shared_ptr<FlowEdge>>>
    make_flowEdges(const HypoJetGroupCIter& groups_b,
		   const HypoJetGroupCIter& groups_e,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>& collector,
                   int& V,
		   std::map<int, pHypoJet>& nodeToJet) const override;
 
  std::string toString() const override;

 private:

  // tree structure for Conditions objects. Flow networks
  // will be constructed using Conditions and jet group objects.
  // The conditions tree gives relations among conditions (eg parent-child)
  Tree m_tree;

  //a vector of shared nodes. All shared nodes are leaf node that
  // see the jet collection.
  std::vector<std::vector<int>> m_sharedNodes;

  std::optional<std::vector<std::shared_ptr<FlowEdge>>>
    make_flowEdges_(const HypoJetGroupCIter& groups_b,
		    const HypoJetGroupCIter& groups_e,
		    const std::unique_ptr<ITrigJetHypoInfoCollector>& collector,
		    int& V,
		    std::map<int, pHypoJet>& nodeToJet,
		    const std::vector<int>& leaves) const;
  
  void makeSourceToLeafEdges(std::vector<std::shared_ptr<FlowEdge>>& edges,
			     const std::vector<int>& leaves) const;

  void makeLeafJobGroupEdges(std::vector<std::shared_ptr<FlowEdge>>& edges,
			     const std::vector<int>& leaves,
			     const HypoJetGroupCIter& groups_b,
			     const HypoJetGroupCIter& groups_e,
			     std::map<int, pHypoJet>& nodeToJet,
			     std::map<std::size_t, HypoJetGroupVector>& satBy,
			     IDJetGroup& jetgroups,
 			     std::set<std::size_t>& toSink, 
			     std::size_t& cur_jg,
			     const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const;

  
  void propagateEdges(std::vector<std::shared_ptr<FlowEdge>>& edges,
		      const std::vector<int>& leaves,
		      std::map<std::size_t, HypoJetGroupVector>& satisfiedBy,
		      std::size_t& cur_jg,
		      IDJetGroup& jetgroups,
		      std::set<std::size_t>& toSink, 
		      int& V,
		      const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const;
  

};

#endif

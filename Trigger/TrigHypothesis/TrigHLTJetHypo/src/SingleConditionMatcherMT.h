/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_SINGLECONDITIONMATCHERMT_H
#define TRIGHLTJETHYPO_SINGLECONDITIONMATCHERMT_H


// ********************************************************************
//
// NAME:     SingleConditionMatcherMT.h
// PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
//
// AUTHOR:  P Sherwood
//
// ********************************************************************
//

#include "./IGroupsMatcherMT.h"
#include "./ConditionsDefsMT.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJet.h"
#include <utility>  // std::pair
#include <set>


class SingleConditionMatcherMT: virtual public IGroupsMatcherMT {

  /* Used to find jets pass multithreshold,
     possibly overlapping eta regions
     where any particular jet can be assigned to at most
     one threshold. Uses the Ford Fulkerson Alg.
     See Algorithms, Sedgewick and Wayne 4th edition */

public:
  SingleConditionMatcherMT(const ConditionBridgeMT&);
  ~SingleConditionMatcherMT(){}
  bool match(const HypoJetGroupCIter&,
             const HypoJetGroupCIter&,
             std::unique_ptr<IConditionVisitor>&) override;
  std::string toString() const noexcept override;
  ConditionsMT getConditions() const noexcept override;
private:
  ConditionBridgeMT m_condition;

};

#endif

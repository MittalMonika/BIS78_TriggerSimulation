/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_CONDITIONBRIDGEMT_H
#define TRIGHLTJETHYPO_CONDITIONBRIDGEMT_H
/********************************************************************
 *
 * NAME:     ConditionBridgeMT.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * AUTHOR:   P. Sherwood
 *********************************************************************/

#include "./IConditionMT.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"
#include <string>
#include <memory>
#include <ostream>

class IConditionVisitor;

class ConditionBridgeMT{
  // Bridge object: convert polymorphic pointer to monomorphic class
  // can be passed to STL algs.
 public:
 ConditionBridgeMT(std::shared_ptr<IConditionMT>& condition):
  m_pCondition(condition){}
  
  bool isSatisfied(const HypoJetVector& ips,
                   std::unique_ptr<IConditionVisitor>& v) const{
    return m_pCondition -> isSatisfied(ips, v);
  }

  bool operator()(const HypoJetVector& ips,
                  std::unique_ptr<IConditionVisitor> v) const{
    return isSatisfied(ips, v);
  }

  std::string toString() const noexcept{
    return m_pCondition -> toString();
  }

 private:
  std::shared_ptr<IConditionMT> m_pCondition;
};

#endif

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPOUNITTESTS_SimpleHypoJetVectorGenerator_H
#define TRIGHLTJETHYPOUNITTESTS_SimpleHypoJetVectorGenerator_H

#include "TrigHLTJetHypoUnitTests/IHypoJetVectorGenerator.h"
#include "AthenaBaseComps/AthAlgTool.h"


class SimpleHypoJetVectorGenerator: public extends<AthAlgTool,
  IHypoJetVectorGenerator> {

 public:
  
  SimpleHypoJetVectorGenerator (const std::string& type,
				const std::string& name,
				const IInterface* parent);

  ~SimpleHypoJetVectorGenerator() {}

  virtual StatusCode initialize() override;
  virtual HypoJetVector get() const override;
  virtual std::string toString() const override;
  
 private:

  Gaudi::Property<std::vector<double>>
    m_ets{this, "ets", {}, "jet et values"};

  Gaudi::Property<std::vector<double>>
    m_etas{this, "etas", {}, "jet eta  values"};

 

  Gaudi::Property<int>
    m_nbkgd{this, "n_bkgd", 0, "No of background jets to generate "};
  
  Gaudi::Property<double>
    m_bkgd_etmax{this, "bkgd_etmax", 0., "Max Et  background jets"};
  
  Gaudi::Property<double>
    m_bkgd_etamax{this, "bkgd_etamax", 5., "Lower limit Eta range background "};
};

#endif

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJet/Jet.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/xAODJetAsIJet.h"
#include "./TLorentzVectorAsIJet.h"
#include "TrigHLTJetHypo/../src/xAODJetCollector.h"
#include <TLorentzVector.h>
#include "gtest/gtest.h"

/*
using ::testing::Return;
using ::testing::_;
using ::testing::SetArgReferee;
*/

class xAODJetCollectorTest: public ::testing::Test {
public:
  xAODJetCollectorTest() {}
};


TEST_F(xAODJetCollectorTest, zeroJets){
  // zero jets in. Jet collector is empty.
  
  HypoJetVector jets;
  xAODJetCollector collector;
  collector.addJets(jets.begin(), jets.end());
  EXPECT_TRUE(collector.empty());
}


TEST_F(xAODJetCollectorTest, multipleInputJets){
  //multiple jets in. Jet collector is not empty and has the correct number.

  constexpr std::size_t njets{11};
  HypoJetVector jets;
  std::vector<xAOD::Jet> xaodjets (njets);
  std::vector<HypoJet::xAODJetAsIJet> ijets;
  ijets.reserve (njets);
  for(unsigned int ijet = 0; ijet < njets; ++ijet){
    ijets.emplace_back (&xaodjets[ijet], ijet);
    jets.push_back(&ijets.back());
  }

  xAODJetCollector collector;
  collector.addJets(jets.begin(), jets.end());
 
  EXPECT_FALSE(collector.empty());
  EXPECT_FALSE((collector.xAODJets()).size() == 10);
}


TEST_F(xAODJetCollectorTest, nonXAODJets){
  // multiple non-xAOD jets in. Jet collector is empty.

  unsigned int njets{11};
  HypoJetVector jets;
  std::vector<TLorentzVectorAsIJet> ijets;
  ijets.reserve (njets);
  for(unsigned int ijet = 0; ijet < njets; ++ijet){
    TLorentzVector v;
    ijets.emplace_back (v);
    jets.push_back(&ijets.back());
  }

  xAODJetCollector collector;
  collector.addJets(jets.begin(), jets.end());
 
  EXPECT_TRUE(collector.empty()); //Not xAODJets
}


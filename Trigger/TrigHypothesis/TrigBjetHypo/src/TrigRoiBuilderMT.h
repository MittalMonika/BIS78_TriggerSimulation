// emacs: this is -*- c++ -*-
/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
//
//   @file    TrigRoiBuilderMT.h        
//


#ifndef  TRIGROIBUILDER_MT_H
#define  TRIGROIBUILDER_MT_H

#include <iostream>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"

class TrigRoiBuilderMT : public AthAlgorithm {

 public:
  TrigRoiBuilderMT(const std::string&, ISvcLocator*);
  ~TrigRoiBuilderMT();
  
  StatusCode initialize();
  StatusCode finalize();
  StatusCode execute();

 private:
  Gaudi::Property< float > m_etaHalfWidth {this,"EtaHalfWidth",0.1,"Eta Half Width"};
  Gaudi::Property< float > m_phiHalfWidth {this,"PhiHalfWidth",0.1,"Phi Half Width"};
  Gaudi::Property< float > m_minJetEt {this,"JetMinEt",30.0,"Jet Min Et"};
  Gaudi::Property< float > m_maxJetEta {this,"JetMaxEta",2.6,"Jet Max Eta : 2.5 + Eta Half Width"};
  Gaudi::Property< int >   m_nJetsMax {this,"NJetsMax",-1,"Option to limit the number of jets that form the super RoI  "};
  Gaudi::Property< bool >  m_dynamicMinJetEt {this,"DynamicMinJetEt",false,"if (X > -1 && nJets > X) minJetEt = m_minJetEt + (nJets-X)*Y  "};
  Gaudi::Property< int >   m_dynamicNJetsMax {this,"DynamicNJetsMax",9999,"variable X above"};
  Gaudi::Property< float > m_dynamicEtFactor {this,"DynamicEtFactor",0,"variable Y above "};

  SG::ReadHandleKey< xAOD::JetContainer > m_jetInputKey {this,"JetInputKey","TrigJetRec","Input Jet Collection Key, retrieved from reconstructed jets"};
  SG::WriteHandleKey< TrigRoiDescriptorCollection > m_roIOutputKey {this,"RoIOutputKey","EMViewRoIs","Output RoI Collection Key"};
};
 

#endif

//** ---------------------------------------------------------------------------------------


// this file is -*- C++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//JetConstituentModSequence.h

#ifndef JETRECTOOLS_JETCONSTITUENTMODSEQUENCE_H
#define JETRECTOOLS_JETCONSTITUENTMODSEQUENCE_H

//
// // Michael Nelson, CERN & Univesity of Oxford
// // February, 2016

#include <string>
#include "AsgTools/AsgTool.h"
#include "JetInterface/IJetExecuteTool.h"
#include "JetInterface/IJetConstituentModifier.h"
#include "AsgTools/ToolHandleArray.h"
#include "xAODCore/ShallowCopy.h"

#include "xAODBase/IParticleContainer.h"
// #include "xAODCaloEvent/CaloCluster.h" 
#include "xAODCaloEvent/CaloClusterContainer.h"
// #include "xAODTruth/TruthParticle.h" 
#include "xAODTruth/TruthParticleContainer.h" 
// #include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODPFlow/PFOContainer.h"


class JetConstituentModSequence: public asg::AsgTool, virtual public IJetExecuteTool { // Changed from IJetExecute
  ASG_TOOL_CLASS(JetConstituentModSequence, IJetExecuteTool)
  public:
  JetConstituentModSequence(const std::string &name); // MEN: constructor 
  StatusCode initialize();
  int execute() const;
  void setInputClusterCollection(const xAOD::IParticleContainer* cont);
  
protected:
  std::string m_inputContainer = "";
  std::string m_outputContainer = "";

  // used in trigger context only
  const xAOD::IParticleContainer* m_trigInputClusters; 
  bool m_trigger;
  
  // P-A : a property defining the type name of the input constituent
  std::string m_inputTypeName = "CaloCluster"; // MEN: Check this 
  // P-A : the actual type
  xAOD::Type::ObjectType m_inputType; // 
  
  
  ToolHandleArray<IJetConstituentModifier> m_modifiers;

  bool m_saveAsShallow = true;

  // note: not all keys will be used for a particular instantiation
  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_caloClusterKey;
  SG::WriteHandleKey<xAOD::TruthParticleContainer> m_truthParticleKey;
  SG::WriteHandleKey<xAOD::TrackParticleContainer> m_trackParticleKey;

  SG::WriteHandleKey<xAOD::PFOContainer> m_outPFOChargedKey{
    "ChargedParticleFlowObjects"};

  SG::WriteHandleKey<xAOD::PFOContainer> m_outPFONeutralKey{
    "NeutralParticleFlowObjects"};

  SG::WriteHandleKey<ConstDataVector<xAOD::PFOContainer>> m_outPFOAllKey{};

  SG::ReadHandleKey<xAOD::PFOContainer> m_inPFOChargedKey{};
  SG::ReadHandleKey<xAOD::PFOContainer> m_inPFONeutralKey{};
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_inCaloClusterKey{};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_inTruthParticleKey{};
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inTrackParticleKey{};
  SG::ReadHandleKey<xAOD::PFOContainer> m_inPFOChargedCopyKey{};
  SG::ReadHandleKey<xAOD::PFOContainer> m_inPFONeutralCopyKey{};

  StatusCode copyModRecordPFO() const;

  /// helper function to cast, shallow copy and record a container.

  template<class T>
  StatusCode
  copyModRecord(const SG::ReadHandleKey<T>&,
                const SG::WriteHandleKey<T>&) const;
};

template<class T>
StatusCode
JetConstituentModSequence::copyModRecord(const SG::ReadHandleKey<T>& inKey,
                                         const SG::WriteHandleKey<T>& outKey) const {

  /* Read in a container of (type is template parameter),
     optionally modify the elements of this container, and store.
     This puts a (modified) copy of the container  into storegate.
  */
  
  auto inHandle = makeHandle(inKey);
  if(!inHandle.isValid()){
    ATH_MSG_WARNING("Unable to retrieve input container from " << inKey.key());
    return StatusCode::FAILURE;
  }

  std::pair< T*, xAOD::ShallowAuxContainer* > newclust = 
    xAOD::shallowCopyContainer(*inHandle);    
  newclust.second->setShallowIO(m_saveAsShallow);

  auto handle = makeHandle(outKey);
  ATH_CHECK(handle.record(std::unique_ptr<T>(newclust.first),
                          std::unique_ptr<xAOD::ShallowAuxContainer>(newclust.second)));
  
  for (auto t : m_modifiers) {ATH_CHECK(t->process(newclust.first));}
  
  return StatusCode::SUCCESS;
}

#endif

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// PFlowPseudoJetGetter.h

#ifndef JETRECTOOLS_CORRECTPFOTOOL_H
#define JETRECTOOLS_CORRECTPFOTOOL_H

////////////////////////////////////////////
/// \class CorrectPFOTool
///
/// Applies various corrections to PFO
///
/// Properties:
///  InputIsEM - If true, EM-scale is used for the neutral pflow
///  CalibratePFO - If true the EM-scale pflow is calibrated
///
/// \author Jennifer Roloff and John Stupak - based on PFlowPseudoJetGetter by P-A Delsart, D. Adams
//////////////////////////////////////////////////

#include "JetRecTools/JetConstituentModifierBase.h"
#include "xAODPFlow/PFOContainer.h"
#include "AsgTools/ToolHandle.h"
#include "PFlowUtils/IWeightPFOTool.h"

namespace jet{
  class TrackVertexAssociation;
}

class CorrectPFOTool : public JetConstituentModifierBase{
  ASG_TOOL_CLASS(CorrectPFOTool, IJetConstituentModifier)

    public:

  CorrectPFOTool(const std::string& name);
  StatusCode initialize();
  StatusCode process(xAOD::IParticleContainer* cont) const; 
  StatusCode process(xAOD::PFOContainer* cont) const;

 private:
  bool m_inputIsEM;   /// If true EM clusters are used for neutral PFOs.
  bool m_calibrate;   /// If true, EM PFOs are calibrated to LC.
  bool m_correctneutral;   //If true, correct neutral PFO
  bool m_correctcharged;   //If true, correct charged PFO
  bool m_usevertices; //If true, then we make use of the primary vertex information
  bool m_useChargedWeights; //If true, them we make use of the charged PFO weighting scheme
  bool m_useTrackToVertexTool; //If true, use jet tracktovertex tool
  bool m_applyCHS; //If true, remove charged PFO not matched to the PV
  ToolHandle<CP::IWeightPFOTool> m_weightPFOTool;

  SG::ReadHandleKey<xAOD::VertexContainer> 
    m_vertexContainer_key{"PrimaryVertices"};

  SG::ReadHandleKey<jet::TrackVertexAssociation> m_trkVtxAssoc_key;
  
};

#endif

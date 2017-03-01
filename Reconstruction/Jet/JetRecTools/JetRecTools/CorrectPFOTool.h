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
/// \author John Stupak and Jennifer Roloff- based on PFlowPseudoJetGetter by P-A Delsart, D. Adams
//////////////////////////////////////////////////

#include "JetRecTools/JetConstituentModifierBase.h"
#include "xAODPFlow/PFOContainer.h"
#include "AsgTools/ToolHandle.h"
//#include "PFlowUtils/IRetrievePFOTool.h"
#include "PFlowUtils/IWeightPFOTool.h"

namespace jet{
  class TrackVertexAssociation;
}

class CorrectPFOTool : public JetConstituentModifierBase{
  ASG_TOOL_CLASS(CorrectPFOTool, IJetConstituentModifier)

    public:

  CorrectPFOTool(const std::string& name);
  StatusCode process(xAOD::IParticleContainer* cont) const; 
  StatusCode process(xAOD::PFOContainer* cont) const;

 private:
  bool m_inputIsEM;   /// If true EM clusters are used for neutral PFOs.
  bool m_calibrate;   /// If true, EM PFOs are calibrated to LC.
  bool m_correctneutral;
  bool m_correctcharged;
  bool m_usevertices; //If true, then we make use of the primary vertex information
  bool m_useChargedWeights; //If true, them we make use of the charged PFO weighting scheme
  bool m_useTrackToVertexTool; //If true, use jet tracktovertex tool
  bool m_applyCHS; //If true, remove charged PFO not associated to the PV
  ToolHandle<CP::IWeightPFOTool> m_weightPFOTool;
  std::string m_trkVtxAssocName; //Name of track-vertex association container
};

#endif

/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUJETBDTEVALUATOR_H
#define TAURECTOOLS_TAUJETBDTEVALUATOR_H

// tauRecTools include(s)
#include "tauRecTools/TauRecToolBase.h"
#include "tauRecTools/HelperFunctions.h"

/**
 * @brief Implementation of a generic BDT for tau ID
 * @brief Can support dynamic loading of floats attached
 * @brief to the tau object, wih a few modifications to
 * @brief HelperFunctions TRTBDT, should handle ints too
 * @author Justin Griffiths
 *                                                                              
 */

class TauJetBDTEvaluator
: public TauRecToolBase
{
 public:
  ASG_TOOL_CLASS2(TauJetBDTEvaluator, TauRecToolBase, ITauToolBase)

  TauJetBDTEvaluator(const std::string& name="TauJetBDTEvaluator");
  virtual ~TauJetBDTEvaluator() { }
    
  StatusCode initialize() override;
  StatusCode execute(xAOD::TauJet& xTau) override;
  StatusCode finalize() override;
  
 private:

  std::string m_weightsFile;
  std::string m_outputVarName;

  std::unique_ptr<tauRecTools::TRTBDT> m_myBdt;
  int m_minNTracks;
  int m_maxNTracks;
  float m_minAbsTrackEta;
  float m_maxAbsTrackEta;
  float m_dummyValue;// in case no configs are set, set a dummy value.

  bool m_isGrad;
};
#endif

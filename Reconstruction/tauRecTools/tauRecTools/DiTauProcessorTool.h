/*
Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUREC_DITAUPROCESSORTOOL_H
#define TAUREC_DITAUPROCESSORTOOL_H

#include "tauRecTools/ITauToolExecBase.h"
#include "AsgTools/ToolHandleArray.h"
#include "tauRecTools/IDiTauToolBase.h"


class DiTauProcessorTool : public asg::AsgTool, virtual public ITauToolExecBase {
 public:

  ASG_TOOL_CLASS1( DiTauProcessorTool, ITauToolExecBase )
    
  DiTauProcessorTool(const std::string& type);
  ~DiTauProcessorTool();

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

 private:
  std::string                 m_sDiTauContainerNameHadHad;
  std::string                 m_sDiTauContainerNameHadEl;
  std::string                 m_sDiTauContainerNameHadMu;

  ToolHandleArray<tauRecTools::IDiTauToolBase>  m_aVarCalculatorTools;
  
  ToolHandleArray<tauRecTools::IDiTauToolBase>  m_aVarCalculatorToolsHadHad;

  ToolHandleArray<tauRecTools::IDiTauToolBase>  m_aVarCalculatorToolsHadEl;
  
  ToolHandleArray<tauRecTools::IDiTauToolBase>  m_aVarCalculatorToolsHadMu;

  StatusCode processContainer(std::string& diTauContainerName, 
                              ToolHandleArray<tauRecTools::IDiTauToolBase>& varCalculatorTools);
};
#endif //TAUREC_DITAUPROCESSORTOOL_H

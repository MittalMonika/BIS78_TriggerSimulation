/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TOPCPTOOLS_TOPEGAMMACPTOOLS_H_
#define TOPCPTOOLS_TOPEGAMMACPTOOLS_H_

// Include what you use
#include <vector>
#include <string>

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/ToolHandleArray.h"
#include "AsgTools/AnaToolHandle.h"

// Egamma include(s):
#include "EgammaAnalysisInterfaces/IEgammaCalibrationAndSmearingTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IElectronPhotonShowerShapeFudgeTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "ElectronPhotonSelectorTools/AsgForwardElectronLikelihoodTool.h"
#include "TrigBunchCrossingTool/WebBunchCrossingTool.h"

namespace top {

class TopConfig;

class EgammaCPTools final : public asg::AsgTool {
 public:
  explicit EgammaCPTools(const std::string& name);
  virtual ~EgammaCPTools() {}

  StatusCode initialize();

 private:
  std::shared_ptr<top::TopConfig> m_config;

  int m_release_series = 24;  // Default to 2.4

  std::string m_electronEffTriggerFile;
  std::string m_electronEffTriggerLooseFile;
  std::string m_electronEffSFTriggerFile;
  std::string m_electronEffSFTriggerLooseFile;
  std::string m_electronEffSFRecoFile;
  std::string m_electronEffSFIDFile;
  std::string m_electronEffSFIDLooseFile;
  std::string m_electronEffSFIsoFile;
  std::string m_electronEffSFIsoLooseFile;
  std::string m_electronEffSFChargeIDFile;
  std::string m_electronEffSFChargeIDLooseFile;
  std::string m_electronEffSFChargeMisIDFile;
  std::string m_electronEffSFChargeMisIDLooseFile;
  
  std::string m_fwdElectronEffSFIDFile;
  std::string m_fwdElectronEffSFIDLooseFile;

  ToolHandle<CP::IEgammaCalibrationAndSmearingTool> m_egammaCalibrationAndSmearingTool;

  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffSFTrigger;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffSFTriggerLoose;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffTrigger;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffTriggerLoose;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffSFReco;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffSFID;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffSFIDLoose;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffSFIso;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffSFIsoLoose;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffSFChargeID;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_electronEffSFChargeIDLoose;

  ToolHandle<IElectronPhotonShowerShapeFudgeTool> m_photonFudgeTool;
  ToolHandle<IAsgPhotonEfficiencyCorrectionTool> m_photonEffSF;
  ToolHandleArray<IAsgPhotonEfficiencyCorrectionTool> m_photonIsoSFTools;

  ToolHandle<IAsgPhotonIsEMSelector> m_photonTightIsEMSelector;
  ToolHandle<IAsgPhotonIsEMSelector> m_photonMediumIsEMSelector;
  ToolHandle<IAsgPhotonIsEMSelector> m_photonLooseIsEMSelector;
  
  ToolHandle<AsgForwardElectronLikelihoodTool> m_fwdElectronSelector;
  ToolHandle<AsgForwardElectronLikelihoodTool> m_fwdElectronSelectorLoose;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_fwdElectronEffSFID;
  ToolHandle<IAsgElectronEfficiencyCorrectionTool> m_fwdElectronEffSFIDLoose;
  
  ToolHandle<Trig::WebBunchCrossingTool> m_webBunchCrossingTool;
  
  StatusCode setupSelectors();
  StatusCode setupCalibration();
  StatusCode setupScaleFactors();

  IAsgElectronEfficiencyCorrectionTool*
    setupElectronSFTool(const std::string& name, const std::vector<std::string>& file_list, int data_type);

  // Helper function to deal with path resolving the
  // egamma groups very long file names for SFs and efficiencies.
  std::string electronSFFilePath(const std::string& type, const std::string& ID, std::string ISO);

  // Helper for using maps from egamma groups
  IAsgElectronEfficiencyCorrectionTool* setupElectronSFToolWithMap(const std::string& name, std::string map_path, std::string reco_key, std::string ID_key, std::string iso_key, std::string trigger_key, int data_type);

  std::string electronSFMapFilePath(const std::string& type);

  std::string mapWorkingPoints(const std::string& type);


};
}  // namespace top

#endif  // TOPCPTOOLS_TOPEGAMMACPTOOLS_H_

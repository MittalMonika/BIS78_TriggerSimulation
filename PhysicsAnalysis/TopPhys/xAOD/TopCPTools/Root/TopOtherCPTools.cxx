/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TopCPTools/TopOtherCPTools.h"

#include <map>
#include <string>
#include <cstdio>

// Top includes
#include "TopConfiguration/TopConfig.h"
#include "TopEvent/EventTools.h"

// PathResolver include(s):
#include "PathResolver/PathResolver.h"

// GRL include(s):
#include "GoodRunsLists/GoodRunsListSelectionTool.h"

// Pileup Reweighting include(s):
#include "PileupReweighting/PileupReweightingTool.h"

namespace top {

OtherCPTools::OtherCPTools(const std::string& name) :
    asg::AsgTool(name) {
  declareProperty("config", m_config);
  declareProperty("release_series", m_release_series );

  declareProperty( "GRLTool" , m_grlTool );
}

StatusCode OtherCPTools::initialize() {
  ATH_MSG_INFO("top::OtherCPTools initialize...");

  if (!m_config->isMC() || m_config->PileupUseGRLTool())
    top::check(setupGRL(), "Failed to setup GRL tool");

  if (m_config->doPileupReweighting())
    top::check(setupPileupReweighting(), "Failed to setup pile-up reweighting");

  if (m_config->isMC())
    top::check(setupPMGTools(), "Failed to setup PMG tools");

  return StatusCode::SUCCESS;
}

StatusCode OtherCPTools::setupGRL() {
  using IGRLTool = IGoodRunsListSelectionTool;
  const std::string grl_tool_name = "GoodRunsListSelectionTool";
  if (asg::ToolStore::contains<IGRLTool>(grl_tool_name)) {
    m_grlTool = asg::ToolStore::get<IGRLTool>(grl_tool_name);
  } else {
    if (!m_config->grlFile().size()) {
      // do not configure GRL tool
      ATH_MSG_WARNING("No GRL files provided -> Will NOT setup GoodRunsListSelectionTool.");
      return StatusCode::SUCCESS;
    }
    
    IGRLTool* grlTool =  new GoodRunsListSelectionTool(grl_tool_name);
    std::vector<std::string> names = {};
    std::string grlDir = m_config->grlDir();
    if (grlDir.size()) grlDir += "/";
    for (const std::string& grl : m_config->grlFile()) {
      names.push_back(PathResolverFindCalibFile(grlDir + grl));
    }
    
    top::check(asg::setProperty(grlTool, "GoodRunsListVec", names),
                "GRLSelector failed to set GRL");
    top::check( grlTool->initialize() ,
                "Failed to initialize GRLSelectionTool" );
    m_grlTool = grlTool;
  }
  return StatusCode::SUCCESS;
}

StatusCode OtherCPTools::setupPileupReweighting() {
  const std::string prw_tool_name = "CP::PileupReweightingTool";
  if (asg::ToolStore::contains<CP::IPileupReweightingTool>(prw_tool_name)) {
    m_pileupReweightingTool = asg::ToolStore::get<CP::IPileupReweightingTool>(prw_tool_name);
  } else {
    CP::IPileupReweightingTool* pileupReweightingTool
      = new CP::PileupReweightingTool(prw_tool_name);

    std::vector<std::string> pileup_config = m_config->PileupConfig();
    for (std::string& s : pileup_config)
      s = PathResolverFindCalibFile(s);
    
    std::vector<std::string> pileup_lumi_calc = m_config->PileupLumiCalc();
    for (std::string& s : pileup_lumi_calc)
      s = PathResolverFindCalibFile(s);

    if (m_config->PileupUseGRLTool())
      top::check(asg::setProperty(pileupReweightingTool, "GRLTool", m_grlTool),
                "Failed to give GRLtool to pileup reweighting tool");

    // Config file is not needed on Data
    // see "Just random run numbers and lumi corrections"
    // case in twiki page below
    // However, the tool would spit out warnings for Data if we didn't supply ConfigFiles.
    top::check(asg::setProperty(pileupReweightingTool, "ConfigFiles", pileup_config),
              "Failed to set pileup reweighting config files");

    // data scale-factors, initialised to recommended values
    // can also be customised, thanks to PRWCustomScaleFactor option
    double SF_nominal = 1.0/1.09;
    double SF_up = 1.0;
    double SF_down = 1.0/1.18;

    // if custom data SFs
    if (m_config->PileUpCustomScaleFactors().size()!=0) {
      SF_nominal = m_config->PileUpCustomScaleFactors()[0];
      SF_up = m_config->PileUpCustomScaleFactors()[1];
      SF_down = m_config->PileUpCustomScaleFactors()[2];
      ATH_MSG_INFO("Using custom Data Scale-Factors for pile-up reweighting");
      std::ostringstream oss;
      oss << "Nominal:" << SF_nominal << " up:"<< SF_up << " down:" << SF_down  << std::endl;
      ATH_MSG_INFO(oss.str());
    }

    top::check(asg::setProperty(pileupReweightingTool, "LumiCalcFiles", pileup_lumi_calc),
              "Failed to set pileup reweighting lumicalc files");
    // see [http://cern.ch/go/hx7d]
    top::check(asg::setProperty(pileupReweightingTool, "DataScaleFactor", static_cast<Float_t>(SF_nominal)),
              "Failed to set pileup reweighting data scale factor");
    top::check(asg::setProperty(pileupReweightingTool, "DataScaleFactorUP", SF_up),
              "Failed to set pileup reweighting data scale factor up");
    top::check(asg::setProperty(pileupReweightingTool, "DataScaleFactorDOWN", SF_down),
              "Failed to set pileup reweighting data scale factor down");
    top::check(pileupReweightingTool->initialize(),
              "Failed to initialize pileup reweighting tool");

    m_pileupReweightingTool = pileupReweightingTool;
  }
  return StatusCode::SUCCESS;
}


StatusCode OtherCPTools::setupPMGTools() {
  // Setup any PMG tools
  // Currently only a single tool for reweighting
  // Sherpa 2.2 V+jets samples based on the number of truth jets
  const std::string name = "PMGSherpa22VJetsWeightTool";
  PMGTools::PMGSherpa22VJetsWeightTool* tool = nullptr;
  if (asg::ToolStore::contains<PMGTools::PMGSherpa22VJetsWeightTool>(name)) {
    tool = asg::ToolStore::get<PMGTools::PMGSherpa22VJetsWeightTool>(name);
  } else {
    tool = new PMGTools::PMGSherpa22VJetsWeightTool(name);
    top::check(asg::setProperty(tool, "TruthJetContainer",
                                m_config->sgKeyTruthJets()),
                "Failed to set TruthJetContainer to " + name);
    top::check(tool->initialize(), "Failed to initialize " + name);
  }
  m_pmg_sherpa22_vjets_tool = tool;
  return StatusCode::SUCCESS;
}

}  // namespace top

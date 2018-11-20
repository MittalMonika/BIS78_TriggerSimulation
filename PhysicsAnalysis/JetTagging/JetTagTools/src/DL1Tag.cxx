/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////////
/// Name    : DL1Tag
/// Package : JetTagTools
/// Author  : Marie Lanfermann (Universite de Geneve), Dan Guest (UC Irvine)
/// Created : 2015
///
/// DESCRIPTION:
///
/// This class computes the Neural Network probability for being b-jet,
/// c-jet or uds-jet for a single event. The NN is constructed outside Atheana
/// using standard Data Science libraries, i.e. Keras (Theano or TensorFlow backend)
/// for the actual training.
///
/////////////////////////////////////////////////////////////////////////////

#include "GaudiKernel/IToolSvc.h"
#include "xAODTracking/TrackParticle.h"

#include "JetTagTools/DL1Tag.h"

#include "xAODBTagging/BTagging.h"
#include "xAODJet/Jet.h"

#include "TObjString.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <limits>
#include <cmath>

namespace{
  // copy the inputs, but with NaN replaced with default values. Don't
  // do anything with NaN values with no defined default.
  std::map<std::string, double> replace_nan_with_defaults(
							  const std::map<std::string, double>& inputs,
							  const std::map<std::string, double>& defaults);

  std::map<std::string, double> add_check_variables(
						const std::map<std::string, double>& inputs);

  // use this value when an output isn't defined by the NN
  const double missing_weight = 0.;
}

namespace Analysis {

  DL1Tag::DL1Tag(const std::string& name, const std::string& n, const IInterface* p):
    AthAlgTool(name, n,p),
    m_calibrationDirectory("DL1"),
    m_n_compute_errors(0),
    m_runModus("analysis")
  {
    // python binding
    declareInterface<IMultivariateJetTagger>(this);

    // access to JSON NN configuration file from COOL:
    declareProperty("calibration_directory", m_calibrationDirectory);
    declareProperty("forceDL1CalibrationAlias", m_forceDL1CalibrationAlias = true);
    declareProperty("DL1CalibAlias", m_DL1CalibAlias = "AntiKt4TopoEM");
    declareProperty("xAODBaseName", m_xAODBaseName);

    // global configuration
    declareProperty("Runmodus", m_runModus);
    declareProperty("LocalNNConfigurationFile", m_LocalNNConfigFile);
  }

  DL1Tag::~DL1Tag() {
  }

  StatusCode DL1Tag::initialize() {
    // prepare readKey for calibration data:
    ATH_CHECK(m_readKey.initialize());

    // Read in the configuration of the neural net for DL1:
    if (m_LocalNNConfigFile.size() != 0) { // retrieve map of NN config and default values from local JSON file
      std::ifstream nn_config_ifstream(m_LocalNNConfigFile);
      build_nn("local", nn_config_ifstream);
    }
    else { // done in condition algorithm
    }

    ATH_MSG_INFO(" Initialization of DL1Tag successful" );
    return StatusCode::SUCCESS;
  }


  void DL1Tag::build_nn(const std::string& jetauthor, std::istream& nn_config_istream) {
    if (m_NeuralNetworks.count(jetauthor)) {
      m_map_variables.erase(jetauthor);
      m_map_defaults.erase(jetauthor);
      m_NeuralNetworks.erase(jetauthor);
    }

    lwt::JSONConfig nn_config = lwt::parse_json(nn_config_istream);
    ATH_MSG_DEBUG("#BTAG# making NN with " << nn_config.layers.size() << " layers");

    if (!(std::find((nn_config.outputs).begin(), (nn_config.outputs).end(), "bottom") != (nn_config.outputs).end())) {
      ATH_MSG_WARNING( "#BTAG# b-tagger without b-tagging option 'bottom' - please check the NN output naming convention.");
    }

    m_NeuralNetworks.insert(std::make_pair(jetauthor, new lwt::LightweightNeuralNetwork(nn_config.inputs, nn_config.layers, nn_config.outputs)));
    m_map_variables.insert(std::make_pair(jetauthor, nn_config.inputs));
    m_map_defaults.insert(std::make_pair(jetauthor, nn_config.defaults));
  }

  void DL1Tag::load_calibration(const std::string& jetauthor) {
    SG::ReadCondHandle<JetTagCalibCondData> readCdo(m_readKey);
    lwt::JSONConfig nn_config = readCdo->retrieveDL1NN(m_calibrationDirectory , jetauthor);

    if (nn_config.layers.size() == 0){  //catch if no NN config was found
      std::string fuller_name = m_calibrationDirectory + "/" +
      jetauthor + "/net_configuration";
      throw std::logic_error("Cannot retrieve NN config build from string: " + fuller_name);
    }

    ATH_MSG_DEBUG("#BTAG# making NN with " << nn_config.layers.size() << " layers");

    if (!(std::find((nn_config.outputs).begin(), (nn_config.outputs).end(), "bottom") != (nn_config.outputs).end())) {
      ATH_MSG_WARNING( "#BTAG# b-tagger without b-tagging option 'bottom' - please check the NN output naming convention.");
    }
    auto lwNN = std::make_unique<lwt::LightweightNeuralNetwork>(nn_config.inputs, nn_config.layers, nn_config.outputs);
    m_NeuralNetworks.insert(std::make_pair(jetauthor, std::move(lwNN)));
    m_map_variables.insert(std::make_pair(jetauthor, nn_config.inputs));
    m_map_defaults.insert(std::make_pair(jetauthor, nn_config.defaults));

  }

  StatusCode DL1Tag::finalize() { // all taken care of in destructor
    if (m_n_compute_errors > 0) {
      ATH_MSG_WARNING("Neural network was unable to compute. Number of errors: "+ std::to_string(m_n_compute_errors));
    }

    ATH_MSG_INFO(" #BTAG# Finalization of DL1Tag successfull" );
    return StatusCode::SUCCESS;
  }

  void DL1Tag::fill_outputs(xAOD::BTagging *BTag, var_map outputs) {
    if(m_runModus=="analysis") {
      if (!outputs.count("charm")) {
	BTag->setVariable<double>(m_xAODBaseName, "pc", missing_weight);
      }
      else if (outputs.count("charm")) {
	BTag->setVariable<double>(m_xAODBaseName, "pc", outputs.at("charm"));
      }

      if (!outputs.count("light") && !outputs.count("bottom")) {
	BTag->setVariable<double>(m_xAODBaseName, "pu", missing_weight);
      }
      else if (outputs.count("light")) {
	BTag->setVariable<double>(m_xAODBaseName, "pu", outputs.at("light"));
      }

      if (outputs.count("bottom")) {
	BTag->setVariable<double>(m_xAODBaseName, "pb", outputs.at("bottom"));
	if (!outputs.count("light")) {
	  BTag->setVariable<double>(m_xAODBaseName, "pu", 1.-outputs.at("bottom"));
	}
      }
      else if (!outputs.count("bottom")) {
	BTag->setVariable<double>(m_xAODBaseName, "pb", missing_weight);
	ATH_MSG_WARNING( "#BTAG# b-tagger without b-tagging option: " << outputs.at("bottom"));
      }
      ATH_MSG_DEBUG("#BTAG# NN output b: " << outputs.at("bottom") <<
		    " c: " << outputs.at("charm") << " u: " <<  outputs.at("light"));
    }
  }

  void DL1Tag::assignProbability(xAOD::BTagging *BTag,
				 const var_map &inputs,
				 const std::string& assigned_jet_author){

    std::string jetauthor = assigned_jet_author;

    if (m_LocalNNConfigFile.size()==0) {
      if (m_forceDL1CalibrationAlias) {
	jetauthor = m_DL1CalibAlias;
      }
      ATH_MSG_DEBUG("#BTAG# Jet author set (via forceDL1CalibrationAlias) to: " << jetauthor );
      if ( jetauthor.empty() ) {
	ATH_MSG_WARNING(" #BTAG# Hypothesis or jetauthor is empty."
			" No likelihood value given back. ");
      }
      try {
	load_calibration(jetauthor);
      } catch (std::exception& e) {
	ATH_MSG_WARNING(
	  "problem loading calibration for " + jetauthor +
	  " (" + e.what() + "), skipping this jet");
	return;
      }
    }
    else {
      jetauthor = "local";
    }

    if(!m_NeuralNetworks.count(jetauthor)) {
      ATH_MSG_WARNING("NN not loaded correctly.");
      return;
    }

    var_map complete_inputs = add_check_variables(inputs);
    var_map cleaned_inputs = replace_nan_with_defaults(complete_inputs, m_map_defaults.at(jetauthor));

    for (const auto& var: m_map_variables.at(jetauthor)) {
      if (cleaned_inputs.count(var.name) && std::isnan(cleaned_inputs.at(var.name))) {
      ATH_MSG_WARNING( "#BTAG# 'nan' input for variable " + var.name + " --> will result in 'nan' classification output. Check NN configuration file for default settings.");
      }
    }

    var_map outputs;
    try {
      outputs = m_NeuralNetworks.at(jetauthor)->compute(cleaned_inputs);
    } catch (lwt::NNEvaluationException& e) {
      ATH_MSG_WARNING("Can't compute outputs, probably missing input values");
      m_n_compute_errors++;
      return;
    }

    fill_outputs(BTag, outputs);
  } //end assign_probability
}

namespace {
  std::map<std::string, double> replace_nan_with_defaults(
							  const std::map<std::string, double>& inputs,
							  const std::map<std::string, double>& defaults) {
    // return a new map with the NaN values replaced where possible.
    std::map<std::string, double> outputs;

    // loop over all inputs and create map of cleaned inputs
    for (const auto& in: inputs) {
      if (std::isnan(in.second) && defaults.count(in.first)) {
	outputs[in.first] = defaults.at(in.first);
      } else {
	outputs[in.first] = in.second;
      }
    }
    return outputs;
  } // end replace_nan_with_defaults

  std::map<std::string, double> add_check_variables(
						const std::map<std::string, double>& inputs) {
    // return a new map with additional binary variables for each input variable
    std::map<std::string, double> complete_inputs;

    for (const auto& in: inputs) {
      // copy everything from the inputs map
      complete_inputs.insert(std::make_pair(in.first, in.second));
      // add binary check variables
      if (std::isnan(in.second)) {
	complete_inputs.insert(std::make_pair(in.first + "_check", 1.));
      }
      else {
	complete_inputs.insert(std::make_pair(in.first + "_check", 0.));
      }
    }
    return complete_inputs;
  }// end add_check_variables
} // end namespace

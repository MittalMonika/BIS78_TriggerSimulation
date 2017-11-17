// for editors : this file is -*- C++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BOOSTEDJETSTAGGERS_HBBTaggerDNN_H_
#define BOOSTEDJETSTAGGERS_HBBTaggerDNN_H_

#include "AsgTools/AsgTool.h"
#include "JetInterface/IJetSelector.h"

namespace lwt {
  class LightweightGraph;
}

class HbbTaggerDNN :   public asg::AsgTool ,
                       virtual public IJetSelector
{
  ASG_TOOL_CLASS1(HbbTaggerDNN, IJetSelector )
public:
  HbbTaggerDNN(const std::string &name);
  ~HbbTaggerDNN();
  StatusCode initialize();
  StatusCode finalize();
  virtual int keep(const xAOD::Jet& jet) const;
  double getScore(const xAOD::Jet& jet) const;
  void decorate(const xAOD::Jet& jet) const;

protected:
   // the location where CVMFS files live
  std::string m_configFile;
  std::unique_ptr<lwt::LightweightGraph> m_lwnn;

  // threshold to cut on for keep()
  // default is INFINITY (i.e. tag no jets)
  double m_tag_threshold;

  // internal stuff to keep track of the output node for the NN
  std::string m_output_node_name;
  std::string m_output_value_name;

  // if no decoration name is given we look it up from the
  // configuration output name
  std::string m_decoration_name;
  SG::AuxElement::Decorator<float> m_decorator;

};

#endif

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

#ifndef ELECTRONPHOTONSELECTORTOOLS_ASGELECTRONCHARGEIDSELECTORTOOL_H
#define ELECTRONPHOTONSELECTORTOOLS_ASGELECTRONCHARGEIDSELECTORTOOL_H
// Atlas includes
#include "AsgTools/AsgTool.h"
#include "PATCore/AcceptData.h"
#include "MVAUtils/BDT.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "ElectronPhotonSelectorTools/AsgElectronLikelihoodTool.h"
#include <unordered_map>

class AsgElectronChargeIDSelectorTool : public asg::AsgTool, 
					virtual public IAsgElectronLikelihoodTool
{
  ASG_TOOL_CLASS2(AsgElectronChargeIDSelectorTool, IAsgElectronLikelihoodTool, CP::ISelectionTool)

public:
  /** Standard constructor */
  AsgElectronChargeIDSelectorTool( const std::string myname);


  /** Standard destructor */
  virtual ~AsgElectronChargeIDSelectorTool();
public:
  /** Gaudi Service Interface method implementations */
  virtual StatusCode initialize();

  /** Gaudi Service Interface method implementations */
  virtual StatusCode finalize();

  /** Method to get the plain AcceptInfo.
      This is needed so that one can already get the AcceptInfo 
      and query what cuts are defined before the first object 
      is passed to the tool. */
  const asg::AcceptInfo& getAcceptInfo() const { return m_acceptInfo; }

    // Main methods for IAsgSelectorTool interface
  /** The main accept method: using the generic interface */
  asg::AcceptData accept( const xAOD::IParticle* part ) const;

  /** The main accept method: the actual cuts are applied here */
  asg::AcceptData accept( const xAOD::Electron* eg ) const {
    return accept (eg, -99); // mu = -99 as input will force accept to grab the pileup variable from the xAOD object
  }

  /** The main accept method: the actual cuts are applied here */
  asg::AcceptData accept( const xAOD::Egamma* eg ) const {
    return accept (eg, -99); // mu = -99 as input will force accept to grab the pileup variable from the xAOD object
  }

  /** The main accept method: in case mu not in EventInfo online */
  asg::AcceptData accept( const xAOD::Electron* eg, double mu ) const;

  /** The main accept method: in case mu not in EventInfo online */
  asg::AcceptData accept( const xAOD::Egamma* eg, double mu ) const;
  
  // Main methods for IAsgCalculatorTool interface

  /** The main result method: the actual likelihood is calculated here */
  double calculate( const xAOD::IParticle* part ) const;

  /** The main result method: the actual likelihood is calculated here */
  double calculate( const xAOD::Electron* eg ) const {
    return calculate (eg, -99); // mu = -99 as input will force accept to grab the pileup variable from the xAOD object
  }

  /** The main result method: the actual likelihood is calculated here */
  double calculate( const xAOD::Egamma* eg ) const {
    return calculate (eg, -99); // mu = -99 as input will force accept to grab the pileup variable from the xAOD object
  }

  /** The main result method: the actual likelihood is calculated here */
  double calculate( const xAOD::Electron* eg, double mu ) const;

  /** The main result method: the actual likelihood is calculated here */
  double calculate( const xAOD::Egamma* eg, double mu ) const; 


  /// Get the name of the current operating point
  inline virtual std::string getOperatingPointName( ) const
  // std::string AsgElectronChargeIDSelectorTool::getOperatingPointName() const
  {    return m_WorkingPoint;  };
  
  // Private methods
private:
  /// Get the number of primary vertices
  unsigned int getNPrimVertices() const;

  //BDT instances for different ID operating points (Tight, Medium, Loose) and the vector corresponds to n-fold
  const unsigned m_nfold  =1;
  const unsigned m_bdt_index=0;
  std::vector<MVAUtils::BDT*> m_v_bdts;

  TString m_pid_name;
  float m_cutOnBDT;

  asg::AcceptInfo m_acceptInfo;
  int m_cutPosition_bdt;

  /** Working Point */
  std::string m_WorkingPoint;

  /// Whether to use the PV (not available for trigger)
  bool m_usePVCont;

  /// defualt nPV (when not using PVCont)
  unsigned int m_nPVdefault;

  /// The primary vertex container name
  std::string m_primVtxContName;

  /// The input ROOT file name that holds the PDFs
  std::string m_trainingFile;

  /// Flag for calo only LH
  //bool m_caloOnly;

  // BDT input variables
  mutable std::unordered_map<std::string,float> m_map_inputs;
  mutable std::vector<std::string> m_inputVars;

  bool SetVariableRefs(std::vector<std::string> inputs, std::vector<float*> &inputPointers);

}; // End: class definition





#endif


/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: EgammaObjectCollectionMaker.h 806381 2017-06-09 14:58:44Z iconnell $
#ifndef ANALYSISTOP_TOPSYSTEMATICOBJECTMAKER_EGAMMAOBJECTCOLLECTIONMAKER_H
#define ANALYSISTOP_TOPSYSTEMATICOBJECTMAKER_EGAMMAOBJECTCOLLECTIONMAKER_H

/**
  * @author John Morris <john.morris@cern.ch>
  * 
  * @brief EgammaObjectCollectionMaker
  *   Makes all systematic variations of photons and electrons
  * 
  * $Revision: 806381 $
  * $Date: 2017-06-09 15:58:44 +0100 (Fri, 09 Jun 2017) $
  * 
  **/ 

// system include(s):
#include <memory>
#include <set>
#include <list>

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"

// Systematic include(s):
#include "PATInterfaces/SystematicSet.h"

// CP Tool include(s):
#include "ElectronPhotonFourMomentumCorrection/IEgammaCalibrationAndSmearingTool.h"
#include "ElectronPhotonShowerShapeFudgeTool/IElectronPhotonShowerShapeFudgeTool.h"
#include "IsolationSelection/IIsolationSelectionTool.h"
#include "IsolationCorrections/IIsolationCorrectionTool.h"

// Forward declaration(s):
namespace top{
  class TopConfig;
}

namespace top{

  class EgammaObjectCollectionMaker final : public asg::AsgTool {
    public:
      explicit EgammaObjectCollectionMaker( const std::string& name );
      virtual ~EgammaObjectCollectionMaker(){}

      // Delete Standard constructors
      EgammaObjectCollectionMaker(const EgammaObjectCollectionMaker& rhs) = delete;
      EgammaObjectCollectionMaker(EgammaObjectCollectionMaker&& rhs) = delete;
      EgammaObjectCollectionMaker& operator=(const EgammaObjectCollectionMaker& rhs) = delete;

      StatusCode initialize();

      StatusCode executePhotons();
      StatusCode executeElectrons();

      StatusCode printoutPhotons();
      StatusCode printoutElectrons();

      // return specific Systematic
      inline virtual const std::list<CP::SystematicSet>& specifiedSystematicsPhotons() const {return m_specifiedSystematicsPhotons;}
      inline virtual const std::list<CP::SystematicSet>& specifiedSystematicsElectrons() const {return m_specifiedSystematicsElectrons;}
  
      // return all recommendedSystematics
      inline const std::list<CP::SystematicSet>& recommendedSystematicsPhotons() const {return m_recommendedSystematicsPhotons;}
      inline const std::list<CP::SystematicSet>& recommendedSystematicsElectrons() const {return m_recommendedSystematicsElectrons;}

    protected:
      // specify Systematic
      virtual void specifiedSystematicsPhotons  ( const std::set<std::string>& specifiedSystematics );
      virtual void specifiedSystematicsElectrons( const std::set<std::string>& specifiedSystematics );

    private:
      std::shared_ptr<top::TopConfig> m_config;

      std::list<CP::SystematicSet> m_specifiedSystematicsPhotons;
      std::list<CP::SystematicSet> m_specifiedSystematicsElectrons;

      std::list<CP::SystematicSet> m_recommendedSystematicsPhotons;
      std::list<CP::SystematicSet> m_recommendedSystematicsElectrons;

      ToolHandle<CP::IEgammaCalibrationAndSmearingTool> m_calibrationTool;
      ToolHandle<IElectronPhotonShowerShapeFudgeTool> m_photonFudgeTool;
      
      ///-- Isolation --///
      ToolHandle<CP::IIsolationSelectionTool> m_isolationTool_LooseTrackOnly;
      ToolHandle<CP::IIsolationSelectionTool> m_isolationTool_Loose;
      ToolHandle<CP::IIsolationSelectionTool> m_isolationTool_Gradient;
      ToolHandle<CP::IIsolationSelectionTool> m_isolationTool_GradientLoose;     
      ToolHandle<CP::IIsolationSelectionTool> m_isolationTool_FixedCutTight;
      ToolHandle<CP::IIsolationSelectionTool> m_isolationTool_FixedCutTightTrackOnly;
      ToolHandle<CP::IIsolationSelectionTool> m_isolationTool_FixedCutTightCaloOnly;
      ToolHandle<CP::IIsolationSelectionTool> m_isolationTool_FixedCutLoose;
      
      ToolHandle<CP::IIsolationCorrectionTool> m_isolationCorr;

      // Flag for applying calibration to objects
      bool calibrateElectrons;
      bool calibratePhotons;

  };
} // namespace
#endif

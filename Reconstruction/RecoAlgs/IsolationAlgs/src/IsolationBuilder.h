///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// IsolationBuilder.h 
// Header file for class IsolationBuilder
/////////////////////////////////////////////////////////////////// 
#ifndef ISOLATIONALGS_ISOLATIONBUILDER_H
#define ISOLATIONALGS_ISOLATIONBUILDER_H 1

// to allow vector<vector<int>> properties
#include "VectorVectorIntParser.h"

// STL includes
#include <string>
#include <set>
#include <vector>
#include <utility>

// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/AuxElement.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODPrimitives/IsolationType.h"
#include "xAODPrimitives/IsolationFlavour.h"
#include "RecoToolInterfaces/IsolationCommon.h"

#include "IsolationCorrections/IIsolationCorrectionTool.h"
#include "xAODPrimitives/IsolationConeSize.h"
#include "xAODPrimitives/IsolationHelpers.h"
#include "xAODPrimitives/IsolationFlavour.h"
#include "xAODBase/IParticleContainer.h"

#include "RecoToolInterfaces/ICaloCellIsolationTool.h"
#include "RecoToolInterfaces/ICaloTopoClusterIsolationTool.h"
#include "RecoToolInterfaces/ITrackIsolationTool.h"
#include "RecoToolInterfaces/INeutralEFlowIsolationTool.h"

#include "CaloEvent/CaloCellContainer.h"


class IsolationBuilder
  : public ::AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  IsolationBuilder( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~IsolationBuilder(); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize();
  virtual StatusCode  execute();
  virtual StatusCode  finalize();

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  IsolationBuilder();

  /// Containers (Is it best to make them as strings? Used by multiple handles)
  Gaudi::Property <std::string> m_ElectronContainerName {this,
      "ElectronCollectionContainerName", "Electrons"};

  Gaudi::Property <std::string> m_FwdElectronContainerName {this,
      "FwdElectronCollectionContainerName", "ForwardElectrons"};

  Gaudi::Property <std::string> m_PhotonContainerName {this,
      "PhotonCollectionContainerName", "Photons"};

  Gaudi::Property <std::string> m_MuonContainerName {this,
      "MuonCollectionContainerName", "Muons"};

  /** @brief Tool for cell isolation calculation */
  ToolHandle<xAOD::ICaloCellIsolationTool> m_cellIsolationTool {this, 
      "CaloCellIsolationTool", "", "Handle of the calo cell IsolationTool"};

  /** @brief Cell container*/
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey {this,
            "CellCollectionName", "AllCalo", "Name of container which contain calo cells"};
  const CaloCellContainer* m_cellColl = nullptr;


  /** @brief Tool for topo isolation calculation */
  ToolHandle<xAOD::ICaloTopoClusterIsolationTool> m_topoIsolationTool {this,
      "CaloTopoIsolationTool", "", "Handle of the calo topo IsolationTool"};

  /** @brief Tool for neutral pflow isolation calculation */
  ToolHandle<xAOD::INeutralEFlowIsolationTool> m_pflowIsolationTool {this,
      "PFlowIsolationTool", "", "Handle of the pflow IsolationTool"};

  /** @brief Tool for neutral pflow isolation calculation */
  ToolHandle<xAOD::ITrackIsolationTool> m_trackIsolationTool {this,
      "TrackIsolationTool", "", "Handle of the track IsolationTool"};

  // FIXME:  need to add the electron bremAssoc stuff
  Gaudi::Property<bool> m_useBremAssoc {this, 
      "useBremAssoc", true, "use track to track assoc after brem"};

  Gaudi::Property<bool> m_allTrackRemoval {this, 
      "AllTrackRemoval", true};

  /** @brief Isolation types */
  Gaudi::Property<std::vector<std::vector<int> > > m_elisoInts {this,
      "ElIsoTypes", {}, 
      "The isolation types to do for electrons: vector of vector of enum type Iso::IsolationType"};

  Gaudi::Property<std::vector<std::vector<int> > > m_elcorInts {this,
      "ElCorTypes", {}, 
      "The correction types to do for electron iso: vector of vector of enum type Iso::IsolationCalo/TrackCorrection"};

  Gaudi::Property<std::vector<std::vector<int> > > m_elcorIntsExtra {this,
      "ElCorTypesExtra", {}, 
      "The extra correction types to store but not apply for electrons"};

  /** @brief Isolation types (for the alg. properties, only vector<vector<double>> available */
  Gaudi::Property<std::vector<std::vector<int> > > m_phisoInts {this,
      "PhIsoTypes", {}, 
      "The isolation types to do for photons: vector of vector of enum type Iso::IsolationType"};

  Gaudi::Property<std::vector<std::vector<int> > > m_phcorInts {this,
      "PhCorTypes", {}, 
      "The correction types to do for photons iso: vector of vector of enum type Iso::IsolationCalo/TrackCorrection"};

  Gaudi::Property<std::vector<std::vector<int> > > m_phcorIntsExtra {this,
      "PhCorTypesExtra", {}, 
      "The extra correction types to store but not apply for photons"};

  Gaudi::Property<std::vector<std::vector<int> > > m_muisoInts {this,
      "MuIsoTypes", {}, 
      "The isolation types to do for Muons : vector of vector of enum type Iso::IsolationType"};

  Gaudi::Property<std::vector<std::vector<int> > > m_mucorInts {this,
      "MuCorTypes", {}, 
      "The correction types to do for Muon iso: vector of vector of enum type Iso::IsolationCalo/TrackCorrection"};

  Gaudi::Property<std::vector<std::vector<int> > > m_mucorIntsExtra {this,
      "MuCorTypesExtra", {}, 
      "The extra correction types to store but not apply for muons"};

  Gaudi::Property<std::vector<std::vector<int> > > m_feisoInts {this,
      "FeIsoTypes", {},
      "The isolation types to do for forward electron: vector of vector of enum type Iso::IsolationType"};

  Gaudi::Property<std::vector<std::vector<int> > > m_fecorInts {this,
      "FeCorTypes", {}, 
      "The correction types to do for forward electron iso: vector of vector of enum type Iso::IsolationCalo/TrackCorrection"};

  Gaudi::Property<std::vector<std::vector<int> > > m_fecorIntsExtra {this,
      "FeCorTypesExtra", {}, 
      "The extra correction types to store but not apply for forward electrons"};

  struct CaloIsoHelpKey {
    std::vector<SG::WriteDecorHandleKey<xAOD::IParticleContainer> > isoDeco;
    std::map<xAOD::Iso::IsolationCaloCorrection, 
	     SG::WriteDecorHandleKey<xAOD::IParticleContainer> > coreCorDeco;
    std::map<xAOD::Iso::IsolationCaloCorrection, 
	     std::vector<SG::WriteDecorHandleKey<xAOD::IParticleContainer> > > noncoreCorDeco;
    std::vector<xAOD::Iso::IsolationType> isoTypes;
    xAOD::CaloCorrection CorrList;
    // xAOD::CaloCorrection CorrListExtra; // should ideally pass this, but not possible yet
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> corrBitsetDeco;
  };

  std::vector<std::pair<xAOD::Iso::IsolationFlavour,CaloIsoHelpKey> > m_elCaloIso;
  std::vector<std::pair<xAOD::Iso::IsolationFlavour,CaloIsoHelpKey> > m_phCaloIso;
  std::vector<std::pair<xAOD::Iso::IsolationFlavour,CaloIsoHelpKey> > m_feCaloIso;
  std::vector<std::pair<xAOD::Iso::IsolationFlavour,CaloIsoHelpKey> > m_muCaloIso;

  struct TrackIsoHelpKey {
    std::vector<SG::WriteDecorHandleKey<xAOD::IParticleContainer> > isoDeco;
    std::vector<SG::WriteDecorHandleKey<xAOD::IParticleContainer> > isoDecoV;
    std::map<xAOD::Iso::IsolationTrackCorrection, 
	     SG::WriteDecorHandleKey<xAOD::IParticleContainer> > coreCorDeco;
    std::vector<xAOD::Iso::IsolationType> isoTypes;
    xAOD::TrackCorrection CorrList;
    // xAOD::TrackCorrection CorrListExtra; // should ideally pass this, but not possible yet
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> corrBitsetDeco;
  };

  std::vector<std::pair<xAOD::Iso::IsolationFlavour,TrackIsoHelpKey> > m_elTrackIso;
  std::vector<std::pair<xAOD::Iso::IsolationFlavour,TrackIsoHelpKey> > m_phTrackIso;
  std::vector<std::pair<xAOD::Iso::IsolationFlavour,TrackIsoHelpKey> > m_muTrackIso;

  struct CaloIsoHelpHandles {

    CaloIsoHelpHandles(const CaloIsoHelpKey& keys);

    std::vector<SG::WriteDecorHandle<xAOD::IParticleContainer, float> > isoDeco;
    std::map<xAOD::Iso::IsolationCaloCorrection, 
	     SG::WriteDecorHandle<xAOD::IParticleContainer, float> > coreCorDeco;
    std::map<xAOD::Iso::IsolationCaloCorrection, 
	     std::vector<SG::WriteDecorHandle<xAOD::IParticleContainer, float> > > noncoreCorDeco;
    SG::WriteDecorHandle<xAOD::IParticleContainer, uint32_t> corrBitsetDeco;
  };


  struct TrackIsoHelpHandles {

    TrackIsoHelpHandles(const TrackIsoHelpKey& keys);

    std::vector<SG::WriteDecorHandle<xAOD::IParticleContainer, float> > isoDeco;
    std::vector<SG::WriteDecorHandle<xAOD::IParticleContainer, float> > isoDecoV;
    std::map<xAOD::Iso::IsolationTrackCorrection, 
	     SG::WriteDecorHandle<xAOD::IParticleContainer, float> > coreCorDeco;
    SG::WriteDecorHandle<xAOD::IParticleContainer, uint32_t> corrBitsetDeco;
  };


  // for the time being, only mu vs eg, no separation in eg
  Gaudi::Property<std::string> m_customConfigEl {this,
      "CustomConfigurationNameEl", "", 
      "use a custom configuration for electron"};

  Gaudi::Property<std::string> m_customConfigPh {this,
      "CustomConfigurationNamePh", "", 
      "use a custom configuration for photon"};

  Gaudi::Property<std::string> m_customConfigFwd {this,
      "CustomConfigurationNameFwd", "", 
      "use a custom configuration for forward electron"};

  Gaudi::Property<std::string> m_customConfigMu {this,
      "CustomConfigurationNameMu", "",
      "use a custom configuration for muon"}; 


  static bool isCoreCor(xAOD::Iso::IsolationCaloCorrection corr);

  StatusCode initializeIso(std::set<xAOD::Iso::IsolationFlavour>& runIsoType, // out
			   std::vector<std::pair<xAOD::Iso::IsolationFlavour,CaloIsoHelpKey > >* caloIsoMap, // out
			   std::vector<std::pair<xAOD::Iso::IsolationFlavour,TrackIsoHelpKey > >* trackIsoMap, // out
			   const std::string& containerName,
			   const std::vector<std::vector<int> >& isoInts,
			   const std::vector<std::vector<int> >& corInts,
			   const std::vector<std::vector<int> >& corIntsExtra,
			   const std::string& customConfig);

  StatusCode executeCaloIso(const std::vector<std::pair<xAOD::Iso::IsolationFlavour,CaloIsoHelpKey> >& caloIsoMap);

  StatusCode executeTrackIso(const std::vector<std::pair<xAOD::Iso::IsolationFlavour,TrackIsoHelpKey> >& trackIsoMap);
  
  
}; 


#endif //> !ISOLATIONALGS_ISOLATIONBUILDER_H

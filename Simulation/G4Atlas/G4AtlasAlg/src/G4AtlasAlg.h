/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASALG_G4AtlasAlg_H
#define G4ATLASALG_G4AtlasAlg_H

// Base class header
#include "AthenaBaseComps/AthAlgorithm.h"

// STL headers
#include <map>
#include <string>

// Gaudi headers
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// Athena headers
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "G4AtlasInterfaces/IUserActionSvc.h"
#include "G4AtlasInterfaces/IDetectorGeometrySvc.h"
#include "G4AtlasInterfaces/ISensitiveDetectorMasterTool.h"
#include "G4AtlasInterfaces/IFastSimulationMasterTool.h"
#include "G4AtlasInterfaces/IPhysicsListTool.h"
#include "G4AtlasInterfaces/IG4AtlasSvc.h"
#include "GeneratorObjects/McEventCollection.h"

// ISF includes
#include "ISF_Interfaces/ITruthSvc.h"
#include "ISF_Interfaces/IGeoIDSvc.h"
#include "ISF_Interfaces/IInputConverter.h"

/// @class G4AtlasAlg
/// @brief Primary Athena algorithm for ATLAS simulation.
///
/// During initialization, this class sets up several things, including:
/// - the ATLAS (master) run manager
/// - physics list assignment to G4
/// - detector construction (currently FADS::FadsDetectorConstruction)
///
/// During the event loop, it handles processing of the event by
/// invoking the (worker) run manager.
///
class G4AtlasAlg : public AthAlgorithm
{

public:

  /// Standard algorithm constructor
  G4AtlasAlg(const std::string& name, ISvcLocator* pSvcLocator);

  /// Virtual destructor
  virtual ~G4AtlasAlg() {  };

  /// this Alg is Clonable (for AthenaMT)
  bool isClonable() const override { return true; }

  /// @brief Initialize the algorithm.
  ///
  /// Here we setup several things for simulation, including:
  /// - force intialization of the UserActionSvc
  /// - apply custom G4 UI commands (like custom physics list)
  /// - configure the particle generator and random generator svc
  StatusCode initialize() override;

  /// Finalize the algorithm and invoke G4 run termination.
  StatusCode finalize() override;

  /// @brief Simulate one Athena event.
  StatusCode execute() override;

  /// Poorly named possibly unused method which sets some verbosities.
  void initializeG4();

  /// G4 initialization called only by the first alg instance.
  /// This is done (for now) because we get multiple alg instances in hive.
  void initializeOnce();

  /// G4 finalization called only by the first alg instance.
  /// This is done (for now) because we get multiple alg instances in hive.
  void finalizeOnce();

private:

  /// This command prints a message about a G4Command depending on its returnCode
  void commandLog(int returnCode, const std::string& commandString) const;

  /// Releases the GeoModel geometry from memory once it has been used
  /// to build the G4 geometry and is no-longer required
  void releaseGeoModel();

  /// @name Configurable Properties
  /// @{
  bool m_killAbortedEvents{false};
  bool m_flagAbortedEvents{false};
  SG::ReadHandleKey<McEventCollection>    m_inputTruthCollectionKey{this, "InputTruthCollection", "BeamTruthEvent", "Input hard scatter collection"}; //!< input hard scatter collection
  SG::WriteHandleKey<McEventCollection>   m_outputTruthCollectionKey{this, "OutputTruthCollection", "TruthEvent", "Output hard scatter truth collection"};//!< output hard scatter truth collection
  /// Central Truth Service
  ServiceHandle<ISF::ITruthSvc> m_truthRecordSvc{this, "TruthRecordService", "ISF_TruthRecordSvc", ""};
  /// Geo ID Service
  ServiceHandle<ISF::IGeoIDSvc> m_geoIDSvc{this, "GeoIDSvc", "ISF_GeoIDSvc", ""};

  /// Verbosity settings for Geant4
  std::map<std::string,std::string> m_verbosities;
  /// @}

  /// @name Configurable Properties (common with TransportTool)
  /// @{
  std::string m_libList{""};
  std::string m_physList{""};
  std::string m_fieldMap{""};
  std::string m_rndmGen{"athena"};
  bool m_releaseGeoModel{true};
  bool m_recordFlux{false};
  /// Commands to send to the G4 UI
  std::vector<std::string> m_g4commands;
  /// Activate multi-threading configuration
  bool m_useMT{false};
  /// Random number service
  ServiceHandle<IAthRNGSvc> m_rndmGenSvc{this, "AtRndmGenSvc", "AthRNGSvc", ""}; // TODO rename property
  /// G4Atlas Service - handles G4 initialization
  ServiceHandle<IG4AtlasSvc> m_g4atlasSvc{this, "G4AtlasSvc", "G4AtlasSvc", ""};
  /// User Action Service
  ServiceHandle<G4UA::IUserActionSvc> m_userActionSvc{this, "UserActionSvc", "G4UA::UserActionSvc", ""};
  /// Detector Geometry Service (builds G4 Geometry)
  ServiceHandle<IDetectorGeometrySvc> m_detGeoSvc{this, "DetGeoSvc", "DetectorGeometrySvc", ""};
  /// Service to convert ISF_Particles into a G4Event
  ServiceHandle<ISF::IInputConverter> m_inputConverter{this, "InputConverter", "ISF_InputConverter", ""};
  /// Physics List Tool
  PublicToolHandle<IPhysicsListTool> m_physListTool{this, "PhysicsListTool", "PhysicsListToolBase", ""};
  /// Sensitive Detector Master Tool
  PublicToolHandle<ISensitiveDetectorMasterTool> m_senDetTool{this, "SenDetMasterTool", "SensitiveDetectorMasterTool", ""};
  /// Fast Simulation Master Tool
  PublicToolHandle<IFastSimulationMasterTool> m_fastSimTool{this, "FastSimMasterTool", "FastSimulationMasterTool", ""};
  /// @}

};

#endif// G4ATLASALG_G4AtlasAlg_H

################################################################################
# Package: D2PDMaker
################################################################################

# Declare the package name:
atlas_subdir( D2PDMaker )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          Calorimeter/CaloEvent
                          Control/AthenaBaseComps
                          Control/AthenaKernel
                          Control/CLIDSvc
                          Control/DataModel
                          Control/Navigation
                          Control/SGTools
                          Control/StoreGate
                          Event/EventKernel
                          Event/NavFourMom
                          GaudiKernel
                          Generators/GeneratorObjects
                          PhysicsAnalysis/AnalysisCommon/AnalysisUtils
                          PhysicsAnalysis/AnalysisCommon/PATCore
                          PhysicsAnalysis/AnalysisCommon/ParticleEvent
                          PhysicsAnalysis/AssociationBuilder/AssociationKernel
                          PhysicsAnalysis/TruthParticleID/McParticleEvent
                          Reconstruction/Jet/JetEvent
                          Reconstruction/MuonIdentification/muonEvent
                          Reconstruction/Particle
                          Reconstruction/egamma/egammaEvent
                          Reconstruction/tauEvent
                          Tracking/TrkEvent/VxVertex
                          PRIVATE
                          DetectorDescription/GeoPrimitives
                          Generators/GenAnalysisTools/TruthHelper
                          Reconstruction/MissingETPerformance
                          Tracking/TrkEvent/TrkTrackSummary )

# External dependencies:
find_package( Boost COMPONENTS filesystem thread system )
find_package( CLHEP )
find_package( Eigen )
find_package( HepMC )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( D2PDMaker
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${HEPMC_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} ${HEPMC_LIBRARIES} ${EIGEN_LIBRARIES} CaloEvent AthenaBaseComps AthenaKernel DataModel Navigation SGTools StoreGateLib SGtests EventKernel NavFourMom GaudiKernel GeneratorObjects AnalysisUtilsLib PATCoreLib ParticleEvent AssociationKernel McParticleEvent JetEvent muonEvent Particle egammaEvent tauEvent VxVertex GeoPrimitives TruthHelper MissingETPerformanceLib TrkTrackSummary )

# Install files from the package:
atlas_install_headers( D2PDMaker )
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py )
atlas_install_runtime( xml/*.xml )


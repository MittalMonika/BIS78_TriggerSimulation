################################################################################
# Package: TrkTruthTrackTools
################################################################################

# Declare the package name:
atlas_subdir( TrkTruthTrackTools )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PRIVATE
                          Control/AthenaBaseComps
                          DetectorDescription/AtlasDetDescr
                          GaudiKernel
                          Tracking/TrkDetDescr/TrkSurfaces
                          Tracking/TrkEvent/TrkEventUtils
                          Tracking/TrkEvent/TrkPrepRawData
                          Tracking/TrkEvent/TrkRIO_OnTrack
                          Tracking/TrkEvent/TrkTrack
                          Tracking/TrkEvent/TrkTruthData
                          Tracking/TrkExtrapolation/TrkExInterfaces
                          Tracking/TrkFitter/TrkFitterInterfaces
                          Tracking/TrkTools/TrkToolInterfaces
                          Tracking/TrkTruthTracks/TrkTruthTrackInterfaces )

# External dependencies:
find_package( HepMC )
find_package( HepPDT )

# Component(s) in the package:
atlas_add_component( TrkTruthTrackTools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${HEPPDT_INCLUDE_DIRS} ${HEPMC_INCLUDE_DIRS}
                     LINK_LIBRARIES ${HEPPDT_LIBRARIES} ${HEPMC_LIBRARIES} AthenaBaseComps AtlasDetDescr GaudiKernel TrkSurfaces TrkEventUtils TrkPrepRawData TrkRIO_OnTrack TrkTrack TrkTruthData TrkExInterfaces TrkFitterInterfaces TrkToolInterfaces )


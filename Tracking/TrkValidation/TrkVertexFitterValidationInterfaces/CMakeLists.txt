################################################################################
# Package: TrkVertexFitterValidationInterfaces
################################################################################

# Declare the package name:
atlas_subdir( TrkVertexFitterValidationInterfaces )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          GaudiKernel
                          Tracking/TrkEvent/TrkTrack
                          Tracking/TrkEvent/TrkTruthData
                          Tracking/TrkEvent/VxVertex )

# External dependencies:
find_package( HepMC )

# Install files from the package:
atlas_install_headers( TrkVertexFitterValidationInterfaces )


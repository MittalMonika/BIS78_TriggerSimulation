################################################################################
# Package: STGC_Overlay
################################################################################

# Declare the package name:
atlas_subdir( STGC_Overlay )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          GaudiKernel
                          MuonSpectrometer/MuonDigitContainer
                          MuonSpectrometer/MuonOverlay/MuonOverlayBase
                          PRIVATE
                          Control/StoreGate
                          MuonSpectrometer/MuonDigitization/MuonDigToolInterfaces
                          MuonSpectrometer/MuonIdHelpers )

# Component(s) in the package:
atlas_add_component( STGC_Overlay
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES GaudiKernel MuonDigitContainer MuonOverlayBase StoreGateLib SGtests MuonIdHelpersLib )

# Install files from the package:
atlas_install_headers( STGC_Overlay )
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py )

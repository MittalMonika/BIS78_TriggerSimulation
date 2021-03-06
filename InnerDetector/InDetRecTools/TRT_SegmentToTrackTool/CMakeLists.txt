################################################################################
# Package: TRT_SegmentToTrackTool
################################################################################

# Declare the package name:
atlas_subdir( TRT_SegmentToTrackTool )

# Declare the package's dependencies:
atlas_depends_on_subdirs( PUBLIC
                          Control/AthenaBaseComps
                          GaudiKernel
                          InnerDetector/InDetRecTools/InDetRecToolInterfaces
                          Tracking/TrkEvent/TrkEventPrimitives
                          PRIVATE
                          InnerDetector/InDetDetDescr/InDetIdentifier
                          InnerDetector/InDetRecEvent/InDetRIO_OnTrack
                          MagneticField/MagFieldInterfaces
                          Tracking/TrkEvent/TrkPseudoMeasurementOnTrack
                          Tracking/TrkExtrapolation/TrkExInterfaces
                          Tracking/TrkFitter/TrkFitterInterfaces
                          Tracking/TrkTools/TrkToolInterfaces
			  Tracking/TrkDetDescr/TrkSurfaces)

# Component(s) in the package:
atlas_add_component( TRT_SegmentToTrackTool
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel InDetRecToolInterfaces TrkEventPrimitives InDetIdentifier InDetRIO_OnTrack MagFieldInterfaces TrkPseudoMeasurementOnTrack TrkExInterfaces TrkFitterInterfaces TrkToolInterfaces )

# Install files from the package:
atlas_install_headers( TRT_SegmentToTrackTool )


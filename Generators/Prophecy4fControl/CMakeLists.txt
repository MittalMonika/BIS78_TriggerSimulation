################################################################################
# Package: Prophecy4fControl
################################################################################

# Declare the package name:
atlas_subdir( Prophecy4fControl )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread MathMore Minuit Minuit2 Matrix Physics HistPainter Rint )

# Component(s) in the package:
atlas_add_executable( DRM
                      bin/DRM.C
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} )

# Install files from the package:
atlas_install_python_modules( python/*.py python/decorators )
atlas_install_joboptions( share/*.py )


################################################################################
# Package: TriggerRelease
################################################################################

# Declare the package name:
atlas_subdir( TriggerRelease )

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py )


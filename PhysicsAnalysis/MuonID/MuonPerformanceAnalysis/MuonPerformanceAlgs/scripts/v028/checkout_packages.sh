
# e.g. use: svn cat svn+ssh://svn.cern.ch/reps/atlasoff/PhysicsAnalysis/MuonID/MuonPerformanceAnalysis/MuonPerformanceAlgs/trunk/scripts/v028/checkout_packages.sh > checkout_packages.sh

# check for AtlasDerivation vs. AthAnalysis
if [ "AtlasDerivation" = $AtlasProject ]; then
    pkgco.py MuonPerformanceAlgs-00-00-62
    pkgco.py MuonPerformanceHistUtils-00-00-26
    pkgco.py MuonTPTools-00-00-44
    pkgco.py MuonPtCalibNtupleMaker-00-00-15
elif [ "AthAnalysisBase" = $AtlasProject ]; then 
    cmt co -r MuonPerformanceAlgs-00-00-62 PhysicsAnalysis/MuonID/MuonPerformanceAnalysis/MuonPerformanceAlgs
    cmt co -r MuonPerformanceHistUtils-00-00-26 PhysicsAnalysis/MuonID/MuonPerformanceAnalysis/MuonPerformanceHistUtils
    cmt co -r MuonTPTools-00-00-44 PhysicsAnalysis/MuonID/MuonPerformanceAnalysis/MuonTPTools
    cmt co -r MuonPtCalibNtupleMaker-00-00-15 PhysicsAnalysis/MuonID/MuonPerformanceAnalysis/MuonPtCalibNtupleMaker

    # stuff not there in AthAnalysisBase,2.4.21
    cmt co -r TrkValHistUtils-00-01-15 Tracking/TrkValidation/TrkValHistUtils
    cmt co -r MuonHistUtils-00-03-34 MuonSpectrometer/MuonValidation/MuonHistogramming/MuonHistUtils
    cmt co -r TrigMuonMatching-00-00-12 Trigger/TrigAnalysis/TrigMuonMatching
    cmt co -r MuonResonanceTools-00-00-12 PhysicsAnalysis/MuonID/MuonPerformanceAnalysis/MuonResonanceTools
else
    echo "$AtlasProject is not supported"
fi
  
setupWorkArea.py


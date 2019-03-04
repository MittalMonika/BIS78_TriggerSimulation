# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python
import sys
from MuonEfficiencyCorrections.CommonToolSetup import *


# a simple testing macro for the MuonEfficiencyCorrections_xAOD package in athena
#
# Usage: athena --filesInput <InputFile> MuonEfficiencyCorrections/MuonEfficiencyCorrections_xAOD_Testing_jobOptions.py
#  E.g.:  athena --filesInput /ptmp/mpp/junggjo9/ClusterTP/Datasets/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.deriv.DAOD_MUON1.e3601_s3126_r9364_r9315_p3374/DAOD_MUON1.13055759._000989.pool.root.1 MuonEfficiencyCorrections/MuonEfficiencyCorrections_xAOD_Testing_jobOptions.py
# Access the algorithm sequence:
AssembleIO("MUONEFFTESTER")
from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()

# Add the test algorithm:
from MuonEfficiencyCorrections.MuonEfficiencyCorrectionsConf import CP__MuonEfficiencyCorrections_TestAlg
alg = CP__MuonEfficiencyCorrections_TestAlg("EffiTestAlg")
alg.PileupReweightingTool = GetPRWTool()
alg.DefaultRelease="cWinter2019_BeforeFix"
alg.ValidationRelease="cWinter2019"
## Select 30 GeV muons for the high-pt WP only
alg.MinPt = 15000
alg.MaxEta = 2.5

WPs = [
         # reconstruction WPs
     #   "LowPt",
     #   "Loose", 
     #   "Medium", 
     #   "Tight", 
        "HighPt",
         # track-to-vertex-association WPs
   #      "TTVA",
         # BadMuon veto SFs
   #     "BadMuonVeto_HighPt",
         # isolation WPs
      #  "FixedCutPflowTightIso",
    #    "FixedCutPflowLooseIso",
    #    "FixedCutHighPtTrackOnlyIso",
    #    "FCTight_FixedRadIso",
    #    "FCTightTrackOnly_FixedRadIso",
    #    "FCTightTrackOnlyIso",
    #    "FCTightIso",
    #    "FCLoose_FixedRadIso",
        ]

for WP in WPs: 
    alg.EfficiencyTools += [GetMuonEfficiencyTool(WP, #Release="190220_Winter_r21", 
                                                  CustomInput = "/ptmp/mpp/junggjo9/ClusterTP/SFFiles/Winter_2019_fixedTruth_v2/")]
    #alg.EfficiencyTools += [GetMuonEfficiencyTool(WP, Release = "190220_Winter_r21")]
  
theJob += alg

# Do some additional tweaking:
from AthenaCommon.AppMgr import theApp
#theApp.EvtMax = 200

#ServiceMgr.MessageSvc.OutputLevel = INFO
#ServiceMgr.MessageSvc.defaultLimit = 100

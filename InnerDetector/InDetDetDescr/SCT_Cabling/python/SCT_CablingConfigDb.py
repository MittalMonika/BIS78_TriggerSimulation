# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

# https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/ConfiguredFactory

from AthenaCommon.CfgGetter import addService, addTool, addAlgorithm

addService("SCT_Cabling.SCT_CablingConfig.getSCT_CablingSvc", "SCT_CablingSvc")
addTool("SCT_Cabling.SCT_CablingConfig.getSCT_CablingTool", "SCT_CablingTool")
addAlgorithm("SCT_Cabling.SCT_CablingConfig.getSCT_CablingCondAlgFromCoraCool", "SCT_CablingCondAlgFromCoraCool")

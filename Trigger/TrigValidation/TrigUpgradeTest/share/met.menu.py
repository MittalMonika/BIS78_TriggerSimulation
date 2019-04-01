#
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

# import flags
include("TrigUpgradeTest/testHLT_MT.py")

##########################################
# menu
##########################################
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import Chain, ChainStep


from TriggerMenuMT.HLTMenuConfig.MET.metMenuDefs import metCellMenuSequence, metJetMenuSequence

metCellSeq = metCellMenuSequence()
metCellStep = ChainStep("Step1_met_cell", [metCellSeq])

testChains = [
   Chain(name="HLT_xe65_L1XE50", Seed="L1_XE50", ChainSteps=[metCellStep]),
   Chain(name="HLT_xe30_L1XE10", Seed="L1_XE10", ChainSteps=[metCellStep]),
]


##########################################
# CF construction
##########################################

##### Make all HLT #######
from TriggerMenuMT.HLTMenuConfig.Menu.HLTCFConfig import makeHLTTree
makeHLTTree(testChains)



##########################################  
# Some debug
##########################################  
from AthenaCommon.AlgSequence import dumpSequence
dumpSequence(topSequence)





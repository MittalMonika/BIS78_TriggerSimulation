#
#  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
#

# import flags
include("TrigUpgradeTest/testHLT_MT.py")

##########################################
# menu
##########################################
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import Chain, ChainStep


from TrigUpgradeTest.metMenuDefs import metCellSequence

metCellSeq = metCellSequence()
metCellStep = ChainStep("Step1_met_cell", [metCellSeq])
testChains = [
    Chain(name="HLT_xe65_L1XE50", Seed="L1_XE50", ChainSteps=[metCellStep]),
	Chain(name="HLT_xe30_L1XE10", Seed="L1_XE10", ChainSteps=[metCellStep])
]

''' ## Will eventually automate the creation of MET trigger chains
from TrigUpgradeTest.metMenuDefs import TrigMissingETChainFromName
testChains = [ TrigMissingETChainFromName(chain) for chain in ["HLT_xe65_L1XE50"] ]
'''
#################################
# Configure L1Decoder
#################################

# provide a minimal menu information
if globalflags.InputFormat.is_bytestream():
   topSequence.L1DecoderTest.ctpUnpacker.OutputLevel=DEBUG
   topSequence.L1DecoderTest.roiUnpackers[0].OutputLevel=DEBUG

# Hacking the decisions name. Otherwise doesn't work. 
topSequence.L1DecoderTest.roiUnpackers["METRoIsUnpackingTool"].Decisions="L1XE"

        
# this is a temporary hack to include new test chains
EnabledChainNamesToCTP = dict([ (c.name, c.seed)  for c in testChains])
topSequence.L1DecoderTest.ChainToCTPMapping = EnabledChainNamesToCTP



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





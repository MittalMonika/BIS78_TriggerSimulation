#
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#

# import flags
include("TrigUpgradeTest/testHLT_MT.py")

##########################################
# menu
##########################################
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import Chain, ChainStep

# We should retrieve all the steps here
from TrigUpgradeTest.bjetMenuDefs import getBJetSequence
step1 = ChainStep("Step1_bjet", [getBJetSequence('j')])
#step3 = ChainStep("Step3_bjet", [getBJetSequence('bTag')])

testChains  = [                                                                                                                                                                         
    Chain(name='HLT_j35_gsc45_boffperf_split' , Seed="L1_J20",  ChainSteps=[step1] ),
    Chain(name='HLT_j35_gsc45_bmv2c1070_split', Seed="L1_J20",  ChainSteps=[step1] ),
    Chain(name='HLT_j35_gsc45_bmv2c1070'      , Seed="L1_J20",  ChainSteps=[step1] )
    ]                                                                                                                                                                                   

#################################
# Configure L1Decoder
#################################

# provide a minimal menu information
if globalflags.InputFormat.is_bytestream():
   topSequence.L1DecoderTest.ctpUnpacker.OutputLevel=DEBUG
   topSequence.L1DecoderTest.roiUnpackers[0].OutputLevel=DEBUG
   
# map L1 decisions for menu
for unpack in topSequence.L1DecoderTest.roiUnpackers:
   if unpack.name() is "JRoIsUnpackingTool":
#      unpack.Decisions="L1J"
      unpack.FSDecisions="L1J"
      jetUnpacker=unpack
      
# this is a temporary hack to include new test chains
EnabledChainNamesToCTP = dict([ (c.name, c.seed)  for c in testChains])
topSequence.L1DecoderTest.ChainToCTPMapping = EnabledChainNamesToCTP

#################################

topSequence.L1DecoderTest.prescaler.Prescales = ["HLT_j35_gsc45_boffperf_split:1",
                                                 "HLT_j35_gsc45_bmv2c1070_split:1",
                                                 "HLT_j35_gsc45_bmv2c1070:1"]


##### Make all HLT #######
from TriggerMenuMT.HLTMenuConfig.Menu.HLTCFConfig import makeHLTTree
makeHLTTree(testChains)

##########################################  
# Some debug
##########################################  
from AthenaCommon.AlgSequence import dumpSequence
dumpSequence(topSequence)




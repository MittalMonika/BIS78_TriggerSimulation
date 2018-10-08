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
#from TrigUpgradeTest.bjetMenuDefs import BjetGscSequence,BjetNoGscSequence
#from TrigUpgradeTest.bjetMenuDefs import BjetSequence, BjetSplitSequence

from TrigUpgradeTest.bjetMenuDefs import getBJetSequence
step1 = ChainStep("Step1_bjet", [getBJetSequence('j')])
step3 = ChainStep("Step3_bjet", [getBJetSequence('bTag')])

testChains  = [                                                                                                                                                                         
    Chain(name='HLT_j35_gsc45_boffperf_split' , Seed="L1_J20",  ChainSteps=[step1] ),
    Chain(name='HLT_j35_gsc45_bmv2c1070_split', Seed="L1_J20",  ChainSteps=[step1] ),
    Chain(name='HLT_j35_gsc45_bmv2c1070'      , Seed="L1_J20",  ChainSteps=[step1] )
    ]                                                                                                                                                                                   

#################################

topSequence.L1DecoderTest.prescaler.Prescales = ["HLT_j35_gsc45_boffperf_split:1",
                                                 "HLT_j35_gsc45_bmv2c1070_split:1",
                                                 "HLT_j35_gsc45_bmv2c1070:1"]



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




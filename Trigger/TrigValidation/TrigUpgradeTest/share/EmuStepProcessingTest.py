###############################################################
#
# Job options file
#
# Based on AthExStoreGateExamples
#
#==============================================================

#--------------------------------------------------------------
# ATLAS default Application Configuration options
#--------------------------------------------------------------

# Configure the scheduler
from AthenaCommon.AlgScheduler import AlgScheduler
AlgScheduler.ShowControlFlow( True )
AlgScheduler.ShowDataFlow( True )




data = {'noreco': [';', ';', ';']}  # in the lists there are the events

data['emclusters'] = ['eta:1,phi:1,et:180000; eta:1,phi:-1.2,et:35000;',
                      'eta:0.5,phi:0,et:120000; eta:1,phi:-1.2,et:65000;',
                      'eta:-0.6,phi:1.7,et:9000;']

data['msmu']  = [';',
                 'eta:-1.2,phi:0.7,pt:6500; eta:-1.1,phi:0.6,pt:8500;',
                 'eta:-1.7,phi:-0.2,pt:9500;']

data['ctp'] = [ 'HLT_g100',  'HLT_2g50 HLT_e20', 'HLT_mu20 HLT_mu8 HLT_2mu8 HLT_mu8_e8' ]

data['l1emroi'] = ['1,1,0,EM3,EM7,EM15,EM20,EM50,EM100; 1,-1.2,0,EM3,EM7',
                   '-0.6,0.2,0,EM3,EM7,EM15,EM20,EM50,EM100; 1,-1.1,0,EM3,EM7,EM15,EM20,EM50',
                   '-0.6,1.5,0,EM3,EM7,EM7']

data['l1muroi'] = ['0,0,0,MU0;',
                   '-1,0.5,0,MU6,MU8; -1,0.5,0,MU6,MU8,MU10',
                   '-1.5,-0.1,0,MU6,MU8']

data['tracks'] = ['eta:1,phi:1,pt:120000; eta:1,phi:-1.2,et:32000;',
                  'eta:0.5,phi:0,pt:130000; eta:1,phi:-1.2,pt:60000;eta:-1.2,phi:0.7,pt:6700; eta:-1.1,phi:0.6,pt:8600;',
                  'eta:-0.6,phi:1.7,et:9000;'] # no MU track for MS candidate 'eta:-1.7,phi:-0.2,pt:9500;'

data['mucomb'] = [';',
                  'eta:-1.2,phi:0.7,pt:6600; eta:-1.1,phi:0.6,pt:8600;',
                  ';']

data['electrons'] = ['eta:1,phi:1,pt:120000; eta:1,phi:-1.2,et:32000;',
                     ';',
                     ';']
data['photons'] = ['eta:1,phi:1,pt:130000;',
                   ';',
                   ';']



from TrigUpgradeTest.TestUtils import writeEmulationFiles
writeEmulationFiles(data)


from AthenaCommon.CFElements import parOR, seqAND, stepSeq

########################## L1 #################################################

L1UnpackingSeq = parOR("L1UnpackingSeq")

from L1Decoder.L1DecoderConf import CTPUnpackingEmulationTool, RoIsUnpackingEmulationTool, L1Decoder
l1Decoder = L1Decoder( OutputLevel=DEBUG, RoIBResult="" )
l1Decoder.prescaler.EventInfo=""

ctpUnpacker = CTPUnpackingEmulationTool( OutputLevel =  DEBUG, ForceEnableAllChains=False , InputFilename="ctp.dat" )
#ctpUnpacker.CTPToChainMapping = [ "0:HLT_g100",  "1:HLT_e20", "2:HLT_mu20", "3:HLT_2mu8", "3:HLT_mu8", "33:HLT_2mu8", "15:HLT_mu8_e8" ]
l1Decoder.ctpUnpacker = ctpUnpacker

emUnpacker = RoIsUnpackingEmulationTool("EMRoIsUnpackingTool", OutputLevel=DEBUG, InputFilename="l1emroi.dat", OutputTrigRoIs="L1EMRoIs", Decisions="L1EM" )
emUnpacker.ThresholdToChainMapping = ["EM7 : HLT_mu8_e8", "EM20 : HLT_e20", "EM50 : HLT_2g50",   "EM100 : HLT_g100" ]

muUnpacker = RoIsUnpackingEmulationTool("MURoIsUnpackingTool", OutputLevel=DEBUG, InputFilename="l1muroi.dat",  OutputTrigRoIs="L1MURoIs", Decisions="L1MU" )
muUnpacker.ThresholdToChainMapping = ["MU6 : HLT_mu6", "MU8 : HLT_mu8", "MU8 : HLT_2mu8",  "MU8 : HLT_mu8_e8",  "MU10 : HLT_mu20",   "EM100 : HLT_g100" ]

l1Decoder.roiUnpackers = [emUnpacker, muUnpacker]

#print l1Decoder
L1UnpackingSeq += l1Decoder
print L1UnpackingSeq

########################## L1 #################################################

# Cati's menu code
#print "=============== MEOW ================"
#include("TrigUpgradeTest/TriggerMenuMT.py")
#genMenu = GenerateMenu()
#genMenu.generate()
#print "=============== WOOF ================"


# steps: sequential AND of 1=Filter 2=Processing
# chainstep=single chain step
# global step=joint for all chains
# filters: one SeqFilter per step, per chain
# inputMakers: one per each first RecoAlg in a step (so one per step), one input per chain that needs that step

# map L1 decisions for menu
for unpack in l1Decoder.roiUnpackers:
    if unpack.name() is "EMRoIsUnpackingTool":
        unpack.Decisions="L1EM"
    if unpack.name() is "MURoIsUnpackingTool":
        unpack.Decisions="L1MU"

        
# signatures
from TrigUpgradeTest.HLTCFConfig import decisionTree_From_Chains
from TrigUpgradeTest.MenuComponents import NodeSequence, MenuSequence, Chain, ChainStep
#from TrigUpgradeTest.HLTSignatureConfig import *

doMuon=True
doElectron=True
doCombo=True

group_of_chains = []
# muon chains
if doMuon:
    from TrigUpgradeTest.HLTSignatureConfig import muStep1Sequence, muStep2Sequence
    muStep1 = muStep1Sequence()
    muStep2 = muStep2Sequence()


    MuChains  = [
        Chain(name='HLT_mu20', Seed="L1_MU10",   ChainSteps=[ChainStep("Step1_mu20", [muStep1]) , ChainStep("Step2_mu20", [muStep2] )]) ,
        Chain(name='HLT_mu8',  Seed="L1_MU6",    ChainSteps=[ChainStep("Step1_mu8",  [muStep1]) , ChainStep("Step2_mu8",  [muStep2] ) ] )
        ]

    group_of_chains += MuChains




## #electron chains
if doElectron:
    from TrigUpgradeTest.HLTSignatureConfig import elStep1Sequence, elStep2Sequence
    elStep1 = elStep1Sequence()
    elStep2 = elStep2Sequence()
    ElChains  = [
        Chain(name='HLT_e20' , Seed="L1_EM10", ChainSteps=[ ChainStep("Step1_e20",  [elStep1]), ChainStep("Step2_e20",  [elStep2]) ] )
        ]

    group_of_chains += ElChains


# combined chain
if doCombo:
    from TrigUpgradeTest.HLTSignatureConfig import combStep1Sequence, combStep2Sequence
    muelStep1 = combStep1Sequence()
    muelStep2 = combStep2Sequence()
    CombChains =[
        Chain(name='HLT_mu8_e8' , Seed="L1_EM6_MU6", ChainSteps=[ ChainStep("Step1_mu8_e8",  [muelStep1]),
                                                                ChainStep("Step2_mu8_e8",  [muelStep2]) ] )
        ]

    group_of_chains += CombChains




# main HLT top sequence
from AthenaCommon.AlgSequence import AlgSequence, AthSequencer, dumpSequence
topSequence = AlgSequence()
#dumpSequence(topSequence)
   
TopHLTRootSeq = seqAND("TopHLTRootSeq") # Root
topSequence += TopHLTRootSeq

#add the L1Upcacking
TopHLTRootSeq += L1UnpackingSeq

# add the HLT steps Node
HLTAllStepsSeq = seqAND("EmuTest_HLTAllStepsSequence")
TopHLTRootSeq += HLTAllStepsSeq

# make CF tree
decisionTree_From_Chains(HLTAllStepsSeq, group_of_chains)


from AthenaCommon.AlgSequence import dumpMasterSequence
dumpMasterSequence()

theApp.EvtMax = 3


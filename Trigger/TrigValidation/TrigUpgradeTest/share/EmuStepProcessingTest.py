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
                 'eta:-1.2,phi:0.7,pt:6500,pt2:8500; eta:-1.1,phi:0.6,pt:8500,pt2:8500;',
                 'eta:-1.7,phi:-0.2,pt:9500,pt2:8500;']

data['ctp'] = [ 'HLT_e20',
                'HLT_mu8 HLT_mu81step HLT_e20 HLT_e8 HLT_mu8_e8',
                'HLT_mu20 HLT_mu8 HLT_mu81step HLT_2mu8 HLT_e8' ]

## data['ctp'] = [ 'HLT_g100',
##                 'HLT_2g50 HLT_e20',
##                 'HLT_mu20 HLT_mu8 HLT_2mu8 HLT_mu8_e8' ]

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


# signatures
from TriggerMenuMT.HLTMenuConfig.Menu.HLTCFConfig import makeHLTTree
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence, Chain, ChainStep


doMuon=True
doElectron=True
doCombo=True

HLTChains = []
EnabledElChains = []
EnabledMuChains = []
EnabledMuComboChains = []
EnabledElComboChains = []


# muon chains
if doMuon:
    from TrigUpgradeTest.HLTSignatureConfig import muStep1Sequence, muStep2Sequence
    muStep1 = muStep1Sequence()
    muStep2 = muStep2Sequence()


    MuChains  = [
        Chain(name='HLT_mu20', Seed="L1_MU10",   ChainSteps=[ChainStep("Step1_mu", [muStep1]) , ChainStep("Step2_mu", [muStep2] )]) ,
        Chain(name='HLT_mu81step', Seed="L1_MU6",   ChainSteps=[ChainStep("Step1_mu", [muStep1]) ]) ,
        Chain(name='HLT_mu8',  Seed="L1_MU6",    ChainSteps=[ChainStep("Step1_mu", [muStep1]) , ChainStep("Step2_mu",  [muStep2] ) ] )
        ]

    HLTChains += MuChains
    EnabledMuChains= [c.seed.strip().split("_")[1] +" : "+ c.name for c in MuChains]




## #electron chains
if doElectron:
    from TrigUpgradeTest.HLTSignatureConfig import elStep1Sequence, elStep2Sequence
    elStep1 = elStep1Sequence()
    elStep2 = elStep2Sequence()
    ElChains  = [
        Chain(name='HLT_e20' , Seed="L1_EM7", ChainSteps=[ ChainStep("Step1_em",  [elStep1]), ChainStep("Step2_em",  [elStep2]) ] ),
        Chain(name='HLT_e8'  , Seed="L1_EM7", ChainSteps=[ ChainStep("Step1_em",  [elStep1]), ChainStep("Step2_em",  [elStep2]) ] )
        ]

    HLTChains += ElChains
    EnabledElChains= [c.seed.strip().split("_")[1] +" : "+ c.name for c in ElChains]


# combined chain
if doCombo:
    from TrigUpgradeTest.HLTSignatureConfig import elStep1Sequence, muStep1Sequence, elStep2Sequence, muStep2Sequence
    elStep1 = elStep1Sequence()
    muStep1 = muStep1Sequence()
    elStep2 = elStep2Sequence()
    muStep2 = muStep2Sequence()
 
    CombChains =[
        Chain(name='HLT_mu8_e8' , Seed="L1_MU6_EM7", ChainSteps=[ ChainStep("Step1_mu_em",  [muStep1, elStep1]), ChainStep("Step2_mu_em",  [muStep2, elStep2])] )         
        ]

    HLTChains += CombChains
    for c in CombChains:
        seeds=c.seed.split("_")
        seeds.pop(0) #remove first L1 string
        for s in seeds:
            if "MU" in s: EnabledMuComboChains.append(s +" : "+ c.name)
            if "EM" in s: EnabledElComboChains.append(s +" : "+ c.name) 

    print "enabled Combo chains: ", EnabledMuComboChains,EnabledElComboChains

EnabledChainNamesToCTP = [str(n)+":"+c.name for n,c in enumerate(HLTChains)]


print "EnabledChainNamesToCTP: ",EnabledChainNamesToCTP



########################## L1 #################################################

L1UnpackingSeq = parOR("L1UnpackingSeq")
from L1Decoder.L1DecoderConf import CTPUnpackingEmulationTool, RoIsUnpackingEmulationTool, L1Decoder
l1Decoder = L1Decoder( OutputLevel=DEBUG, RoIBResult="" )
l1Decoder.prescaler.EventInfo=""
l1Decoder.ChainToCTPMapping = {'HLT_mu20'     : 'L1_MU8', 
                               'HLT_mu81step' : 'L1_MU8', 
                               'HLT_mu8'      : 'L1_MU8', 
                               'HLT_e20'      : 'L1_EM12', 
                               'HLT_e8'       : 'L1_EM7', 
                               'HLT_mu8_e8'   : 'L1_EM3_MU6'}
l1Decoder.Chains="HLTChainsResult"

ctpUnpacker = CTPUnpackingEmulationTool( OutputLevel =  DEBUG, ForceEnableAllChains=False , InputFilename="ctp.dat" )
l1Decoder.ctpUnpacker = ctpUnpacker

emUnpacker = RoIsUnpackingEmulationTool("EMRoIsUnpackingTool", OutputLevel=DEBUG, InputFilename="l1emroi.dat", OutputTrigRoIs="L1EMRoIs", Decisions="L1EM" )
emUnpacker.ThresholdToChainMapping = EnabledElChains + EnabledElComboChains
print "EMRoIsUnpackingTool enables chians:"
print emUnpacker.ThresholdToChainMapping

muUnpacker = RoIsUnpackingEmulationTool("MURoIsUnpackingTool", OutputLevel=DEBUG, InputFilename="l1muroi.dat",  OutputTrigRoIs="L1MURoIs", Decisions="L1MU" )
muUnpacker.ThresholdToChainMapping = EnabledMuChains + EnabledMuComboChains
print "MURoIsUnpackingTool enables chians:"
print muUnpacker.ThresholdToChainMapping

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

        

from AthenaCommon.AlgSequence import AlgSequence, AthSequencer, dumpSequence
topSequence = AlgSequence()
#topSequence += L1UnpackingSeq
topSequence += l1Decoder
##### Make all HLT #######
makeHLTTree(HLTChains)
   

from AthenaCommon.AlgSequence import dumpMasterSequence
dumpMasterSequence()

theApp.EvtMax = 3


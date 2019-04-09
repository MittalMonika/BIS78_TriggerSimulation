
##### obsolete functions for ctests

allAlgs={}
def useExisting(name):
    global allAlgs
    return allAlgs[name]

def AlgExisting(name):
    global allAlgs
    return name in allAlgs


def remember(name, instance):
    global allAlgs
    allAlgs[name] = instance
    return instance




from AthenaCommon.CFElements import parOR, seqAND, stepSeq
from TriggerMenuMT.HLTMenuConfig.Menu.HLTCFConfig import *

def seqFilter(name, Inputs=[], Outputs=[], Chains=[]):
    from DecisionHandling.DecisionHandlingConf import RoRSeqFilter, DumpDecisions
    global allAlgs
    input_list = list(set(Inputs))
    output_list = list (set(Outputs))
    chain_list = list (set(Chains))
    fname = "F_"+name
    f = RoRSeqFilter(fname, Input=input_list, Output=output_list,  Chains=chain_list)
    if "Step1" in name: # so that we see events running through, will be gone once L1 emulation is included
        f.AlwaysPass = True        
    allAlgs[name] = f
    return f

    
   
from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestRecoAlg
def TestRecoAlg(name, Output,  FileName="noreco.dat", Input=""):
    fname = "R_"+name
    a = HLTTest__TestRecoAlg(fname, FileName=FileName, Output=Output, Input=Input)
    allAlgs[name] = a
    return a


from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestHypoAlg
def TestHypoAlg(name, Input, Output):
    fname = "H_"+name
    h = HLTTest__TestHypoAlg(fname, HypoInputDecisions=Input, HypoOutputDecisions=Output)
    allAlgs[name] = h
    return h


def merger(name, Inputs, Output ):
    from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestInputMaker
    outdec=["%s_decision"%i for i in Output]
    return remember(name,   HLTTest__TestInputMaker( "M_"+name, InputMakerInputDecisions=Inputs, Output=Output, InputMakerOutputDecisions=outdec, LinkName="initialRoI" ) )


def addSteps(s):
    return seqAND("HLTChainsSeq", s)





##### new

from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestRecoAlg
from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestHypoAlg
from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestHypoTool
from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestComboHypoAlg


def TestHypoTool(name, prop):
    threshold_value=''.join(filter(lambda x: x.isdigit(), name))
    value  =  int(threshold_value)*1000
    h = HLTTest__TestHypoTool(name, Threshold=value, Property=prop)
    return h

from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestInputMaker
def InputMakerAlg(name):
    return HLTTest__TestInputMaker(name, LinkName="initialRoI")


from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestRoRSeqFilter
def SeqFilterNew(name):
    return HLTTest__TestRoRSeqFilter(name)


def TestRecAlgNew(name, FileName="noreco.dat"):
    return HLTTest__TestRecoAlg(name=name, FileName=FileName)

def TestHypoAlgNew(name):
    return HLTTest__TestHypoAlg(name=name)

def muTestHypoTool(name):
    return TestHypoTool(name, "pt")

def elTestHypoTool(name):
    return TestHypoTool(name, "et")


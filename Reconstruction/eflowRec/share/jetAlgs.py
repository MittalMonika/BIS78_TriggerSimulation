#Run PFlow jet finding
from JetRec.JetRecStandard import jtm
jtm.addJetFinder("AntiKt4EMPFlowJets",  "AntiKt", 0.4,  "empflow", "pflow_ungroomed", ghostArea=0.01, ptmin= 5000, ptminFilter= 10000, calibOpt="arj:pflow")
from JetRec.JetAlgorithm import addJetRecoToAlgSequence
addJetRecoToAlgSequence(eventShapeTools=["empflow"])

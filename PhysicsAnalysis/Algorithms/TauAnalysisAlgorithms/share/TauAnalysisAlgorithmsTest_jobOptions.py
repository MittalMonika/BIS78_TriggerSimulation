# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


import AthenaRootComps.ReadAthenaxAODHybrid

theApp.EvtMax = 500
testFile = os.getenv ('ASG_TEST_FILE_DATA')
svcMgr.EventSelector.InputCollections = [testFile]

algSeq = CfgMgr.AthSequencer("AthAlgSeq")

from AnaAlgorithm.DualUseConfig import setCfgMgr

setCfgMgr (CfgMgr)

# ideally we'd run over all of them, but we don't have a mechanism to
# configure per-sample right now
dataType = "data"
#dataType = "mc"
#dataType = "afii"

config = CfgMgr.CP__SysListLoaderAlg('SysLoaderAlg' )
config.sigmaRecommended = 1
algSeq += config

from TauAnalysisAlgorithms.TauAnalysisSequence import makeTauAnalysisSequence

sequence = makeTauAnalysisSequence (dataType=dataType)


from AsgAnalysisAlgorithms.SequencePostConfiguration import sequencePostConfiguration

sequencePostConfiguration (sequence, "TauJets")

for alg in sequence :
    config = alg["alg"]

    # set everything to debug output
    config.OutputLevel = 1

    algSeq += config
    pass

# Allow the histogram writer algorithm to work correctly:
algSeq.TauCutFlowDumperAlg.RootStreamName = "/TAUTEST"
algSeq.TauKinematicDumperAlg.RootStreamName = "/TAUTEST"
ServiceMgr += CfgMgr.THistSvc()
ServiceMgr.THistSvc.Output += [
    "TAUTEST DATAFILE='TauAnalysisAlgorithmsTest.hist.root' OPT='RECREATE'"
    ]

# create our algorithm with teh given name
#alg = CfgMgr.MyxAODAnalysis()

# later on we'll add some configuration options for our algorithm that go here

#algSeq += alg

# optional include for reducing printout from athena
include("AthAnalysisBaseComps/SuppressLogging.py")

# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#
# @author Tadej Novak

# Ideally we'd run over all of them, but we don't have a mechanism to
# configure per-sample right now
dataType = "data"
#dataType = "mc"
#dataType = "afii"
inputfile = {"data": 'ASG_TEST_FILE_DATA',
             "mc":   'ASG_TEST_FILE_MC',
             "afii": 'ASG_TEST_FILE_MC_AFII'}

# Set up the reading of the input file:
import AthenaRootComps.ReadAthenaxAODHybrid
theApp.EvtMax = 500
testFile = os.getenv ( inputfile[dataType] )
svcMgr.EventSelector.InputCollections = [testFile]

# Config:
triggerChains = [
    'HLT_2mu14',
    'HLT_mu20_mu8noL1',
    'HLT_2e17_lhvloose_nod0'
]

# Import(s) needed for the job configuration.
from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm

# Set up the main analysis algorithm sequence.
algSeq = AlgSequence( 'AnalysisSequence' )

# Set up the systematics loader/handler algorithm:
algSeq += createAlgorithm( 'CP::SysListLoaderAlg', 'SysLoaderAlg' )
algSeq.SysLoaderAlg.sigmaRecommended = 1

# Include, and then set up the pileup analysis sequence:
from TriggerAnalysisAlgorithms.TriggerAnalysisSequence import \
    makeTriggerAnalysisSequence
triggerSequence = makeTriggerAnalysisSequence( dataType, triggerChains=triggerChains )
algSeq += triggerSequence

# Set up an ntuple to check the job with:
ntupleMaker = createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMaker' )
ntupleMaker.TreeName = 'events'
ntupleMaker.Branches = [
    'EventInfo.runNumber   -> runNumber',
    'EventInfo.eventNumber -> eventNumber',
]
ntupleMaker.Branches += ['EventInfo.trigPassed_' + t + ' -> trigPassed_' + t for t in triggerChains]
ntupleMaker.systematicsRegex = '.*'
algSeq += ntupleMaker

# For debugging.
print( algSeq )

# Add all algorithms from the sequence to the job.
athAlgSeq += algSeq

# Set up a histogram output file for the job:
ServiceMgr += CfgMgr.THistSvc()
ServiceMgr.THistSvc.Output += [
    "ANALYSIS DATAFILE='TriggerAlgorithmsTest.root' OPT='RECREATE'"
    ]

# Reduce the printout from Athena:
include( "AthAnalysisBaseComps/SuppressLogging.py" )

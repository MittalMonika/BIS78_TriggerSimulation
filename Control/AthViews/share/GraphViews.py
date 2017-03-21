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
from GaudiHive.GaudiHiveConf import ForwardSchedulerSvc
svcMgr += ForwardSchedulerSvc()
svcMgr.ForwardSchedulerSvc.CheckDependencies = True

# Make a separate alg pool for the view algs
from GaudiHive.GaudiHiveConf import AlgResourcePool
viewAlgPoolName = "ViewAlgPool"
svcMgr += AlgResourcePool( viewAlgPoolName )

# Set of view algs
from AthenaCommon.AlgSequence import AthSequencer
allViewAlgorithms = AthSequencer( "allViewAlgorithms" )

# Filter to stop view algs from running on whole event
allViewAlgorithms += CfgMgr.AthPrescaler( "alwaysFail" )
allViewAlgorithms.alwaysFail.PercentPass = 0.0

# Method to set up a view algorithm
def DeclareViewAlgorithm( viewAlg ):
	global svcMgr, allViewAlgorithms
	svcMgr.ViewAlgPool.TopAlg += [ viewAlg.name() ]
	allViewAlgorithms += viewAlg

# Event-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()


# Make views
job += CfgMgr.AthViews__ViewSubgraphAlg("make_alg")
job.make_alg.ViewBaseName = "view"
job.make_alg.ViewNumber = 5
job.make_alg.AlgPoolName = viewAlgPoolName
job.make_alg.AlgorithmNameSequence = [ "dflow_alg1", "dflow_alg2", "dflow_alg3" ] #Eventually scheduler will do this
job.make_alg.ExtraOutputs += [ ( 'std::vector<int>', 'dflow_ints' ) ] #Dataflow hack

# View algorithms
dflow_alg1 = CfgMgr.AthViews__DFlowAlg1("dflow_alg1")
DeclareViewAlgorithm( dflow_alg1 )
#
dflow_alg2 = CfgMgr.AthViews__DFlowAlg2("dflow_alg2")
DeclareViewAlgorithm( dflow_alg2 )
#
dflow_alg3 = CfgMgr.AthViews__DFlowAlg3("dflow_alg3")
DeclareViewAlgorithm( dflow_alg3 )

# Merge views
job += CfgMgr.AthViews__ViewMergeAlg("merge_alg")
job.merge_alg.ExtraInputs += [ ( 'int', 'all_views_done_dflow' ) ] #Dataflow hack

# Add the view algorithms to the job
job += allViewAlgorithms

#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
theApp.EvtMax = 10

#==============================================================
#
# End of job options file
#
###############################################################


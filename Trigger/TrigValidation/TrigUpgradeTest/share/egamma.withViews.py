#
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
#

include("TrigUpgradeTest/testHLT_MT.py")

from InDetRecExample.InDetJobProperties import InDetFlags
InDetFlags.doCaloSeededBrem = False


from InDetRecExample.InDetJobProperties import InDetFlags
InDetFlags.InDet25nsec = True 
InDetFlags.doPrimaryVertex3DFinding = False 
InDetFlags.doPrintConfigurables = False
InDetFlags.doResolveBackTracks = True 
InDetFlags.doSiSPSeededTrackFinder = True
InDetFlags.doTRTPhaseCalculation = True
InDetFlags.doTRTSeededTrackFinder = True
InDetFlags.doTruth = False
InDetFlags.init()

# PixelLorentzAngleSvc and SCTLorentzAngleSvc
include("InDetRecExample/InDetRecConditionsAccess.py")

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()


from InDetRecExample.InDetKeys import InDetKeys

# provide a minimal menu information
if globalflags.InputFormat.is_bytestream():
   topSequence.L1DecoderTest.ctpUnpacker.OutputLevel=DEBUG
   topSequence.L1DecoderTest.roiUnpackers[0].OutputLevel=DEBUG

CTPToChainMapping = {"HLT_e3_etcut": "L1_EM3",
                    "HLT_e5_etcut":  "L1_EM3",
                    "HLT_e7_etcut":  "L1_EM7",
                    "HLT_2e3_etcut": "L1_2EM3",
                    "HLT_e3e5_etcut":"L1_2EM3"}

topSequence.L1DecoderTest.prescaler.Prescales = ["HLT_e3_etcut:2", "HLT_2e3_etcut:2.5"]

# this is a temporary hack to include only new test chains
testChains =[x for x, y in CTPToChainMapping.items()]
topSequence.L1DecoderTest.ChainToCTPMapping = CTPToChainMapping

 



from AthenaCommon.CFElements import parOR, seqOR, seqAND, stepSeq, findAlgorithm
from DecisionHandling.DecisionHandlingConf import RoRSeqFilter, DumpDecisions
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm

clustersKey = "HLT_xAOD__TrigEMClusterContainer_L2CaloClusters" #"L2CaloClusters"

def createFastCaloSequence(rerun=False):
   __prefix = "Rerurn_" if rerun else ""
   __l1RoIDecisions = "RerunEMRoIDecisions" if rerun else "EMRoIDecisions"
   __forViewDecsions = "RerunEMRoIDecisions"  if rerun else "Filtered"+__l1RoIDecisions 

   from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import T2CaloEgamma_FastAlgo
   #clusterMaker=T2CaloEgamma_FastAlgo(__prefix+"FastClusterMaker" )
   clusterMaker=T2CaloEgamma_FastAlgo( "FastClusterMaker" )
   clusterMaker.OutputLevel=VERBOSE
   clusterMaker.ClustersName=clustersKey
   svcMgr.ToolSvc.TrigDataAccess.ApplyOffsetCorrection=False

   from TrigMultiVarHypo.TrigL2CaloRingerFexMTInit import init_ringer
   trigL2CaloRingerFexMT = init_ringer()
   trigL2CaloRingerFexMT.ClustersKey = clusterMaker.ClustersName
   trigL2CaloRingerFexMT.OutputLevel = DEBUG    
   
   
   fastCaloInViewAlgs = seqAND( __prefix+"fastCaloInViewAlgs", [ clusterMaker, trigL2CaloRingerFexMT ])

   filterL1RoIsAlg = RoRSeqFilter( __prefix+"filterL1RoIsAlg")
   filterL1RoIsAlg.Input = [__l1RoIDecisions]
   filterL1RoIsAlg.Output = ["Filtered"+__l1RoIDecisions]
   filterL1RoIsAlg.Chains = testChains
   filterL1RoIsAlg.OutputLevel = DEBUG

   fastCaloViewsMaker = EventViewCreatorAlgorithm( __prefix+"fastCaloViewsMaker", OutputLevel=DEBUG)
   fastCaloViewsMaker.ViewFallThrough = True
   fastCaloViewsMaker.InputMakerInputDecisions =  [ __forViewDecsions ]
   fastCaloViewsMaker.RoIsLink = "initialRoI" # -||-
   fastCaloViewsMaker.InViewRoIs = "EMCaloRoIs" # contract with the fastCalo
   fastCaloViewsMaker.Views = __prefix+"EMCaloViews"
   fastCaloViewsMaker.ViewNodeName = __prefix+"fastCaloInViewAlgs"
   fastCaloViewsMaker.InputMakerOutputDecisions = [ "L2CaloLinks"]
   clusterMaker.RoIs = fastCaloViewsMaker.InViewRoIs

   from TrigEgammaHypo.TrigEgammaHypoConf import TrigL2CaloHypoAlgMT
   from TrigEgammaHypo.TrigL2CaloHypoTool import TrigL2CaloHypoToolFromName
   fastCaloHypo = TrigL2CaloHypoAlgMT( __prefix+"L2CaloHypo" )
   fastCaloHypo.OutputLevel = DEBUG
   fastCaloHypo.HypoInputDecisions =  fastCaloViewsMaker.InputMakerOutputDecisions[0] #   __l1RoIDecisions
#   fastCaloHypo.Views = fastCaloViewsMaker.Views
   fastCaloHypo.CaloClusters = clusterMaker.ClustersName
#   fastCaloHypo.RoIs = fastCaloViewsMaker.InViewRoIs
   fastCaloHypo.HypoOutputDecisions = __prefix+"EgammaCaloDecisions"
   fastCaloHypo.HypoTools =  [ TrigL2CaloHypoToolFromName( c,c ) for c in testChains ]

   for t in fastCaloHypo.HypoTools:
      t.OutputLevel = DEBUG

   fastCaloSequence = seqAND( __prefix+"fastCaloSequence", [fastCaloViewsMaker, fastCaloInViewAlgs, fastCaloHypo ])
   #if rerun: 
   #   return parOR(__prefix+"egammaCaloStep", [ fastCaloSequence ] )
   return stepSeq(__prefix+"egammaCaloStep", filterL1RoIsAlg, [ fastCaloSequence ])

egammaCaloStep = createFastCaloSequence( rerun=False )


from TrigUpgradeTest.InDetSetup import makeInDetAlgs

(viewAlgs, eventAlgs) = makeInDetAlgs()
from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinder_eGamma

theFTF = TrigFastTrackFinder_eGamma()
theFTF.isRoI_Seeded = True
viewAlgs.append(theFTF)


# A simple algorithm to confirm that data has been inherited from parent view
# Required to satisfy data dependencies
ViewVerify = CfgMgr.AthViews__ViewDataVerifier("electronViewDataVerifier")
ViewVerify.DataObjects = [('xAOD::TrigEMClusterContainer','StoreGateSvc+'+clustersKey)]
ViewVerify.OutputLevel = DEBUG
viewAlgs.append(ViewVerify)

TrackParticlesName = "HLT_xAOD_TrackParticleContainer_L2ElectronTracks"
for viewAlg in viewAlgs:
  if viewAlg.name() == "InDetTrigTrackParticleCreatorAlg":
     viewAlg.TrackParticlesName = TrackParticlesName
    

from TrigEgammaHypo.TrigL2ElectronFexMTConfig import L2ElectronFex_1
theElectronFex= L2ElectronFex_1()
theElectronFex.TrigEMClusterName = clustersKey
theElectronFex.TrackParticlesName = TrackParticlesName
theElectronFex.ElectronsName=  "HLT_xAOD__TrigElectronContainer_L2ElectronFex" #"Electrons"
theElectronFex.OutputLevel=VERBOSE



filterCaloRoIsAlg = RoRSeqFilter("filterCaloRoIsAlg")
caloHypoDecisions = findAlgorithm(egammaCaloStep, "L2CaloHypo").HypoOutputDecisions
print "kkkk ", caloHypoDecisions
filterCaloRoIsAlg.Input = [caloHypoDecisions]
filterCaloRoIsAlg.Output = ["Filtered" + caloHypoDecisions]
filterCaloRoIsAlg.Chains = testChains
filterCaloRoIsAlg.OutputLevel = DEBUG


l2ElectronViewsMaker = EventViewCreatorAlgorithm("l2ElectronViewsMaker", OutputLevel=DEBUG)
# topSequence += l2ElectronViewsMaker
l2ElectronViewsMaker.InputMakerInputDecisions = [ filterCaloRoIsAlg.Output[0] ] # output of L2CaloHypo
l2ElectronViewsMaker.RoIsLink = "roi" # -||-
l2ElectronViewsMaker.InViewRoIs = "EMIDRoIs" # contract with the fastCalo
l2ElectronViewsMaker.Views = "EMElectronViews"
l2ElectronViewsMaker.ViewFallThrough = True
l2ElectronViewsMaker.InputMakerOutputDecisions = ["L2ElectronLinks"]

for viewAlg in viewAlgs:
  if viewAlg.properties().has_key("RoIs"):
    viewAlg.RoIs = l2ElectronViewsMaker.InViewRoIs
  if viewAlg.properties().has_key("roiCollectionName"):
    viewAlg.roiCollectionName = l2ElectronViewsMaker.InViewRoIs
theElectronFex.RoIs = l2ElectronViewsMaker.InViewRoIs    

electronInViewAlgs = parOR("electronInViewAlgs", viewAlgs + [ theElectronFex ])

l2ElectronViewsMaker.ViewNodeName = "electronInViewAlgs"


from TrigEgammaHypo.TrigEgammaHypoConf import TrigL2ElectronHypoAlgMT
from TrigEgammaHypo.TrigL2ElectronHypoTool import TrigL2ElectronHypoToolFromName
theElectronHypo = TrigL2ElectronHypoAlgMT()
theElectronHypo.RunInView=True
theElectronHypo.HypoInputDecisions = "L2ElectronLinks"
theElectronHypo.HypoOutputDecisions = "ElectronL2Decisions"
theElectronHypo.Electrons = theElectronFex.ElectronsName

theElectronHypo.OutputLevel = VERBOSE

theElectronHypo.HypoTools = [ TrigL2ElectronHypoToolFromName( c,c ) for c in testChains ]

for t in theElectronHypo.HypoTools:
  t.OutputLevel = VERBOSE
# topSequence += theElectronHypo
# InDetCacheCreatorTrigViews,
electronSequence = seqAND("electronSequence", eventAlgs + [l2ElectronViewsMaker, electronInViewAlgs, theElectronHypo ] )

egammaIDStep = stepSeq("egammaIDStep", filterCaloRoIsAlg, [ electronSequence ] )


# CF construction

from DecisionHandling.DecisionHandlingConf import TriggerSummaryAlg
summaryStep0 = TriggerSummaryAlg( "TriggerSummaryStep1" )
summaryStep0.InputDecision = "HLTChains"
summaryStep0.HLTSummary = "MonitoringSummaryStep1"
summaryStep0.FinalDecisions = [ caloHypoDecisions ]
summaryStep0.OutputLevel = DEBUG


step0 = parOR("step0", [ egammaCaloStep, summaryStep0 ] )
step1 = parOR("step1", [ egammaIDStep ] )


egammaCaloStepRR = createFastCaloSequence( rerun=True )

step0r = parOR("step0r", [ egammaCaloStepRR ])

summary = TriggerSummaryAlg( "TriggerSummaryAlg" )
summary.InputDecision = "HLTChains"
summary.FinalDecisions = [ "ElectronL2Decisions", "MuonL2Decisions" ]

from TrigOutputHandling.TrigOutputHandlingConf import HLTEDMCreator
egammaViewsMerger = HLTEDMCreator("egammaViewsMerger")
egammaViewsMerger.TrigCompositeContainer = [ "L2ElectronLinks", "filterCaloRoIsAlg", "EgammaCaloDecisions","ElectronL2Decisions", "MuonL2Decisions", "EMRoIDecisions", "METRoIDecisions", "MURoIDecisions", "HLTChainsResult", "JRoIDecisions", "MonitoringSummaryStep1", "RerunEMRoIDecisions", "RerunMURoIDecisions", "TAURoIDecisions", "L2CaloLinks", "FilteredEMRoIDecisions", "FilteredEgammaCaloDecisions" ]

egammaViewsMerger.TrackParticleContainerViews = [ l2ElectronViewsMaker.Views ]
egammaViewsMerger.TrackParticleContainerInViews = [ TrackParticlesName ]
egammaViewsMerger.TrackParticleContainer = [ TrackParticlesName ]

# this merging directive causes the issue
egammaViewsMerger.TrigElectronContainerViews = [ l2ElectronViewsMaker.Views ]
egammaViewsMerger.TrigElectronContainerInViews = [ theElectronFex.ElectronsName ]
egammaViewsMerger.TrigElectronContainer = [ theElectronFex.ElectronsName ]

egammaViewsMerger.TrigEMClusterContainerViews = [ "EMCaloViews" ]
egammaViewsMerger.TrigEMClusterContainerInViews = [ clustersKey ]
egammaViewsMerger.TrigEMClusterContainer = [ clustersKey ]

egammaViewsMerger.OutputLevel = VERBOSE

svcMgr.StoreGateSvc.OutputLevel = VERBOSE

summary.OutputTools = [ egammaViewsMerger ]


summary.OutputLevel = DEBUG

step0filter = parOR("step0filter", [ findAlgorithm( egammaCaloStep, "filterL1RoIsAlg") ] )
step1filter = parOR("step1filter", [ findAlgorithm(egammaIDStep, "filterCaloRoIsAlg") ] )
step0rfilter = parOR("step0rfilter", [ findAlgorithm(egammaCaloStepRR, "Rerurn_filterL1RoIsAlg") ] )


steps = seqAND("HLTSteps", [ step0filter, step0, step1filter, step1, step0rfilter, step0r ]  )

from TrigSteerMonitor.TrigSteerMonitorConf import TrigSignatureMoniMT, DecisionCollectorTool
mon = TrigSignatureMoniMT()
mon.FinalDecisions = [ "ElectronL2Decisions", "MuonL2Decisions", "WhateverElse" ]
from TrigUpgradeTest.TestUtils import MenuTest
mon.ChainsList = list( set( topSequence.L1DecoderTest.ChainToCTPMapping.keys() ) )
#mon.ChainsList = list( set( MenuTest.CTPToChainMapping.keys() ) )
mon.OutputLevel = DEBUG

step1Collector = DecisionCollectorTool("Step1Collector")
step1Collector.Decisions = ["EgammaCaloDecisions"]

step2Collector = DecisionCollectorTool("Step2Collector")
step2Collector.Decisions = ["ElectronL2Decisions"]
mon.CollectorTools = [step1Collector, step2Collector]


import AthenaPoolCnvSvc.WriteAthenaPool
from OutputStreamAthenaPool.OutputStreamAthenaPool import  createOutputStream
StreamESD=createOutputStream("StreamESD","myESD.pool.root",True)
topSequence.remove( StreamESD )

def addTC(name):   
   StreamESD.ItemList += [ "xAOD::TrigCompositeContainer#"+name, "xAOD::TrigCompositeAuxContainer#"+name+"Aux." ]

for tc in egammaViewsMerger.TrigCompositeContainer:
   addTC( tc + "_remap" )

addTC("HLTSummary")

StreamESD.ItemList += [ "xAOD::TrigElectronContainer#HLT_xAOD__TrigElectronContainer_L2ElectronFex", 
                        "xAOD::TrackParticleContainer#HLT_xAOD_TrackParticleContainer_L2ElectronTracks",
                        "xAOD::TrigEMClusterContainer#HLT_xAOD__TrigEMClusterContainer_L2CaloClusters"]

StreamESD.ItemList += [ "xAOD::TrigElectronAuxContainer#HLT_xAOD__TrigElectronContainer_L2ElectronFexAux.", 
                        "xAOD::TrackParticleAuxContainer#HLT_xAOD_TrackParticleContainer_L2ElectronTracksAux.", 
                        "xAOD::TrigEMClusterAuxContainer#HLT_xAOD__TrigEMClusterContainer_L2CaloClustersAux."]

StreamESD.ItemList += [ "EventInfo#ByteStreamEventInfo" ]

StreamESD.ItemList += [ "TrigRoiDescriptorCollection#EMRoIs" ]
StreamESD.ItemList += [ "TrigRoiDescriptorCollection#JRoIs" ]
StreamESD.ItemList += [ "TrigRoiDescriptorCollection#METRoI" ]
StreamESD.ItemList += [ "TrigRoiDescriptorCollection#MURoIs" ]
StreamESD.ItemList += [ "TrigRoiDescriptorCollection#TAURoIs" ]

StreamESD.ItemList += [ "ROIB::RoIBResult#*" ]

print "ESD file content " 
print StreamESD.ItemList

hltTop = seqOR( "hltTop", [ steps, mon, summary, StreamESD ] )
topSequence += hltTop

from AthenaCommon.AlgSequence import dumpSequence
dumpSequence(topSequence)

#print theElectronFex
#print ViewVerify

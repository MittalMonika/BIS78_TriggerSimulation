#
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

include("TrigUpgradeTest/testHLT_MT.py")


from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()


CTPToChainMapping = {"HLT_e3_etcut_L1EM3": "L1_EM3",
                     "HLT_e5_etcut_L1EM3":  "L1_EM3",
                     "HLT_e7_etcut_L1EM7":  "L1_EM7",
                     "HLT_2e3_etcut_L1EM3": "L1_2EM3",
                     "HLT_e3_e5_etcut_L1EM3":"L1_2EM3"}

topSequence.L1Decoder.prescaler.Prescales = ["HLT_e3_etcut_L1EM3:2", "HLT_2e3_etcut_L1EM3:2.5"]

# this is a temporary hack to include only new test chains
testChains =[x for x, y in CTPToChainMapping.items()]
topSequence.L1Decoder.ChainToCTPMapping = CTPToChainMapping

 



from AthenaCommon.CFElements import parOR, seqOR, seqAND, stepSeq, findAlgorithm
from DecisionHandling.DecisionHandlingConf import RoRSeqFilter, DumpDecisions
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm

clustersKey = "HLT_xAOD__TrigEMClusterContainer_L2CaloClusters" #"L2CaloClusters"

def createFastCaloSequence(rerun=False):
   __prefix = "Rerurn_" if rerun else ""
   __l1RoIDecisions = "RerunL1EM" if rerun else "L1EM"
   __forViewDecsions = "RerunEMRoIDecisions"  if rerun else "Filtered"+__l1RoIDecisions 
   
   from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import T2CaloEgamma_ReFastAlgo
   clusterMaker=T2CaloEgamma_ReFastAlgo( "FastClusterMaker" )   


   clusterMaker.ClustersName=clustersKey

   
   #from TrigMultiVarHypo.TrigL2CaloRingerFexMTInit import init_ringer
   #trigL2CaloRingerFexMT = init_ringer()
   #trigL2CaloRingerFexMT.ClustersKey = clusterMaker.ClustersName

   #fastCaloInViewAlgs = seqAND( __prefix+"fastCaloInViewAlgs", [ clusterMaker, trigL2CaloRingerFexMT ])
   fastCaloInViewAlgs = seqAND( __prefix+"fastCaloInViewAlgs", [ clusterMaker ])
   

   filterL1RoIsAlg = RoRSeqFilter( __prefix+"filterL1RoIsAlg")
   filterL1RoIsAlg.Input = [__l1RoIDecisions]
   filterL1RoIsAlg.Output = ["Filtered"+__l1RoIDecisions]
   filterL1RoIsAlg.Chains = testChains

   fastCaloViewsMaker = EventViewCreatorAlgorithm( __prefix+"fastCaloViewsMaker")
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
   fastCaloHypo.HypoInputDecisions =  fastCaloViewsMaker.InputMakerOutputDecisions[0] #   __l1RoIDecisions
#   fastCaloHypo.Views = fastCaloViewsMaker.Views
   fastCaloHypo.CaloClusters = clusterMaker.ClustersName
#   fastCaloHypo.RoIs = fastCaloViewsMaker.InViewRoIs
   fastCaloHypo.HypoOutputDecisions = __prefix+"EgammaCaloDecisions"
   fastCaloHypo.HypoTools =  [ TrigL2CaloHypoToolFromName( c,c ) for c in testChains ]
   fastCaloSequence = seqAND( __prefix+"fastCaloSequence", [fastCaloViewsMaker, fastCaloInViewAlgs, fastCaloHypo ])
   #if rerun: 
   #   return parOR(__prefix+"egammaCaloStep", [ fastCaloSequence ] )
   return stepSeq(__prefix+"egammaCaloStep", filterL1RoIsAlg, [ fastCaloSequence ])

egammaCaloStep = createFastCaloSequence( rerun=False )


from TriggerMenuMT.HLTMenuConfig.CommonSequences.InDetSetup import makeInDetAlgs
(viewAlgs, eventAlgs) = makeInDetAlgs()

from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinder_eGamma
theFTF = TrigFastTrackFinder_eGamma()
theFTF.isRoI_Seeded = True
viewAlgs.append(theFTF)


# A simple algorithm to confirm that data has been inherited from parent view
# Required to satisfy data dependencies
ViewVerify = CfgMgr.AthViews__ViewDataVerifier("electronViewDataVerifier")
ViewVerify.DataObjects = [('xAOD::TrigEMClusterContainer','StoreGateSvc+'+clustersKey)]
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



filterCaloRoIsAlg = RoRSeqFilter("filterCaloRoIsAlg")
caloHypoDecisions = findAlgorithm(egammaCaloStep, "L2CaloHypo").HypoOutputDecisions
filterCaloRoIsAlg.Input = [caloHypoDecisions]
filterCaloRoIsAlg.Output = ["Filtered" + caloHypoDecisions]
filterCaloRoIsAlg.Chains = testChains


l2ElectronViewsMaker = EventViewCreatorAlgorithm("l2ElectronViewsMaker")
# topSequence += l2ElectronViewsMaker
l2ElectronViewsMaker.InputMakerInputDecisions = [ filterCaloRoIsAlg.Output[0] ] # output of L2CaloHypo
l2ElectronViewsMaker.RoIsLink = "roi" # -||-
l2ElectronViewsMaker.InViewRoIs = "EMIDRoIs" # contract with the fastCalo
l2ElectronViewsMaker.Views = "EMElectronViews"
l2ElectronViewsMaker.ViewFallThrough = True
l2ElectronViewsMaker.RequireParentView = True
l2ElectronViewsMaker.InputMakerOutputDecisions = ["L2ElectronLinks"]

for viewAlg in viewAlgs:
  if viewAlg.properties().has_key("RoIs"):
    viewAlg.RoIs = l2ElectronViewsMaker.InViewRoIs
  if viewAlg.properties().has_key("roiCollectionName"):
    viewAlg.roiCollectionName = l2ElectronViewsMaker.InViewRoIs
theElectronFex.RoIs = l2ElectronViewsMaker.InViewRoIs    

electronInViewAlgs = parOR("electronInViewAlgs", viewAlgs + [ theElectronFex ])

l2ElectronViewsMaker.ViewNodeName = electronInViewAlgs.name()


from TrigEgammaHypo.TrigEgammaHypoConf import TrigL2ElectronHypoAlgMT
from TrigEgammaHypo.TrigL2ElectronHypoTool import TrigL2ElectronHypoToolFromName
theElectronHypo = TrigL2ElectronHypoAlgMT()
theElectronHypo.RunInView=True
theElectronHypo.HypoInputDecisions = "L2ElectronLinks"
theElectronHypo.HypoOutputDecisions = "ElectronL2Decisions"
theElectronHypo.Electrons = theElectronFex.ElectronsName
theElectronHypo.HypoTools = [ TrigL2ElectronHypoToolFromName( c,c ) for c in testChains ]

electronSequence = seqAND("electronSequence", eventAlgs + [l2ElectronViewsMaker, electronInViewAlgs, theElectronHypo ] )
egammaIDStep = stepSeq("egammaIDStep", filterCaloRoIsAlg, [ electronSequence ] )



filterL2ElectronRoIsAlg = RoRSeqFilter("filterL2ElectronRoIsAlg")
electronHypoDecisions = findAlgorithm(egammaIDStep, "TrigL2ElectronHypoAlgMT").HypoOutputDecisions

filterL2ElectronRoIsAlg.Input = [electronHypoDecisions]
filterL2ElectronRoIsAlg.Output = ["Filtered" + electronHypoDecisions]
filterL2ElectronRoIsAlg.Chains = testChains


efClusterViewsMaker = EventViewCreatorAlgorithm("efClusterViewsMaker")
efClusterViewsMaker.InputMakerInputDecisions = [ filterL2ElectronRoIsAlg.Output[0] ] # output of L2CaloHypo
efClusterViewsMaker.RoIsLink = "roi" # -||-
efClusterViewsMaker.InViewRoIs = "CaloRoIs" # contract with the fastCalo
efClusterViewsMaker.Views = "EFCaloViews"
efClusterViewsMaker.ViewFallThrough = True
efClusterViewsMaker.InputMakerOutputDecisions = ["EFClusterLinks"]




efClusterSequence = seqAND("efClusterSequence", [efClusterViewsMaker] )
egammaEFCaloStep = stepSeq("egammaEFCalotep", filterL2ElectronRoIsAlg, [ efClusterSequence ] )




# CF construction

from DecisionHandling.DecisionHandlingConf import TriggerSummaryAlg
summaryStep0 = TriggerSummaryAlg( "TriggerSummaryStep1" )
summaryStep0.InputDecision = "L1DecoderSummary"
summaryStep0.FinalDecisions = [ caloHypoDecisions ]


step0 = parOR("step0", [ egammaCaloStep, summaryStep0 ] )
step1 = parOR("step1", [ egammaIDStep ] )
step2 = parOR("step2", [ egammaEFCaloStep ] )


egammaCaloStepRR = createFastCaloSequence( rerun=True )

step0r = parOR("step0r", [ egammaCaloStepRR ])

summary = TriggerSummaryAlg( "TriggerSummaryAlg" )
summary.InputDecision = "L1DecoderSummary"
summary.FinalDecisions = [ "ElectronL2Decisions", "MuonL2Decisions" ]

from TrigOutputHandling.TrigOutputHandlingConf import HLTEDMCreator, HLTEDMCreatorAlg
egammaCaloViewsMerger = HLTEDMCreator("egammaCaloViewsMerger", FixLinks=["EgammaCaloDecisions"])
egammaCaloViewsMerger.TrigCompositeContainer = ["EgammaCaloDecisions"]

egammaCaloViewsMerger.TrigEMClusterContainerViews = [ "EMCaloViews" ]
egammaCaloViewsMerger.TrigEMClusterContainerInViews = [ clustersKey ]
egammaCaloViewsMerger.TrigEMClusterContainer = [ clustersKey ]

#[ "filterCaloRoIsAlg", "EgammaCaloDecisions","ElectronL2Decisions", "MuonL2Decisions", "EMRoIDecisions", "METRoIDecisions", "MURoIDecisions", "L1DecoderSummary", "JRoIDecisions", "MonitoringSummaryStep1", "RerunEMRoIDecisions", "RerunMURoIDecisions", "TAURoIDecisions", "L2CaloLinks", "FilteredEMRoIDecisions", "FilteredEgammaCaloDecisions" ]

egammaElectronViewsMerger = HLTEDMCreator("egammaelectronViewsMerger", FixLinks=["ElectronL2Decisions"])
egammaElectronViewsMerger.TrigCompositeContainer = ["ElectronL2Decisions"]
egammaElectronViewsMerger.TrackParticleContainerViews = [ l2ElectronViewsMaker.Views ]
egammaElectronViewsMerger.TrackParticleContainerInViews = [ TrackParticlesName ]
egammaElectronViewsMerger.TrackParticleContainer = [ TrackParticlesName ]
egammaElectronViewsMerger.TrigElectronContainerViews = [ l2ElectronViewsMaker.Views ]
egammaElectronViewsMerger.TrigElectronContainerInViews = [ theElectronFex.ElectronsName ]
egammaElectronViewsMerger.TrigElectronContainer = [ theElectronFex.ElectronsName ]


decisionsNotPointingtoViews = [ "filterCaloRoIsAlg",  "EMRoIDecisions", "METRoIDecisions", "MURoIDecisions", "L1DecoderSummary", "JRoIDecisions", "TAURoIDecisions", "L2CaloLinks", "FilteredEMRoIDecisions", "FilteredEgammaCaloDecisions" ]

gapsFiller =  HLTEDMCreator("GapsFiller") # no links fixing
gapsFiller.TrigCompositeContainer = decisionsNotPointingtoViews


edmMakerAlg = HLTEDMCreatorAlg("EDMMaker")
edmMakerAlg.OutputTools = [ egammaCaloViewsMerger, egammaElectronViewsMerger, gapsFiller ]




step0filter = parOR("step0filter", [ findAlgorithm( egammaCaloStep, "filterL1RoIsAlg") ] )
step1filter = parOR("step1filter", [ findAlgorithm(egammaIDStep, "filterCaloRoIsAlg") ] )
step2filter = parOR("step2filter", [ findAlgorithm(egammaEFCaloStep, "filterL2ElectronRoIsAlg") ] )
step0rfilter = parOR("step0rfilter", [ findAlgorithm(egammaCaloStepRR, "Rerurn_filterL1RoIsAlg") ] )



steps = seqAND("HLTSteps", [ step0filter, step0, step1filter, step1, step2filter, step2,  step0rfilter, step0r ]  )




from TrigSteerMonitor.TrigSteerMonitorConf import TrigSignatureMoniMT, DecisionCollectorTool
mon = TrigSignatureMoniMT()
from TrigUpgradeTest.TestUtils import MenuTest
mon.ChainsList = list( set( topSequence.L1Decoder.ChainToCTPMapping.keys() ) )
#mon.ChainsList = list( set( MenuTest.CTPToChainMapping.keys() ) )


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

     
for tc in decisionsNotPointingtoViews + ["EgammaCaloDecisions", "ElectronL2Decisions"]:
   addTC( tc )

addTC("HLTNav_Summary")

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

from TrigOutputHandling.TrigOutputHandlingConf import DecisionSummaryMakerAlg, HLTResultMTMakerAlg, HLTResultMTMaker, StreamTagMakerTool, TriggerBitsMakerTool
summMaker = DecisionSummaryMakerAlg()
summMaker.FinalDecisionKeys = [ theElectronHypo.HypoOutputDecisions ]
summMaker.FinalStepDecisions =  dict( [ ( tool.getName(), theElectronHypo.HypoOutputDecisions ) for tool in theElectronHypo.HypoTools ] )

################################################################################
# test online HLT Result maker

from TrigOutputHandling.TrigOutputHandlingConfig import TriggerEDMSerialiserToolCfg

serialiser = TriggerEDMSerialiserToolCfg("Serialiser")
serialiser.addCollectionListToMainResult([
   "xAOD::TrigCompositeContainer_v1#EgammaCaloDecisions",
   "xAOD::TrigCompositeAuxContainer_v2#EgammaCaloDecisionsAux.remap_linkCollKeys.remap_linkCollIndices",
   "xAOD::TrigCompositeContainer_v1#ElectronL2Decisions",
   "xAOD::TrigCompositeAuxContainer_v2#ElectronL2DecisionsAux.remap_linkCollKeys.remap_linkCollIndices",
   "xAOD::TrigEMClusterContainer_v1#HLT_xAOD__TrigEMClusterContainer_L2CaloClusters",
   "xAOD::TrigEMClusterAuxContainer_v2#HLT_xAOD__TrigEMClusterContainer_L2CaloClustersAux.RoIword.clusterQuality.e233.e237.e277.e2tsts1.ehad1.emaxs1.energy.energySample.et.eta.eta1.fracs1.nCells.phi.rawEnergy.rawEnergySample.rawEt.rawEta.rawPhi.viewIndex.weta2.wstot",
   "xAOD::TrigElectronContainer_v1#HLT_xAOD__TrigElectronContainer_L2ElectronFex",
   "xAOD::TrigElectronAuxContainer_v1#HLT_xAOD__TrigElectronContainer_L2ElectronFexAux.pt.eta.phi.rawEnergy.rawEt.rawEta.nCells.energy.et.e237.e277.fracs1.weta2.ehad1.wstot",
])


streamPhysicsMain = ['Main', 'physics', "True", "True"]
streamPhotonPerf = ['PhotonPerf', 'calibration', "True", "True"] # just made up the name

stmaker = StreamTagMakerTool()
stmaker.ChainDecisions = "HLTNav_Summary"
stmaker.ChainToStream = dict( [(c, streamPhysicsMain) for c in testChains ] )
stmaker.ChainToStream["HLT_e5_etcut"] = streamPhotonPerf
bitsmaker = TriggerBitsMakerTool()
bitsmaker.ChainDecisions = "HLTNav_Summary"
bitsmaker.ChainToBit = dict( [ (chain, 10*num) for num,chain in enumerate(testChains) ] ) 

hltResultMakerTool =  HLTResultMTMaker("MKTool")
hltResultMakerTool.MakerTools = [ stmaker, bitsmaker, serialiser ]
hltResultMakerTool.OutputLevel = DEBUG    # want to monitor EDM size

hltResultMakerAlg =  HLTResultMTMakerAlg("HLTRMakerAlg")


from AthenaMonitoring.GenericMonitoringTool import GenericMonitoringTool, defineHistogram
hltResultMakerTool.MonTool = GenericMonitoringTool("MonOfHLTResultMTtest")
hltResultMakerTool.MonTool.HistPath = "OutputMonitoring"
hltResultMakerTool.MonTool.Histograms = [ defineHistogram( 'TIME_build', path='EXPERT', type='TH1F', title='Time of result construction in;[micro seccond]',
                                                           xbins=100, xmin=0, xmax=1000 ),
                                          defineHistogram( 'nstreams', path='EXPERT', type='TH1F', title='number of streams',
                                                           xbins=60, xmin=0, xmax=60 ),
                                          defineHistogram( 'nfrags', path='EXPERT', type='TH1F', title='number of HLT results',
                                                           xbins=10, xmin=0, xmax=10 ),
                                          defineHistogram( 'sizeMain', path='EXPERT', type='TH1F', title='Main (physics) HLT Result size;4B words',
                                                           xbins=100, xmin=-1, xmax=999 ) ] # 1000 k span

hltResultMakerAlg.ResultMaker = hltResultMakerTool

from TrigOutputHandling.TrigOutputHandlingConf import TriggerEDMDeserialiserAlg
deserialiser = TriggerEDMDeserialiserAlg()
deserialiser.Prefix="SERIALISED_"

# # add prefix + remove version to class name
l = [ c.split("#")[0].split("_")[0] + "#" + deserialiser.Prefix + c.split("#")[1] for c in serialiser.CollectionsToSerialize ] 
#StreamESD.ItemList += l


if not hasattr( svcMgr, "ByteStreamAddressProviderSvc" ):
   from ByteStreamCnvSvcBase.ByteStreamCnvSvcBaseConf import ByteStreamAddressProviderSvc 
   svcMgr += ByteStreamAddressProviderSvc()
svcMgr.ByteStreamAddressProviderSvc.TypeNames = ["ROIB::RoIBResult/RoIBResult", ]

from ByteStreamCnvSvc import WriteByteStream
streamBS = WriteByteStream.getStream("EventStorage","StreamBSFileOutput")
ServiceMgr.ByteStreamCnvSvc.IsSimulation = False
ServiceMgr.ByteStreamCnvSvc.InitCnvs += ["HLT::HLTResultMT"]
streamBS.ItemList += ["HLT::HLTResultMT#HLTResultMT"]

svcMgr.EventPersistencySvc.CnvServices += [ "ByteStreamCnvSvc" ]



################################################################################
# assemble top list of algorithms
hltTop = seqOR( "hltTop", [ steps,  summMaker, mon, edmMakerAlg, hltResultMakerAlg, deserialiser, StreamESD, streamBS ] )



topSequence += hltTop

###### Begin Cost Monitoring block 
## TODO: Express this as a ComponentAccumulator

from AthenaCommon.AppMgr import ServiceMgr
from GaudiSvc.GaudiSvcConf import AuditorSvc
from TrigCostMonitorMT.TrigCostMonitorMTConf import TrigCostMTAuditor, TrigCostMTSvc
from AthenaCommon.ConcurrencyFlags import jobproperties as jps

# This collects and summarises all cost data from all threads
trigCostService = TrigCostMTSvc()
trigCostService.MonitorAll = True # During testing only
trigCostService.PrintTimes = True # During testing only
trigCostService.SaveHashes = True # During testing only
trigCostService.EventSlots = jps.ConcurrencyFlags.NumConcurrentEvents()
ServiceMgr += trigCostService
print("NumConcurrentEvents = " + str(jps.ConcurrencyFlags.NumConcurrentEvents()))

# This causes Gaudi to ping the trigCostService before & after all algorithms  
trigCostAuditor = TrigCostMTAuditor()
theAuditorSvc = ServiceMgr.AuditorSvc
theAuditorSvc += trigCostAuditor
theApp.AuditAlgorithms=True
ServiceMgr += AuditorSvc()

# This triggers the L1 decoder to signal the start of processing, 
# and the HLT summary alg to signal end of processing and handle the writing of data.
topSequence.L1Decoder.EnableCostMonitoring = True
summMaker.EnableCostMonitoring = True

# Write out the data at the end
addTC("TrigCostContainer")

###### End Cost Monitoring block

print("Dump of topSequence")
from AthenaCommon.AlgSequence import dumpSequence
dumpSequence(topSequence)
print("Dump of serviceMgr")
dumpSequence(ServiceMgr)
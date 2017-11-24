# 
#  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration 
# 
#  OutputLevel: INFO < DEBUG < VERBOSE 
# 
  
include("TrigUpgradeTest/testHLT_MT.py") 
 
viewTest = opt.enableViews   # from testHLT_MT.py
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()
if viewTest:
  allViewAlgorithms = topSequence.allViewAlgorithms
 
# provide a minimal menu information
topSequence.L1DecoderTest.ctpUnpacker.OutputLevel=DEBUG
topSequence.L1DecoderTest.roiUnpackers[0].OutputLevel=DEBUG
topSequence.L1DecoderTest.roiUnpackers[1].OutputLevel=DEBUG
 
# for Desplaying StoreGate Dump
from AthenaCommon.AppMgr import ServiceMgr
ServiceMgr.StoreGateSvc=Service("StoreGateSvc") # if below is not working, try this
ServiceMgr.StoreGateSvc.Dump=True #  try this first
 
# for Control Flow
from AthenaCommon.CFElements import parOR, seqAND, stepSeq
 
 
from AthenaCommon.AlgScheduler import AlgScheduler
AlgScheduler.CheckDependencies( True )
AlgScheduler.OutputLevel( DEBUG )
AlgScheduler.ShowDataDependencies( True )
AlgScheduler.setDataLoaderAlg( 'SGInputLoader' )
 
if TriggerFlags.doMuon:
  # Load data from Muon detectors
  #topSequence.SGInputLoader.Load = [ ('MdtCsmContainer','MDTCSM'), ('RpcPadContainer','RPCPAD'), ('TgcRdoContainer','TGCRDO'), ('CscRawDataContainer','CSCRDO')]
 
  from AthenaCommon.CfgGetter import getAlgorithm
  if muonRecFlags.doCSCs():
    CscDecoder = getAlgorithm("CscRdoToCscPrepData", tryDefaultConfigurable=True)
    CscDecoder.DoSeededDecoding=True
    CscDecoder.RoIs="MURoIs"
    topSequence += CscDecoder
 
  if muonRecFlags.doMDTs():
    MdtDecoder = getAlgorithm("MdtRdoToMdtPrepData", tryDefaultConfigurable=True)
    MdtDecoder.DoSeededDecoding=True
    MdtDecoder.RoIs="MURoIs"
    topSequence += MdtDecoder
 
  if muonRecFlags.doRPCs():
    RpcDecoder = getAlgorithm("RpcRdoToRpcPrepData", tryDefaultConfigurable=True)
    RpcDecoder.DoSeededDecoding=True
    RpcDecoder.RoIs="MURoIs"
    topSequence += RpcDecoder

  if muonRecFlags.doTGCs():
    TgcDecoder = getAlgorithm("TgcRdoToTgcPrepData", tryDefaultConfigurable=True)
    TgcDecoder.DoSeededDecoding=True
    TgcDecoder.RoIs="MURoIs"
    topSequence += TgcDecoder
    
  if muonRecFlags.doCSCs():
    topSequence += getAlgorithm("CscThresholdClusterBuilder")

  if muonRecFlags.doCreateClusters():
    from AthenaCommon.CfgGetter import getPublicTool
    from AthenaCommon import CfgMgr
    getPublicTool("MuonClusterizationTool")
    topSequence += CfgMgr.MuonClusterizationAlg("MuonClusterizationAlg",TgcPrepDataContainer="TGC_MeasurementsAllBCs" )
 
  # set up L1RoIsFilter 
  from DecisionHandling.DecisionHandlingConf import RoRSeqFilter
  filterL1RoIsAlg = RoRSeqFilter("filterL1RoIsAlg")
  filterL1RoIsAlg.Input = ["MURoIDecisions"]
  filterL1RoIsAlg.Output = ["FilteredMURoIDecisions"]
  filterL1RoIsAlg.Chains = ["HLT_mu6"]
  filterL1RoIsAlg.OutputLevel = DEBUG
 
  # set up MuFastSteering
  from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
  from TrigL2MuonSA.TrigL2MuonSAConfig import MuFastSteering
  muFastAlg = MuFastSteering("muFastAlg")
  muFastAlg.OutputLevel = DEBUG
  svcMgr.ToolSvc.TrigDataAccess.ApplyOffsetCorrection = False

  # set up MuFastHypo
  from TrigMuonHypo.TrigMuonHypoConfig import MufastHypo
  muFastHypo = MufastHypo("muFastHypo")
  muFastHypo.OutputLevel = DEBUG

  if viewTest:
    # MuFastAlg setup  
    allViewAlgorithms += muFastAlg
    svcMgr.ViewAlgPool.TopAlg += [ muFastAlg.getName() ]
    l2MuViewsMaker = EventViewCreatorAlgorithm("l2MuViewsMaker", OutputLevel=DEBUG)
    l2MuViewsMaker.ViewFallThrough = True
 
    l2MuViewsMaker.Decisions = "MURoIDecisions"
    l2MuViewsMaker.RoIsLink = "initialRoI" # -||-
    l2MuViewsMaker.InViewRoIs = "MURoIs" # contract with the consumer
    l2MuViewsMaker.Views = "MUViewRoIs"
 
    l2MuViewsMaker.AlgorithmNameSequence = [ muFastAlg.getName() ]

    muFastAlg.MuRoIs = l2MuViewsMaker.InViewRoIs
    muFastAlg.RecMuonRoI = "RecMURoIs"
    muFastAlg.MuFastDecisions = "MUFastDecisions"
    muFastAlg.MuFastComposite = "MUFastComposite"
    muFastAlg.MuFastForID = "MURoIIDDecisions"
    muFastAlg.MuFastForMS = "MURoIMSDecisions"
 
    # MuFastHypo setup
    #muFastHypo.MURoIs = l2MuViewsMaker.InViewRoIs
    #muFastHypo.ViewRoIs = l2MuViewsMaker.Views
    #muFastHypo.MuFastDeci = muFastAlg.MuFastDecisions
    #muFastHypo.Decisions = "MURoIDecisions"    # not yet impliment
    #muFastHypo.HypoTools =  [ TrigL2CaloHypoToolFromName("HLT_mu5"),   TrigL2CaloHypoToolFromName("HLT_mu7") ]
    #for t in muFastHypo.HypoTools:
    #  t.OutputLevel = DEBUG

    #muFastStep = stepSeq("muFastStep", filterL1RoIsAlg, [ l2MuViewsMaker, muFastHypo ])

  else:
    muFastAlg.MuRoIs = "MURoIs"
    muFastAlg.RecMuonRoI = "RecMURoIs"
    muFastAlg.MuFastDecisions = "MUFastDecisions"
    muFastAlg.MuFastComposite = "MUFastComposite"
    muFastAlg.MuFastForID = "MURoIIDDecisions"
    muFastAlg.MuFastForMS = "MURoIMSDecisions"

    #muFastHypo.MURoIs = "MURoIs"
    #muFastAlg.RecMuonRoI = "RecMURoIs"
    #muFastHypo.ViewRoIs = Nothing
    #muFastHypo.MuFastDeci = muFastAlg.MuFastDecisions 

  # CF construction
  if viewTest:
    topSequence += filterL1RoIsAlg
    topSequence += l2MuViewsMaker
    #step0 = parOR("step0", [ muFastStep ] )
    #topSequence += muFastStep

  else:
    topSequence += filterL1RoIsAlg
    topSequence += muFastAlg
    #topSequence += muFastHypo


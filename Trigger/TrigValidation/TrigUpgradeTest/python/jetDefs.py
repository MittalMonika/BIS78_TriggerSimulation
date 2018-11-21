#
#  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
#

def jetRecoSequence(RoIs, myRoIMode = True):

    from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    from AthenaCommon.AppMgr import ToolSvc
    from AthenaCommon.Constants import VERBOSE,DEBUG,INFO

    from AthenaCommon.CFElements import parOR, seqAND, seqOR, stepSeq
    jetRecoSequence = parOR("jetRecoSequence")



  # jets reco
    from AthenaMonitoring.GenericMonitoringTool import (GenericMonitoringTool,
                                                        defineHistogram)

    from TrigT2CaloCommon.TrigT2CaloCommonConf import TrigCaloDataAccessSvc
    import math
    mon = GenericMonitoringTool("CaloDataAccessSvcMon")
    mon.Histograms += [
        defineHistogram("TIME_locking_LAr_RoI",
                        title="Time spent in unlocking the LAr collection",
                        xbins=100, xmin=0, xmax=100 ),
        defineHistogram("roiROBs_LAr",
                        title="Number of ROBs unpacked in RoI requests",
                        xbins=20, xmin=0, xmax=20 ),
        defineHistogram("TIME_locking_LAr_FullDet",
                        title="Time spent in unlocking the LAr collection",
                        xbins=100, xmin=0, xmax=100 ),
        defineHistogram("roiEta_LAr,roiPhi_LAr",
                        type="TH2F",
                        title="Geometric usage",
                        xbins=50, xmin=-5, xmax=5,
                        ybins=64, ymin=-math.pi, ymax=math.pi )]

    svcMgr += TrigCaloDataAccessSvc()
    svcMgr.TrigCaloDataAccessSvc.OutputLevel=INFO
    svcMgr.TrigCaloDataAccessSvc.MonTool = mon


    

    from TrigCaloRec.TrigCaloRecConfig import HLTCaloCellMaker
    from TrigCaloRec.TrigCaloRecConf import HLTCaloCellSumMaker
    algo1=HLTCaloCellMaker("HLTCaloCellMaker")
    algo1.RoIs=RoIs
    #     algo1.RoIs="StoreGateSvc+FSJETRoIs"
    algo1.TrigDataAccessMT=svcMgr.TrigCaloDataAccessSvc
    algo1.roiMode = myRoIMode
    algo1.OutputLevel=VERBOSE
    jetRecoSequence += algo1

    

    from TrigCaloRec.TrigCaloRecConfig import TrigCaloClusterMakerMT_topo
    algo2 = TrigCaloClusterMakerMT_topo(doMoments=True, doLC=False)
    if myRoIMode:
        algo2.Cells = "CellsClusters"
    else:
        algo2.Cells = "CellsVClusters"
   
    algo2.OutputLevel = INFO
    jetRecoSequence += algo2


    # PseudoJetAlgorithm uses a tool to convert IParticles (eg CaloClusters)
    # to PseudoJets, which are the input to FastJet. The PseudoJets are
    # stored in a PseudoJetContainer, which is written top the event store.

    from JetRec.JetRecConf import (PseudoJetAlgorithm,
                                       PseudoJetGetter)


    pseudoJetGetter = PseudoJetGetter('simpleJobPJGetter')

    pseudoJetGetter.InputContainer = 'StoreGateSvc+caloclusters'
    pseudoJetGetter.OutputContainer = 'StoreGateSvc+PseudoJetEMTopo'
    pseudoJetGetter.Label = ''
    pseudoJetGetter.OutputLevel = DEBUG

    ToolSvc += pseudoJetGetter

    algo3 = PseudoJetAlgorithm()
    algo3.OutputLevel = VERBOSE
    algo3.PJGetter = pseudoJetGetter

    jetRecoSequence += algo3



    # JetAlgorithm and its Tools... Reads in PseudoJetContainers,
    # alls FastJet to cluster PseudoJets,
    # and then convert the output of FastJet (new pseudojets) to Atlas jets.

    from JetRec.JetRecConf import (JetAlgorithm,
                                       JetRecTool,
                                       JetFromPseudojet,
                                       JetFinder)


    name = 'simpleJob'

    # jet from Pseudo jet takes a pseudo jet returned by FastJet

    from JetRec.JetRecConf import JetFromPseudojet  
    jetBuilder = JetFromPseudojet(name+'JetBuilder')
    jetBuilder.OutputLevel = DEBUG
    ToolSvc += jetBuilder

    jetFinder = JetFinder(name+'JetFinder')
    jetFinder.JetBuilder = jetBuilder
    jetFinder.OutputLevel = DEBUG
    jetFinder.JetAlgorithm = 'AntiKt'
    jetFinder.VariableRMinRadius = -1
    jetFinder.VariableRMassScale = -1
    jetFinder.GhostArea = 0.01
    jetFinder.JetRadius = 0.4
    jetFinder.PtMin = 70000. 
    jetFinder.RandomOption = 1  #  1: used run/evt number to make seed

    ToolSvc += jetFinder

    jetRecTool = JetRecTool()
    jetRecTool.InputContainer = ''  # name of a jet collection.
    jetRecTool.OutputContainer = 'StoreGateSvc+jets'
    jetRecTool.JetFinder = jetFinder
    jetRecTool.JetModifiers = []
    jetRecTool.Trigger = False
    jetRecTool.OutputLevel = DEBUG
    jetRecTool.InputPseudoJets = ['StoreGateSvc+PseudoJetEMTopo'] 

    ToolSvc += jetRecTool

    algo4 = JetAlgorithm()
    algo4.Tools = [jetRecTool]

    jetRecoSequence += algo4


    sequenceOut= jetRecTool.OutputContainer

    return (jetRecoSequence,sequenceOut)

# 
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration 
# 
#  OutputLevel: INFO < DEBUG < VERBOSE 
#

from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaConfiguration.AllConfigFlags import ConfigFlags

TrackParticlesName = "HLT_xAODTracks_Muon"
  
def dimuL2Sequence(name = 'Dimu'):

    from TriggerMenuMT.HLTMenuConfig.Muon.MuonSequenceSetup import muNotCombAlgSequence
    (l2muNotCombSequence, l2muNotCombViewsMaker) = RecoFragmentsPool.retrieve(muNotCombAlgSequence, ConfigFlags)

    ### set up muCombHypo algorithm ###
    from TrigBphysHypo.TrigMultiTrkHypoMTConfig import TrigMultiTrkHypoMT
    from TrigBphysHypo.TrigMultiTrkHypoMTMonitoringConfig import TrigMultiTrkHypoAlgMTMonitoring
    
    jpsiHypo = TrigMultiTrkHypoMT(name+"HypoAlg")
    jpsiHypo.trackCollectionKey = TrackParticlesName
    jpsiHypo.bphysCollectionKey = "TrigBphys"+name
    jpsiHypo.nTrackMassMin = [100]
    jpsiHypo.nTrackMassMax = [20000]
    jpsiHypo.nTrk = 2
    jpsiHypo.MonTool = TrigMultiTrkHypoAlgMTMonitoring("TrigMultiTrkHypoAlgMTMonitoring_"+name)

    from TrigBphysHypo.TrigMultiTrkHypoMTConfig import TrigMultiTrkHypoToolMTFromDict

    return MenuSequence( Sequence    = l2muNotCombSequence,
                         Maker       = l2muNotCombViewsMaker,
                         Hypo        = jpsiHypo,
                         HypoToolGen = TrigMultiTrkHypoToolMTFromDict )
                         
def dimuEFSequence(name = 'Dimu'):
    from AthenaCommon import CfgMgr
    from AthenaCommon.CFElements import parOR, seqAND
    from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm


    dimuefViewNode = parOR("dimuefViewNode")
    
    dimuefViewsMaker = EventViewCreatorAlgorithm("dimuefViewsMaker")
    dimuefViewsMaker.ViewFallThrough = True
    dimuefViewsMaker.RoIsLink = "roi" # -||-
    dimuefViewsMaker.InViewRoIs = "DimuEFRoIs" # contract with the consumer
    dimuefViewsMaker.Views = "DimuEFViewRoIs"
    dimuefViewsMaker.ViewNodeName = dimuefViewNode.name()
   
    dimuefRecoSequence = parOR("dimuefViewNode")
    
    ViewVerifyEFCB = CfgMgr.AthViews__ViewDataVerifier("dimuefViewDataVerifier")
    ViewVerifyEFCB.DataObjects = [( 'xAOD::MuonContainer' , 'StoreGateSvc+MuonsCB' )]
    dimuefRecoSequence += ViewVerifyEFCB
    dimuefSequence = seqAND( "dimuefSequence", [dimuefViewsMaker, dimuefRecoSequence] )

    ### set up muCombHypo algorithm ###
    from TrigBphysHypo.TrigMultiTrkHypoMTConfig import TrigMultiTrkHypoMT
    from TrigBphysHypo.TrigMultiTrkHypoMTMonitoringConfig import TrigMultiTrkHypoAlgMTMonitoring
    
    jpsiHypo = TrigMultiTrkHypoMT(name+"HypoAlgEF")
    jpsiHypo.MuonCollectionKey = "MuonsCB"
    jpsiHypo.particleType = 1
    jpsiHypo.bphysCollectionKey = "TrigBphysEF"+name
    jpsiHypo.nTrackMassMin = [100]
    jpsiHypo.nTrackMassMax = [20000]
    jpsiHypo.nTrk = 2
    jpsiHypo.MonTool = TrigMultiTrkHypoAlgMTMonitoring("TrigMultiTrkHypoAlgEFMTMonitoring_"+name)

    from TrigBphysHypo.TrigMultiTrkHypoMTConfig import TrigMultiTrkHypoToolMTFromDict

    return MenuSequence( Sequence    = dimuefSequence,
                         Maker       = dimuefViewsMaker,
                         Hypo        = jpsiHypo,
                         HypoToolGen = TrigMultiTrkHypoToolMTFromDict )

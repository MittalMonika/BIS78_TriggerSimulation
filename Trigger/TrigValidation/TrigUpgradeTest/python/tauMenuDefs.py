#
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Include import include
from AthenaCommon.Constants import VERBOSE,DEBUG
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
import AthenaCommon.CfgMgr as CfgMgr
from AthenaConfiguration.AllConfigFlags import ConfigFlags

# menu components   
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence, RecoFragmentsPool
from AthenaCommon.CFElements import parOR, seqOR, seqAND, stepSeq
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from TrigUpgradeTest.tauDefs import tauCaloSequence

def inDetSetup():
    from InDetRecExample.InDetJobProperties import InDetFlags
    InDetFlags.doCaloSeededBrem = False
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
    from InDetRecExample.InDetKeys import InDetKeys
    include("InDetRecExample/InDetRecConditionsAccess.py")

# ====================================================================================================  
#    Get MenuSequences
# ==================================================================================================== 

def getTauSequence( step ):
    if step == "calo":
        return tauCaloMenuSequence("Tau")
    if step == "track_core":
        return tauCoreTrackSequence()
    return None

# ===============================================================================================
#      Fist step - tau calo
# ===============================================================================================

def tauCaloMenuSequence(name):
    (sequence, tauCaloViewsMaker, sequenceOut) = RecoFragmentsPool.retrieve(tauCaloSequence,ConfigFlags)

    # hypo
    from TrigTauHypo.TrigTauHypoConf import TrigTauCaloHypoAlgMT
    theTauCaloHypo = TrigTauCaloHypoAlgMT(name+"L2CaloHypo")
    theTauCaloHypo.taujets     = sequenceOut

    from TrigTauHypo.TrigL2TauHypoTool import TrigL2TauHypoToolFromDict

    return  MenuSequence( Sequence    = sequence,
                          Maker       = tauCaloViewsMaker,
                          Hypo        = theTauCaloHypo,
                          HypoToolGen = TrigL2TauHypoToolFromDict )

# ===============================================================================================
#      Second step - fast tracking
# ===============================================================================================

def tauCoreTrackSequence():

    from TriggerMenuMT.HLTMenuConfig.CommonSequences.InDetSetup import makeInDetAlgs
    (viewAlgs, eventAlgs) = makeInDetAlgs("TauCore")

    from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinder_TauCore

    theFTFCore = TrigFastTrackFinder_TauCore()
    theFTFCore.isRoI_Seeded = True
    viewAlgs.append(theFTFCore)


    # A simple algorithm to confirm that data has been inherited from parent view
    # Required to satisfy data dependencies
    ViewVerify = CfgMgr.AthViews__ViewDataVerifier("tauViewDataVerifier")
    ViewVerify.DataObjects = [('xAOD::TauJetContainer','StoreGateSvc+taujets')]
    ViewVerify.OutputLevel = DEBUG
    viewAlgs.append(ViewVerify)


    from TrigTauHypo.TrigTauHypoConf import TrigTauTrackRoiUpdaterMT
    TrackRoiUpdater = TrigTauTrackRoiUpdaterMT("TrackRoiUpdater")
    TrackRoiUpdater.OutputLevel  = DEBUG
    TrackRoiUpdater.RoIInputKey  = "TAUCaloRoIs"
    TrackRoiUpdater.RoIOutputKey = "RoiForID2"
    TrackRoiUpdater.fastTracksKey = "TrigFastTrackFinder_Tracks"

    l2TauViewsMaker = EventViewCreatorAlgorithm("l2TauViewsMaker", OutputLevel=DEBUG)
    l2TauViewsMaker.RoIsLink = "roi" # -||-
    l2TauViewsMaker.InViewRoIs = "TCoreRoIs" # contract with the fastCalo
    l2TauViewsMaker.Views = "TAUIDViews"
    l2TauViewsMaker.ViewFallThrough = True


    for viewAlg in viewAlgs:
       if viewAlg.properties().has_key("RoIs"):
         viewAlg.RoIs = l2TauViewsMaker.InViewRoIs
       if viewAlg.properties().has_key("roiCollectionName"):
         viewAlg.roiCollectionName = l2TauViewsMaker.InViewRoIs
    TrackRoiUpdater.RoIInputKey = l2TauViewsMaker.InViewRoIs

    tauInViewAlgs = parOR("tauInViewAlgs", viewAlgs + [ TrackRoiUpdater ])

    l2TauViewsMaker.ViewNodeName = "tauInViewAlgs"

    tauCoreTrkAthSequence = seqAND("tauCoreTrkAthSequence", eventAlgs + [l2TauViewsMaker, tauInViewAlgs ] )

    from TrigTauHypo.TrigTauHypoConf import  TrigTrackPreSelHypoAlgMT
    fastTrkHypo = TrigTrackPreSelHypoAlgMT("TrackPreSelHypoAlg")
    fastTrkHypo.OutputLevel = DEBUG
    fastTrkHypo.trackcollection = TrackRoiUpdater.fastTracksKey

    from TrigTauHypo.TrigTrackPreSelHypoTool import TrigTauTrackHypoToolFromDict

    return  MenuSequence( Sequence    = tauCoreTrkAthSequence,
                          Maker       = l2TauViewsMaker,
                          Hypo        = fastTrkHypo,
                          HypoToolGen = TrigTauTrackHypoToolFromDict )

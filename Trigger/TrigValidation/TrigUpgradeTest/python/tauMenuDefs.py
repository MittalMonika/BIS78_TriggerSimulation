#
#  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Include import include
from AthenaCommon.Constants import VERBOSE,DEBUG
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
import AthenaCommon.CfgMgr as CfgMgr


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
#include("InDetRecExample/InDetRecConditionsAccess.py")
from InDetRecExample.InDetKeys import InDetKeys

# menu components   
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence

# ===============================================================================================
#      First step - Calo Roi Updater
# ===============================================================================================

from TrigCaloRec.TrigCaloRecConfig import TrigCaloCellMakerMT_tau
cellMaker = TrigCaloCellMakerMT_tau("CaloCellMakerTau")
cellMaker.OutputLevel=DEBUG

from TrigCaloRec.TrigCaloRecConfig import TrigCaloClusterMakerMT_topo
clusMaker = TrigCaloClusterMakerMT_topo("CaloClusMakerTopo")
clusMaker.OutputLevel=VERBOSE
 
from AthenaCommon.CFElements import parOR, seqOR, seqAND, stepSeq
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm

fastCaloViewsMaker = EventViewCreatorAlgorithm("fastCaloViewsMakerTau", OutputLevel=VERBOSE)
fastCaloViewsMaker.ViewFallThrough = True
fastCaloViewsMaker.RoIsLink = "initialRoI"
fastCaloViewsMaker.InViewRoIs = "TAUCaloRoIs"
fastCaloViewsMaker.Views = "TAUCaloViews"
fastCaloViewsMaker.ViewNodeName = "fastCaloInViewAlgsTau"
cellMaker.RoIs = fastCaloViewsMaker.InViewRoIs

CaloViewVerify = CfgMgr.AthViews__ViewDataVerifier("FastCaloViewDataVerifier")
CaloViewVerify.DataObjects = [('TrigRoiDescriptorCollection' , 'StoreGateSvc+fastCaloViewsMaker_InViewRoIs_out')]

from TrigTauHypo.TrigTauHypoConf import TrigTauCaloRoiUpdaterMT
CaloRoiUpdater = TrigTauCaloRoiUpdaterMT("CaloRoiUpdater")
CaloRoiUpdater.OutputLevel  = DEBUG
CaloRoiUpdater.RoIInputKey  = "TAUCaloRoIs"
CaloRoiUpdater.RoIOutputKey = "RoiForTau"

from TrigTauRec.TrigTauRecConfigMT import TrigTauRecMerged_TauCaloOnly
caloRec = TrigTauRecMerged_TauCaloOnly()
caloRec.OutputLevel  = DEBUG
caloRec.RoIInputKey = "RoiForTau"
caloRec.L1RoIKey    = "TAUCaloRoIs"
caloRec.clustersKey = "caloclusters"

fastCaloInViewAlgs  = seqAND("fastCaloInViewAlgsTau", [cellMaker,clusMaker,CaloRoiUpdater,caloRec])
fastCaloAthSequence = seqAND("fastCaloAthSequenceTau",[fastCaloViewsMaker, fastCaloInViewAlgs])

from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestHypoAlg
from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestHypoTool

def genCaloHypoToolTau( name, conf ):
    return HLTTest__TestHypoTool("name")

def tauCaloSequence():
    return  MenuSequence( Sequence    = fastCaloAthSequence,
                          Maker       = fastCaloViewsMaker,
                          Hypo        = HLTTest__TestHypoAlg("DummyTauCaloHypo", Input=""),
                          HypoToolGen = genCaloHypoToolTau )


# ===============================================================================================
#      Second step - TrigTauRec CaloOnly
# ===============================================================================================

#caloRecViewsMaker = EventViewCreatorAlgorithm("caloRecViewsMakerTau", OutputLevel=VERBOSE)
#caloRecViewsMaker.ViewFallThrough = True
#caloRecViewsMaker.RoIsLink        = "initialRoI"
#caloRecViewsMaker.InViewRoIs      = "TAUCaloRecRoIs"
#caloRecViewsMaker.Views           = "TAUCaloRecViews"
#caloRecViewsMaker.ViewNodeName    = "caloRecInViewAlgsTau"

#caloRec.RoIs = caloRecViewsMaker.InViewRoIs

#caloRecInViewAlgs  = seqAND("caloRecInViewAlgsTau", [caloRec])

#caloRecAthSequence =  seqAND("caloRecAthSequenceTau",[caloRecViewsMaker, caloRecInViewAlgs])

#def genCaloRecHypoToolTau( name, conf ):
#    return HLTTest__TestHypoTool("name")

#def tauCaloRecSequence():
#    return  MenuSequence( Sequence    = caloRecAthSequence,
#                          Maker       = caloRecViewsMaker,
#                          Hypo        = HLTTest__TestHypoAlg("DummyTauCaloRecHypo", Input=""),
#                          HypoToolGen = genCaloRecHypoToolTau )

# ===============================================================================================
#      Third step - Generic Hypo Track Calo Rec
# ===============================================================================================

#from TrigTauHypo.TrigL2TauHypoTool import TrigL2TauHypoToolFromName

#from TrigTauHypo.TrigTauHypoConf import TrigL2TauCaloHypoAlgMT
#fastCaloHypo = TrigL2TauCaloHypoAlgMT("L2TauCaloHypo")
#fastCaloHypo.OutputLevel = DEBUG


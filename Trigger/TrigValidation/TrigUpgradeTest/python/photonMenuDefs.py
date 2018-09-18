#
#  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Include import include
from AthenaCommon.Constants import VERBOSE,DEBUG
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
import AthenaCommon.CfgMgr as CfgMgr

# menu components   
from TrigUpgradeTest.MenuComponents import MenuSequence

# ===============================================================================================
#      L2 Calo
# ===============================================================================================

from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import T2CaloEgamma_ReFastAlgo
theFastCaloAlgo=T2CaloEgamma_ReFastAlgo("FastCaloAlgo" )
theFastCaloAlgo.OutputLevel=VERBOSE
theFastCaloAlgo.ClustersName="L2CaloClusters"
svcMgr.ToolSvc.TrigDataAccess.ApplyOffsetCorrection=False


from AthenaCommon.CFElements import parOR, seqOR, seqAND, stepSeq
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm

fastCaloInViewAlgs = seqAND("fastCaloInViewAlgs", [theFastCaloAlgo])


fastCaloViewsMaker = EventViewCreatorAlgorithm("fastCaloViewsMaker", OutputLevel=DEBUG)
fastCaloViewsMaker.ViewFallThrough = True
fastCaloViewsMaker.RoIsLink = "initialRoI" # -||-
fastCaloViewsMaker.InViewRoIs = "EMCaloRoIs" # contract with the fastCalo
fastCaloViewsMaker.Views = "EMCaloViews"
fastCaloViewsMaker.ViewNodeName = "fastCaloInViewAlgs"
theFastCaloAlgo.RoIs = fastCaloViewsMaker.InViewRoIs

# are these needed?
CaloViewVerify = CfgMgr.AthViews__ViewDataVerifier("FastCaloViewDataVerifier")
CaloViewVerify.DataObjects = [('TrigRoiDescriptorCollection' , 'StoreGateSvc+fastCaloViewsMaker_InViewRoIs_out')]


from TrigEgammaHypo.TrigEgammaHypoConf import TrigL2CaloHypoAlgMT
theFastCaloHypo = TrigL2CaloHypoAlgMT("L2CaloHypo")
theFastCaloHypo.OutputLevel = DEBUG
theFastCaloHypo.CaloClusters = theFastCaloAlgo.ClustersName


fastCaloAthSequence =  seqAND("fastCaloAthSequence",[fastCaloViewsMaker, fastCaloInViewAlgs ])


from TrigEgammaHypo.TrigL2CaloHypoTool import TrigL2CaloHypoToolFromName

def fastCaloSequence():
    return MenuSequence( Sequence    = fastCaloAthSequence,
                         Maker       = fastCaloViewsMaker, 
                         Hypo        = theFastCaloHypo,
                         HypoToolGen = TrigL2CaloHypoToolFromName )



ViewVerify = CfgMgr.AthViews__ViewDataVerifier("photonViewDataVerifier")
ViewVerify.DataObjects = [('xAOD::TrigEMClusterContainer','StoreGateSvc+L2CaloClusters')]
ViewVerify.OutputLevel = DEBUG

from TrigEgammaHypo.TrigL2PhotonFexMTConfig import L2PhotonFex_1
thePhotonFex= L2PhotonFex_1()
thePhotonFex.TrigEMClusterName = theFastCaloAlgo.ClustersName
thePhotonFex.PhotonsName="Photons"
thePhotonFex.OutputLevel=VERBOSE
#thePhotonFex.RoIs="EMIDRoIs"



l2PhotonViewsMaker = EventViewCreatorAlgorithm("l2PhotonViewsMaker", OutputLevel=DEBUG)
l2PhotonViewsMaker.RoIsLink = "roi"
l2PhotonViewsMaker.InViewRoIs = "EMIDRoIs" 
#l2PhotonViewsMaker.InViewRoIs = "EMCaloRoIs"
l2PhotonViewsMaker.Views = "EMPhotonViews"
l2PhotonViewsMaker.ViewFallThrough = True
thePhotonFex.RoIs = l2PhotonViewsMaker.InViewRoIs


photonInViewAlgs = parOR("photonInViewAlgs", [ViewVerify, thePhotonFex ])

l2PhotonViewsMaker.ViewNodeName = "photonInViewAlgs"


from TrigEgammaHypo.TrigEgammaHypoConf import TrigL2PhotonHypoAlgMT
thePhotonHypo = TrigL2PhotonHypoAlgMT()
thePhotonHypo.Photons = thePhotonFex.PhotonsName
thePhotonHypo.RunInView=True
thePhotonHypo.OutputLevel = VERBOSE

# this needs to be added:
#electronDecisionsDumper = DumpDecisions("electronDecisionsDumper", OutputLevel=DEBUG, Decisions = theElectronHypo.Output )    



photonAthSequence = seqAND("photonAthSequence",  [l2PhotonViewsMaker, photonInViewAlgs] )

from TrigEgammaHypo.TrigL2PhotonHypoTool import TrigL2PhotonHypoToolFromName
def photonSequence():
    return MenuSequence( Maker=l2PhotonViewsMaker,
                             Sequence=photonAthSequence,
                             Hypo=thePhotonHypo,
                             HypoToolGen=TrigL2PhotonHypoToolFromName)


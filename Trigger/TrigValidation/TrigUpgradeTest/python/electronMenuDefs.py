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
include("InDetRecExample/InDetRecConditionsAccess.py")
from InDetRecExample.InDetKeys import InDetKeys

# menu components   
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence

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
    return  MenuSequence( Sequence    = fastCaloAthSequence,
                          Maker       = fastCaloViewsMaker,
                          Hypo        = theFastCaloHypo,
                          HypoToolGen = TrigL2CaloHypoToolFromName )

#########################################
# second step:  tracking.....
#########################################
#


from TrigUpgradeTest.InDetSetup import makeInDetAlgs

(viewAlgs, eventAlgs) = makeInDetAlgs()
from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinder_eGamma

theFTF = TrigFastTrackFinder_eGamma()
theFTF.isRoI_Seeded = True
viewAlgs.append(theFTF)


# A simple algorithm to confirm that data has been inherited from parent view
# Required to satisfy data dependencies
ViewVerify = CfgMgr.AthViews__ViewDataVerifier("electronViewDataVerifier")
ViewVerify.DataObjects = [('xAOD::TrigEMClusterContainer','StoreGateSvc+L2CaloClusters')]
ViewVerify.OutputLevel = DEBUG
viewAlgs.append(ViewVerify)

TrackParticlesName = ""
for viewAlg in viewAlgs:
  if viewAlg.name() == "InDetTrigTrackParticleCreatorAlg":
    TrackParticlesName = viewAlg.TrackParticlesName
    

from TrigEgammaHypo.TrigL2ElectronFexMTConfig import L2ElectronFex_1
theElectronFex= L2ElectronFex_1()
theElectronFex.TrigEMClusterName = theFastCaloAlgo.ClustersName
theElectronFex.TrackParticlesName = TrackParticlesName
theElectronFex.ElectronsName="Electrons"
theElectronFex.OutputLevel=VERBOSE


l2ElectronViewsMaker = EventViewCreatorAlgorithm("l2ElectronViewsMaker", OutputLevel=DEBUG)
l2ElectronViewsMaker.RoIsLink = "roi" # -||-
l2ElectronViewsMaker.InViewRoIs = "EMIDRoIs" # contract with the fastCalo
l2ElectronViewsMaker.Views = "EMElectronViews"
l2ElectronViewsMaker.ViewFallThrough = True


for viewAlg in viewAlgs:
  if viewAlg.properties().has_key("RoIs"):
    viewAlg.RoIs = l2ElectronViewsMaker.InViewRoIs
  if viewAlg.properties().has_key("roiCollectionName"):
    viewAlg.roiCollectionName = l2ElectronViewsMaker.InViewRoIs
theElectronFex.RoIs = l2ElectronViewsMaker.InViewRoIs    

electronInViewAlgs = parOR("electronInViewAlgs", viewAlgs + [ theElectronFex ])

l2ElectronViewsMaker.ViewNodeName = "electronInViewAlgs"


from TrigEgammaHypo.TrigEgammaHypoConf import TrigL2ElectronHypoAlgMT
theElectronHypo = TrigL2ElectronHypoAlgMT()
theElectronHypo.Electrons = theElectronFex.ElectronsName
theElectronHypo.RunInView=True
theElectronHypo.OutputLevel = VERBOSE

# this needs to be added:
#electronDecisionsDumper = DumpDecisions("electronDecisionsDumper", OutputLevel=DEBUG, Decisions = theElectronHypo.Output )    

electronAthSequence = seqAND("electronAthSequence", eventAlgs + [l2ElectronViewsMaker, electronInViewAlgs ] )

from TrigEgammaHypo.TrigL2ElectronHypoTool import TrigL2ElectronHypoToolFromName

def electronSequence():
    return  MenuSequence( Maker       = l2ElectronViewsMaker,                                        
                          Sequence    = electronAthSequence,
                          Hypo        = theElectronHypo,
                          HypoToolGen = TrigL2ElectronHypoToolFromName )


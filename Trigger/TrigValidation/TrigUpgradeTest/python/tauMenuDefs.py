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

# from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import T2CaloEgamma_ReFastAlgo
# theFastCaloAlgo=T2CaloEgamma_ReFastAlgo("FastCaloAlgo" )
# theFastCaloAlgo.OutputLevel=VERBOSE
# theFastCaloAlgo.ClustersName="L2CaloClusters"
# svcMgr.ToolSvc.TrigDataAccess.ApplyOffsetCorrection=False

# ================
#      CellMaker
# ================

from TrigCaloRec.TrigCaloRecConfig import TrigCaloCellMakerMT_tau
cellMaker = TrigCaloCellMakerMT_tau("CaloCellMakerTau")
cellMaker.OutputLevel=DEBUG

from TrigCaloRec.TrigCaloRecConfig import TrigCaloClusterMakerMT_topo
clusMaker = TrigCaloClusterMakerMT_topo("CaloClusMakerTopo")
clusMaker.OutputLevel=VERBOSE
 
from AthenaCommon.CFElements import parOR, seqOR, seqAND, stepSeq
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm

fastCaloInViewAlgs = seqAND("fastCaloInViewAlgsTau", [cellMaker,clusMaker])

fastCaloViewsMaker = EventViewCreatorAlgorithm("fastCaloViewsMakerTau", OutputLevel=VERBOSE)
fastCaloViewsMaker.ViewFallThrough = True
fastCaloViewsMaker.RoIsLink = "initialRoI" # -||-
fastCaloViewsMaker.InViewRoIs = "TAUCaloRoIs" # contract with the fastCalo
fastCaloViewsMaker.Views = "TAUCaloViews"
fastCaloViewsMaker.ViewNodeName = "fastCaloInViewAlgsTau"
cellMaker.RoIs = fastCaloViewsMaker.InViewRoIs

# are these needed?
CaloViewVerify = CfgMgr.AthViews__ViewDataVerifier("FastCaloViewDataVerifier")
CaloViewVerify.DataObjects = [('TrigRoiDescriptorCollection' , 'StoreGateSvc+fastCaloViewsMaker_InViewRoIs_out')]

fastCaloAthSequence =  seqAND("fastCaloAthSequenceTau",[fastCaloViewsMaker, fastCaloInViewAlgs ])

#from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestHypoAlg
#from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestHypoTool


#def genCaloHypoToolTau( name, conf ):
#    return HLTTest__TestHypoTool("name")

#def tauCaloSequence():
#    return  MenuSequence( Sequence    = fastCaloAthSequence,
#                          Maker       = fastCaloViewsMaker,
#                          Hypo        = HLTTest__TestHypoAlg("L2TauCaloHypoAlg", Input=""),
#                          HypoToolGen = genCaloHypoToolTau )

from TrigTauHypo.TrigTauHypoConf import TrigL2TauCaloHypoAlgMT
fastCaloHypo = TrigL2TauCaloHypoAlgMT("L2TauCaloHypo")
fastCaloHypo.OutputLevel = DEBUG
#fastCaloHypo.CaloClusters = 


fastCaloAthSequence =  seqAND("fastCaloAthSequence",[fastCaloViewsMaker, fastCaloInViewAlgs ])

from TrigTauHypo.TrigL2TauHypoTool import TrigL2TauHypoToolFromName
#from TrigTauHypo.TrigTauHypoBase import 
#from TrigTauHypo.L2TauHypoTool import TrigL2TauHypoTool
#from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestHypoTool

#def genCaloHypoToolTau( name, conf ):
#    return HLTTest__TestHypoTool("name")

#from TrigTauHypo.TrigL2CaloHypoTool import TrigL2CaloHypoToolFromName

def tauCaloSequence():
    return  MenuSequence( Sequence    = fastCaloAthSequence,
                          Maker       = fastCaloViewsMaker,
                          Hypo        = fastCaloHypo,
                          HypoToolGen = TrigL2TauHypoToolFromName )


# #########################################
# # second step:  tracking.....
# #########################################
# #


# from TrigUpgradeTest.InDetSetup import makeInDetAlgs

# (viewAlgs, eventAlgs) = makeInDetAlgs()
# from TrigFastTrackFinder.TrigFastTrackFinder_Config import TrigFastTrackFinder_eGamma

# theFTF = TrigFastTrackFinder_eGamma()
# theFTF.isRoI_Seeded = True
# viewAlgs.append(theFTF)


# # A simple algorithm to confirm that data has been inherited from parent view
# # Required to satisfy data dependencies
# ViewVerify = CfgMgr.AthViews__ViewDataVerifier("electronViewDataVerifier")
# ViewVerify.DataObjects = [('xAOD::TrigEMClusterContainer','StoreGateSvc+L2CaloClusters')]
# ViewVerify.OutputLevel = DEBUG
# viewAlgs.append(ViewVerify)

# TrackParticlesName = ""
# for viewAlg in viewAlgs:
#   if viewAlg.name() == "InDetTrigTrackParticleCreatorAlg":
#     TrackParticlesName = viewAlg.TrackParticlesName
    

# from TrigEgammaHypo.TrigL2ElectronFexMTConfig import L2ElectronFex_1
# theElectronFex= L2ElectronFex_1()
# theElectronFex.TrigEMClusterName = theFastCaloAlgo.ClustersName
# theElectronFex.TrackParticlesName = TrackParticlesName
# theElectronFex.ElectronsName="Electrons"
# theElectronFex.OutputLevel=VERBOSE


# l2ElectronViewsMaker = EventViewCreatorAlgorithm("l2ElectronViewsMaker", OutputLevel=DEBUG)
# l2ElectronViewsMaker.RoIsLink = "roi" # -||-
# l2ElectronViewsMaker.InViewRoIs = "EMIDRoIs" # contract with the fastCalo
# l2ElectronViewsMaker.Views = "EMElectronViews"
# l2ElectronViewsMaker.ViewFallThrough = True


# for viewAlg in viewAlgs:
#   if viewAlg.properties().has_key("RoIs"):
#     viewAlg.RoIs = l2ElectronViewsMaker.InViewRoIs
#   if viewAlg.properties().has_key("roiCollectionName"):
#     viewAlg.roiCollectionName = l2ElectronViewsMaker.InViewRoIs
# theElectronFex.RoIs = l2ElectronViewsMaker.InViewRoIs    

# electronInViewAlgs = parOR("electronInViewAlgs", viewAlgs + [ theElectronFex ])

# l2ElectronViewsMaker.ViewNodeName = "electronInViewAlgs"


# from TrigEgammaHypo.TrigEgammaHypoConf import TrigL2ElectronHypoAlgMT
# theElectronHypo = TrigL2ElectronHypoAlgMT()
# theElectronHypo.Electrons = theElectronFex.ElectronsName
# theElectronHypo.RunInView=True
# theElectronHypo.OutputLevel = VERBOSE

# # this needs to be added:
# #electronDecisionsDumper = DumpDecisions("electronDecisionsDumper", OutputLevel=DEBUG, Decisions = theElectronHypo.Output )    

# electronAthSequence = seqAND("electronAthSequence", eventAlgs + [l2ElectronViewsMaker, electronInViewAlgs ] )

# from TrigEgammaHypo.TrigL2ElectronHypoTool import TrigL2ElectronHypoToolFromName

# def electronSequence():
#     return  MenuSequence( Maker       = l2ElectronViewsMaker,                                        
#                           Sequence    = electronAthSequence,
#                           Hypo        = theElectronHypo,
#                           HypoToolGen = TrigL2ElectronHypoToolFromName )


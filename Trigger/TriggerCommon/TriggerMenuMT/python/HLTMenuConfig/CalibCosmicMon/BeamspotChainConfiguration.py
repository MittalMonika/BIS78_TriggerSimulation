# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger("TriggerMenuMT.HLTMenuConfig.CalibCosmicMon.BeamspotChainConfiguration")

from TriggerMenuMT.HLTMenuConfig.Menu.ChainConfigurationBase import ChainConfigurationBase
from TrigStreamerHypo.TrigStreamerHypoConfigMT import StreamerHypoToolMTgenerator
from TrigStreamerHypo.TrigStreamerHypoConf import TrigStreamerHypoAlgMT
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence
from AthenaCommon.CFElements import seqAND
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm, ViewCreatorInitialROITool


#----------------------------------------------------------------
# fragments generating configuration will be functions in New JO, 
# so let's make them functions already now
#----------------------------------------------------------------

 

def allTE_trkfast_Cfg( flags ):
        return allTE_trkfast()

def allTE_trkfast():
        inputMakerAlg = EventViewCreatorAlgorithm("IM_beamspot")
        inputMakerAlg.ViewFallThrough = True
        inputMakerAlg.RoIsLink = "initialRoI"
        inputMakerAlg.RoITool = ViewCreatorInitialROITool()
        inputMakerAlg.InViewRoIs = "beamspotInputRoIs"
        inputMakerAlg.Views = "beamspotViewRoIs"

        from TrigInDetConfig.InDetSetup import makeInDetAlgs
        viewAlgs, viewVerify = makeInDetAlgs(whichSignature='FS', rois=inputMakerAlg.InViewRoIs)
        from TrigT2BeamSpot.T2VertexBeamSpotConfig import T2VertexBeamSpot_activeAllTE
        T2VertexBeamSpot_activeAllTE.TrackCollections = ["TrigFastTrackFinder_Tracks_FS"]

        viewVerify.DataObjects += [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+beamspotInputRoIs' ),
                                   ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' )]

        # Make sure the event info is still available at whole-event level
        from AthenaCommon.AlgSequence import AlgSequence
        topSequence = AlgSequence()
        topSequence.SGInputLoader.Load += [( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' )]

        beamspotSequence = seqAND("beamspotSequence",viewAlgs+[T2VertexBeamSpot_activeAllTE()])
        inputMakerAlg.ViewNodeName = beamspotSequence.name()

        #hypo
        beamspotHypoAlg = TrigStreamerHypoAlgMT("BeamspotHypoAlg")
        beamspotHypoAlg.RuntimeValidation = False #Needed to avoid the ERROR ! Decision has no 'feature' ElementLink
        beamspotHypoToolGen= StreamerHypoToolMTgenerator
        beamspotViewsSequence = seqAND("beamspotViewsSequence", [ inputMakerAlg, beamspotSequence ])

        return  MenuSequence( Sequence    = beamspotViewsSequence,
                          Maker       = inputMakerAlg,
                          Hypo        = beamspotHypoAlg,
                          HypoToolGen = beamspotHypoToolGen )

# Class to configure chain
#----------------------------------------------------------------
class BeamspotChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)


    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChain(self):                            
        chainSteps = []
        log.debug("Assembling chain for " + self.chainName)
        # --------------------
        # define here the names of the steps and obtain the chainStep configuration 
        # --------------------
        stepDictionary = {
            "allTE_trkfast":[self.getAllTEStep()]
            #"activeTE_trkfast":[self.activeTE_trkfast()],
            #"trkFS_trkfast":[self.trkFS_trkfast()],
        }

        #key = self.chainPart['EFrecoAlg']
        key = self.chainPart['addInfo'][0] + "_" + self.chainPart['l2IDAlg'][0]#TODO: hardcoded index
        steps=stepDictionary[key]
        for step in steps:
            chainSteps+=[step]
            
        myChain = self.buildChain(chainSteps)
        return myChain

   
    # --------------------
    # Configuration of costmonitor
    # --------------------
    def getAllTEStep(self):
        return self.getStep(1,"allTE_trkfast",[allTE_trkfast_Cfg])

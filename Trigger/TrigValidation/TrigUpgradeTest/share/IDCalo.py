#
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

include("TrigUpgradeTest/testHLT_MT.py")

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

# ----------------------------------------------------------------
# Setup Views
# ----------------------------------------------------------------
from AthenaCommon.AlgSequence import AthSequencer
viewSeq = AthSequencer("AthViewSeq", Sequential=True, ModeOR=False, StopOverride=False)
topSequence += viewSeq

  
# View maker alg
from AthenaCommon import CfgMgr
viewNodeName = "allViewAlgorithms"
viewMaker = CfgMgr.AthViews__RoiCollectionToViews("viewMaker")
viewMaker.ViewBaseName = "testView"
viewMaker.InputRoICollection = "EMRoIs"
viewMaker.ViewNodeName = viewNodeName
viewMaker.OutputRoICollection = "EMViewRoIs"
viewMaker.ViewFallThrough = True
viewSeq += viewMaker

# Set of view algs
allViewAlgorithms = AthSequencer(viewNodeName, Sequential=False, ModeOR=False, StopOverride=False)


if TriggerFlags.doID:

  from TriggerMenuMT.HLTMenuConfig.CommonSequences.InDetSetup import makeInDetAlgs
  
  (viewAlgs, eventAlgs) = makeInDetAlgs("FS")

  for eventAlg in eventAlgs:
    viewSeq += eventAlg

  for viewAlg in viewAlgs:
    allViewAlgorithms += viewAlg
    if "RoIs" in viewAlg.properties():
        viewAlg.RoIs = "EMViewRoIs"

   #
   # --- Ambiguity solver algorithm
   #
 
  from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigTrackSummaryTool
  from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigExtrapolator
  from InDetTrackScoringTools.InDetTrackScoringToolsConf import InDet__InDetAmbiScoringTool
  InDetTrigMTAmbiScoringTool =  InDet__InDetAmbiScoringTool( name                        = 'InDetTrigMTScoringTool',
                                                             Extrapolator                = InDetTrigExtrapolator,
                                                             InputEmClusterContainerName = '', #need to be reset to empty string
                                                             doEmCaloSeed                = False,
                                                             SummaryTool                 = InDetTrigTrackSummaryTool)



  ToolSvc += InDetTrigMTAmbiScoringTool

  from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigAmbiTrackSelectionTool
  from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigTrackFitter
  from TrkAmbiguityProcessor.TrkAmbiguityProcessorConf import Trk__SimpleAmbiguityProcessorTool as ProcessorTool
  InDetTrigMTAmbiguityProcessor = ProcessorTool(name          = 'InDetTrigMTAmbiguityProcessor',
                                                Fitter        = InDetTrigTrackFitter,
                                                ScoringTool   = InDetTrigMTAmbiScoringTool,
                                                SelectionTool = InDetTrigAmbiTrackSelectionTool)


  ToolSvc += InDetTrigMTAmbiguityProcessor


  from TrkAmbiguitySolver.TrkAmbiguitySolverConf import Trk__TrkAmbiguitySolver
  InDetTrigMTAmbiguitySolver = Trk__TrkAmbiguitySolver(name         = 'InDetTrigMTAmbiguitySolver',
                                                 TrackInput         =['TrigFastTrackFinder_Tracks_FS'], #FTF default
                                                 TrackOutput        = 'AmbiSolver_Tracks' , #Change
                                                 AmbiguityProcessor = InDetTrigMTAmbiguityProcessor)


  allViewAlgorithms += InDetTrigMTAmbiguitySolver


  #
  # --- Track particle conversion algorithm
  #


  from TrkParticleCreator.TrkParticleCreatorConf import Trk__TrackParticleCreatorTool
  InDetTrigMTxAODParticleCreatorTool = Trk__TrackParticleCreatorTool(name =  "InDetTrigMTxAODParticleCreatorTool",
                                                                     Extrapolator = InDetTrigExtrapolator,
                                                                     #TrackSummaryTool = InDetTrigTrackSummaryToolSharedHits) 
                                                                     TrackSummaryTool = InDetTrigTrackSummaryTool)

  ToolSvc += InDetTrigMTxAODParticleCreatorTool
  print InDetTrigMTxAODParticleCreatorTool


  from xAODTrackingCnv.xAODTrackingCnvConf import xAODMaker__TrackCollectionCnvTool
  InDetTrigMTxAODTrackCollectionCnvTool= xAODMaker__TrackCollectionCnvTool(name = "InDetTrigMTxAODTrackCollectionCnvTool",
                                                                           TrackParticleCreator = InDetTrigMTxAODParticleCreatorTool)

  ToolSvc += InDetTrigMTxAODTrackCollectionCnvTool
  print InDetTrigMTxAODTrackCollectionCnvTool

  from xAODTrackingCnv.xAODTrackingCnvConf import  xAODMaker__RecTrackParticleContainerCnvTool
  InDetTrigMTRecTrackParticleContainerCnvTool=  xAODMaker__RecTrackParticleContainerCnvTool(name = "InDetTrigMTRecTrackContainerCnvTool",
                                                                           TrackParticleCreator = InDetTrigMTxAODParticleCreatorTool)

  ToolSvc += InDetTrigMTRecTrackParticleContainerCnvTool
  print InDetTrigMTRecTrackParticleContainerCnvTool

  from xAODTrackingCnv.xAODTrackingCnvConf import xAODMaker__TrackParticleCnvAlg
  InDetTrigMTxAODTrackParticleCnvAlg = xAODMaker__TrackParticleCnvAlg(name = "InDetTrigMTxAODParticleCreatorAlg",
                                                                      TrackContainerName = 'InDetTrigMTAmbiSolTracks',
                                                                      xAODContainerName = 'InDetTrigMTAmbiSolxAODTracks',
                                                                      TrackCollectionCnvTool = InDetTrigMTxAODTrackCollectionCnvTool,
                                                                      RecTrackParticleContainerCnvTool = InDetTrigMTRecTrackParticleContainerCnvTool,
                                                                      TrackParticleCreator = InDetTrigMTxAODParticleCreatorTool
                                                                            )


  allViewAlgorithms += InDetTrigMTxAODTrackParticleCnvAlg
  print  InDetTrigMTxAODTrackParticleCnvAlg



if TriggerFlags.doCalo:
  from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import T2CaloEgamma_ReFastAlgo
  algo=T2CaloEgamma_ReFastAlgo("testFastAlgo")

  algo.RoIs="EMViewRoIs"
  allViewAlgorithms += algo

viewSeq += allViewAlgorithms
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

# menu components
from AthenaCommon.CFElements import seqAND
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence
from TrigEDMConfig.TriggerEDMRun3 import recordable

#from AthenaCommon.Constants import DEBUG

# ====================================================================================================  
#    Get MenuSequences
# ==================================================================================================== 

def getBJetSequence( step ):
    if step == "j":
        return bJetStep1Sequence()
    if step == "btag":
        return bJetStep2Sequence()
    return None

# ==================================================================================================== 
#    step 1: This is Jet code. Not here!
# ==================================================================================================== 

# ==================================================================================================== 
#    step 2: retrieving jets from jet-trigger code and optional first stage of fast tracking with primary vertex
# ==================================================================================================== 

def bJetStep1Sequence():
    jetsKey = "HLT_AntiKt4EMTopoJets_subjesgscIS_ftf"
    prmVtxKey = "HLT_EFHistoPrmVtx"

    from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
    InputMakerAlg = EventViewCreatorAlgorithm( "IMBJet_step2", RoIsLink="initialRoI" )
    InputMakerAlg.Views = "FullScanBjetView"
    InputMakerAlg.InViewRoIs = "FullScanRoI"
    InputMakerAlg.ViewFallThrough = True


    outputJetName = "HLT_GSCJet"
    outputRoIName = "HLT_GSCJet_RoIs"

    # Jet Selector
    from TrigBjetHypo.TrigBjetHypoConf import TrigJetSelectorMT
    jetSelector = TrigJetSelectorMT( "BJetSelector" )
    jetSelector.InputJets = jetsKey
    jetSelector.InputVertex = prmVtxKey
    jetSelector.OutputJets = recordable( outputJetName )
    jetSelector.OutputRoi = outputRoIName

    bJetEtSequence = seqAND( "bJetEtSequence",[jetSelector] )
    InputMakerAlg.ViewNodeName = "bJetEtSequence"

    # Sequence
    BjetAthSequence = seqAND("BjetAthSequence_step1",[InputMakerAlg,bJetEtSequence])

    # hypo
    from TrigBjetHypo.TrigBjetHypoConf import TrigBjetEtHypoAlgMT
    hypo = TrigBjetEtHypoAlgMT( "TrigBjetEtHypoAlgMT" )
    hypo.RoILink = "step1RoI"
    hypo.PrmVtxLink = prmVtxKey.replace( "HLT_","" )
    hypo.Jets = outputJetName
    hypo.RoIs = outputRoIName
    hypo.PrmVtx = prmVtxKey
    hypo.RetrieveVertexFromEventView = False

    from TrigBjetHypo.TrigBjetEtHypoTool import TrigBjetEtHypoToolFromDict
    return MenuSequence( Sequence    = BjetAthSequence,
                         Maker       = InputMakerAlg,
                         Hypo        = hypo,
                         HypoToolGen = TrigBjetEtHypoToolFromDict )

# ==================================================================================================== 
#    step 3: Second stage of fast tracking, Precision tracking, and flavour tagging
# ==================================================================================================== 

def bJetStep2Sequence():
    roisLink = "step1RoI"
    prmVtxKey = "HLT_EFHistoPrmVtx"

    from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithmWithJets
    InputMakerAlg = EventViewCreatorAlgorithmWithJets( "IMBJet_step3",RoIsLink=roisLink )
    InputMakerAlg.ViewFallThrough = True
    InputMakerAlg.RequireParentView = True
    InputMakerAlg.Views = "BTagViews"
    InputMakerAlg.InViewRoIs = "InViewRoIs"
    InputMakerAlg.InViewJets = "InViewJets"
        

    # Second stage of Fast Tracking and Precision Tracking
    from TriggerMenuMT.HLTMenuConfig.Bjet.BjetTrackingConfiguration import getSecondStageBjetTracking
    secondStageAlgs, PTTracks, PTTrackParticles = getSecondStageBjetTracking( inputRoI=InputMakerAlg.InViewRoIs )

    # Flavour Tagging
    from TriggerMenuMT.HLTMenuConfig.Bjet.BjetFlavourTaggingConfiguration import getFlavourTagging
    flavourTaggingAlgs = getFlavourTagging( inputJets=InputMakerAlg.InViewJets, inputVertex=prmVtxKey, inputTracks=PTTrackParticles[0] )

    preAlgs = []

    bJetBtagSequence = seqAND( "bJetBtagSequence", preAlgs + secondStageAlgs + flavourTaggingAlgs )
    InputMakerAlg.ViewNodeName = "bJetBtagSequence"

    # Sequence
    BjetAthSequence = seqAND( "BjetAthSequence_step2",[InputMakerAlg,bJetBtagSequence] )

    from TrigBjetHypo.TrigBjetHypoConf import TrigBjetBtagHypoAlgMT
    hypo = TrigBjetBtagHypoAlgMT( "TrigBjetBtagHypoAlg" )
    hypo.Tracks = PTTrackParticles[0]

    from TrigBjetHypo.TrigBjetBtagHypoTool import TrigBjetBtagHypoToolFromDict
    return MenuSequence( Sequence    = BjetAthSequence,
                         Maker       = InputMakerAlg,
                         Hypo        = hypo,
                         HypoToolGen = TrigBjetBtagHypoToolFromDict )




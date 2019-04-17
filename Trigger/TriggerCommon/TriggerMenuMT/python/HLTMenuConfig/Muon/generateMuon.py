# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence, ChainStep, Chain, getChainStepName, createStepView
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from TriggerMenuMT.HLTMenuConfig.Muon.MuonMenuConfig import l2MuFastRecoCfg, l2MuFastHypoCfg
from TrigMuonHypoMT.TrigMuonHypoMTConfig import TrigMufastHypoToolFromDict


def generateChains( flags, chainDict ):

    stepName = getChainStepName('Muon', 1)
    stepReco, stepView = createStepView(stepName)

    acc = ComponentAccumulator()
    acc.addSequence(stepView)

    l2muFastReco = l2MuFastRecoCfg(flags)
    acc.merge( l2muFastReco, sequenceName=stepReco.getName() )

    ### Set muon step1 ###
    l2muFastHypo = l2MuFastHypoCfg( flags,
                                    name = 'TrigL2MuFastHypo',
                                    muFastInfo = 'MuonL2SAInfo' )

    l2muFastHypo.HypoTools = [ TrigMufastHypoToolFromDict(chainDict) ]

    acc.addEventAlgo(l2muFastHypo, sequenceName=stepView.getName())

    l2muFastSequence = MenuSequence( Sequence = l2muFastReco.sequence(),
                                     Maker = l2muFastReco.inputMaker(),
                                     Hypo = l2muFastHypo,
                                     HypoToolGen = None,
                                     CA = acc )

    l2muFastStep = ChainStep( stepName, [l2muFastSequence] )

    ### Set muon step2 ###
    # Please set up L2muComb step here 

    import pprint
    pprint.pprint(chainDict)

    chain = Chain( chainDict['chainName'], chainDict['L1item'], [ l2muFastStep ] )

    return chain

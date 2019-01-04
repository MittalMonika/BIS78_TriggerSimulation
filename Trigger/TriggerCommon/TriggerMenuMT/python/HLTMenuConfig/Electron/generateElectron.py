# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

from TrigUpgradeTest.ElectronMenuConfig import l2CaloRecoCfg, l2CaloHypoCfg
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence, \
    ChainStep, Chain, RecoFragmentsPool, getChainStepName

from TrigEgammaHypo.TrigL2CaloHypoTool import TrigL2CaloHypoToolFromName
from TrigEgammaHypo.TrigEgammaHypoConf import TrigL2CaloHypoAlgMT
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def generateChains( flags,  chainDict ):
    import pprint
    pprint.pprint( chainDict )

    acc = ComponentAccumulator()

    l2CaloHypo = RecoFragmentsPool.retrieve( l2CaloHypoCfg,
                                             flags,
                                             name = 'L2ElectronCaloHypo',
                                             CaloClusters = 'L2CaloEMClusters' )

    l2CaloReco = RecoFragmentsPool.retrieve(l2CaloRecoCfg, flags)
    acc.merge(l2CaloReco)

    fastCaloSequence = MenuSequence( Sequence    = l2CaloReco.sequence(),
                                     Maker       = l2CaloReco.inputMaker(),
                                     Hypo        = l2CaloHypo,
                                     HypoToolGen = TrigL2CaloHypoToolFromName )

    fastCaloStep = ChainStep(getChainStepName('Electron', 1), [fastCaloSequence])

           
    # # # fast ID
    from TrigUpgradeTest.InDetConfig import indetInViewRecoCfg
    fastInDetReco = RecoFragmentsPool.retrieve( indetInViewRecoCfg,
                                                flags,
                                                viewMakerName="ElectronInDet" )
    acc.merge( fastInDetReco )
    # TODO once tracking fully works, 
    from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestHypoAlg
    from TrigUpgradeTest.TrigUpgradeTestConf import HLTTest__TestHypoTool

    def makeFakeHypo(name, cfg):
        return HLTTest__TestHypoTool(name)
    
    fastInDetSequence = MenuSequence( Sequence    = fastInDetReco.sequence(),
                                      Maker       = fastInDetReco.inputMaker(),
                                      Hypo        = HLTTest__TestHypoAlg("FakeHypoForElectron", Input=""),
                                      HypoToolGen = makeFakeHypo )
    
    fastInDetStep = ChainStep(getChainStepName("Electron", 2), [fastInDetSequence])

    
    # # # EF calo

    # # # EF ID
    
    # # # offline egamma

    chain = Chain(chainDict['chainName'], chainDict['L1item'], [fastCaloStep, fastInDetStep] )
    return acc, chain

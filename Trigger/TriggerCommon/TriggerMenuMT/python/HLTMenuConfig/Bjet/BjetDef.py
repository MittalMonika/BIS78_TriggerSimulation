# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger("TriggerMenuMT.HLTMenuConfig.Egamma.BjetDef")


from TriggerMenuMT.HLTMenuConfig.Menu.ChainConfigurationBase import ChainConfigurationBase
from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import ChainStep, RecoFragmentsPool

from TriggerMenuMT.HLTMenuConfig.Bjet.BjetSequenceSetup import getBJetSequence

#----------------------------------------------------------------
# fragments generating configuration will be functions in New JO, 
# so let's make them functions already now
#----------------------------------------------------------------
def bjetSequenceCfg_j( flags ):    
    return getBJetSequence('j')

def bjetSequenceCfg_gsc( flags ):    
    return getBJetSequence('gsc')

#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class BjetChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        
    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChain(self):                            
        log.debug("Assembling chain for " + self.chainName)

        # --------------------
        # define here the names of the steps and obtain the chainStep configuration 
        # --------------------
        stepDictionary = {
            "": [self.getBjetSequence_j(), self.getBjetSequence_gsc()]
        }
        
        ## This needs to be configured by the Bjet Developer!!
        key = self.chainPart['extra'] 
        steps=stepDictionary[key]

        chainSteps = []
        for step in steps:
            chainSteps+=[step]
    
        myChain = self.buildChain(chainSteps)
        return myChain
        

    # --------------------
    # Configuration of steps
    # --------------------
    def getBjetSequence_j(self, name):
        stepName = "Step1_bjet"
        log.debug("Configuring step " + stepName)
        bjetSeq1 = RecoFragmentsPool.retrieve( bjetSequenceCfg_j, None ) # the None will be used for flags in future
        return ChainStep(stepName, [bjetSeq1])

    def getBjetSequence_gsc(self, name):
        stepName = "Step2_bjet"
        log.debug("Configuring step " + stepName)
        bjetSeq1 = RecoFragmentsPool.retrieve( bjetSequenceCfg_gsc, None ) # the None will be used for flags in future
        return ChainStep(stepName, [bjetSeq1])
        
            
            

        
                

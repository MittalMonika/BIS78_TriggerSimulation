# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

##########################################################################################
##########################################################################################
__doc__=""
__version__="Implementation of Electron Slice single electron signatures"

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)


from TriggerMenu.met.MissingETDef import L2EFChain_met
from TriggerMenu.menu.MenuUtils import splitChainDict, mergeChainDefs



##########################################################################################
##########################################################################################



def generateChainDefs(chainDict):          

    listOfChainDicts = splitChainDict(chainDict)
    listOfChainDefs = []
    
    for subChainDict in listOfChainDicts:      
        
        MissingET = L2EFChain_met(subChainDict)
        
        listOfChainDefs += [MissingET.generateHLTChainDef()]
        
    if len(listOfChainDefs)>1:
        theChainDef = mergeChainDefs(listOfChainDefs)
    else:
        theChainDef = listOfChainDefs[0]        
    
    return theChainDef


##########################################################


    

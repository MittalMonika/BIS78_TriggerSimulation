# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLTMenuConfig.Menu.ChainDictTools import splitChainDict
from TriggerMenuMT.HLTMenuConfig.Egamma.ElectronDef import ElectronChainConfiguration as ElectronChainConfiguration
from TriggerMenuMT.HLTMenuConfig.Menu.ChainMerging import mergeChainDefs


from AthenaCommon.Logging import logging
log = logging.getLogger( 'TriggerMenuMT.HLTMenuConfig.Egamma.generateChainConfigs' )
log.info("Importing %s",__name__)



def generateChainConfigs( chainDict ):
    import pprint
    pprint.pprint( chainDict )

    
    listOfChainDicts = splitChainDict(chainDict)
    listOfChainDefs = []

    for subChainDict in listOfChainDicts:
        
        Electron = ElectronChainConfiguration(subChainDict).assembleChain() 

        listOfChainDefs += [Electron]
        log.debug('length of chaindefs %s', len(listOfChainDefs) )
        

    if len(listOfChainDefs)>1:
        theChainDef = mergeChainDefs(listOfChainDefs, chainDict)
    else:
        theChainDef = listOfChainDefs[0]

    log.debug("theChainDef %s" , theChainDef)

    return theChainDef



    

    
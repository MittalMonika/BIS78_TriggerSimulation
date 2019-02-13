# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.CFElements import seqAND
from TriggerMenuMT.HLTMenuConfig.Menu.HLTCFConfig import decisionTree_From_Chains, \
    generateDecisionTree

from AthenaCommon.Logging import logging
from AthenaCommon.Constants import VERBOSE,INFO,DEBUG
_log = logging.getLogger('GenerateMenuMT_newJO')
_log.setLevel( VERBOSE )

def fillGeneratorsMap( sigMap, signature ):
    """ Fill the mapping from the flag container name to the function responsible for generating the Chain objects
    
    Here the files naming convention is employed: the chains mentioned in Trigger.menu.XYZ are served by the function in HLTMenuConfig.XYZ.generateChains"""

    if signature in sigMap:
        return

    capitalizedSignature = signature.capitalize()
    importString = 'TriggerMenuMT.HLTMenuConfig.{}.generate{}'.format(capitalizedSignature, capitalizedSignature)

    try:
        gen = __import__(importString, globals(), locals(), ['generateChains'])
        sigMap[signature] = gen.generateChains
        _log.info( 'Imported generator for %s' % signature )
    except Exception as ex:
        _log.warning( 'Cant import {} {}'.format(signature, ex) )

    

def generateMenu( flags ):
    """
    Using flags generate appropriate Control Flow Graph wiht all HLT algorithms
    """

    # convert to chainDefs
    from TriggerMenuMT.HLTMenuConfig.Menu.DictFromChainName import DictFromChainName
    toChainDictTranslator = DictFromChainName()

    counter = 0
    signatureToGenerator = {}
    menuChains = []

    menuAcc = ComponentAccumulator()
    mainSequenceName = 'HLTAllSteps'
    menuAcc.addSequence( seqAND(mainSequenceName) )

    chainAccumulators = []

    for name, cfgFlag in flags._flagdict.iteritems():
        if not 'Trigger.menu.' in name:
            continue

        # fill the map[signature, generating function]
        signature = name.split('.')[-1]
        fillGeneratorsMap( signatureToGenerator, signature )

        if signature not in signatureToGenerator:
            _log.warning('Generator for {} is missing. Chain dict will not be built'.format(signature))
            continue

        for chain in cfgFlag.get():

            chainDict = toChainDictTranslator.getChainDict( chain )

            counter += 1
            chainDict['chainCounter'] = counter
            # TODO topo threshold

            # call generating function and pass to CF builder

            chain = signatureToGenerator[signature](flags, chainDict)
            menuChains.append( chain )


    _log.info('Obtained Menu Chain objects')

    # pass all menuChain to CF builder    

    chainsAcc = generateDecisionTree(menuAcc.getSequence(mainSequenceName), menuChains)

    chainsAcc.printConfig()

    menuAcc.merge( chainsAcc )
    menuAcc.printConfig()

    _log.info('CF is built')

    return menuAcc



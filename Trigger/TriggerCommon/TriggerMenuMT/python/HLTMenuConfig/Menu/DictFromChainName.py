# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

"""
Class to obtain the chain configuration dictionary from the short or long name

Author: Catrin Bernius
Original code from TriggerMenu with CB, Joerg Stelzer, Moritz Backes

"""
__author__  = 'Catrin Bernius'
__version__=""
__doc__="Decoding of chain name into a dictionary"


from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
logDict = logging.getLogger('TriggerMenu.menu.DictFromChainName')
import re

def getOverallL1item(chainName):
    """
    Extracts the L1 seed from the chain name, checks for issue in naming, if found, fails

    In simple case the L1 items is just the last part of the name, after the _L1...
    There are though more complicated names like ...._L1...._L1..._L1... in such cases the L1 seed item is the last one
    """
    assert '_L1' in chainName, 'ERROR IN CHAIN {}, missing L1 seed at the end i.e. _L1...' .format(chainName)
    # this assumes that the last string of a chain name is the overall L1 item
    cNameParts = chainName.split("_L1")
    return 'L1_' + cNameParts[-1]

def getL1item(chainName):
    mainL1 = getOverallL1item(chainName)
    #replace the '_' left-closest-to ETA by '.' so that L1J75_31ETA49 becomes L1J75.31ETA49
    if 'ETA' in mainL1:
        r = re.compile("_(?P<eta>..ETA..)")
        mainL1 = r.sub(".\\g<eta>", mainL1)
    return mainL1

def getAllThresholdsFromItem(item):
    """
    Breaks the item name into parts separated by the _ and from each element extract the threshold name

    Fails when used on topo items or ill formatted items
    Examples: L1_MU4 returns [MU4]
    L1_2MU4 returns [MU4, MU4]
    L1_MU4_MU6 returns [MU4, MU6]
    """
    assert item.startswith("L1_"), "The L1 item  {} name does not start with L1".format( item )
    assert "-" not in item, "The {} is Topo item and can not be parsed".format( item )

    multThressholds = item.split("_")[1:] # skips the L1_
    thresholds = []
    for mt in multThressholds:
        assert mt[0] != '1', 'Issue while parsing {}, multiplicity of 1, should not be configured, if multiplicity greater than 9 is needed then support for it needs to be added'.format( mt )
        if mt[0] in '23456789': # first letter is a number == multiplicity, multiplicity 1 is not specified
            thresholds.extend( int(mt[0])*[mt[1:]] )
        else:
            thresholds.append( mt ) # no muliplicity prefix
    return thresholds

def getUniqueThresholdsFromItem(item):
    """
    As above but eliminates repeated thresholds
    """
    allThreshold = getAllThresholdsFromItem(item)
    s = set()
    def _(t):
        s.add(t)
        return t
    return  [ _(t) for t in allThreshold if t not in s ]

class DictFromChainName(object):

    def getChainDict(self,chainInfo):
        logDict.debug("chainInfo %s", chainInfo)

        # ---- Loop over all chains (keys) in dictionary ----
        # ---- Then complete the dict with other info    ----
        # Default input format will be namedtuple:
        # ChainProp: ['name', 'L1chainParts'=[], 'stream', 'groups',
        # 'merging'=[], 'topoStartFrom'=False],

        # these if/elif/else statements are due to temporary development
        if type(chainInfo) == str:
            chainName       = chainInfo
            L1chainParts    = []
            stream          = ''
            groups          = []
            mergingStrategy = 'parallel'
            mergingOffset   = -1
            mergingOrder    = []
            topoStartFrom   = ''

        elif 'ChainProp' in str(type(chainInfo)):
            chainName       = chainInfo.name
            L1chainParts    = chainInfo.l1SeedThresholds
            stream          = chainInfo.stream
            groups          = chainInfo.groups
            mergingStrategy = chainInfo.mergingStrategy
            mergingOffset   = chainInfo.mergingOffset
            mergingOrder    = chainInfo.mergingOrder
            topoStartFrom   = chainInfo.topoStartFrom

        else:
            logDict.error("Format of chainInfo passed to genChainDict not known")

        L1item = getL1item(chainName)

        logDict.debug("Analysing chain with name: %s", chainName)
        chainDict = self.analyseShortName(chainName,  L1chainParts, L1item)
        logDict.debug('ChainProperties: %s', chainDict)

        # setting the L1 item
        chainDict['L1item']          = L1item
        chainDict['stream']          = stream
        chainDict['groups']          = groups
        chainDict['mergingStrategy'] = mergingStrategy
        chainDict['mergingOffset']   = mergingOffset
        chainDict['mergingOrder']    = mergingOrder
        chainDict['topoStartFrom']   = topoStartFrom

        logDict.debug('Setting chain multiplicities')
        allChainMultiplicities = self.getChainMultFromDict(chainDict)
        chainDict['chainMultiplicities'] = allChainMultiplicities

        if logDict.isEnabledFor(logging.DEBUG):
            import pprint
            pp = pprint.PrettyPrinter(indent=4, depth=8)
            logDict.debug('SUPER FINAL dictionary: %s', pp.pformat(chainDict))

        return chainDict





    def getChainMultFromDict(self, chainDict):
        allMultis = []
        for cpart in chainDict['chainParts']:
            if cpart['multiplicity'] != '':
                allMultis.append(cpart['multiplicity'])
        return allMultis


    def getChainMultFromName(self, chainName):
        cNameParts = chainName.split("_")

        from SignatureDicts import getBasePattern
        pattern = getBasePattern()

        allMultis = []
        for cpart in cNameParts:
            m = pattern.match(cpart)
            if m:
                logDict.debug("In getChainMultFromName: Pattern found in this string: %s", cpart)
                groupdict = m.groupdict()
                if groupdict['multiplicity'] == '':
                    multiplicity = 1
                else:
                    multiplicity = groupdict['multiplicity']
                allMultis.append(multiplicity)
        return allMultis

    def getChainThresholdFromName(self, chainName, signature):
        cNameParts = chainName.split("_")

        from SignatureDicts import getBasePattern
        pattern = getBasePattern()
        trigType = []
        thresholdToPass = 0

        allThresh = []
        for cpart in cNameParts:
            m = pattern.match(cpart)
            if m:
                logDict.debug("In getChainThresholdFromName: Pattern found in this string: %s", cpart)
                groupdict = m.groupdict()
                allThresh.append(groupdict['threshold'])
                trigType.append(groupdict['trigType'])
                if signature == groupdict['trigType']:
                    thresholdToPass = groupdict['threshold']
                    break
        return thresholdToPass


    def analyseShortName(self, chainName, L1thresholds, L1item):
        """
        Function to obtain the chain configuration dictionay from the short name
        by parsing its components and finding the corrsponding properties which
        are defined in SliceDicts
        The naming ocnvention is defined inthis document http://
        """

        # ---- dictionary with all chain properties ----
        from SignatureDicts import ChainDictTemplate
        from copy import deepcopy
        genchainDict = deepcopy(ChainDictTemplate)
        genchainDict['chainName'] = chainName

        # remove the L1 item from the name
        hltChainName = chainName[:chainName.index("_L1")]

        # ---- specific chain part information ----
        allChainProperties=[]
        cparts = hltChainName.split("_")
        if 'HLT' in hltChainName:
            cparts.remove('HLT')


        # ---- identify the topo algorithm and add to genchainDict -----
        from SignatureDicts import AllowedTopos
        topo = ''
        topos=[]
        toposIndexed={}
        topoindex = -5
        for cindex, cpart in enumerate(cparts):
            if  cpart in AllowedTopos:
                logDict.debug('" %s" is in this part of the name %s -> topo alg', AllowedTopos, cpart)
                topo = cpart
                topoindex = cindex
                toposIndexed.update({topo : topoindex})
                hltChainName=hltChainName.replace('_'+cpart, '')
                topos.append(topo)

        genchainDict['topo'] = topos

        # replace these lines belwo with cparts = chainName.split("_")
        for t, i in enumerate(toposIndexed):
            if (t in cparts):
                logDict.debug('topo %s with index %s', t, i)
                del cparts[i]


        # ---- Find the signature defining patterns ----
        # ---- and write them out in dictionary     ----
        # ---- expected format: <Multiplicity(int)><TriggerType(str)>
        #      <Threshold(int)><isolation,...(str|str+int)> ----
        # EXCEPT FOR CHAINS ...
        from SignatureDicts import getBasePattern
        pattern = getBasePattern()
        mdicts=[]
        multichainindex=[]


        # ---- obtain dictionary parts for signature defining patterns ----
        from SignatureDicts import getSignatureNameFromToken, AllowedCosmicChainIdentifiers, \
            AllowedCalibChainIdentifiers, AllowedMonitorChainIdentifiers, AllowedBeamspotChainIdentifiers

        def buildDict(signature, sigToken ):
            groupdict = {'signature': signature, 'threshold': '', 'multiplicity': '',
                         'trigType': sigToken, 'extra': ''}
            mdicts.append( groupdict ) 

        logDict.debug("chain parts: %s", cparts)
        for cpart in cparts:

            logDict.debug("Looping over chain part: %s", cpart)
            m = pattern.match(cpart)
            if m:
                logDict.debug("Pattern found in this string: %s", cpart)
                groupdict = m.groupdict()
                # Check whether the extra contains a special keyword

                multiChainIndices = [i for i in range(len(hltChainName)) if ( hltChainName.startswith(cpart, i) ) ]
                logDict.debug("MultiChainIndices: %s", multiChainIndices)
                for theMultiChainIndex in multiChainIndices:
                    # this check is necessary for the bjet chains, example: j45_bloose_3j45
                    # j45 would be found in [0, 13], and 3j45 in [12]
                    # so need to make sure the multiplicities are considered here!
                    if (theMultiChainIndex != 0) & (hltChainName[theMultiChainIndex-1] != '_'):
                        continue

                    if theMultiChainIndex not in multichainindex:
                        multichainindex.append(theMultiChainIndex)

                logDict.debug("HLTChainName: %s", hltChainName)
                logDict.debug("cpart: %s", cpart)
                logDict.debug("groupdict: %s", groupdict)
                logDict.debug("multichainindex: %s", multichainindex)

                sName = getSignatureNameFromToken(cpart)
                groupdict['signature'] = sName
                logDict.debug('groupdictionary groupdict: %s', groupdict)
                mdicts.append(groupdict)

            # TODO, move it elsewhere or streamline further
            elif cpart =='noalg':
                multichainindex.append(hltChainName.index(cpart))
                buildDict( 'Streaming', 'streamer')
                break # stop loop here so mb doesn't get picked up from min bias slice as it's streaming info

            elif cpart=='mb':
                logDict.debug('Doing MB')
                multichainindex.append(hltChainName.index(cpart))
                buildDict('MinBias', 'mb')

            elif cpart=='hi':
                logDict.debug('Doing HI')
                multichainindex.append(hltChainName.index(cpart))
                buildDict( 'HeavyIon', 'mb') # this is cloned from previous code, not sure it is correct that HI == MB

            elif cpart in AllowedCosmicChainIdentifiers:
                logDict.debug('COSMIC CHAIN from CosmicDef.py')
                multichainindex.append(hltChainName.index(cpart))
                buildDict('Cosmic', 'cosmic')

            elif cpart in AllowedCalibChainIdentifiers:
                logDict.debug('CALIB CHAIN from Calibration')
                multichainindex.append(hltChainName.index(cpart))
                buildDict('Calibration', 'calib')

            elif cpart in AllowedMonitorChainIdentifiers:
                logDict.debug('Mon CHAIN from Monitor')
                multichainindex.append(hltChainName.index(cpart))
                buildDict( 'Monitoring', 'calib' ) # this is cloned from previous code, not sure it is correct that Monitoring == calib

            elif cpart in AllowedBeamspotChainIdentifiers:
                logDict.debug('Beamspot CHAIN from Beamspot')
                multichainindex.append(hltChainName.index(cpart))
                buildDict( 'Beamspot', 'beamspot' )

            elif cpart=='eb':
                logDict.debug('EnhancedBias chain')
                multichainindex.append(hltChainName.index(cpart))
                buildDict( 'EnhancedBias', 'eb')


        # If multiple parts exist, split the string and analyse each
        # part depending on the signature it belongs to
        multichainparts=[]
        multichainindex = sorted(multichainindex, key=int)
        cN = deepcopy(hltChainName)
        for i in reversed(multichainindex):
            if i!=0:
                logDict.debug('Appending to multichainparts (i!=0): %s', hltChainName[i:len(cN)])

                multichainparts.append(hltChainName[i:len(cN)])
                cN = cN[0:i-1]
            else:
                logDict.debug('Appending to multichainparts: %s', hltChainName[i:len(cN)])
                multichainparts.append(cN)
        logDict.debug("multichainparts: %s",multichainparts)

        # build the chainProperties dictionary for each part of the chain
        # add it to a allChainProperties
        multichainparts.reverse()

        # verify if the L1 setup is consistent with the chain
        # sizes of seeds matter
        if len(L1thresholds) != 0:
            assert len(L1thresholds) ==  len(multichainparts), 'ERROR IN CHAIN {} definition, l1 thresholds specified {} have different length than chain parts {}'\
                .format(chainName, str(L1thresholds), str(multichainparts) )

        # tmp removed this check since the L1seeds depend on sym or asym cases
       # if len(L1thresholds) == 0:
#            getAllThresholdsFromItem, getUniqueThresholdsFromItem
        #    assert len( getAllThresholdsFromItem( L1item )) == len(multichainparts), 'ERROR IN CHAIN {} definition, l1 thresholds extracted from the L1 item name {} have different length than chain parts {}, set L1 thresholds ChainProp'\
         #       .format( chainName, str( getAllThresholdsFromItem( L1item ) ), str(multichainparts))

        # check the case when _L1 appears more than once in the name
        if chainName.count("_L1") > 1:
            indices = [ chainName.index( th ) for th in L1thresholds  ]
            # verify if all thresholds are mentioned in chain parts, if they are not then one of the indices will be -1
            assert all( [i > 0 for i in indices] ), "Some thresholds are not part of the chain name name {}, {}".format(chainName, L1thresholds)
            # verify that the order of threshold and order of threshold mentioned in the name (there they are prexixed by L1) is identical, else there may be mistake
            assert sorted(indices), "The order of L1 threshlds mentioned in chain name {} are not the same as threshold passed {}".format(chainName, L1thresholds)


        for chainindex, chainparts in enumerate(multichainparts):

            chainProperties = {} #will contain properties for one part of chain if multiple parts

            if len(L1thresholds) != 0:
                chainProperties['L1threshold'] = L1thresholds[chainindex]
            else:
                chainProperties['L1threshold'] = getAllThresholdsFromItem ( L1item )[chainindex]  #replced getUniqueThresholdsFromItem


            chainpartsNoL1 = chainparts

            logDict.debug('chainparts %s', chainparts)

            parts=chainpartsNoL1.split('_')
            parts=filter(None,parts)

            chainProperties['trigType']=mdicts[chainindex]['trigType']
            chainProperties['extra']=mdicts[chainindex]['extra']
            multiplicity = mdicts[chainindex]['multiplicity'] if not mdicts[chainindex]['multiplicity'] == '' else '1'
            chainProperties['multiplicity'] = multiplicity
            chainProperties['threshold']=mdicts[chainindex]['threshold']
            chainProperties['signature']=mdicts[chainindex]['signature']

            # if we have a L1 topo in a multi-chain then we want to remove it from the chain name
            # but only if it's the same as the L1item_main; otherwise it belongs to chain part and we q
            # have to keep it in the name
            chainProperties['chainPartName'] = chainparts
            if ('-' in L1item) and (len(multichainparts) > 1):
                chainProperties['chainPartName'] = chainpartsNoL1
            ##if we have the overall L1 item in a multi-part chains, then we want to remove it from the chain part name
            if len(multichainparts) > 1 and L1item.count("_") > 1 :
                chainProperties['chainPartName'] = chainpartsNoL1

            logDict.debug('Chainparts: %s', chainparts)
            if (chainProperties['signature'] != 'Cosmic') \
                    & (chainProperties['signature'] != 'Calibration')\
                    & (chainProperties['signature'] != 'Streaming') \
                    & (chainProperties['signature'] != 'Beamspot') \
                    & (chainProperties['signature'] != 'Monitoring') :
                parts.pop(0)


            #---- Check if topo is a bphsyics topo -> change signature ----
            from SignatureDicts import AllowedTopos_Bphysics
            for t in genchainDict['topo']:
                if (t in AllowedTopos_Bphysics):
                    chainProperties['signature'] = 'Bphysics'


            # ---- import the relevant dictionaries for each part of the chain ----
            from SignatureDicts import getSignatureInformation
            SignatureDefaultValues, allowedSignaturePropertiesAndValues = getSignatureInformation(chainProperties['signature'])
            logDict.debug('SignatureDefaultValues: %s', SignatureDefaultValues)

            # ---- update chain properties with default properties ----
            result = deepcopy(SignatureDefaultValues)
            result.update(chainProperties)
            chainProperties = result

            # ---- check remaining parts for complete machtes in allowedPropertiesAndValues Dict ----
            # ---- unmatched = list of tokens that are not found in the allowed values as a whole ----
            parts = filter(None, parts)     #removing empty strings from list

            matchedparts = []
            for pindex, part in enumerate(parts):
                origpart = part
                for prop, allowedValues in allowedSignaturePropertiesAndValues.items():
                    if part in allowedValues:
                        if type(chainProperties[prop]) is list:
                            chainProperties[prop] += [part]
                        else:
                            chainProperties[prop] = part
                        matchedparts.append(part)

            logDict.debug("matched parts %s", matchedparts)
            leftoverparts = set(parts)-set(matchedparts)
            logDict.debug('leftoverparts %s', leftoverparts)
            for pindex, part in enumerate(leftoverparts):
                for prop, allowedValues in allowedSignaturePropertiesAndValues.items():
                    if prop in chainProperties.keys():
                        continue
                    for aV in allowedValues:
                        if (aV in part):
                            if (chainProperties['signature'] in ['Egamma', 'Muon'] )& (prop in ['trkInfo','hypoInfo']):
                                chainProperties[prop] = part
                                part = part.replace(part,'')
                            elif (chainProperties['signature'] in ['Jet'] )& (prop in ['gscThreshold']):
                                chainProperties[prop] = part
                                part = part.replace(part,'')
                            else:
                                chainProperties[prop] = aV
                                part = part.replace(aV,'')
                                break # done with allowed values for that property

                assert len(part.split()) == 0, "These parts of the chain name {} are not understood {}".format(origpart,part)


            # ---- remove properties that aren't allowed in the chain properties for a given siganture ----
            forbiddenProperties = set(chainProperties.keys()) - set(allowedSignaturePropertiesAndValues.keys())
            logDict.debug('%s', set(chainProperties.keys()))
            logDict.debug('%s', set(allowedSignaturePropertiesAndValues.keys()))
            for fb in forbiddenProperties:
                forbiddenValue = chainProperties.pop(fb)
                assert forbiddenValue == '', "Property {} not allowed for signature '{}', but specified '{}'".format (fb, chainProperties['signature'], forbiddenValue)

            # ---- the info of the general and the specific chain parts dict ----
            allChainProperties.append(chainProperties)


        # ---- depending on if signatures are different in this chain, break up the chainProperties dictionary ----
        # ---- finally also taking care of the signature key ----
        genchainDict['chainParts'] = allChainProperties
        for cPart in allChainProperties:
            if cPart['signature'] == 'Jet' and cPart['bTag'] != '':
                cPart['signature'] = 'Bjet'
            genchainDict['signatures'] += [cPart['signature']]

        #genchainDict['signature'] = allChainProperties[0]['signature']

        logDict.debug('genchainDict that is passed as Final dict %s', genchainDict)

        return genchainDict

# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
__author__  = 'Javier Montejo'
__version__="$Revision: 1.01 $"
__doc__="Class containing all the information of an HLT chain"

import re
from TriggerMenu.api.TriggerEnums import TriggerType, TriggerPeriod
from collections import Counter
from copy import deepcopy

class TriggerInfo:
    ''' Object containing all the HLT information related to a given period.
        Stores a list of TriggerChain objects and the functions to skim them
    '''
    def __init__(self,period=0, customGRL=None, release=None):
        self.triggerChains = []
        self.period = period
        self.totalLB = 0

        if not period: return
        from TriggerDataAccess import getHLTlist
        HLTlist, totalLB = getHLTlist(period, customGRL, release)
        self.totalLB = totalLB
        for hlt, l1, livefraction, activeLB, hasRerun in HLTlist:
            self.triggerChains.append( TriggerChain(hlt, l1, livefraction, activeLB, hasRerun))

    @classmethod
    def merge(cls,listofTI):
        mergedTI = TriggerInfo()
        mergedHLTmap = {}
        for ti in listofTI:
            mergedTI.period |= ti.period
            mergedTI.totalLB += ti.totalLB
            for tc in ti.triggerChains:
                if tc.name not in mergedHLTmap: mergedHLTmap[tc.name] = deepcopy(tc)
                else: mergedHLTmap[tc.name].activeLB += tc.activeLB
        for tc in mergedHLTmap.itervalues():
            tc.livefraction = tc.activeLB/float(mergedTI.totalLB)
        mergedTI.triggerChains = mergedHLTmap.values()
        return mergedTI
        

    @classmethod
    def testCustomGRL(cls, grl):
        from TriggerMenu.api.TriggerPeriodData import TriggerPeriodData
        return TriggerPeriodData.testCustomGRL(grl)

    def reparse(self):
        self.triggerChains = [ TriggerChain(t.name, t.l1seed, t.livefraction, t.activeLB) for t in self.triggerChains ]

    def _getUnprescaled(self,triggerType, additionalTriggerType, matchPattern, livefraction=1.0):
        return [x.name for x in self.triggerChains if x.isUnprescaled(livefraction) and x.passType(triggerType, additionalTriggerType) and re.search(matchPattern, x.name)]

    def _getLowestUnprescaled(self,triggerType, additionalTriggerType, matchPattern, livefraction=1.0):
        chainList = [ x for x in self.triggerChains if x.isUnprescaled(livefraction) and x.passType(triggerType, additionalTriggerType) and re.search(matchPattern, x.name)]
        typeMap = {}
        for chain in chainList:
            if chain.triggerType not in typeMap:
                typeMap[chain.triggerType] = [chain]
                continue
            append = False
            for other in typeMap[chain.triggerType][:]:
                comp = chain.isLowerThan(other,self.period)
                if comp ==  0: 
                    append = False
                    break
                append = True
                if comp ==  1:    typeMap[chain.triggerType].remove(other)
            if append:
                typeMap[chain.triggerType].append(chain)
        return [x.name for t in typeMap.itervalues() for x in t ]


    def _getAllHLT(self,triggerType, additionalTriggerType, matchPattern, livefraction):
        return {x.name: x.livefraction for x in self.triggerChains if x.passType(triggerType, additionalTriggerType) and re.search(matchPattern, x.name) and x.isUnprescaled(livefraction)}

    def _getActive(self,triggerType, additionalTriggerType, matchPattern, livefraction):
        return [x.name for x in self.triggerChains if x.isActive(livefraction) and x.passType(triggerType, additionalTriggerType) and re.search(matchPattern, x.name)]
    def _getInactive(self,triggerType, additionalTriggerType, matchPattern, livefraction):
        return [x.name for x in self.triggerChains if x.isInactive(livefraction) and x.passType(triggerType, additionalTriggerType) and re.search(matchPattern, x.name)]

    def _checkPeriodConsistency(self,triggerType, additionalTriggerType, matchPattern):
        inconsistent = set()
        for i in range(len(self.triggerChains)):
            probe1 = self.triggerChains[i]
            if not (probe1.passType(triggerType, additionalTriggerType) and re.search(matchPattern, probe1.name)): continue
            for j in range(i+1,len(self.triggerChains)):
                probe2 = self.triggerChains[j]
                if not (probe2.passType(triggerType, additionalTriggerType) and re.search(matchPattern, probe2.name)): continue
                if probe1.isUnprescaled() and not probe2.isUnprescaled() and probe1.isLowerThan(probe2,self.period)==1: inconsistent.add(probe2.name)
                if probe2.isUnprescaled() and not probe1.isUnprescaled() and probe2.isLowerThan(probe1,self.period)==1: inconsistent.add(probe1.name)
                
        return inconsistent


class TriggerLeg:
    types          = ('e','j','mu','tau','xe','g','ht')
    legpattern     = re.compile('([0-9]*)(%s)([0-9]+)(noL1)?' % '|'.join(types))
    detailpattern  = re.compile('(?:-?\d+)|(?:[^0-9 -]+)') #split into text-only vs number-only
    bjetpattern    = re.compile('bmv|bhmv|btight|bmedium|bloose')
    bphyspattern   = re.compile('b[A-Z]')
    exoticspattern = re.compile('llp|LLP|muvtx|hiptrt|LATE|NOMATCH')
    afppattern     = re.compile('afp|AFP')

    def __init__(self,legname, chainseed, chainname):
        self.legname = legname
        self.l1seed = ""
        details = []
        chainseed= chainseed.replace("L1_","")
        blocks = legname.split("_L1")

        for token in blocks[0].split("_"):
            m = self.legpattern.match(token)
            if m:
                count,legtype,thr,noL1 = m.groups()
                self.count = int(count) if count else 1
                self.thr = int(thr)
                if legtype == 'e':
                    if self.count > 1: self.legtype = TriggerType.el_multi
                    else:              self.legtype = TriggerType.el_single
                elif legtype == 'mu':
                    if self.count > 1: self.legtype = TriggerType.mu_multi
                    else:              self.legtype = TriggerType.mu_single
                elif legtype == 'j':
                    if self.count > 1: self.legtype = TriggerType.j_multi
                    else:              self.legtype = TriggerType.j_single
                elif legtype == 'tau':
                    if self.count > 1: self.legtype = TriggerType.tau_multi
                    else:              self.legtype = TriggerType.tau_single
                elif legtype == 'g':
                    if self.count > 1: self.legtype = TriggerType.g_multi
                    else:              self.legtype = TriggerType.g_single
                elif legtype == 'xe':
                    self.legtype = TriggerType.xe
                elif legtype == 'ht':
                    self.legtype = TriggerType.ht
                else:
                    print "Unknown trigger type:",legtype
                if noL1: details.append(noL1)
            else:
                if self.bjetpattern.match(token):
                    if self.legtype == TriggerType.j_single: self.legtype = TriggerType.bj_single
                    if self.legtype == TriggerType.j_multi:  self.legtype = TriggerType.bj_multi
                if self.bphyspattern.match(token):
                    self.legtype = TriggerType.mu_bphys
                if self.exoticspattern.search(token):
                    self.legtype = TriggerType.exotics
                if self.afppattern.search(token):
                    self.legtype = TriggerType.afp
                details.append(token)

        for l1seed in blocks[1:]:
            if self.exoticspattern.search(l1seed):
                self.legtype = TriggerType.exotics
            if self.afppattern.search(l1seed):
                self.legtype = TriggerType.afp
            if l1seed == chainseed: continue
            else: 
                assert self.l1seed=="", (self.l1seed, chainseed, chainname, blocks[1:])
                self.l1seed = l1seed
        if not self.l1seed: self.l1seed = chainseed
        self.details = tuple(details)

    def __eq__(self,other):
        return (self.l1seed == other.l1seed and self.count == other.count and self.thr == other.thr and self.legtype == other.legtype and self.details == other.details)

    def __hash__(self):
        return hash((self.l1seed,self.count,self.thr,self.legtype,self.details))

    def __repr__(self):
        return "{0} {1} {2} {3} {4} {5:b}".format(self.legname,self.l1seed,self.count,self.thr,self.details,self.legtype)

    def isLegLowerThan(self, other, is2015, debug=False):
        ''' Returns -9 if none of them is lower than the other (e.g. different met flavour).
            Returns -1 if identical
            Returns  0 if other is lower than self.
            Returns  1 if self  is lower than other.
        '''
        if debug:
            print "DEBUG LEGS --------"
            print self.legname, other.legname
            print self.legtype, other.legtype
            print self.l1seed, other.l1seed
            print self.details, other.details
            print self.thr, other.thr
            print self.compareDetails(other, is2015, debug=True)
            print self.details == other.details
            print "DEBUG LEGS END --------"

        if self.legtype != other.legtype: return -9
        if self.compareDetails(other, is2015) == -1:
            if self.thr < other.thr: return 1
            if self.thr > other.thr: return 0
            else: return -1

        if self.compareDetails(other, is2015) == 1 and self.thr  <= other.thr: return 1
        if self.compareDetails(other, is2015) == 0 and other.thr <= self.thr:  return 0
        return -9

    def compareDetails(self, other, is2015, debug=False):
        ''' Returns -9 if none of them is lower than the other (e.g. different met flavour).
            Returns -1 if identical
            Returns  0 if other is lower than self.
            Returns  1 if self  is lower than other.
        '''

        if debug: print "compareDetails:",len(self.details), len(other.details),(self.l1seed == other.l1seed),(self.details == other.details) 
        compl1seed  = self.compareTags(self.l1seed, other.l1seed, stringSubset=True, is2015=is2015, debug=debug)
        compdetails = self.compareTags(self.details, other.details, is2015=is2015, debug=debug )
        if self.l1seed == other.l1seed:
            if self.details == other.details: return -1
            if debug: print "compareTags 1:",compdetails
            return compdetails

        if self.details == other.details:
            if debug: print "compareTags 2:",compl1seed
            return compl1seed

        if compl1seed == compdetails:
            return compl1seed
        return -9

    def compareTags(self, tags1, tags2, stringSubset=False,is2015=False,debug=False):
        def is_cutable(s):
            if all(["loose" in x or "medium" in x or "tight" in x for x in s]): return True
            if any(["bmv" in x for x in s]): return True
            #tags with numeric values that don't reflect a strict ordering
            noncutable = ("bVertex","bBmumuxv","a","rcu","rcc","nscan","kaonpi","dipion","nod","nomucomb","LArH")
            if any([x in s for x in noncutable]): return 0
            return [is_number(x) for x in s].count(True)==1
        def is_number(s):
            try:
                float(s)
                return True
            except ValueError:
                return False
        def mycomp(x,y):
            ''' Return -9 for different strings, 
                -1 for identical strings/nums, 
                0/1 for high/low numbers or string subsets
            '''
            try: 
                x,y = int(x), int(y)
                if x < y: return 1
                elif x > y: return 0
                else: return -1
            except ValueError: 
                if x==y: return -1
                if x == y.replace("vloose","loose"): return 0
                if x == y.replace("vloose","loose").replace("loose","medium"): return 0
                if x == y.replace("vloose","loose").replace("loose","medium").replace("medium","tight"): return 0
                if y == x.replace("vloose","loose"): return 1
                if y == x.replace("vloose","loose").replace("loose","medium"): return 1
                if y == x.replace("vloose","loose").replace("loose","medium").replace("medium","tight"): return 1
                if stringSubset:
                    if x in y: return 1
                    if y in x: return 0
                return -9

        if tags1 == tags2: return -1
        if not isinstance(tags1, basestring):
            settags1 = set(tags1)
            settags2 = set(tags2)
            assert len(tags1)==len(settags1),(tags1,settags1)
            assert len(tags2)==len(settags2),(tags2,settags2)
            loosercut = ('noEF','noL1','noL2','nod0','noiso','nodeta','noL2','nodphires','noos','novtx')
            if settags1.issubset(settags2):
                diff = settags2.difference(settags1)
                #diff = [x.replace("nomucomb","mucomb") for x in diff]
                if is2015: diff = [x.replace("nod0","hack") for x in diff]
                if all([x in loosercut for x in diff]):
                    return 0
                if all([is_cutable(self.detailpattern.findall(x)) for x in diff]):
                    return 1
            elif settags2.issubset(settags1):
                diff = settags1.difference(settags2)
                if is2015: diff = [x.replace("nod0","hack") for x in diff]
                if debug: print diff
                if all([x in loosercut for x in diff]):
                    return 1
                if all([is_cutable(self.detailpattern.findall(x)) for x in diff]):
                    return 0
            elif len(settags1)!=len(settags2):
                minlen = min(len(settags1),len(settags2))
                comp1 = self.compareTags(tags1[:minlen],tags2[:minlen],stringSubset,is2015,debug)
                comp2 = self.compareTags(tags1[minlen:],tags2[minlen:],stringSubset,is2015,debug)
                if comp1==comp2: return comp1
                return -9
            thecomp = [ self.compareTags(tag1,tag2,stringSubset,is2015,debug) for tag1, tag2 in zip(tags1,tags2) ]
        else: #single string
            tag1, tag2 = tags1.replace("-"," "), tags2.replace("-"," ") #avoid MJJ-X confusion
            #lower mv2 and deltaR/deltaZ/deltaPhi values are tighter, put a minus sign to trick it
            inverseCuts = ("mv2c","dr","dz","dphi","bDimu")
            for cut in inverseCuts:
                tag1 = tag1.replace(cut,cut+"-")
                tag2 = tag2.replace(cut,cut+"-")
            #only make a statement on the numerical values, with everything else identical
            reself  = self.detailpattern.findall(tag1)
            reother = self.detailpattern.findall(tag2)

            if len(reself) != len(reother): return -9
            if not stringSubset and not is_cutable(reself): return -9
            thecomp = [mycomp(a,b) for a,b in zip(reself,reother)]
            if debug: print "thecomp:",thecomp,reself,reother
        if any([x == -9 for x in thecomp]): return -9
        if all([x !=0 for x in thecomp]) and any([x == 1 for x in thecomp]): return 1
        if all([x !=1 for x in thecomp]) and any([x == 0 for x in thecomp]): return 0
        return -9

    @classmethod
    def parse_legs(cls,name,l1seed,chainname):
        legsname = []
        name = name.replace("HLT_","")
        for token in name.split("_"):
            m = cls.legpattern.match(token)
            if m:
                legsname.append(token)
            elif legsname: 
                legsname[-1] += "_"+token
            else: #first token doesn't match
                #print "parse_legs: Problem parsing",name
                return []
        return [TriggerLeg(l,l1seed,chainname) for l in legsname]

class TriggerChain:
    l1types        = ('EM','J','MU','TAU','XE','XS','HT')
    l1pattern      = re.compile('([0-9]*)(%s)([0-9]+)' % '|'.join(l1types))

    def __init__(self,name,l1seed,livefraction,activeLB=1,hasRerun=False):
        self.name = name
        self.l1seed = l1seed
        tmplegs = TriggerLeg.parse_legs(name,l1seed,name)
        self.legs = self.splitAndOrderLegs(tmplegs)
        self.livefraction = livefraction
        self.activeLB = activeLB
        self.hasRerun = hasRerun
        self.triggerType = self.getTriggerType(self.legs, l1seed)

    def splitAndOrderLegs(self, legs):
        newLegs = []
        for triggerType in TriggerType:
            for l in legs:
                if not l.legtype == triggerType: continue
                for i in range(l.count): #split into N single legs
                    tmp = deepcopy(l)
                    tmp.count = 1
                    if tmp.legtype & TriggerType.el_multi:
                        tmp.legtype |=  TriggerType.el_single
                        tmp.legtype &= ~TriggerType.el_multi
                    elif tmp.legtype & TriggerType.mu_multi:
                        tmp.legtype |=  TriggerType.mu_single
                        tmp.legtype &= ~TriggerType.mu_multi
                    elif tmp.legtype & TriggerType.tau_multi:
                        tmp.legtype |=  TriggerType.tau_single
                        tmp.legtype &= ~TriggerType.tau_multi
                    elif tmp.legtype & TriggerType.j_multi:
                        tmp.legtype |=  TriggerType.j_single
                        tmp.legtype &= ~TriggerType.j_multi
                    elif tmp.legtype & TriggerType.bj_multi:
                        tmp.legtype |=  TriggerType.bj_single
                        tmp.legtype &= ~TriggerType.bj_multi
                    elif tmp.legtype & TriggerType.g_multi:
                        tmp.legtype |=  TriggerType.g_single
                        tmp.legtype &= ~TriggerType.g_multi
                    newLegs.append(tmp)
        return newLegs

    def getTriggerType(self, legs, l1seed):
        mtype = TriggerType.UNDEFINED
        for l in legs:
            if mtype & TriggerType.el and l.legtype & TriggerType.el:
                mtype |=  TriggerType.el_multi
                mtype &= ~TriggerType.el_single
            elif mtype & TriggerType.mu and l.legtype & TriggerType.mu:
                mtype |=  TriggerType.mu_multi
                mtype &= ~TriggerType.mu_single
            elif mtype & TriggerType.tau and l.legtype & TriggerType.tau:
                mtype |=  TriggerType.tau_multi
                mtype &= ~TriggerType.tau_single
            elif mtype & TriggerType.j and l.legtype & TriggerType.j:
                mtype |=  TriggerType.j_multi
                mtype &= ~TriggerType.j_single
            elif mtype & TriggerType.bj and l.legtype & TriggerType.bj:
                mtype |=  TriggerType.bj_multi
                mtype &= ~TriggerType.bj_single
            elif mtype & TriggerType.g and l.legtype & TriggerType.g:
                mtype |=  TriggerType.g_multi
                mtype &= ~TriggerType.g_single
            elif l.legtype & TriggerType.mu_bphys:
                mtype |=  TriggerType.mu_bphys
                mtype &= ~(TriggerType.mu_single | TriggerType.mu_multi)
            elif l.legtype & TriggerType.exotics:
                mtype |=  TriggerType.exotics
            elif l.legtype & TriggerType.afp:
                mtype  =  TriggerType.afp #on purpose not OR-ed
            else:
                mtype |= l.legtype

        l1seed= l1seed.replace("L1_","")
        if mtype & TriggerType.exotics or mtype & TriggerType.afp:
            return mtype
        for token in l1seed.split("_"):
            m = self.l1pattern.match(token)
            if m:
                count,legtype,thr = m.groups()
                count = int(count) if count else 1
                if legtype == 'EM' or legtype == 'TAU':
                    pass
                elif legtype == 'MU':
                    if not mtype & TriggerType.mu_bphys:
                        if count > 1: mtype |= TriggerType.mu_multi
                        elif not mtype & TriggerType.mu_multi: mtype |= TriggerType.mu_single
                elif legtype == 'J':
                    if not mtype & TriggerType.bj and not mtype & TriggerType.j and not mtype & TriggerType.tau and not mtype & TriggerType.ht:
                        if count > 1: mtype |= TriggerType.j_multi
                        elif not mtype & TriggerType.j_multi: mtype |= TriggerType.j_single
                elif legtype == 'XE' or legtype == 'XS':
                    mtype |= TriggerType.xe
                elif legtype == 'HT':
                    mtype |= TriggerType.ht
                else:
                    print "Unknown trigger type:",(legtype, mtype, token, self.name)
        return mtype

    def isActive(self, livefraction=1e-99):
        return self.livefraction > livefraction or self.hasRerun
    def isInactive(self, livefraction=1e-99):
        return not self.isActive(livefraction)

    def isUnprescaled(self, livefraction=1.0):
        return self.livefraction >= livefraction

    def getType(self):
        return self.triggerType

    def passType(self, triggerType, additionalTriggerType):
        if self.triggerType == TriggerType.UNDEFINED: return False
        if self.triggerType == TriggerType.ALL:       return True
        match   = (self.triggerType & triggerType)
        if not match: return False
        tmpType = self.triggerType & ~triggerType

        try: 
            for t in additionalTriggerType:
                match   = (tmpType & t)
                if not match: return False
                tmpType = tmpType & ~t
        except TypeError: #Not iterable
            if additionalTriggerType!=TriggerType.UNDEFINED:
                match   = (tmpType & additionalTriggerType)
                if not match: return False
                tmpType = tmpType & ~additionalTriggerType

        return tmpType == TriggerType.UNDEFINED #After matches nothing remains

    def __repr__(self):
        print self.name, self.legs, "{0:b}".format(self.triggerType), self.livefraction, self.activeLB
        return ""

    def isSubsetOf(self, other):
        ''' Returns -1 if none of them is a strict subset of the other
            Returns  0 if the legs in other are a subset of self.
            Returns  1 if the legs in self  are a subset of other.
        '''
        if not self.legs or not other.legs: return -1 #problems with AFP
        selfcounter = Counter(self.legs)
        othercounter = Counter(other.legs)
        for leg, count in selfcounter.iteritems():
            if not leg in othercounter or count > othercounter[leg]: break
        else: return 1
        for leg, count in othercounter.iteritems():
            if not leg in selfcounter or count > selfcounter[leg]: break
        else: return 0
        return -1

    def isLowerThan(self, other,period=TriggerPeriod.future, debug=False):
        ''' Returns -1 if none of them is lower than the other (e.g. asymmetric dilepton).
            Returns  0 if other is lower than self.
            Returns  1 if self  is lower than other.
        '''
        is2015  = period & TriggerPeriod.y2015 and not TriggerPeriod.isRunNumber(period)
        is2015 |= period <= 284484             and     TriggerPeriod.isRunNumber(period)
        if self.triggerType != other.triggerType: return -1
        if len(self.legs) != len(other.legs): return -1
        comp = -1
        #if re.search("HLT_j55_gsc75_bmv2c1040_split_3j55_gsc75_boffperf_split", self.name): debug = True
        if debug: print "DEBUG:",self.name,other.name
        for selfleg, otherleg in zip(self.legs, other.legs):
            legcomp = selfleg.isLegLowerThan(otherleg, is2015, debug)
            if debug: print "DEBUG LEG return:", legcomp
            if legcomp == -9: return -1
            elif legcomp == -1: continue
            elif legcomp == 0 and comp == 1: return -1
            elif legcomp == 1 and comp == 0: return -1
            elif legcomp == 0 : comp = 0
            elif legcomp == 1 : comp = 1
        if debug: print "DEBUG FINAL:",comp
        return comp


def test():
    a = TriggerChain("HLT_j50_gsc65_bmv2c1040_split_3j50_gsc65_boffperf_split", "L1J100",1)
    print a
    print bin(a.getType())
    print a.passType(TriggerType.j_multi, TriggerType.UNDEFINED)
    print a.passType(TriggerType.j_multi | TriggerType.bj_single, TriggerType.UNDEFINED)
    print a.isUnprescaled()

if __name__ == "__main__":
    import sys
    sys.exit(test())

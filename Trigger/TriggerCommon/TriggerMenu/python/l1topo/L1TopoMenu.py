# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from TopoOutput import TriggerLine
from AthenaCommon.Logging import logging
log = logging.getLogger("TriggerMenu.l1topo.L1TopoMenu")

class L1TopoMenu:

    def __init__(self, menuName):
        self.menuName = menuName
        # algorithms and their output location
        self.topoOutput = []

        self.globalConfig = dict()

    def __iter__(self):
        return self.topoOutput.__iter__()

    def __call__(self):
        return self.topoOutput
        
    def algoNames(self):
        return [x.algoname for x in self.topoOutput]         

    def __len__(self):
        return len(self.topoOutput)

    def addTopoLine(self, topoline):
        self.topoOutput += topoline

    def __iadd__(self, topoline):
        if topoline is not None:
            self.topoOutput += [ topoline ]
        return self

    def addGlobalConfig(self, name, value):
        self.globalConfig[name]=str(value)


    def getTriggerLines(self):
        from operator import attrgetter
        outputLines = []
        for output in self.topoOutput:
            outputLines += TriggerLine.createTriggerlinesFromTopoOutput(output)
        return sorted(outputLines, key=attrgetter('ordinal')) # return the TriggerLines, sorted by the ordinal



    def xml(self, ind=1, step=2):
        from operator import attrgetter        
        s  = '<?xml version="1.0" ?>\n'
        s += '<TOPO_MENU menu_name="Topo_pp_vX" menu_version="1">\n' # % (self.menuName.replace("Physics_pp_","").replace("MC_pp_",""))
        s += '  <!--File is generated by TriggerMenu-->\n'
        s += '  <!--No. L1Topo algos defined: %i-->\n' % len(self.topoOutput)
        s += '  <OutputList>\n'
        for output in self.topoOutput:  # (Setup for module, fpga, firstbit, clock should be modified in more flexible way.)
            s += output.xml(ind+1,step)
        s += '  </OutputList>\n\n'
        s += '  <TopoConfig>\n'
        
        for gPar in sorted(self.globalConfig.keys()):
            s += '    <Entry name="%s" value="%s"/>\n' % (gPar, self.globalConfig[gPar])
        s += '  </TopoConfig>\n\n'
        
        allSortAlgs = set()
        for output in self.topoOutput:
            allSortAlgs.update(output.sortingAlgos)
        for sortAlg in sorted(allSortAlgs, key=attrgetter('algoId')):
            s += sortAlg.xml()+"\n"

        for output in self.topoOutput:
            s += output.algo.xml()+"\n"

        s += '</TOPO_MENU>\n'
        return s




    def readMenuFromXML(self,inputFile):
        from XMLMenuReader import readMenuFromXML
        readMenuFromXML(self, inputFile)



    def check(self):

        allOk = self.check_consecutiveAlgId()
        
        allOk &= self.check_OutputOverlap()

        allOk &= self.check_LUTassignment()

        return allOk

    def check_consecutiveAlgId(self):
        """
        This method is only for quick solutions but should be intermediate
        """
        allOk = True
        idlist = [x.algo.algoId for x in self.topoOutput]
        if len(idlist)>0 and len(idlist) != max(idlist)+1:
            idlist.sort()
            from itertools import groupby
            partition = [list(g) for k,g in groupby(enumerate(idlist), lambda (x,y) : y-x)]
            log.error("Algorithm IDs must start at 0 and be consecutive, but algorithm IDs are %s" % ','.join(["%i-%i" % (x[0][1],x[-1][1]) for x in partition]))
            allOk = False
        return allOk
        

    def check_OutputOverlap(self):

        triggerlines = self.getTriggerLines()

        allOk = True

        tlKeys = [ (tl.cable, tl.fpga, tl.clock, tl.bit) for tl in triggerlines]
        
        from collections import Counter
        c = Counter( tlKeys ).most_common()
        for key,count in c:
            if count == 1: break
            print "Output cable %s, fpga %s, clock %s, bit %i" % key, "is uses more than once (%i times)!" % count
            print "Check these trigger lines:"
            for tl in triggerlines:
                if key == (tl.cable, tl.fpga, tl.clock, tl.bit):
                    print "     ",tl.trigger

            allOk = False


        return allOk


    def check_LUTassignment(self):
        """
        Check if multibit algos are on a single LUT
        """

        allOk = True

        return allOk

# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# Elliot Lipeles, Univ. of Pennsylvania (2011)
# Tae Min Hong, Univ. of Pennsylvania (2011, 2012)
#
# Read Rate from TriggerRatePresenter root files
#
# Results are one rate per lumiblock
#

import sys
import os
import re
import optparse
import string
import math
import TrigCostAnalysis

import logging
logging.basicConfig(level=logging.INFO)
log = logging.getLogger('TrigCostTRP')

# Assume 10s sampling rate (current TRP setting)
samplingrate = 10

#----------------------------------------------------------------------
class TrigCostTRP:
    def __init__(self):
        pass

# for debugging
#import inspect
#print inspect.getframeinfo(inspect.currentframe())[:3]

#----------------------------------------------------------------------
# Note: prescale reading not used right now
#
def ReadTRPHLT(runnumber, lb_beg, lb_end, options=[], myafspath='', myhttppath='', levels='HLT'):
    HLTcollection = TrigCostAnalysis.CostResultCollection()
    HLTcollection.run   = runnumber
    HLTcollection.lbbeg = lb_beg
    HLTcollection.lbend = lb_end
    # Retrieve data
    filename = GetFileName(runnumber, myafspath, myhttppath)
    tchains  = GetTChains(runnumber, filename)

    for lvl in levels.split(','):

        log.info("Now processing Level = %s", lvl)

        # Retrieve lvl-dependent quantities
        sfx_in, sfx_ps, sfx_out = GetSuffixes(lvl, runnumber)
        tree          = tchains[lvl]                      # Get tree
        branches      = GetBranches(tree, lvl, sfx_out)   # Get branches

        # Counters for this tree
        entries       = tree.GetEntries()
        startpoint    = GetStartpoint(tree, lb_beg, entries)
        lblast        = 0                                 # Last LB number
        count         = 0                                 # Number of lb to average
        log.info("Tree name = %s, n-entries = %d", tree.GetName(), entries)
        # Average samplings in lumiblock
        for i in xrange(startpoint, entries):

            tree.GetEvent(i)
            lb = tree.LumiBlock
            log.info("Now processing new lumiblock == %d", lb)

            # Append to collection, if necessary
            if StopLoopOrInstantiate(lb, lb_beg, lb_end, lblast, lvl, HLTcollection, options): break

            # Loop over branch names
            for bname in branches:
                #print ' Branch', bname
                ProcessBranch(tree, lb, lblast, bname, lvl, count, "output", HLTcollection)

            # Increment
            count += 1
            lblast = lb

        # Record end of range read
        HLTcollection.lbend=lblast

    log.info("Done with branch loop")
    return HLTcollection
def ReadTRPL1(runnumber, lb_beg, lb_end, options=[], myafspath='', myhttppath='', levels='L1'):

    # Return object
    L1collection = TrigCostAnalysis.CostResultCollection()
    L1collection.run   = runnumber
    L1collection.lbbeg = lb_beg
    L1collection.lbend = lb_end

    # Retrieve data
    filename = GetFileName(runnumber, myafspath, myhttppath)
    tchains  = GetTChains(runnumber, filename)

    for lvl in levels.split(','):

        log.info("Now processing Level = %s", lvl)

        # Retrieve lvl-dependent quantities
        sfx_in, sfx_ps, sfx_out = GetSuffixes(lvl, runnumber)
        tree          = tchains[lvl]                      # Get tree
        branches      = GetBranches(tree, lvl, sfx_out)   # Get branches

        # Counters for this tree
        entries       = tree.GetEntries()
        startpoint    = GetStartpoint(tree, lb_beg, entries)
        lblast        = 0                                 # Last LB number
        count         = 0                                 # Number of lb to average
        log.info("Tree name = %s, n-entries = %d", tree.GetName(), entries)
        # Average samplings in lumiblock
        for i in xrange(startpoint, entries):

            tree.GetEvent(i)
            lb = tree.LumiBlock
            log.info("Now processing new lumiblock == %d", lb)

            # Append to collection, if necessary
            if StopLoopOrInstantiate(lb, lb_beg, lb_end, lblast, lvl, L1collection, options): break

            # Loop over branch names
            for bname in branches:
                #print ' Branch', bname
                ProcessBranch(tree, lb, lblast, bname, lvl, count, "TBP", L1collection)

            # Increment
            count += 1
            lblast = lb

        # Record end of range read
        L1collection.lbend=lblast
    log.info("Appending HLT collection!;?")

    log.info("Done with branch loop")
    return L1collection

#----------------------------------------------------------------------
def GetFileName(runnumber, myafspath='', myhttppath=''):

    eos_trp_path = "/eos/atlas/atlascerngroupdisk/tdaq-mon/coca/2017/TRP-Rates"
    filename = os.path.join(eos_trp_path, "TriggerRates_ATLAS_%d.root" % runnumber)

    return filename

#----------------------------------------------------------------------
def GetTChains(runnumber, filename):
    # Setup reading root file
    try:
        import ROOT                 ; #print '\tLoaded special ROOT package'
        import subprocess
        #import mda                  ; #print '\tLoaded mda'
        ROOT.gROOT.SetBatch(True)   ; #print '\tROOT set batch mode'
    except:
        print "pyROOT not avaiable: setup the release and then "
        print "export PYTHONPATH=$PYTHONPATH:$ROOTSYS/lib"
        sys.exit(-1)

#    subprocess.call(['/bin/sh', 'getfile.sh', filename])

    # Name change --- See e-mail thread below
    prefix = ''
    if runnumber < 178292:
        prefix = 'ISS_TRP.'

    tchains = { 'L1' : ROOT.TChain(prefix+'L1_Rate', 'L1'),
                'HLT' : ROOT.TChain(prefix+'HLT_Rate', 'HLT') }

    # Open TFiles
    for tc in tchains.values():
        log.info('For tchain, adding tfile=%s' % filename)
#        tc.Add(filename)
#        tc.Add(mda.FileRead().openFile(filename).GetEndpointUrl().GetUrl())
        tc.Add(filename)
    log.info(">>>>>>>>>> Finished getting TChains! <<<<<<<<<<")

    return tchains

#----------------------------------------------------------------------
# Skip rapidly (by 100s ~ 10s of lbs) to where lb_beg starts
#
def GetStartpoint(tree, lb_beg, entries):

    startpoint = 0
    stepsize = 1
    log.info("Finding TRP start point with %d entries", entries)

    for startpoint in xrange(0,entries,stepsize):
        tree.GetEvent(startpoint)
        lb = tree.LumiBlock
        if lb >= lb_beg:
            break
    log.info("Found TRP start point, lb = %d",lb)

    if startpoint>=stepsize:
        startpoint=startpoint-stepsize

    log.info(">>>>>>>>>> Finished finding GetStartPoint === %d <<<<<<<<<", startpoint)
    return startpoint

#----------------------------------------------------------------------
# L1 nomenclature in TRP ntuple
#   _TBP = Rate before applying L1 prescale rejection
#   _TAP = Rate after applying L1 prescale rejection
#   _TAV = Rate after applying L1 algorithmic rejection
#   _PS  = Prescale factor, maybe?
#   _DT  = Dead time calculated from above
#
# HLT nomenclature in TRP ntuple
#   _input    = Rate before applying HLT prescale rejection
#   _prescale = Rate after applying HLT prescale rejection
#   _raw      = Rate after applying HLT algorithmic rejection
#   _output   = Same as _raw (except for pass-thru & rerun)
#   _algoIn   = Rate at which the chain was rerun
#   _rerun    = In case of rerun or pass-thru, this counts the
#               number of times the algorithms were really
#               executed for this chain. (Something like
#               resource usage)
#
def GetSuffixes(lvl, runnumber):

    sfx_in  = 'TBP'
    sfx_ps  = 'TAP'
    sfx_out = 'TAV'

    if lvl!='L1':
        sfx_out = 'output'
        sfx_in  = 'input'
        sfx_ps  = 'prescale'
        if runnumber >= 202466:
            sfx_ps = 'prescaled' # Name change -- bug #92673

    return sfx_in, sfx_ps, sfx_out


#----------------------------------------------------------------------
# tmhong: This method of listing has been replaced by below
# find list of trigger chains (mathing br_out regexp)
#
# branches[lvl] = [b.GetName()[:-(len(br_out)-4)] \
#                  for b in list(tree.GetListOfBranches())
#                  if comp.match(b.GetName()) ]
#
def GetBranches(tree, lvl, sfx_out):

    # Return object
    branches = []

    # Exit
    if not tree.GetListOfBranches():
        log.error("Failed to open TRP file")
        sys.exit(-1)

    # Historical that this gets special treatment: e.g., 'L1.*_TBP'
    br_out = lvl + '.*_' + sfx_out

    # Regex pattern
    comp = re.compile(br_out)

    for branch in list(tree.GetListOfBranches()):
        if comp.match( branch.GetName() ):
            bname = branch.GetName()
            if "HLT" in bname :
                pos   = -(len(br_out)-5)
            else :
                pos   = -(len(br_out)-4)

            cname = bname[:pos]
            if ("L1_XE" in bname or "pufit" in bname) :
                branches.append( cname )

    return branches

#----------------------------------------------------------------------
# Return True to break out of event loop
def StopLoopOrInstantiate(lb, lb_beg, lb_end, lblast, lvl, collection, options):
    if 'fast' in options and lb==lblast: return False                               # If in same lumiblock
    if lb < lb_beg: return False                                                    # Current LB is below beginning LB range
    if lb > lb_end and lb_end!=-1: log.info("Done reading %s" % lvl) ; return True  # Breaks Loop
    if not (lblast > 0 and lb != lblast): return False                              # Not sure what this is

    # If new LB, then dump previous LB data
    if lvl=='L1':  collection.results[lb] = TrigCostAnalysis.CostResult()
    if lvl=='HLT': collection.results[lb] = TrigCostAnalysis.CostResult()

    sys.stdout.flush()
    return False

#----------------------------------------------------------------------
def ProcessBranch(tree, lb, lblast, bname, lvl, count, sfx_out, collection):

    # Is CPS = "coherent prescale"?
    cpsval=None
    if string.count(bname,"CPS"):
        cpsval = IsCPS(bname, tree, sfx_out)
        return

    # Set rate, counts, errors
    if lb!=lblast: ch = SetNewBranch(tree, lb, bname, lvl, count, cpsval, collection)
    else:          ch = SetOldBranch(tree, lb, bname, lvl, count, collection)

    # Print
#    PrintChain(bname, ch)
    return

#----------------------------------------------------------------------
def SetNewBranch(tree, lb, bname, lvl, count, cpsval, collection):

    # If new LB, then dump previous LB data
    ch = TrigCostAnalysis.CostChain()
    count = 1
    ch.SetName(bname)
    ch.SetLevel(lvl)

    if cpsval:
        ch.SetRate(cpsval)
    else:
#        rate = getattr(tree,bname+'_'+sfx_out)
#        ch.SetRate(rate)
#	        # Debug
#	        print bname, 'New lb=', lb, 'lblast=', lblast, 'count=', count, 'rate=', rate

        # Add all rates, not just output rates
        if lvl=='L1': SetNewChainL1 (tree, ch, bname)
        else:         SetNewChainHLT(tree, ch, bname)

    collection.SetCostChain(lb,bname,ch)
    if collection.lbbeg==-1:
        collection.lbbeg=lb

    return ch

#----------------------------------------------------------------------
def SetOldBranch(tree, lb, bname, lvl, count, collection):
    # Combine sample previous samples (#samples=count)
    ch = GetChain(tree, bname, lb, count, collection)

    if lvl=='L1': SetOldChainL1 (tree, ch, bname, count)
    else:         SetOldChainHLT(tree, ch, bname, count)

    return ch

#----------------------------------------------------------------------
def PrintChain(bname, ch):
    if ch is None:
      return
    print 'ch.GetRate()', bname, ch.GetRate()
    print 'ch.GetTBPCnt()', bname, ch.GetTBPCnt()
    print 'ch.GetTAPCnt()', bname, ch.GetTAPCnt()
    print 'ch.GetTAVCnt()', bname, ch.GetTAVCnt()
    print 'ch.GetTBPRate()', bname, ch.GetTBPRate()
    print 'ch.GetTAPRate()', bname, ch.GetTAPRate()
    print 'ch.GetTAVRate()', bname, ch.GetTAVRate()
    return

#----------------------------------------------------------------------
def IsCPS(bname, tree, sfx_out):
    cpsbr  = tree.GetBranch(bname+'_'+sfx_out)
    if string.count(bname,":")==0:
        leaf   = bname+'_'+sfx_out
    else:
        leaf   = string.split(bname,":")[1]+'_'+sfx_out
#       cpsbr.GetListOfLeaves().Print()
        cpsval = cpsbr.GetLeaf(leaf).GetValue()
#       print bname,cpsbr.GetLeaf(leaf),cpsval

    return cpsval

#----------------------------------------------------------------------
# Note the formula:
#   err = sqrt(events in time T)/T = sqrt(rate*T/T^2) = sqrt(rate/T)
#
def GetChain(tree, bname, lb, count, collection):
    ch          = collection.GetCostChain(lb,bname)
    return ch

#----------------------------------------------------------------------
def SetNewChainL1(tree, ch, bname):
    ch.SetPrescale(getattr(tree,bname+'_PS'))

    if ch.GetName() == 'L1_XE60':
        log.info("L1_XE60 TBP = %f", getattr(tree,bname+'_TBP'))
        log.info("L1_XE60 TAP = %f", getattr(tree,bname+'_TAP'))

    # Reverse-engineer counts -- will be approximate
    ch.SetTBPCnt (getattr(tree,bname+'_TBP')*samplingrate)
    ch.SetTAPCnt (getattr(tree,bname+'_TAP')*samplingrate)
    ch.SetTAVCnt (getattr(tree,bname+'_TAV')*samplingrate)

    # Rates are stored in TRP ntuples
    ch.SetTBPRate(getattr(tree,bname+'_TBP'))
    ch.SetTAPRate(getattr(tree,bname+'_TAP'))
    ch.SetTAVRate(getattr(tree,bname+'_TAV')) # Don't uncomment!! Already done in SetRate
    return

#----------------------------------------------------------------------
def SetNewChainHLT(tree, ch, bname):
#	  ch.SetPrescale(getattr(tree,bname+'_prescale'))

    if ch.GetName() == 'HLT_xe110_pufit_L1XE60':
        log.info("HLT_xe110_pufit_L1XE60 input  = %f", getattr(tree,bname+'_input'))
        log.info("HLT_xe110_pufit_L1XE60 output = %f", getattr(tree,bname+'_output'))

    # Reverse-engineer counts -- will be approximate
    ch.SetTBPCnt (getattr(tree,bname+'_input')*samplingrate)
    ch.SetTAPCnt (getattr(tree,bname+'_output')*samplingrate)
    ch.SetTAVCnt (getattr(tree,bname+'_raw')*samplingrate)

#   if getattr(tree,bname+'_output'     )-getattr(tree,bname+'_raw'     ) > 1e-10:
#   print "not equal",bname,getattr(tree,bname+'_'+sfx_out     ),getattr(tree,bname+'_raw'     )

    # Rates are stored in TRP ntuples
    ch.SetTBPRate(getattr(tree,bname+'_input'))
    ch.SetTAPRate(getattr(tree,bname+'_output'))
    ch.SetTAVRate(getattr(tree,bname+'_raw')) # Don't uncomment!! Already done in SetRate
    return

#----------------------------------------------------------------------
def SetOldChainL1(tree, ch, bname, count):
    if ch is None:
      return
    ch.SetPrescale(getattr(tree,bname+'_PS'))


    # Reverse-engineer counts -- will be approximate
    ch.SetTBPCnt (getattr(tree,bname+'_TBP')*samplingrate + ch.GetTBPCnt())
    ch.SetTAPCnt (getattr(tree,bname+'_TAP')*samplingrate + ch.GetTAPCnt())
    ch.SetTAVCnt (getattr(tree,bname+'_TAV')*samplingrate + ch.GetTAVCnt())

    # Rates are stored in TRP ntuples
    # This needs to be a running average over the entries in the LB

    setattr(ch,"ratesCounts", ch.GetAttrWithDefault("ratesCounts",0) + 1.0)

    setattr(ch,"cumulativeTBP", ch.GetAttrWithDefault("cumulativeTBP",0) + getattr(tree,bname+'_TBP'))
    setattr(ch,"cumulativeTAP", ch.GetAttrWithDefault("cumulativeTAP",0) + getattr(tree,bname+'_TAP'))
    setattr(ch,"cumulativeTAV", ch.GetAttrWithDefault("cumulativeTAV",0) + getattr(tree,bname+'_TAV'))

    ch.SetTBPRate(getattr(ch,"cumulativeTBP") / getattr(ch,"ratesCounts"))
    ch.SetTAPRate(getattr(ch,"cumulativeTAP") / getattr(ch,"ratesCounts"))
    ch.SetTAVRate(getattr(ch,"cumulativeTAV") / getattr(ch,"ratesCounts")) 

    #if ch.GetName() == 'L1_EM10VH':
    #    print "DBG OLD L1 " , ch.GetAttrWithDefault("cumulativeTBP",0) , " - " , ch.GetTBPRate()

    # Rate errors
    ch.SetTBPRateErr(math.sqrt( (getattr(ch,"cumulativeTBP")/getattr(ch,"ratesCounts")) / (getattr(ch,"ratesCounts")*samplingrate) ))
    ch.SetTAPRateErr(math.sqrt( (getattr(ch,"cumulativeTAP")/getattr(ch,"ratesCounts")) / (getattr(ch,"ratesCounts")*samplingrate) ))
    ch.SetTAVRateErr(math.sqrt( (getattr(ch,"cumulativeTAV")/getattr(ch,"ratesCounts")) / (getattr(ch,"ratesCounts")*samplingrate) )) 
    return


#----------------------------------------------------------------------
def SetOldChainHLT(tree, ch, bname, count):
#	  ch.SetPrescale(getattr(tree,bname+'_prescale'))

    if ch is None:
      return

    # Reverse-engineer counts -- will be approximate
    ch.SetTBPCnt (getattr(tree,bname+'_input')*samplingrate+ch.GetTBPCnt())
    ch.SetTAPCnt (getattr(tree,bname+'_prescaled')*samplingrate+ch.GetTAPCnt())
    ch.SetTAVCnt (getattr(tree,bname+'_output')*samplingrate+ch.GetTAVCnt())

    # Rate are stored in TRP ntuples
    setattr(ch,"ratesCounts", ch.GetAttrWithDefault("ratesCounts",0) + 1.0)

    setattr(ch,"cumulativeTBP", ch.GetAttrWithDefault("cumulativeTBP",0) + getattr(tree,bname+'_input'))
    setattr(ch,"cumulativeTAP", ch.GetAttrWithDefault("cumulativeTAP",0) + getattr(tree,bname+'_prescaled'))
    setattr(ch,"cumulativeTAV", ch.GetAttrWithDefault("cumulativeTAV",0) + getattr(tree,bname+'_output'))

    ch.SetTBPRate(getattr(ch,"cumulativeTBP") / getattr(ch,"ratesCounts"))
    ch.SetTAPRate(getattr(ch,"cumulativeTAP") / getattr(ch,"ratesCounts"))
    ch.SetTAVRate(getattr(ch,"cumulativeTAV") / getattr(ch,"ratesCounts")) 

    # Rate errors
    ch.SetTBPRateErr(math.sqrt( (getattr(ch,"cumulativeTBP")/getattr(ch,"ratesCounts")) / (getattr(ch,"ratesCounts")*samplingrate) ))
    ch.SetTAPRateErr(math.sqrt( (getattr(ch,"cumulativeTAP")/getattr(ch,"ratesCounts")) / (getattr(ch,"ratesCounts")*samplingrate) ))
    ch.SetTAVRateErr(math.sqrt( (getattr(ch,"cumulativeTAV")/getattr(ch,"ratesCounts")) / (getattr(ch,"ratesCounts")*samplingrate) )) 
    return

# ----------------------------------------------------------------------
# E-mail thread for the Naming change after 178292
#
#	On 04/15/2011 Antonio Sidoti wrote to Tomasz, Ivana, Francesca, Tae
#	    Yeah sorry for that, this was not really something I wanted to
#	    change but it is less cumbersome....  So the change (for run
#	    in ATLAS partitions) is for run >=178292 (runs took after the
#	    28/03/2011 morning)
#
#	On 04/15/2011 02:17 PM, Tomasz Bold wrote:
#	    Hi Antonio, Uff. Do you know from which run we should change?
#
# On 15 April 2011 14:06, Antonio Sidoti wrote:
#	    Dear TRP-Rates power users,
#	    I've just discovered that when we've installed the new version
#	    of the archiver just before the technical shutdown, the suffix
#	    ISS_TRP was dropped from the TTree name containing the rates
#	    of the TimePoint.  So for example, the name of the TTree that
#	    was containing the L1 rates (from CTP) passed from
#	    "ISS_TRP.L1_Rate" to "L1_Rate". This was not foreseen but I
#	    think it is less cumbersome as it is now.  So please update
#	    your scripts.

#eof

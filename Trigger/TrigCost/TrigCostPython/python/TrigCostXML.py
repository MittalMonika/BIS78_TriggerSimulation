# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# Elliot Lipeles, Univ. of Pennsylvania (2011)
#

#print 'Hello world TrigCostXML.py'

import xml.dom.minidom
import string
import math
import traceback
import sys
#print '\tLoaded standard packages'

from TrigCostAnalysis import *              ;
#print '\tLoaded special TrigCostAnalysis.* package'
#print '\tDone prelim import for TrigCostXML.py'

#---------------------------------------------------------------------
# Function for parsing xml file with trigger rate results
#
def ReadXmlFile(filename):

    # map from number to string level
    lvlmap = { '1':'L1', '2':'L2', '3':'EF' }

    result = CostResult()

    def GetData(nodelist, name):
        #print name,[x.nodeName for x in nodelist]

        for val in nodelist:
            if val.nodeName != name:
                continue

            svalue = ""
            icount = int(0)
            for s in val.childNodes:
                icount = icount+1
                svalue = s.data

            if icount == 1:
                return svalue.strip()
            elif icount == 0:
                return ''
            elif icount > 1 :
                raise Exception('ReadXmlFile - Node has multiple data entries')

        raise Exception('ReadXmlFile - failed to find data, '+name)
        return ""

    #copy result to chain objects

    myf = None
    try:
        if filename.count('http') > 0 :
            import urllib
            myf = urllib.urlopen(filename)
        else :
            myf = open(filename)
    except:
        print "Failed to open file:",filename
        return result

    try:
        dom = xml.dom.minidom.parse(myf)
    except Exception,e:
        print "Failed to parse file:",filename
        traceback.print_exc(file=sys.stdout)
        return result

    #print 'Parsing XML file:',filename
    result.source = 'not_set'
    for top in dom.getElementsByTagName("trigger"):

        for level in top.childNodes:
            if level.nodeName=="Luminosity":
                result.source = 'XML_prediction'

            if level.nodeName!="level":
                continue


            lvl_name = GetData(level.childNodes, "lvl_name")
            #print "Reading level ",lvl_name

            # added a "chain" for the entire level
            ch = CostChain()

            # using TRP compatible level names
            if lvl_name=="L1":
                ch.SetName("L1_L1A")
            if lvl_name=="L2":
                ch.SetName("L2_total_physics")
            if lvl_name=="EF":
                ch.SetName("EF_recording_physics")

            ch.SetLevel(lvl_name)
            result.SetChain(ch.GetName(),ch)

            # level rates are not always there
            try:
                ch.SetRate(string.atof(GetData(level.childNodes, 'lvl_rate')))
                ch.SetRateErr(string.atof(GetData(level.childNodes, 'lvl_rate_err')))
            except Exception,e:
                print "No level rate available for ",lvl_name
                ch.SetRate(0.0)
                ch.SetRateErr(0.0)
                pass

            ch.SetPrescale(0.0)

            # add all the chains
            for sig in level.childNodes:
                if sig.nodeName!="signature":
                    continue

                ch = CostChain()

                ch.SetName(GetData(sig.childNodes, "sig_name"))
                ch.SetLevel(lvl_name)
                ch.SetPrescale(string.atof(GetData(sig.childNodes, 'prescale')))
                ch.SetPassthrough(string.atof(GetData(sig.childNodes, 'passthrough')))
                ch.SetRate(string.atof(GetData(sig.childNodes, 'rate')))
                ch.SetRateErr(string.atof(GetData(sig.childNodes, 'rate_err')))
                ch.SetEff(string.atof(GetData(sig.childNodes, 'efficiency')))
                ch.SetPrescaledEff(string.atof(GetData(sig.childNodes, 'prescaled_efficiency')))

                # not always there
                try:
                    ch.SetLowerChain(GetData(sig.childNodes, "lower_chain_name"))
                except:
                    pass

                nevts = string.atof(GetData(sig.childNodes, 'evts_passed_weighted'))
                ch.SetNEvents(nevts)
                ch.SetNEventsErr(math.sqrt(max(nevts,0)))

                result.SetChain(ch.GetName(),ch)

            # add "chains" for groups
            for sig in top.childNodes:
                if sig.nodeName!="cplx_signature":
                    continue

                gtype=GetData(sig.childNodes, 'type')
                if gtype == 'Group' or gtype == 'Stream' :
                    ch = CostChain()
                    name=GetData(sig.childNodes, "sig_name")

                    # strip stream type from trp streams
                    if string.count(name,"str"):
                        name = name.replace("_physics","")
                        name = name.replace("_calibration","")
                        name = name.replace("_debug","")

                    # add stream tag to TriggerCost streams
                    if gtype == 'Stream':
                        name = name.replace("EF_","EF_str_")

                    ch.SetName(name)
                    ch.SetIsGroup(True)
                    ch.SetRate(string.atof(GetData(sig.childNodes, 'rate')))
                    ch.SetRateErr(string.atof(GetData(sig.childNodes, 'rate_err')))
                    level=None
                    try:
                        level=GetData(sig.childNodes, 'level')
                        ch.SetLevel(lvlmap[level])
                    except:
                        level=name[:2]
                        ch.SetLevel(level)

                    result.SetChain(ch.GetName(),ch)

    return result


#---------------------------------------------------------------------
# Function for export rates to xml file
# rates should be a CostResult
#
def WriteXmlFile(filename,rates,lbset=None):

    #open file / write header
    xout = open(filename, 'w')
    xout.write('<?xml version="1.0" encoding="ISO-8859-1"?>\n')
    xout.write('<trigger>\n')


    for lvl in ['L1','L2','EF']:
        xout.write('  <level>\n')
        xout.write('    <lvl_name>'+lvl+'</lvl_name>\n')

        for chname in rates.GetChainNames(lvl):
            ch = rates.GetChain(chname)
            # skip groups -- they go in later
            if (string.count(chname,"grp") \
                or string.count(chname,"str") \
                or string.count(chname,"RATE:") \
                or string.count(chname,"BW:") \
                or string.count(chname,"CPS:")):
                continue

            xout.write('    <signature>\n')
            xout.write('      <sig_name>'                +chname +'</sig_name>\n')
            xout.write('      <sig_counter>'             +str(0)+'</sig_counter>\n')
            xout.write('      <evts_passed>'             +str(ch.GetAttrWithDefault("nevts",0))\
                           +'</evts_passed>\n')
            xout.write('      <evts_passed_weighted>'    +str(0)\
                           +'</evts_passed_weighted>\n')
            xout.write('      <rate>'                    +str(ch.GetRate())\
                           +'</rate>\n')
            xout.write('      <rate_err>'                +str(ch.GetAttrWithDefault("rateerr",0))\
                           +'</rate_err>\n')
            xout.write('      <prescale>'                +str(ch.GetAttrWithDefault("prescale",0))\
                           +'</prescale>\n')
            xout.write('      <passthrough>'             +str(ch.GetAttrWithDefault("passthrough",0))\
                           +'</passthrough>\n')
            xout.write('      <lower_chain_name>'        +str(ch.GetAttrWithDefault("lowerchain",""))\
                           +'</lower_chain_name>\n')
            xout.write('      <efficiency>'              +str(ch.GetAttrWithDefault("efficiency",0))\
                           +'</efficiency>\n')
            xout.write('      <efficiency_err>'          +str(ch.GetAttrWithDefault("efferr",0))\
                           +'</efficiency_err>\n')
            xout.write('      <prescaled_efficiency>'    +str(ch.GetAttrWithDefault("prescaledeff",0))\
                           +'</prescaled_efficiency>\n')
            xout.write('      <prescaled_efficiency_err>'+str(ch.GetAttrWithDefault("psefferr",0))\
                           +'</prescaled_efficiency_err>\n')
            xout.write('    </signature>\n')

        xout.write('  </level>\n')


    for lvl in ['L1','L2','EF']:

        for chname in rates.GetChainNames(lvl):
            ch = rates.GetChain(chname)

            if not (string.count(chname,"grp") \
                    or string.count(chname,"str") \
                    or string.count(chname,"RATE:") \
                    or string.count(chname,"BW:") \
                    or string.count(chname,"CPS:")):
                continue

            xout.write('<cplx_signature>\n')
            xout.write('<type>Group</type>\n')

            outname = chname.replace("_grp_","_RATE:")

            xout.write('<sig_name>'+outname+'</sig_name>\n')
            xout.write('      <level>'                   +str(ch.GetLevelNum())\
                           +'</level>\n')
            xout.write('      <rate>'                    +str(ch.GetRate())\
                           +'</rate>\n')
            xout.write('      <rate_err>'                +str(ch.GetAttrWithDefault("rateerr",0))\
                           +'</rate_err>\n')
            xout.write('      <prescale>'                +str(ch.GetAttrWithDefault("prescale",0))\
                           +'</prescale>\n')
            xout.write('      <passthrough>'             +str(ch.GetAttrWithDefault("passthrough",0))\
                           +'</passthrough>\n')
            xout.write('      <efficiency>'              +str(ch.GetAttrWithDefault("efficiency",0))\
                           +'</efficiency>\n')
            xout.write('      <efficiency_err>'          +str(ch.GetAttrWithDefault("efferr",0))\
                           +'</efficiency_err>\n')
            xout.write('      <prescaled_efficiency>'    +str(ch.GetAttrWithDefault("prescaledeff",0))\
                           +'</prescaled_efficiency>\n')
            xout.write('      <prescaled_efficiency_err>'+str(ch.GetAttrWithDefault("psefferr",0))\
                           +'</prescaled_efficiency_err>\n')
            xout.write('<unique_rate>0</unique_rate>\n')
            xout.write('<components>\n')
            xout.write('</components>\n')
            xout.write('</cplx_signature>\n')

    # Write out bunch group information
    if lbset:
        xout.write('<lumi_info>\n')
        xout.write('  <integrated_time>%g</integrated_time>\n' % lbset.GetTotalTime())
        xout.write('  <live_time>%g</live_time>\n' % lbset.GetLiveTime())
        xout.write('  <delivered_lumi_ub-1>%g</delivered_lumi_ub-1>\n' % lbset.GetDeliveredLumi())
        xout.write('  <rec_lumi_ub-1>%g</rec_lumi_ub-1>\n' % lbset.GetRecordedLumi())
        lbrange = xrange(lbset.lbbeg,lbset.lbend+1)
        for lb in lbrange:
            xout.write('  <lb>\n')
            xout.write('    <ncolliding>%d</ncolliding>\n' % lbset.GetBunches(lb))
            xout.write('    <nbeam1>%d</nbeam1>\n' % lbset.GetNbunch1(lb))
            xout.write('    <nbeam2>%d</nbeam2>\n' % lbset.GetNbunch2(lb))
            xout.write('    <lbnumber>%d</lbnumber>\n' % lb)
            xout.write('    <nbunchgroup0>%d</nbunchgroup0>\n' % lbset.lbs[lb].bgLength0)
            xout.write('    <nbunchgroup1>%d</nbunchgroup1>\n' % lbset.lbs[lb].bgLength1)
            xout.write('    <nbunchgroup2>%d</nbunchgroup2>\n' % lbset.lbs[lb].bgLength2)
            xout.write('    <nbunchgroup3>%d</nbunchgroup3>\n' % lbset.lbs[lb].bgLength3)
            xout.write('    <nbunchgroup4>%d</nbunchgroup4>\n' % lbset.lbs[lb].bgLength4)
            xout.write('    <nbunchgroup5>%d</nbunchgroup5>\n' % lbset.lbs[lb].bgLength5)
            xout.write('    <nbunchgroup6>%d</nbunchgroup6>\n' % lbset.lbs[lb].bgLength6)
            xout.write('    <nbunchgroup7>%d</nbunchgroup7>\n' % lbset.lbs[lb].bgLength7)
            xout.write('    <bunchgroupName0>%s</bunchgroupName0>\n' % lbset.lbs[lb].bgName0)
            xout.write('    <bunchgroupName1>%s</bunchgroupName1>\n' % lbset.lbs[lb].bgName1)
            xout.write('    <bunchgroupName2>%s</bunchgroupName2>\n' % lbset.lbs[lb].bgName2)
            xout.write('    <bunchgroupName3>%s</bunchgroupName3>\n' % lbset.lbs[lb].bgName3)
            xout.write('    <bunchgroupName4>%s</bunchgroupName4>\n' % lbset.lbs[lb].bgName4)
            xout.write('    <bunchgroupName5>%s</bunchgroupName5>\n' % lbset.lbs[lb].bgName5)
            xout.write('    <bunchgroupName6>%s</bunchgroupName6>\n' % lbset.lbs[lb].bgName6)
            xout.write('    <bunchgroupName7>%s</bunchgroupName7>\n' % lbset.lbs[lb].bgName7)
            xout.write('  </lb>\n')

        xout.write('</lumi_info>\n')

    xout.write('</trigger>\n')

    xout.close()

#-----------------------------------------------------
# Read the dataset xml format and return a list of DatasetDescr objects
#
def ReadDatasetXML(filename) :

    def GetData(nodelist, name):
        #print name,[x.nodeName for x in nodelist]

        for val in nodelist:
            if val.nodeName != name:
                continue

            svalue = ""
            icount = int(0)
            for s in val.childNodes:
                icount = icount+1
                svalue = s.data

            if icount == 1:
                return svalue.strip()
            elif icount == 0:
                return ''
            elif icount > 1 :
                raise Exception('ReadXmlFile - Node has multiple data entries')

        print 'ReadXmlFile - failed to find data'
        return ""

    result = []

    try:
        myf = open(filename)
    except:
        print "Failed to open file:",filename
        return result

    try:
        dom = xml.dom.minidom.parse(myf)
    except Exception,e:
        print "Failed to parse file:",filename
        traceback.print_exc(file=sys.stdout)
        return result

    #print 'Parsing XML file:',filename
    for set in dom.getElementsByTagName('dataset'):

        descr = DatasetDescr()

        descr.name               = GetData(set.childNodes, 'name')
        descr.parent             = GetData(set.childNodes, 'parent')
        descr.path               = GetData(set.childNodes, 'path')
        descr.energy             = AtoF(GetData(set.childNodes, 'energy'))
        descr.cross_section      = AtoF(GetData(set.childNodes, 'cross_section'))
        descr.gen_eff            = AtoF(GetData(set.childNodes, 'gen_eff'))
        descr.collisions_per_evt = AtoF(GetData(set.childNodes, 'collisions_per_evt'))
        descr.menu               = GetData(set.childNodes, 'menu')
        descr.tune               = GetData(set.childNodes, 'tune')
        descr.AtlasProject       = GetData(set.childNodes, 'AtlasProject')
        descr.AtlasVersion       = GetData(set.childNodes, 'AtlasVersion')
        descr.year               = AtoI(GetData(set.childNodes, 'year'))
        descr.month              = AtoI(GetData(set.childNodes, 'month'))
        descr.day                = AtoI(GetData(set.childNodes, 'day'))
        descr.version            = GetData(set.childNodes, 'version')
        descr.comment            = GetData(set.childNodes, 'comment')

        result.append(descr)

    return result

#---------------------------------------------------------------------
def AtoF(string) :

    try :
        return float(string)
    except ValueError :
        return -1.0

#---------------------------------------------------------------------
def AtoI(string) :
    try :
        return int(string)
    except ValueError :
        return -1

#eof

# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
from __future__ import print_function


class LArExtendedFTGrouping:
    def __init__(self):
        self._EMBC=range(3,35)#+[1006]
        self._EMBA=range(35,67)#+[1005]
        self._EMBCPS=range(117,149)#+[]
        self._EMBAPS=range(149,181)#+[]

        #List of EMEC OW channels (C-side)
        EMEC=(67, 68, 69, 71, 72, 74, 75, 76, 78, 79, 80, 81, 82, 84, 85, 86, 87, 88, 90, 91)
        self._EMECC=EMEC+(231,233,235,237) # EMEC C OW + IW
        self._EMECA=()
        for i in EMEC: #EMECA OW is EMECC OW +25
            self._EMECA+=(i+25,)
        self._EMECA+=(232,234,236,238) #EMECA OW + EMECA IW

        self._EMECCPS=()
        self._EMECAPS=()
        for i in EMEC:
            self._EMECCPS+=(i+114,)
            self._EMECAPS+=(i+139,)

        self._HECC=(70,77,83,89)
        self._HECA=()
        for i in self._HECC:
            self._HECA+=(i+25,)

        self._FCALC=(73,)
        self._FCALA=(73+25,)

        self._emptyC=(184,187,191,197,203)
        self._emptyA=(209,212,216,222,228)
        self._empty=self._emptyC + self._emptyA
        
        self._withCorr=True

        #Build dictionary:
        self._partitions=dict()
        self._partitions['EMBAPS'] = self._EMBAPS
        self._partitions['EMBCPS'] = self._EMBCPS
        self._partitions['EMECAPS'] = self._EMECAPS
        self._partitions['EMECCPS'] = self._EMECCPS
        self._partitions['EMBA'] = self._EMBA
        self._partitions['EMBC'] = self._EMBC
        self._partitions['EMECA']= self._EMECA
        self._partitions['EMECC'] = self._EMECC
        self._partitions['HECA'] = self._HECA
        self._partitions['HECC'] = self._HECC
        self._partitions['FCALA'] = self._FCALA
        self._partitions['FCALC'] = self._FCALC
        self._partitions['EMPTYA'] = self._emptyA
        self._partitions['EMPTYC'] = self._emptyC

        #Correction Channels:
        self._corr=dict()
        self._corr['EMBAPS'] = 1001
        self._corr['EMBCPS'] = 1002
        self._corr['EMECAPS'] = 1003
        self._corr['EMECCPS'] = 1004
        self._corr['EMBA']  = 1005
        self._corr['EMBC']  = 1006
        self._corr['EMECA'] = 1007
        self._corr['EMECC'] = 1008
        self._corr['HECA']  = 1009
        self._corr['HECC']  = 1010
        self._corr['FCALA'] = 1011
        self._corr['FCALC'] = 1012

        #Lookup-dict indexed by channel (partition is the payload)
        self._revLookup=dict()
        for (p, chs) in self._partitions.iteritems():
            for c in chs:
                self._revLookup[c]=p


    def setWithCorr(self,val):
        self._withCorr=val

    def Print(self):
        print(self._EMBC)
        print(self._EMBA)


    def getChannelList(self,partitions,gains=[0]):
        chans=list()
        for g in gains:
            if g<0 or g>2:
                print("ERROR: Unkown gain",g)
                return None

        extPart=list()
        for partition in partitions:
            p=partition.upper()
            if (p=="ECC"):
                extPart+=["EMECCPS","EMECC","HECC","FCALC"]
            elif (p=="ECA"):
                extPart+=["EMECAPS","EMECA","HECA","FCALA"]
            elif (p=="EC"):
                extPart+=["EMECAPS","EMECA","HECA","FCALA",
                          "EMECCPS","EMECC","HECC","FCALC"]
            elif (p=="EMB"):
                extPart+=["EMBA","EMBAPS","EMBC","EMBCPS"]
            else:
                extPart+=[p]
                
        for p in extPart:
            if self._partitions.has_key(p):
                for g in gains:
                    for c in self._partitions[p]:
                        chans+=[c+(g*236)]
                if (self._withCorr):
                    for g in gains:
                        chans+=[self._corr[p]+g*12]
            else:
                print("ERROR: Unkown partition '",partition,"'")

        return chans

    def makeRange(self,chans):
        chans.sort()
        retVal=""
        if (len(chans)==0):
            return retVal
        retVal=str(chans[0])
        c1=chans[0]
        sep=','
        series=False
        for c2 in chans[1:]:
            if c1 == c2:
                print("Duplicated entry",c2)
                continue
            if c2-1 == c1 or c2-1 in self._empty:
                series=True
            else:
                if series:
                    retVal+=":"+str(c1)+","+str(c2)
                    series=False
                else:
                    retVal+=","+str(c2)
#            print ("c1=",c1,"c2=",c2,"sep=",sep)
            c1=c2
        if series: retVal+=":"+str(c1)
        return retVal



    def getChannelSelection(self,partitions,gains):
        chans=list()
        return self.makeRange(self.getChannelList(partitions,gains))

    def getChannelSelectionAllGains(self,partitions):
        chans=list()
        for p in partitions:
            chans+=self.getChannelList([p],[0,1,2])
        return self.makeRange(chans)


    def getGain(self,c):
        if c<2:
            return (0,None)
        if c<239:
            return (0,c)
        if c<475:
            return (1,c-236)
        if c<711:
            return (2,c-472)
        if c<1001:
            return (0,None)
        if c<1013:
            return (0,c)
        if c<1025:
            return (1,c-12)
        if c<1037:
            return (2,c-24)
        return (0,None)


    def channelsPerPartition(self,chans,show=True):
        class counterElem:
            def __init__(self,l,n):
                self.size=l;
                self.name=n
                self.counts=[0,0,0]
            def inc(self,g):
                self.counts[g]=1+self.counts[g]

            def show(self):
                print("%7s: " % self.name, end="")
                print("HIGH:%2i/%2i" % (self.counts[0],self.size),)
                if self.counts[0] != self.size:
                    print ("*  ",end="")
                else:
                    print ("   ",end="")
                print ("MED:%2i/%2i" % (self.counts[1],self.size),)
                if self.counts[1] != self.size:
                    print ("*  ",end="")
                else:
                    print ("   ",end="")
                print ("LOW:%2i/%2i" % (self.counts[2],self.size),)
                if self.counts[2] != self.size:
                    print ("*  ")
                else:
                    print ("   ")
            
        partCounter=dict()
        for (p, chs) in self._partitions.iteritems():
            partCounter[p]=counterElem(len(chs),p)
        
        for c in chans:
            (gain,cs)=self.getGain(c)
            if cs == None:
                print("ERROR: Unkown channel",c)
            else:    
                if (c<711):
                    p=self._revLookup[cs]
                    partCounter[p].inc(gain)

        return partCounter



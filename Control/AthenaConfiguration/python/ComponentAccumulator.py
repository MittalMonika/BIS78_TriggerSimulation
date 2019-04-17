# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaCommon.Configurable import Configurable,ConfigurableService,ConfigurableAlgorithm,ConfigurableAlgTool
from AthenaCommon.CFElements import isSequence,findSubSequence,findAlgorithm,flatSequencers,findOwningSequence,\
    checkSequenceConsistency, findAllAlgorithmsByName
from AthenaCommon.AlgSequence import AthSequencer

import GaudiKernel.GaudiHandles as GaudiHandles

from Deduplication import deduplicate, deduplicateComponent, DeduplicationFailed

import ast
import collections

from UnifyProperties import unifySet

class ConfigurationError(RuntimeError):
    pass

_servicesToCreate=frozenset(('GeoModelSvc','TileInfoLoader'))

class ComponentAccumulator(object):

    def __init__(self,sequenceName='AthAlgSeq'):
        self._msg=logging.getLogger('ComponentAccumulator')
        self._sequence=AthSequencer(sequenceName,Sequential=True)    #(Nested) sequence of event processing algorithms per sequence + their private tools
        self._conditionsAlgs=[]          #Unordered list of conditions algorithms + their private tools
        self._services=[]                #List of service, not yet sure if the order matters here in the MT age
        self._eventInputs=set()          #List of items (as strings) to be read from the input (required at least for BS-reading).
        self._outputPerStream={}         #Dictionary of {streamName,set(items)}, all as strings
        self._privateTools=None          #A placeholder to carry a private tool(s) not yet attached to its parent
        self._primaryComp=None           #A placeholder to designate the primary service 

        self._theAppProps=dict()        #Properties of the ApplicationMgr

        #Backward compatiblity hack: Allow also public tools:
        self._publicTools=[]

        #To check if this accumulator was merged:
        self._wasMerged=False
        self._isMergable=True

        self._algorithms = {}

    def empty(self):
        return len(self._sequence)+len(self._conditionsAlgs)+len(self._services)+\
            len(self._publicTools)+len(self._outputPerStream)+len(self._theAppProps) == 0

    def __del__(self):
        if not self._wasMerged and not self.empty():
            raise RuntimeError("ComponentAccumulator was not merged!")
            #log = logging.getLogger("ComponentAccumulator")
            #log.error("The ComponentAccumulator listed below was never merged!")

        if self._privateTools is not None:
            raise RuntimeError("Deleting a ComponentAccumulator with and dangling private tool(s)")
        #pass



    def printConfig(self, withDetails=False, summariseProps=False):
        self._msg.info( "Event Inputs" )
        self._msg.info( self._eventInputs )
        self._msg.info( "Event Algorithm Sequences" )

        def printProperties(c, nestLevel = 0):
            for propname, propval in c.getValuedProperties().iteritems():
                # Ignore empty lists
                if propval==[]:
                    continue
                # Printing EvtStore could be relevant for Views?
                if propname in ["DetStore","EvtStore"]:
                    continue

                propstr = str(propval)
                if isinstance(propval,GaudiHandles.PublicToolHandleArray):
                    ths = [th.getFullName() for th in propval]
                    propstr = "PublicToolHandleArray([ {0} ])".format(', '.join(ths))
                elif isinstance(propval,GaudiHandles.PrivateToolHandleArray):
                    ths = [th.getFullName() for th in propval]
                    propstr = "PrivateToolHandleArray([ {0} ])".format(', '.join(ths))
                elif isinstance(propval,ConfigurableAlgTool):
                    propstr = propval.getFullName()
                self._msg.info( " "*nestLevel +"    * {0}: {1}".format(propname,propstr) )
            return

        if withDetails:
            self._msg.info( self._sequence )
        else:
            def printSeqAndAlgs(seq, nestLevel = 0):
                def __prop(name):
                    if name in seq.getValuedProperties():
                        return seq.getValuedProperties()[name]
                    return seq.getDefaultProperties()[name]

                self._msg.info( " "*nestLevel +"\\__ "+ seq.name() +" (seq: %s %s)",
                                "SEQ" if __prop("Sequential") else "PAR", "OR" if __prop("ModeOR") else "AND" )
                nestLevel += 3
                for c in seq.getChildren():
                    if isSequence(c):
                        printSeqAndAlgs(c, nestLevel )
                    else:
                        self._msg.info( " "*nestLevel +"\\__ "+ c.name() +" (alg)" )
                        if summariseProps:
                            printProperties(c, nestLevel)
            printSeqAndAlgs(self._sequence)

        self._msg.info( "Condition Algorithms" )
        self._msg.info( [ a.getName() for a in self._conditionsAlgs ] )
        self._msg.info( "Services" )
        self._msg.info( [ s.getName() for s in self._services ] )
        self._msg.info( "Outputs" )
        self._msg.info( self._outputPerStream )
        self._msg.info( "Public Tools" )
        self._msg.info( "[" )
        for t in self._publicTools:
            self._msg.info( "  {0},".format(t.getFullName()) )
            # Not nested, for now
            if summariseProps:
                printProperties(t)
        self._msg.info( "]" )


    def addSequence(self, newseq, parentName = None ):
        """ Adds new sequence. If second argument is present then it is added under another sequence  """
        if not isSequence(newseq):
            raise TypeError('{} is not a sequence'.format(newseq.name()))

        if parentName is None:
            parent=self._sequence
        else:
            parent = findSubSequence(self._sequence, parentName )
            if parent is None:
                raise ConfigurationError("Missing sequence %s to add new sequence to" % parentName )

        parent += newseq
        algsByName = findAllAlgorithmsByName(newseq)
        for name, existingAlgs in algsByName.iteritems():
            startingIndex = 0
            if name not in self._algorithms:
                firstAlg, parent, idx = existingAlgs[0]
                self._algorithms[name] = firstAlg
                startingIndex = 1
            for alg, parent, idx in existingAlgs[startingIndex:]:
                deduplicateComponent(self._algorithms[name], alg)
                parent.overwriteChild(idx, self._algorithms[name])

        checkSequenceConsistency(self._sequence)
        return newseq

    def moveSequence(self, sequence, destination ):
        """ moves sequence from one sub-sequence to another, primary use case HLT Control Flow """
        seq = findSubSequence(self._sequence, sequence )
        if seq is None:
            raise ConfigurationError("Can not find sequence to move %s " % sequence )

        owner = findOwningSequence(self._sequence, sequence)
        if owner is None:
            raise ConfigurationError("Can not find the sequence owning the %s " % sequence )

        dest = findSubSequence(self._sequence, destination )
        if dest is None:
            raise ConfigurationError("Can not find destination sequence %s to move to " % destination )

        owner.remove( seq )
        dest += seq
        checkSequenceConsistency(self._sequence)
        return seq


    def getSequence(self,sequenceName=None):
        if sequenceName is None:
            return self._sequence
        else:
            return findSubSequence(self._sequence,sequenceName)

    def setPrivateTools(self,privTool):
        """Use this method to carry private AlgTool(s) to the caller when returning this ComponentAccumulator. 
        The method accepts either a single private AlgTool or a list of private AlgTools (typically assigned to ToolHandleArray)
        """
        if self._privateTools is not None:
            raise ConfigurationError("This ComponentAccumulator holds already a (list of) private tool. Only one (list of)  private tool(s) is allowed")

        if isinstance(privTool,collections.Sequence):
            for t in privTool:
                if not isinstance(t,ConfigurableAlgTool):
                    raise  ConfigurationError("ComponentAccumulator.setPrivateTools accepts only ConfigurableAlgTools or lists of ConfigurableAlgTools. Encountered %s in a list" % type(t))
        else: 
            if not isinstance(privTool,ConfigurableAlgTool):
                raise  ConfigurationError("ComponentAccumulator.setPrivateTools accepts only cCnfigurableAlgTools or lists of ConfigurableAlgTools. Encountered %s " % type(privTool))
                
        self._privateTools=privTool
        return
        
    def popPrivateTools(self):
        """Get the (list of) private AlgTools from this ComponentAccumulator. 
        The CA will not keep any reference to the AlgTool.
        """
        tool=self._privateTools
        self._privateTools=None
        return tool
        
    def popToolsAndMerge(self, other):
        """ Merging in the other accumulator and getting the (list of) private AlgTools from this CompoentAccumulator.
        """
        if other is None:
            raise RuntimeError("merge called on object of type None: did you forget to return a CA from a config function?")
        tool = other.popPrivateTools()
        self.merge(other)
        return tool

    def addEventAlgo(self, algorithms,sequenceName=None,primary=False):
        if not isinstance(algorithms,collections.Sequence):
            #Swallow both single algorithms as well as lists or tuples of algorithms
            algorithms=[algorithms,]

        if sequenceName is None:
            seq=self._sequence
        else:
            seq = findSubSequence(self._sequence, sequenceName )
        if seq is None:
            self.printConfig()
            raise ConfigurationError("Can not find sequence %s" % sequenceName )

        for algo in algorithms:
            if not isinstance(algo, ConfigurableAlgorithm):
                raise TypeError("Attempt to add wrong type: %s as event algorithm" % type( algo ).__name__)
            if algo.name() in self._algorithms:
                deduplicateComponent(self._algorithms[algo.name()], algo)
            else:
                self._algorithms[algo.name()] = algo
            existingAlgInDest = findAlgorithm(seq, algo.name())
            if not existingAlgInDest:
                seq += self._algorithms[algo.name()]

        if primary:
            if len(algorithms)>1:
                self._msg.warning("Called addEvenAlgo with a list of algorithms and primary==True. Designating the first algorithm as primary component")
            if self._primaryComp: 
                self._msg.warning("Overwriting primary component of this CA. Was %s/%s, now %s/%s",
                                  self._primaryComp.getType(), self._primaryComp.getName(), algorithms[0].getType(), algorithms[0].getName())
            #keep a ref of the algorithm as primary component
            self._primaryComp=algorithms[0]
        return None

    def getEventAlgo(self,name=None):
        if name not in self._algorithms:
            raise ConfigurationError("Can not find an algorithm of name %s "% name)
        return self._algorithms[name]

    def getEventAlgos(self,seqName=None):
        if seqName is None:
            seq=self._sequence
        else:
            seq = findSubSequence(self._sequence, seqName )
        return list( set( sum( flatSequencers( seq, algsCollection=self._algorithms ).values(), []) ) )

    def addCondAlgo(self,algo,primary=False):
        if not isinstance(algo, ConfigurableAlgorithm):
            raise TypeError("Attempt to add wrong type: %s as conditions algorithm" % type( algo ).__name__)
            pass
        deduplicate(algo,self._conditionsAlgs) #will raise on conflict
        if primary: 
            if self._primaryComp: 
                self._msg.warning("Overwriting primary component of this CA. Was %s/%s, now %s/%s",
                                  self._primaryComp.getType(),self._primaryComp.getName(),algo.getType(),algo.getName())
            #keep a ref of the de-duplicated conditions algorithm as primary component
            self._primaryComp=self.__getOne( self._conditionsAlgs, algo.getName(), "ConditionsAlgos") 
        return algo


    def getCondAlgo(self,name):
        hits=[a for a in self._conditionsAlgs if a.getName()==name]
        if (len(hits)>1):
            raise ConfigurationError("More than one conditions algorithm with name %s found" % name)
        return hits[0]

    def addService(self,newSvc,primary=False):
        if not isinstance(newSvc,ConfigurableService):
            raise TypeError("Attempt to add wrong type: %s as service" % type( newSvc ).__name__)
            pass
        deduplicate(newSvc,self._services)  #will raise on conflict
        if primary: 
            if self._primaryComp: 
                self._msg.warning("Overwriting primary component of this CA. Was %s/%s, now %s/%s",
                                  self._primaryComp.getType(),self._primaryComp.getName(),newSvc.getType(),newSvc.getName())
            #keep a ref of the de-duplicated public tool as primary component
            self._primaryComp=self.__getOne( self._services, newSvc.getName(), "Services") 
        return 


    def addPublicTool(self,newTool,primary=False):
        if not isinstance(newTool,ConfigurableAlgTool):
            raise TypeError("Attempt to add wrong type: %s as AlgTool" % type( newTool ).__name__)
        if newTool.getParent() != "ToolSvc":
            newTool.setParent("ToolSvc")
        deduplicate(newTool,self._publicTools)
        if primary: 
            if self._primaryComp: 
                self._msg.warning("Overwriting primary component of this CA. Was %s/%s, now %s/%s",
                                  self._primaryComp.getType(),self._primaryComp.getName(),newTool.getType(),newTool.getName())
            #keep a ref of the de-duplicated service as primary component
            self._primaryComp=self.__getOne( self._publicTools, newTool.getName(), "Public Tool") 
        return


    def getPrimary(self):
        if self._primaryComp:
            return self._primaryComp
        else:
            return self.popPrivateTools()

    def __call__(self):
        return self.getPrimary()

    def __getOne(self, allcomps, name=None, typename="???"):
        selcomps = allcomps if name is None else [ t for t in allcomps if t.getName() == name ]
        if len( selcomps ) == 1:
            return selcomps[0]            
        raise ConfigurationError("Number of %s available %d which is != 1 expected by this API" % (typename, len(selcomps)) )
        
    def getPublicTools(self):
        return self._publicTools

    def getPublicTool(self, name=None):        
        """Returns single public tool, exception if either not found or to many found"""
        return self.__getOne( self._publicTools, name, "PublicTools")

    def getServices(self):
        return self._services

    def getService(self, name=None):        
        """Returns single service, exception if either not found or to many found"""
        if name is None:
            return self._primarySvc
        else:
            return self.__getOne( self._services, name, "Services")
    
    def addEventInput(self,condObj):
        #That's a string, should do some sanity checks on formatting
        self._eventInput.add(condObj)
        pass



    def addOutputToStream(self,streamName,outputs):
        if streamName in self._outputsPerStream:
            self._outputsPerStream[streamName].update(set(outputs))
        else:
            self._outputsPerStream[streamName]=set(outputs)

        pass


    def setAppProperty(self,key,value,overwrite=False):
        if (overwrite or key not in (self._theAppProps)):
            self._theAppProps[key]=value
        else:
            if isinstance(self._theAppProps[key],collections.Sequence) and not isinstance(self._theAppProps[key],str):
                value=unifySet(self._theAppProps[key],value)
                self._msg.info("ApplicationMgr property '%s' already set to '%s'. Overwriting with %s", key, self._theAppProps[key], value)
                self._theAppProps[key]=value
            else:
                raise DeduplicationFailed("AppMgr property %s set twice: %s and %s",key,(self._theAppProps[key],value))


        pass

    def merge(self,other, sequenceName=None):
        """Merging in the other accumulator"""
        if other is None:
            raise RuntimeError("merge called on object of type None: did you forget to return a CA from a config function?")

        if not isinstance(other,ComponentAccumulator):
            raise TypeError("Attempt merge wrong type %s. Only instances of ComponentAccumulator can be added" % type(other).__name__)

        if (other._privateTools is not None):
            if isinstance(other._privateTools,ConfigurableAlgTool):
                raise RuntimeError("merge called with a ComponentAccumulator a dangling private tool %s/%s",
                                   other._privateTools.getType(),other._privateTools.getName())
            else:
                raise RuntimeError("merge called with a ComponentAccumulator a dangling (array of) private tools")
        
        if not other._isMergable:
            raise ConfigurationError("Attempted to merge the accumulator that was unsafely manipulated (likely with foreach_component, ...)")

        if not Configurable.configurableRun3Behavior:
            raise ConfigurationError("discoverd Configurable.configurableRun3Behavior=False while working woth ComponentAccumulator")

        #destSubSeq = findSubSequence(self._sequence, sequence)
        #if destSubSeq == None:
        #    raise ConfigurationError( "Nonexistent sequence %s in %s (or its sub-sequences)" % ( sequence, self._sequence.name() ) )          #
        def mergeSequences( dest, src ):
            for childIdx, c in enumerate(src.getChildren()):
                if isSequence( c ):
                    sub = findSubSequence( dest, c.name() ) #depth=1 ???
                    if sub:
                        mergeSequences(sub, c )
                    else:
                        self._msg.debug("  Merging sequence %s to a sequence %s", c.name(), dest.name() )
                        algorithmsByName = findAllAlgorithmsByName(c)
                        for name, existingAlgs in algorithmsByName.iteritems():
                            startingIndex = 0
                            if name not in self._algorithms:
                                firstAlg, parent, idx = existingAlgs[0]
                                self._algorithms[name] = firstAlg
                                startingIndex = 1
                            for alg, parent, idx in existingAlgs[startingIndex:]:
                                deduplicateComponent(self._algorithms[name], alg)
                                parent.overwriteChild(idx, self._algorithms[name])
                        dest += c

                else: # an algorithm
                    if c.name() in self._algorithms:
                        deduplicateComponent(self._algorithms[c.name()], c)
                        src.overwriteChild(childIdx, self._algorithms[c.name()])
                    else:
                        self._algorithms[c.name()] = c

                    existingAlgInDest = findAlgorithm( dest, c.name(), depth=1 )
                    if not existingAlgInDest:
                        self._msg.debug("Adding algorithm %s to a sequence %s", c.name(), dest.name() )
                        dest += c

            checkSequenceConsistency(self._sequence)

        #Merge sequences:
        #if (self._sequence.getName()==other._sequence.getName()):
        if sequenceName:
            destSeq = self.getSequence( sequenceName )
        else:
            destSeq=findSubSequence(self._sequence,other._sequence.name()) or self._sequence
        mergeSequences(destSeq,other._sequence)

        # Additional checking and updating other accumulator's algorithms list
        for name, alg in other._algorithms.iteritems():
            if name not in self._algorithms:
                raise ConfigurationError('Error in merging. Algorithm {} missing in destination accumulator'.format(name))
            other._algorithms[name] = self._algorithms[name]

        #self._conditionsAlgs+=other._conditionsAlgs
        for condAlg in other._conditionsAlgs:
            self.addCondAlgo(condAlg) #Profit from deduplicaton here

        for svc in other._services:
            self.addService(svc) #Profit from deduplicaton here

        for pt in other._publicTools:
            self.addPublicTool(pt) #Profit from deduplicaton here

        for k in other._outputPerStream.keys():
            if k in self._outputPerStream:
                self._outputPerStream[k].update(other._outputPerStream[k])
            else: #New stream type
                self._outputPerStream[k]=other._outputPerStream[k]

        #Merge AppMgr properties:
        for (k,v) in other._theAppProps.iteritems():
            self.setAppProperty(k,v)  #Will warn about overrides
            pass
        other._wasMerged=True


    def appendToGlobals(self):

        #Cache old configurable behavior
        oldstat=Configurable.configurableRun3Behavior

        #Turn configurable behavior to old-style (eg return pre-existing instances instead of new'ing them)
        Configurable.configurableRun3Behavior=0
        from AthenaCommon.AppMgr import ToolSvc, ServiceMgr, theApp

        for s in self._services:
            deduplicate(s,ServiceMgr)

            if s.getJobOptName() in _servicesToCreate \
                    and s.getJobOptName() not in theApp.CreateSvc:
                theApp.CreateSvc.append(s.getJobOptName())



        for t in self._publicTools:
            deduplicate(t,ToolSvc)

        condseq=AthSequencer ("AthCondSeq")
        for c in self._conditionsAlgs:
            deduplicate(c,condseq)

        for seqName, algoList in flatSequencers( self._sequence ).iteritems():
            seq=AthSequencer(seqName)
            for alg in algoList:
                seq+=alg


        for (k,v) in self._theAppProps.iteritems():
            if k not in [ 'CreateSvc', 'ExtSvc']:
                setattr(theApp,k,v)

        #Re-instante previous configurable behavior
        Configurable.configurableRun3Behavior=oldstat
        return


    def appendConfigurable(self,confElem):
        name=confElem.getJobOptName() # to be FIXED
        # if self._jocat.has_key(name):
        #     self._msg.info("Not adding duplicate configurable: %s" % name )
        #     return

        #Hack for public Alg tools, drop multiple mentions of ToolSvc
        nTS=0
        for n in name.split("."):
            if n=="ToolSvc":
                nTS+=1
            else:
                break
        if nTS>2:
            name=".".join(name.split(".")[nTS-1:])
        for k, v in confElem.getValuedProperties().items():
            if isinstance(v,Configurable):
                self._jocat[name][k]=v.getFullName()
            elif isinstance(v,GaudiHandles.GaudiHandleArray):
                self._jocat[name][k]=str([ v1.getFullName() for v1 in v ])
            else:
                if name not in self._jocat:
                    self._jocat[name] = {}
                self._jocat[name][k]=str(v)

        #print "All Children:",confElem.getAllChildren()
        for ch in confElem.getAllChildren():
            self.appendConfigurable(ch)
        return


    def store(self,outfile,nEvents=10,useBootStrapFile=True,threaded=False):
        from AthenaCommon.Utils.unixtools import find_datafile
        import pickle
        self._isMergable = True # this is CA we store, it is ok it is not mergable
        if useBootStrapFile:
            import glob
            # first load basics from the bootstrap-pickle
            # a better solution to be discussed
            # prefer local file
            localbs = glob.glob("bootstrap.pkl")
            if len( localbs ) == 0:
                # if local bootstrap is missing, use one from the release
                bsfilename=find_datafile("bootstrap.pkl")
            else:
                bsfilename = "./"+localbs[0]

            bsfile=open(bsfilename)
            self._jocat=pickle.load(bsfile)
            self._jocfg=pickle.load(bsfile)
            self._pycomps=pickle.load(bsfile)
            bsfile.close()
  
        else:
            from AthenaConfiguration.MainServicesConfig import MainServicesThreadedCfg, MainServicesSerialCfg
            if threaded:
              from AthenaConfiguration.AllConfigFlags import ConfigFlags
              flags = ConfigFlags.clone()
              flags.Concurrency.NumThreads = 1
              flags.Concurrency.NumConcurrentEvents = 1
              basecfg = MainServicesThreadedCfg(flags)
              basecfg.printConfig()
              basecfg.merge(self)
              self = basecfg
              self.printConfig()
            else: #Serial
              basecfg = MainServicesSerialCfg()
              basecfg.merge(self)
              self = basecfg
            self._jocat={}
            self._jocfg={}
            self._pycomps={}
            self._jocfg["ApplicationMgr"]={}
            self._jocfg["ApplicationMgr"]["ExtSvc"] = "['ToolSvc/ToolSvc', \
                                                        'AuditorSvc/AuditorSvc', \
                                                        'MessageSvc/MessageSvc', \
                                                        'IncidentSvc/IncidentSvc',\
                                                        'EvtPersistencySvc/EventPersistencySvc',\
                                                        'HistogramSvc/HistogramDataSvc',\
                                                        'NTupleSvc/NTupleSvc',\
                                                        'RndmGenSvc/RndmGenSvc',\
                                                        'ChronoStatSvc/ChronoStatSvc',\
                                                        'StatusCodeSvc/StatusCodeSvc',\
                                                        'StoreGateSvc/StoreGateSvc',\
                                                        'StoreGateSvc/DetectorStore',\
                                                        'StoreGateSvc/HistoryStore',\
                                                        'ClassIDSvc/ClassIDSvc',\
                                                        'AthDictLoaderSvc/AthDictLoaderSvc',\
                                                        'AthenaSealSvc/AthenaSealSvc',\
                                                        'CoreDumpSvc/CoreDumpSvc',\
                                                        'JobOptionsSvc/JobOptionsSvc']"

            #Code seems to be wrong here
            for seqName, algoList in flatSequencers( self._sequence, algsCollection=self._algorithms ).iteritems():
                self._jocat[seqName] = {}
                for alg in algoList:
                  self._jocat[alg.name()] = {}
            for k, v in self._sequence.getValuedProperties().items():
                self._jocat[self._sequence.getName()][k]=str(v)

        #EventAlgorithms
        for seqName, algoList  in flatSequencers( self._sequence, algsCollection=self._algorithms ).iteritems():
            evtalgseq=[]
            for alg in algoList:
                self.appendConfigurable( alg )
                evtalgseq.append( alg.getFullName() )

        for seqName, algoList  in flatSequencers( self._sequence, algsCollection=self._algorithms ).iteritems():
            # part of the sequence may come from the bootstrap, we need to retain the content, that is done here
            for prop in self._jocat[seqName]:
                if prop == "Members":
                    mergedSequence = ast.literal_eval(self._jocat[seqName]["Members"]) +  [alg.getFullName() for alg in algoList]
                    self._jocat[seqName]["Members"] = str( mergedSequence )


        #Conditions Algorithms:
        condalgseq=[]
        for alg in self._conditionsAlgs:
            self.appendConfigurable(alg)
            condalgseq.append(alg.getFullName())
        self._jocat["AthCondSeq"]["Members"]=str(condalgseq)


        #Public Tools:
        for pt in self._publicTools:
            #print "Appending public Tool",pt.getFullName(),pt.getJobOptName()
            self.appendConfigurable(pt)



        #Hack for now:
        self._jocfg["ApplicationMgr"]["CreateSvc"]=['ToolSvc/ToolSvc', 'AthDictLoaderSvc/AthDictLoaderSvc', 'AthenaSealSvc/AthenaSealSvc', 'CoreDumpSvc/CoreDumpSvc']

        svcList=ast.literal_eval(self._jocfg["ApplicationMgr"]["ExtSvc"])

        for svc in self._services:
            if svc.getJobOptName() in _servicesToCreate:
                self._jocfg["ApplicationMgr"]["CreateSvc"].append( svc.getFullName() )

            svcList.append(svc.getFullName())
            #for k, v in svc.getValuedProperties().items():
            #    self._jocat[svcname][k]=str(v)
            self.appendConfigurable(svc)
        self._jocfg["ApplicationMgr"]["ExtSvc"]=str(svcList)

        self._jocfg["ApplicationMgr"]["EvtMax"]=nEvents


        for (k,v) in self._theAppProps.iteritems():
            if k not in [ 'CreateSvc', 'ExtSvc']:
                self._jocfg["ApplicationMgr"][k]=v

        #from pprint import pprint
        #pprint (self._jocat)

        pickle.dump( self._jocat, outfile )
        pickle.dump( self._jocfg, outfile )
        pickle.dump( self._pycomps, outfile )
        self._wasMerged=True

    def wasMerged(self):
        """ Declares CA as merged

        This is temporarily needed by HLT and should not be used elsewhere
        """
        self._wasMerged=True

    def createApp(self,OutputLevel=3):
        self._wasMerged=True
        from Gaudi.Main import BootstrapHelper
        bsh=BootstrapHelper()
        app=bsh.createApplicationMgr()

        for (k,v) in self._theAppProps.iteritems():
            app.setProperty(k,str(v))

        #Assemble createSvc property:
        svcToCreate=[]
        extSvc=[]
        for svc in self._services:
            extSvc+=[svc.getFullName(),]
            if svc.getJobOptName() in _servicesToCreate:
                svcToCreate+=[svc.getFullName(),]

        #print self._services
        #print extSvc
        #print svcToCreate
        app.setProperty("ExtSvc",str(extSvc))
        app.setProperty("CreateSvc",str(svcToCreate))

        app.configure()

        msp=app.getService("MessageSvc")
        bsh.setProperty(msp,"OutputLevel",str(OutputLevel))
        #Feed the jobO service with the remaining options
        jos=app.getService("JobOptionsSvc")

        def addCompToJos(comp):
            name=comp.getJobOptName()
            for k, v in comp.getValuedProperties().items():
                if isinstance(v,Configurable):
                    self._msg.debug("Adding "+name+"."+k+" = "+v.getFullName())
                    bsh.addPropertyToCatalogue(jos,name,k,v.getFullName())
                    addCompToJos(v)
                elif isinstance(v,GaudiHandles.GaudiHandleArray):
                    bsh.addPropertyToCatalogue(jos,name,k,str([ v1.getFullName() for v1 in v ]))
                else:
                    if not isSequence(comp) and k!="Members": #This property his handled separatly
                        self._msg.debug("Adding "+name+"."+k+" = "+str(v))
                        bsh.addPropertyToCatalogue(jos,name,k,str(v))
                    pass
                pass
            for ch in comp.getAllChildren():
                addCompToJos(ch)
            return

        #Add services
        for svc in self._services:
            addCompToJos(svc)
            pass

        #Add tree of algorithm sequences:
        for seqName, algoList in flatSequencers( self._sequence, algsCollection=self._algorithms ).iteritems():
            self._msg.debug("Members of %s : %s" % (seqName,str([alg.getFullName() for alg in algoList])))
            bsh.addPropertyToCatalogue(jos,seqName,"Members",str( [alg.getFullName() for alg in algoList]))
            for alg in algoList:
                addCompToJos(alg)
                pass
            pass


        condalgseq=[]
        for alg in self._conditionsAlgs:
            addCompToJos(alg)
            condalgseq.append(alg.getFullName())
            bsh.addPropertyToCatalogue(jos,"AthCondSeq","Members",str(condalgseq))
            pass


        #Public Tools:
        for pt in self._publicTools:
            addCompToJos(pt)
            pass

        return app


    def run(self,maxEvents=None,OutputLevel=3):
        app = self.createApp (OutputLevel)

        #Determine maxEvents
        if maxEvents is None:
            if "EvtMax" in self._theAppProps:
                maxEvents=self._theAppProps["EvtMax"]
            else:
                maxEvents=-1

        self._msg.info("INITIALIZE STEP")
        sc = app.initialize()
        if not sc.isSuccess():
            self._msg.error("Failed to initialize AppMgr")
            return sc

        app.printAlgsSequences() #could be removed later ....

        sc = app.start()
        if not sc.isSuccess():
            self._msg.error("Failed to start AppMgr")
            return sc

        sc = app.run(maxEvents)
        if not sc.isSuccess():
            self._msg.error("Failure running application")
            return sc

        app.stop().ignore()

        app.finalize().ignore()

        sc1 = app.terminate()
        return sc1

    def foreach_component(self, path):
        """ Utility to set properties of components using wildcards
        
        Example:
        forcomps(ca, "*/HLTTop/*/*Hypo*").OutputLevel=VERBOSE
        
        The compoments name & locations in the CF tree are translated into the unix like path. 
        Components of matching path are taken under consideration in setting the property.
        If the property is set succesfully an INFO message is printed. Else, a warning is printed.      
        
        The convention for path of nested components is as follows:
        Sequencer - only the name is used in the path
        Algorithm - full name - type/instance_name (aka full name) is used
        PrivateTools - the name of the property + the type/instance_name are added
        PublicTools - are located under ToolSvc/ and type/instance_name is used
        Services - located under SvcMgr/ and type/instance_name is used
        """
        from AthenaConfiguration.PropSetterProxy import PropSetterProxy
        self._isMergable=False
        return PropSetterProxy(self, path)



def CAtoGlobalWrapper(cfgmethod,flags):
    Configurable.configurableRun3Behavior+=1
    result=cfgmethod(flags)
    Configurable.configurableRun3Behavior-=1

    result.appendToGlobals()
    return

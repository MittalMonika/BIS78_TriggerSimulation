# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaCommon.Configurable import Configurable,ConfigurableService,ConfigurableAlgorithm,ConfigurableAlgTool
from AthenaCommon.CFElements import isSequence,findSubSequence,findAlgorithm,flatSequencers,findOwningSequence
from AthenaCommon.AlgSequence import AthSequencer

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
import GaudiKernel.GaudiHandles as GaudiHandles
from GaudiKernel.GaudiHandles import PublicToolHandle, PublicToolHandleArray, ServiceHandle, PrivateToolHandle
import ast
import collections

from UnifyProperties import unifyProperty, unifySet


class DeduplicationFailed(RuntimeError):
    pass

class ConfigurationError(RuntimeError):
    pass 

_servicesToCreate=frozenset(('GeoModelSvc','TileInfoLoader'))

class ComponentAccumulator(object): 

    def __init__(self,sequenceName='AthAlgSeq'):        
        self._msg=logging.getLogger('ComponentAccumulator')
        self._sequence=AthSequencer(sequenceName)    #(Nested) sequence of event processing algorithms per sequence + their private tools 
        self._conditionsAlgs=[]          #Unordered list of conditions algorithms + their private tools 
        self._services=[]                #List of service, not yet sure if the order matters here in the MT age
        self._eventInputs=set()          #List of items (as strings) to be read from the input (required at least for BS-reading).
        self._outputPerStream={}         #Dictionary of {streamName,set(items)}, all as strings


        self._theAppProps=dict()        #Properties of the ApplicationMgr

        #Backward compatiblity hack: Allow also public tools:
        self._publicTools=[]

        #To check if this accumulator was merged:
        self._wasMerged=False


    def empty(self):
        return len(self._sequence)+len(self._conditionsAlgs)+len(self._services)+\
            len(self._publicTools)+len(self._outputPerStream)+len(self._theAppProps) == 0 

    def __del__(self):
        if not self._wasMerged and not self.empty():
            #raise ConfigurationError("This componentAccumulator was never merged!")
            log = logging.getLogger("ComponentAccumulator")
            log.error("The ComponentAccumulator listed below was never merged!")
            self.printConfig()
        pass
    


    def printConfig(self, withDetails=False):
        self._msg.info( "Event Inputs" )
        self._msg.info( self._eventInputs )
        self._msg.info( "Event Algorithm Sequences" )
        if withDetails:
            self._msg.info( self._sequence )     
        else:
            def printSeqAndAlgs(seq, nestLevel = 0):
                def __prop(name):
                    if name in seq.getValuedProperties():
                        return seq.getValuedProperties()[name]                    
                    return seq.getDefaultProperties()[name]

                self._msg.info( " "*nestLevel +"\__ "+ seq.name() +" (seq: %s %s)" %(  "SEQ" if __prop("Sequential") else "PAR", "OR" if __prop("ModeOR") else "AND"  ) )
                nestLevel += 3
                for c in seq.getChildren():
                    if isSequence(c):
                        printSeqAndAlgs(c, nestLevel )
                    else:
                        self._msg.info( " "*nestLevel +"\__ "+ c.name() +" (alg)" )
            printSeqAndAlgs(self._sequence) 

        self._msg.info( "Condition Algorithms" )
        self._msg.info( [ a.getName() for a in self._conditionsAlgs ] )
        self._msg.info( "Services" )
        self._msg.info( [ s.getName() for s in self._services ] )
        self._msg.info( "Outputs" )
        self._msg.info( self._outputPerStream )


    def addSequence(self, newseq, parentName = None ):
        """ Adds new sequence. If second argument is present then it is added under another sequence  """
        if parentName is None:
            parent=self._sequence
        else:
            parent = findSubSequence(self._sequence, parentName )
            if parent is None:
                raise ConfigurationError("Missing sequence %s to add new sequence to" % parentName )

        if findSubSequence( parent, newseq.name() ):
            raise ConfigurationError("Sequence %s already present" % newseq.name() )
        parent += newseq
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
        return seq


    def getSequence(self,sequenceName=None): 
        if sequenceName is None:
            return self._sequence
        else:
            return findSubSequence(self._sequence,sequenceName)

        

    def addEventAlgo(self, algorithms,sequenceName=None):
        if not isinstance(algorithms,collections.Sequence): 
            #Swallow both single algorithms as well as lists or tuples of algorithms
            algorithms=[algorithms,]

        if sequenceName is None:
            seq=self._sequence
        else:
            seq = findSubSequence(self._sequence, sequenceName )            
        if seq is None:
            raise ConfigurationError("Can not find sequence %s" % sequenceName )


        for algo in algorithms:
            if not isinstance(algo, ConfigurableAlgorithm):
                raise TypeError("Attempt to add wrong type: %s as event algorithm" % type( algo ).__name__)
             
            seq+=algo #TODO: Deduplication necessary?
            pass
        return None


    def getEventAlgo(self,name,seqName=None):
        if seqName is None:
            seq=self._sequence
        else:
            seq = findSubSequence(self._sequence, seqName )
            
        algo = findAlgorithm( seq, name )
        if algo is None:            
            raise ConfigurationError("Can not find an algorithm of name %s "% name)
        return algo


    def addCondAlgo(self,algo):
        if not isinstance(algo, ConfigurableAlgorithm):
            raise TypeError("Attempt to add wrong type: %s as conditions algorithm" % type( algo ).__name__)
            pass
        self._deduplicate(algo,self._conditionsAlgs) #will raise on conflict
        return algo
        

    def getCondAlgo(self,name):
        hits=[a for a in self._conditionsAlgs if a.getName()==name]
        if (len(hits)>1):
            raise ConfigurationError("More than one conditions algorithm with name %s found" % name)
        return hits[0]

    def addService(self,newSvc):
        if not isinstance(newSvc,ConfigurableService):
            raise TypeError("Attempt to add wrong type: %s as service" % type( newSvc ).__name__)
            pass
        self._deduplicate(newSvc,self._services)  #will raise on conflict
        return newSvc


    def addPublicTool(self,newTool):
        if not isinstance(newTool,ConfigurableAlgTool):
            raise TypeError("Attempt to add wrong type: %s as AlgTool" % type( newTool ).__name__)
        if newTool.getParent() != "ToolSvc":
            newTool.setParent("ToolSvc")
        self._deduplicate(newTool,self._publicTools)
        return


        


    def _deduplicate(self,newComp,compList):
        #Check for duplicates:
        for comp in compList:
            if comp.getType()==newComp.getType() and comp.getFullName()==newComp.getFullName():
                #Found component of the same type and name
                if isinstance(comp,PublicToolHandle) or isinstance(comp,ServiceHandle):
                    continue # For public tools/services we check only their full name because they are already de-duplicated in addPublicTool/addSerivce
                self._deduplicateComponent(newComp,comp)
                #We found a service of the same type and name and could reconcile the two instances
                self._msg.debug("Reconciled configuration of component %s", comp.getJobOptName())
                return False #False means nothing got added
            #end if same name & type
        #end loop over existing components

        #No component of the same type & name found, simply append 
        self._msg.debug("Adding component %s to the job", newComp.getFullName())

        #The following is to work with internal list of service as well as gobal svcMgr as second parameter
        try:
            compList.append(newComp)
        except:
            compList+=newComp
            pass
        return True #True means something got added
    


    def _deduplicateComponent(self,newComp,comp):
        #print "Checking ", comp, comp.getType(), comp.getJobOptName()
        allProps=frozenset(comp.getValuedProperties().keys()+newComp.getValuedProperties().keys())
        for prop in allProps:
            if not prop.startswith('_'):
                try:
                    oldprop=getattr(comp,prop)
                except AttributeError:
                    oldprop=None
                try:
                    newprop=getattr(newComp,prop)
                except AttributeError:
                    newprop=None

                if type(oldprop) != type(newprop):
                    raise DeduplicationFailed(" '%s' defined multiple times with conflicting types %s and %s" % \
                                                      (comp.getJobOptName(),type(oldprop),type(newprop)))
                
                #Note that getattr for a list property works, even if it's not in ValuedProperties
                if (oldprop!=newprop):
                    #found property mismatch
                    if isinstance(oldprop,PublicToolHandle) or isinstance(oldprop,ServiceHandle): 
                        if oldprop.getFullName()==newprop.getFullName():
                            # For public tools/services we check only their full name because they are already de-duplicated in addPublicTool/addSerivce
                            continue
                        else:
                            raise DeduplicationFailed("PublicToolHandle / ServiceHandle '%s.%s' defined multiple times with conflicting values %s and %s" % \
                                                              (comp.getJobOptName(),oldprop.getFullName(),newprop.getFullName()))
                    elif isinstance(oldprop,PublicToolHandleArray):
                            for newtool in newprop:
                                if newtool not in oldprop: 
                                    oldprop+=[newtool,]
                            continue
                    elif isinstance(oldprop,ConfigurableAlgTool):
                        self._deduplicateComponent(oldprop,newprop)
                        pass
                    elif isinstance(oldprop,GaudiHandles.GaudiHandleArray):
                        for newTool in newprop:
                            self._deduplicate(newTool,oldprop)
                        pass
                    elif isinstance(oldprop,list): #if properties are mergeable, do that!
                        propid="%s.%s" % (comp.getType(),str(prop))
                        #Try merging this property. Will raise on failure
                        mergeprop=unifyProperty(propid,oldprop,newprop)
                        setattr(comp,prop,mergeprop)
                    elif isinstance(oldprop,dict): #Dicts/maps can be unified
                        #find conflicting keys 
                        doubleKeys= set(oldprop.keys()) & set(prop.keys())
                        for k in doubleKeys():
                            if oldprop[k]!= prop[k]:
                                raise DeduplicationFailed("Map-property '%s.%s' defined multiple times with conflicting values for key %s" % \
                                                              (comp.getJobOptName(),str(prop),k))
                            pass
                        mergeprop=oldprop
                        mergeprop.update(prop)
                        setattr(comp,prop,mergeprop)
                    elif isinstance(oldprop,PrivateToolHandle):
                        # This is because we get a PTH if the Property is set to None, and for some reason the equality doesn't work as expected here.
                        continue
                    else:
                        #self._msg.error("component '%s' defined multiple times with mismatching configuration", svcs[i].getJobOptName())
                        raise DeduplicationFailed("component '%s' defined multiple times with mismatching property %s" % \
                                                      (comp.getJobOptName(),str(prop)))
                pass 
                #end if prop-mismatch
            pass
        #end if startswith("_")
        pass
      
    
    def getService(self,name):
        for svc in self._services: 
            if svc.getName()==name:
                return svc
        raise KeyError("No service with name %s known" % name)

    def getPublicTool(self,name):
        for pt in self._publicTools:
            if pt.getName()==name:
                return pt
        raise KeyError("No public tool with name %s known" % name)


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
        if (overwrite):
            self._theAppProps[key]=value
        else:
            if key in self._theAppProps and isinstance(self._theAppProps[key],collections.Sequence):
                value=unifySet(self._theAppProps[key],value) 
                self._msg.info("ApplicationMgr property '%s' already set to '%s'. Overwriting with %s", key, self._theAppProps[key], value)
            self._theAppProps[key]=value
        pass


    def mergeAll(self,others):
        if isinstance(others,ComponentAccumulator):
            return self.merge(others)
        
        elif isinstance(others,collections.Sequence):
            for other in others:
                if isinstance (other,ComponentAccumulator):
                    self.merge(other)
                elif isinstance (other,ConfigurableService):
                    self.addService(other)
                elif isinstance(other,ConfigurableAlgorithm):
                    self.addAlgorithm(other)
                elif isinstance(other,ConfigurableAlgTool):
                    self.addPublicTool(other)
                else:
                    raise RuntimeError("mergeAll called with unexpected parameter of type %s" % type(other))

        else:
            raise RuntimeError("mergeAll called with unexpected parameter")

    def merge(self,other):
        """ Merging in the other accumulator """
        if other is None: 
            return

        if isinstance(other,collections.Sequence): #Check if we got more than one argument
            if len(other)==0: 
                raise ConfigurationError("Merge called with empty sequence as argument")
            for par in other[1:]:
                if par is not None: #possible improvment: Check type of par and try to merge if applicable (service, public tool)
                    self._msg.warning("Merge called with a sequence of potentially un-merged components")
                    raise RuntimeError()
            other=other[0]

        if not isinstance(other,ComponentAccumulator):
            raise TypeError("Attempt merge wrong type %s. Only instances of ComponentAccumulator can be added" % type(other).__name__)
                
        
        if not Configurable.configurableRun3Behavior:
            raise ConfigurationError("discoverd Configurable.configurableRun3Behavior=False while working woth ComponentAccumulator")

        #destSubSeq = findSubSequence(self._sequence, sequence)
        #if destSubSeq == None:
        #    raise ConfigurationError( "Nonexistent sequence %s in %s (or its sub-sequences)" % ( sequence, self._sequence.name() ) )          #     
        def mergeSequences( dest, src ):        
            for c in src.getChildren():
                if isSequence( c ):
                    sub = findSubSequence( dest, c.name() ) #depth=1 ???
                    if sub:
                        mergeSequences(sub, c )
                    else:
                        self._msg.debug("  Merging sequence %s to a sequence %s", c.name(), dest.name() )
                        dest += c
                else: # an algorithm
                    existingAlg = findAlgorithm( dest, c.name(), depth=1 )
                    if existingAlg:
                        if existingAlg != c: # if it is the same we can just skip it, else this indicates an error
                            raise ConfigurationError( "Duplicate algorithm %s in source and destination sequences %s" % ( c.name(), src.name()  ) )           
                    else: # absent, adding
                        self._msg.debug("  Merging algorithm %s to a sequence %s", c.name(), dest.name() )
                        dest += c
                        

        #Merge sequences:
        #if (self._sequence.getName()==other._sequence.getName()):
        destTopSeq=findSubSequence(self._sequence,other._sequence.name()) or self._sequence
        mergeSequences(destTopSeq,other._sequence)
            


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
            self._deduplicate(s,ServiceMgr)
            
            if s.getJobOptName() in _servicesToCreate \
                    and s.getJobOptName() not in theApp.CreateSvc:
                theApp.CreateSvc.append(s.getJobOptName())

        

        for t in self._publicTools:
            self._deduplicate(t,ToolSvc)
   
        condseq=AthSequencer ("AthCondSeq")
        for c in self._conditionsAlgs:
            self._deduplicate(c,condseq)

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
                #print name,k,self._jocat[name][k]
            else:
                self._jocat[name][k]=str(v)
        #print "All Children:",confElem.getAllChildren()
        for ch in confElem.getAllChildren():
            self.appendConfigurable(ch)
        return


    def store(self,outfile,nEvents=10):
        from AthenaCommon.Utils.unixtools import find_datafile
        import pickle
        import glob
        # first load basics from the bootstrap-pickle
        # a better solution to be discussed
        # prefer local file 
        localbs = glob.glob("bootstrap.pkl")
        if len( localbs ) == 0:
            # if local bootstrap is missing, use one from the release
            bsfilename=find_datafile("bootstrap.pkl")
        else:
            bsfilename = localbs[0]

        bsfile=open(bsfilename)
        self._jocat=pickle.load(bsfile)
        self._jocfg=pickle.load(bsfile)
        self._pycomps=pickle.load(bsfile)
        bsfile.close()


        #EventAlgorithms
        for seqName, algoList  in flatSequencers( self._sequence ).iteritems():
            evtalgseq=[]
            for alg in algoList:
                self.appendConfigurable( alg )
                evtalgseq.append( alg.getFullName() )


        for seqName, algoList  in flatSequencers( self._sequence ).iteritems():
            self._jocat[seqName]["Members"]=str( [alg.getFullName() for alg in algoList] )

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


    def run(self,maxEvents=None):
        log = logging.getLogger("ComponentAccumulator")
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
            extSvc+=svc.getFullName()
            if svc.getJobOptName() in _servicesToCreate:
                svcToCreate+=svc.getFullName()
    
        app.setProperty("ExtSvc",str(extSvc))
        app.setProperty("CreateSvc",str(svcToCreate))
    
        app.configure()

        #Feed the jobO service with the remaining options
        jos=app.getService("JobOptionsSvc")
    
        def addCompToJos(comp):
            name=comp.getJobOptName()
            for k, v in comp.getValuedProperties().items():
                if isinstance(v,Configurable):
                    log.debug("Adding "+name+"."+k+" = "+v.getFullName())
                    bsh.addPropertyToCatalogue(jos,name,k,v.getFullName())
                    addCompToJos(v)
                else:
                    if not isSequence(comp) and k!="Members": #This property his handled separatly
                        log.debug("Adding"+name+"."+k+" = "+str(v))
                        bsh.addPropertyToCatalogue(jos,name,k,str(v))
                    pass
                pass
            for ch in comp.getAllChildren():
                addCompToJos(ch)
            return


        #Add tree of algorithm sequences: 
        for seqName, algoList in flatSequencers( self._sequence ).iteritems():
            log.debug("Members of %s : %s" % (seqName,str([alg.getFullName() for alg in algoList])))
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

    
        for svc in self._services:
            addCompToJos(svc)
            pass
     #Public Tools:
        for pt in self._publicTools:
            addCompToJos(pt)
            pass

        #Determine maxEvents
        if maxEvents is None:
            if "EvtMax" in self._theAppProps:
                maxEvents=self._theAppProps["EvtMax"]
            else:
                maxEvents=-1



        sc = app.initialize()
        if not sc.isSuccess(): 
            log.error("Failed to initialize AppMgr")
            return sc
            
        app.printAlgsSequences() #could be removed later ....

        sc = app.start()
        if not sc.isSuccess(): 
            log.error("Failed to start AppMgr")
            return sc

        sc = app.run(maxEvents)
        if not sc.isSuccess(): 
            log.error("Failure running application")
            return sc

        app.stop().ignore()
        
        app.finalize().ignore()

        sc1 = app.terminate()
        return sc1



def CAtoGlobalWrapper(cfgmethod,flags):
     Configurable.configurableRun3Behavior+=1
     result=cfgmethod(flags)
     Configurable.configurableRun3Behavior-=1

     result.appendToGlobals()
     return



# self test            
import unittest

class TestComponentAccumulator( unittest.TestCase ):
    def setUp(self):
    
        Configurable.configurableRun3Behavior+=1
        # trivial case without any nested sequences
        from AthenaCommon.Configurable import ConfigurablePyAlgorithm # guinea pig algorithms
        from AthenaCommon.CFElements import seqAND, parOR
        from AthenaCommon.Logging import log
        from AthenaCommon.Constants import DEBUG

        log.setLevel(DEBUG)
        
        dummyCfgFlags=AthConfigFlags()
        dummyCfgFlags.lock()
        
        class Algo(ConfigurablePyAlgorithm):
            def __init__(self, name):
                super( ConfigurablePyAlgorithm, self ).__init__( name )

        def AlgsConf1(flags):
            acc = ComponentAccumulator()
            a1=Algo("Algo1")
            a2=Algo("Algo2")
            return acc,[a1,a2]


        def AlgsConf2(flags):
            acc = ComponentAccumulator()
            result,algs=AlgsConf1( flags )
            acc.merge(result)
            algs.append(Algo("Algo3"))
            return acc,algs

        acc = ComponentAccumulator()
    
        # top level algs
        acc1,algs=AlgsConf2(dummyCfgFlags)
        acc.merge(acc1)
        acc.addEventAlgo(algs)

        def AlgsConf3(flags):
            acc = ComponentAccumulator()
            na1=Algo("NestedAlgo1") 
            return acc,na1

        def AlgsConf4(flags):
            acc,na1= AlgsConf3( flags )
            NestedAlgo2 = Algo("NestedAlgo2")
            NestedAlgo2.OutputLevel=7
            return acc,na1,NestedAlgo2

        acc.addSequence( seqAND("Nest") )
        acc.addSequence( seqAND("subSequence1"), parentName="Nest" )
        acc.addSequence( parOR("subSequence2"), parentName="Nest" )

        acc.addSequence( seqAND("sub2Sequence1"), parentName="subSequence1")
        acc.addSequence( seqAND("sub3Sequence1"), parentName="subSequence1")
        acc.addSequence( seqAND("sub4Sequence1"), parentName="subSequence1")

        accNA1=AlgsConf4(dummyCfgFlags)
        acc.merge(accNA1[0])
        acc.addEventAlgo(accNA1[1:],"sub2Sequence1" )
        acc.store(open("testFile.pkl", "w"))
        self.acc = acc 


    def test_algorihmsAreAdded( self ):
        self.assertEqual( findAlgorithm( self.acc.getSequence(), "Algo1", 1).name(), "Algo1", "Algorithm not added to a top sequence" )
        self.assertEqual( findAlgorithm( self.acc.getSequence(), "Algo2", 1).name(),  "Algo2", "Algorithm not added to a top sequence" )
        self.assertEqual( findAlgorithm( self.acc.getSequence(), "Algo3", 1).name(), "Algo3", "Algorithm not added to a top sequence" )

    def test_sequencesAreAdded( self ):    
        self.assertIsNotNone( self.acc.getSequence("subSequence1" ), "Adding sub-sequence failed" )
        self.assertIsNotNone( self.acc.getSequence("subSequence2" ), "Adding sub-sequence failed" )
        self.assertIsNotNone( self.acc.getSequence("sub2Sequence1"), "Adding sub-sequence failed" )
        self.assertIsNotNone( findSubSequence( self.acc.getSequence("subSequence1"), "sub2Sequence1"), "Adding sub-sequence done in a wrong place" )

    def test_algorithmsInNestedSequences( self ):
        self.assertIsNotNone( findAlgorithm( self.acc.getSequence(), "NestedAlgo1" ), "Algorithm added to nested sequence" )
        self.assertIsNotNone( findAlgorithm( self.acc.getSequence(), "NestedAlgo1", 1 ) is None, "Algorithm mistakenly in top sequence" )
        self.assertIsNotNone( findAlgorithm( findSubSequence( self.acc.getSequence(), "sub2Sequence1"), "NestedAlgo1", 1 ), "Algorithm not in right sequence" )


    def test_readBackConfiguration( self ):
        import pickle
        with open( "testFile.pkl" ) as f:
            s = pickle.load( f )
            self.assertIsNotNone( s, "The pickle has no content")

class TestHLTCF( unittest.TestCase ):
    def runTest( self ):
        # replicate HLT issue, it occured because the sequnces were recorded in the order of storing in the dict and thus the 
        # some of them (in this case hltSteps) did not have properties recorded 
        from AthenaCommon.CFElements import seqAND, seqOR, parOR
        from AthenaCommon.Configurable import ConfigurablePyAlgorithm # guinea pig algorithms
        Configurable.configurableRun3Behavior=1
        acc = ComponentAccumulator()
        acc.addSequence( seqOR("hltTop") )
        algos2 = ConfigurablePyAlgorithm( "RecoAlgInTop" )
        acc.addEventAlgo( algos2, sequenceName="hltTop" ) # some algo
        acc.addSequence( seqAND("hltSteps"), parentName="hltTop" )
        acc.addSequence( parOR("hltStep_1"), parentName="hltSteps" )
        acc.addSequence( seqAND("L2CaloEgammaSeq"), "hltStep_1" )
        acc.addSequence( parOR("hltStep_2"), parentName="hltSteps" )
        acc.moveSequence( "L2CaloEgammaSeq", "hltStep_2" )
        
        acc.store(open("testFile2.pkl", "w"))
        import pickle
        f = open("testFile2.pkl")
        s = pickle.load(f)
        f.close()
        self.assertNotEqual( s['hltSteps']['Members'], '[]', "Empty set of members in hltSteps, Sequences recording order metters" )


class MultipleParrentsInSequences( unittest.TestCase ):
    def runTest( self ):
       # test if an algorithm (or sequence) can be controlled by more than one sequence
        Configurable.configurableRun3Behavior=1
        from AthenaCommon.CFElements import seqAND
        from AthenaCommon.Configurable import ConfigurablePyAlgorithm # guinea pig algorithms
        accTop = ComponentAccumulator()
                
        recoSeq = seqAND("seqReco")
        recoAlg = ConfigurablePyAlgorithm( "recoAlg" )
        recoSeq += recoAlg
        
        acc1 = ComponentAccumulator()
        acc1.addSequence( seqAND("seq1") )
        acc1.addSequence( recoSeq, parentName="seq1" )
        
        acc2 = ComponentAccumulator()
        acc2.addSequence( seqAND("seq2") )
        acc2.addSequence( recoSeq, parentName="seq2" )
        
        accTop.merge( acc1 )
        accTop.merge( acc2 )
                
        accTop.printConfig()

        self.assertIsNotNone( findAlgorithm( accTop.getSequence( "seq1" ), "recoAlg" ), "Algorithm missing in the first sequence" )
        self.assertIsNotNone( findAlgorithm( accTop.getSequence( "seq2" ), "recoAlg" ), "Algorithm missing in the second sequence" )
        s = accTop.getSequence( "seqReco" )
        self.assertEqual( len( s.getChildren() ), 1, "Wrong number of algorithms in reco seq: %d " % len( s.getChildren() ) )
        self.assertIs( findAlgorithm( accTop.getSequence( "seq1" ), "recoAlg" ), findAlgorithm( accTop.getSequence( "seq2" ), "recoAlg" ), "Algorithms are cloned" )
        self.assertIs( findAlgorithm( accTop.getSequence( "seq1" ), "recoAlg" ), recoAlg, "Clone of the original inserted in sequence" )

        accTop.store( open("dummy.pkl", "w") )
        import pickle
        # check if the recording did not harm the sequences
        with open("dummy.pkl") as f:
            s = pickle.load( f )
            self.assertEquals( s['seq1']["Members"], "['AthSequencer/seqReco']", "After pickling recoSeq missing in seq1 " + s['seq1']["Members"])
            self.assertEquals( s['seq2']["Members"], "['AthSequencer/seqReco']", "After pickling recoSeq missing in seq2 " + s['seq2']["Members"])
            self.assertEquals( s['seqReco']["Members"], "['ConfigurablePyAlgorithm/recoAlg']", "After pickling seqReco is corrupt " + s['seqReco']["Members"] )

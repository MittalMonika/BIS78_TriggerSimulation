# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration


#from AthenaCommon.Logging import logging
from AthenaConfiguration.CfgLogMsg import cfgLogMsg
from AthenaCommon.Configurable import Configurable,ConfigurableService,ConfigurableAlgorithm,ConfigurableAlgTool
from AthenaCommon.CFElements import isSequence,findSubSequence,findAlgorithm,flatSequencers
from AthenaCommon.AlgSequence import AlgSequence

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
import GaudiKernel.GaudiHandles as GaudiHandles
import ast
from collections import defaultdict
from copy import deepcopy


class DeduplicatonFailed(RuntimeError):
    pass

class ConfigurationError(RuntimeError):
    pass 

class CurrentSequence:
    sequence = AlgSequence("AthAlgSeq")

    @staticmethod
    def set( newseq ):
        #print "CurrentSequence set.... ", newseq.name()
        CurrentSequence.sequence = newseq

    @staticmethod
    def get(  ):
        #print "CurrentSequence ....get %s " %  CurrentSequence.sequence.name()
        return CurrentSequence.sequence


class ComponentAccumulator(object): 

    def __init__(self):
        self._msg=cfgLogMsg  #logging.getLogger('ComponentAccumulator')
        self._sequence=CurrentSequence.get() # sequence of algorithms
        self._eventAlgs={}     #Unordered list of event processing algorithms per sequence + their private tools 
        self._conditionsAlgs=[]          #Unordered list of conditions algorithms + their private tools 
        self._services=[]                #List of service, not yet sure if the order matters here in the MT age
        self._conditionsInput=set()      #List of database folder (as string), eventually passed to IOVDbSvc
        self._eventInputs=set()          #List of items (as strings) to be read from the input (required at least for BS-reading).
        self._outputPerStream={}        #Dictionary of {streamName,set(items)}, all as strings


        self._theAppProps=dict()        #Properties of the ApplicationMgr

        self._privateTools=[]           #Private tools are not merged! Belong to parent algo

        #Backward compatiblity hack: Allow also public tools:
        self._publicTools=[]


    def printConfig(self, withDetails=False):
        self._msg.info( "Event Inputs" )
        self._msg.info( self._eventInputs )
        self._msg.info( "Event Algorithm Sequences" )
        if withDetails:
            self._msg.info( self._sequence )     
        else:
            def printSeqAndAlgs(seq, nestLevel = 0):
                def __prop(name):
                    if seq.getValuedProperties().has_key(name):
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

        self._msg.info( "Conditions Inputs" )
        self._msg.info( self._conditionsInput )
        self._msg.info( "Condition Algorithms" )
        self._msg.info( [ a.getName() for a in self._conditionsAlgs ] )
        self._msg.info( "Services" )
        self._msg.info( [ s.getName() for s in self._services ] )
        self._msg.info( "Outputs" )
        self._msg.info( self._outputPerStream )


    def addSequence(self, newseq, sequence = None ):
        """ Adds new sequence. If second argument is present then it is added under another sequence  """
        seq =  CurrentSequence.get()
        if sequence:
            seq = findSubSequence(seq, sequence )
        if seq == None:
            raise ConfigurationError("Missing sequence %s to add new sequence to" % sequence )

        if findSubSequence( self._sequence, newseq.name() ):
            raise ConfigurationError("Sequence %s already present" % newseq.name() )
        seq += newseq
        return newseq 

    def addEventAlgo(self, algo,sequence=None):                
        if not isinstance(algo, ConfigurableAlgorithm):
            raise TypeError("Attempt to add wrong type: %s as event algorithm" % type( algo ).__name__)
            pass        
        seq = CurrentSequence.get()
        if sequence != None:
            seq = findSubSequence( seq, sequence )
            if seq == None:
                raise ConfigurationError("Unable to add %s to sequence %s as it is missing" % ( algo.getFullName(), seq.name()) )

        self._msg.debug("Adding %s to sequence %s" % ( algo.getFullName(), seq.name()) )
        seq += algo
        return algo


    def getEventAlgo(self,name):
        """ Looks for an algorithm given the name in current sequence and in nested scopes 
        
        NB. Not sure that this is what we expect. Maybe we want to start from the top always? 
        Limiting to the current scope reduces risk of cross talk. Will see in real life and make adjustments.
        """
        algo = findAlgorithm( CurrentSequence.get(), name )
        if algo == None:            
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
            raise ConfigurationError("More than one Algorithm with name %s found in sequence %s" %(name,sequence))
        return hits[0]

    def addService(self,newSvc):
        if not isinstance(newSvc,ConfigurableService):
            raise TypeError("Attempt to add wrong type: %s as service" % type( newSvc ).__name__)
            pass
        self._deduplicate(newSvc,self._services)  #will raise on conflict
        return newSvc

    def addAlgTool(self,newTool):
        if not isinstance(newTool,ConfigurableAlgTool):
            raise TypeError("Attempt to add wrong type as AlgTool")
        self._privateTools.append(newTool)
        return newTool


    def clearAlgTools(self):
        self._privateTools=[]


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
            if comp.getType()==newComp.getType() and comp.getJobOptName()==newComp.getJobOptName():
                #Found component of the same type and name
                #print "Checking ", comp.getType(), comp.getJobOptName()
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

                        #Note that getattr for a list property works, even if it's not in ValuedProperties
                        if (oldprop!=newprop):
                            #found property mismatch
                            if isinstance(oldprop,list): #if properties are concatinable, do that!
                                oldprop+=newprop
                                #print "concatenating list-property",comp.getJobOptname(),prop
                                setattr(comp,prop,oldprop)
                            else:
                                #self._msg.error("component '%s' defined multiple times with mismatching configuration" % svcs[i].getJobOptName())
                                raise DeduplicationFailed("component '%s' defined multiple times with mismatching property %s" % \
                                                                  comp.getJobOptName(),prop)
                            pass 
                            #end if prop-mismatch
                        pass
                        #end if startswith("_")
                    pass
                    #end loop over properties
                #We found a service of the same type and name and could reconcile the two instances
                self._msg.debug("Reconciled configuration of component %s" % comp.getJobOptName())
                return False #False means nothing got added
            #end if same name & type
        #end loop over existing components

        #No component of the same type & name found, simply append 
        self._msg.debug("Adding service/Tool/CondAlog %s to the job" % newComp.getFullName())
        compList.append(newComp)
        return True #True means something got added
    
    def getService(self,name):
        for svc in self._services: 
            if svc.getName()==name:
                return svc
        raise KeyError("No service with name %s known" % name)

    def getAlgTools(self):
        return self._privateTools
    
    def getAlgTool(self,name):
        for tool in self._privateTools:
            if tool.getName()==name:
                return tool
            raise KeyError("No AlgTool with name %s known" % name)


    def getPublicTool(self,name):
        for pt in self._publicTools:
            if pt.getName()==name:
                return pt
        raise KeyError("No public tool with name %s known" % name)


    def addConditionsInput(self,condObj):
        #That's a string, should do some sanity checks on formatting
        self._conditionsInput.add(condObj);
        pass

    def addEventInput(self,condObj):
        #That's a string, should do some sanity checks on formatting
        self._eventInput.add(condObj);
        pass



    def addOutputToStream(self,streamName,outputs):
        
        if hasattr(outputs,'__iter__'):
            toAdd=list(outputs)
        else:
            toAdd=[outputs,]

        if streamName in self._outputsPerStream:
            self._outputsPerStream[streamName].update(set(outputs))
        else:
            self._outputsPerStream[streamName]=set(outputs)
            
        pass


    def setAppProperty(self,key,value):
        if self._theAppProps.has_key(key) and  self._theAppProps[key]!=value:
            #Not sure if we should allow that ...
            self._msg.info("ApplicationMgr property '%s' already set to '%s'. Overwriting with %s"% (key,str(self._theAppProps[key]),str(value)))
        self._theAppProps[key]=value
        pass

    def __merge(self,other):
        """ Merging in the other accumulator """
        if not isinstance(other,ComponentAccumulator):
            raise TypeError("Attempt merge wrong type %s. Only instances of ComponentAccumulator can be added" % type(other).__name__)
                
        
        #destSubSeq = findSubSequence(self._sequence, sequence)
        #if destSubSeq == None:
        #    raise ConfigurationError( "Nonexistent sequence %s in %s (or its sub-sequences)" % ( sequence, self._sequence.name() ) )          #     
        def mergeSequences( dest, src ):        
            for c in src.getChildren():
                if isSequence( c ):
                    sub = findSubSequence( dest, c.name() )
                    if sub:
                        mergeSequences(sub, c )
                    else:
                        self._msg.debug("  Merging sequence %s to a sequnece %s" % ( c.name(), dest.name() ) )          
                        dest += c
                else: # an algorithm
                    existingAlg = findAlgorithm( dest, c.name(), depth=1 )
                    if existingAlg:
                        if existingAlg != c: # if it is the same we can just skip it, else this indicates an error
                            raise ConfigurationError( "Duplicate algorithm %s in source and destination sequences %s" % ( c.name(), src.name()  ) )           
                    else: # absent, adding
                        self._msg.debug("  Merging algorithm %s to a sequnece %s" % ( c.name(), dest.name() ) )          
                        dest += c
                        


        # Merge Conditions inputs
        self._conditionsInput|=other._conditionsInput
        
        #self._conditionsAlgs+=other._conditionsAlgs
        for condAlg in other._conditionsAlgs:
            self.addCondAlgo(condAlgo) #Profit from deduplicaton here


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

    
    def addConfig(self,fct,configFlags,*args,**kwargs):
        """ The heart and soul of configuration system. You need to read the whole documentation. 

        This method eliminates possibility that a downstream configuration alters the upstream one. 
        It is done by a two-fold measures:
        - the sub-accumulators can not access the upstream accumulators and thus alter any configuration.
          The combination process is defined in the __merge method of this class. Less flexibility == robustness.
        """

        #Safety check: Verify that the ConfigFlags are indeed locked:
        if not isinstance(configFlags,AthConfigFlags):
            raise ConfigurationError("Expected an instance of AthConfigFlags as parameter")

        if not configFlags.locked():
            raise ConfigurationError("You are trying to create a job using unlocked configFlags!")

        
        currentSeq = seq = CurrentSequence.get()
        if kwargs.has_key('sequence'):            
            seq = findSubSequence(seq, kwargs['sequence'] )            
            if seq == None:
                raise ConfigurationError("Can not add algorithms to sequence %s as it does not exist" % kwargs['sequence'] )            
            else:
                del kwargs['sequence']
            CurrentSequence.set( seq )

        self._msg.info("Executing configuration function %s" % fct.__name__)
        retval=fct(configFlags,*args,**kwargs)
        CurrentSequence.set( currentSeq )

        self.__merge(retval)
        
        #Get private tools if there are any
        #Those are not merged and not passed through the call-chain,
        #instead we overwrite whatever was there before
        #The client is supposed to grab the private tools and attach 
        #them to their parent component
        self._privateTools=retval._privateTools
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
        from collections import defaultdict
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
        def __addif( name ):
            for svc in self._services:
                if name == svc.getJobOptName():                
                    self._jocfg["ApplicationMgr"]["CreateSvc"].append( svc.getFullName() )
        __addif('DetDescrCnvSvc')
        __addif('GeoModelSvc')
        __addif('TileInfoLoader')

        
        for svc in self._services:
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

# self test            
if __name__ == "__main__":
    # trivial case without any nested sequences
    from AthenaCommon.Configurable import ConfigurablePyAlgorithm # guinea pig algorithms
    from AthenaCommon.CFElements import *
    cfgLogMsg.setLevel("debug")

    dummyCfgFlags=AthConfigFlags()
    dummyCfgFlags.lock()

    class Algo(ConfigurablePyAlgorithm):
        def __init__(self, name):
            super( ConfigurablePyAlgorithm, self ).__init__( name )

    def AlgsConf1(flags):
        acc = ComponentAccumulator()
        acc.addEventAlgo( Algo("Algo1")  )
        acc.addEventAlgo( Algo("Algo2")  )
        return acc

    def AlgsConf2(flags):
        acc = ComponentAccumulator()
        acc.addConfig( AlgsConf1, flags )
        acc.addEventAlgo( Algo("Algo3") )
        return acc

    acc = ComponentAccumulator()
    
    # top level algs
    acc.addConfig( AlgsConf2,dummyCfgFlags )
    # checks
    assert findAlgorithm(AlgSequence("AthAlgSeq"), "Algo1", 1), "Algorithm not added to a top sequence"
    assert findAlgorithm(AlgSequence("AthAlgSeq"), "Algo2", 1), "Algorithm not added to a top sequence"
    assert findAlgorithm(AlgSequence("AthAlgSeq"), "Algo3", 1), "Algorithm not added to a top sequence"
    print( "Simple Configuration construction OK ")

    def AlgsConf3(flags):
        acc = ComponentAccumulator()
        acc.addEventAlgo( Algo("NestedAlgo1") )
        return acc

    def AlgsConf4(flags):
        acc = ComponentAccumulator()
        acc.addConfig( AlgsConf3,flags )
        NestedAlgo2 = Algo("NestedAlgo2")
        NestedAlgo2.OutputLevel=7
        acc.addEventAlgo( NestedAlgo2 )
        return acc

    acc.addSequence( seqAND("Nest") )
    acc.addSequence( seqAND("subSequence1"), sequence="Nest" )
    acc.addSequence( parOR("subSequence2"), sequence="Nest" )

    assert findSubSequence( AlgSequence("AthAlgSeq"), "subSequence1" ), "Adding sub-sequence failed"
    assert findSubSequence( AlgSequence("AthAlgSeq"), "subSequence2" ), "Adding sub-sequence failed"

    acc.addSequence( seqAND("sub2Sequence1"), "subSequence1")
    acc.addSequence( seqAND("sub3Sequence1"), "subSequence1")
    acc.addSequence( seqAND("sub4Sequence1"), "subSequence1")
    assert findSubSequence(AlgSequence("AthAlgSeq"), "sub2Sequence1"), "Adding sub-sequence failed"
    assert findSubSequence( findSubSequence(AlgSequence("AthAlgSeq"), "subSequence1"), "sub2Sequence1" ), "Adding sub-sequence done in a wrong place"

    acc.addConfig( AlgsConf4, dummyCfgFlags, sequence="sub2Sequence1" )    
    assert findAlgorithm(AlgSequence("AthAlgSeq"), "NestedAlgo1" ), "Algorithm added to nested sequence"
    assert findAlgorithm(AlgSequence("AthAlgSeq"), "NestedAlgo1", 1 ) == None, "Algorithm mistakenly in top sequence"
    assert findAlgorithm( findSubSequence(AlgSequence("AthAlgSeq"), "sub2Sequence1"), "NestedAlgo1", 1 ), "Algorithm not in right sequence"
    print( "Complex sequences construction also OK ")

    #acc.printConfig(True)
    acc.printConfig()

    # try recording
    acc.store(open("testFile.pkl", "w"))
    f = open("testFile.pkl")
    import pickle

    # replicate HLT issue, it occured because the sequnces were recorded in the order of storing in the dict and thus the 
    # some of them (in this case hltSteps) did not have properties recorded 
    acc = ComponentAccumulator()
    acc.addSequence( seqOR("hltTop") )
    acc.addConfig( AlgsConf2, dummyCfgFlags, sequence="hltTop" ) # some algo
    acc.addSequence( seqAND("hltSteps"), sequence="hltTop" )
    acc.addSequence( parOR("hltStep_1"), sequence="hltSteps" )
    acc.addSequence( seqAND("L2CaloEgammaSeq"), "hltStep_1" )
    acc.addSequence( parOR("hltStep_2"), sequence="hltSteps" )
    acc.printConfig()
    
    acc.store(open("testFile2.pkl", "w"))
    f = open("testFile2.pkl")
    s = pickle.load(f)
    f.close()
    assert s['hltSteps']['Members'] != '[]', "Empty set of members in hltSteps, Sequences recording order metters"
    
    print( "\nAll OK" )


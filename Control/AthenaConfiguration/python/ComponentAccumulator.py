# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaCommon.Configurable import Configurable,ConfigurableService,ConfigurableAlgorithm,ConfigurableAlgTool
from AthenaCommon.CFElements import isSequence,findSubSequence,findAlgorithm,flatSequencers,findOwningSequence,checkSequenceConsistency
from AthenaCommon.AlgSequence import AthSequencer

import GaudiKernel.GaudiHandles as GaudiHandles
from GaudiKernel.GaudiHandles import PublicToolHandle, PublicToolHandleArray, ServiceHandle, PrivateToolHandle, PrivateToolHandleArray
import ast
import collections

from UnifyProperties import unifyProperty, unifySet, matchProperty


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

        # small CAs can have only tools
        self._privateTools=[]

        #To check if this accumulator was merged:
        self._wasMerged=False


    def empty(self):
        return len(self._sequence)+len(self._conditionsAlgs)+len(self._services)+\
            len(self._publicTools)+len(self._outputPerStream)+len(self._theAppProps) == 0

    def __del__(self):
        if not self._wasMerged and not self.empty():
            raise RuntimeError("ComponentAccumulator was not merged!")
            #log = logging.getLogger("ComponentAccumulator")
            #log.error("The ComponentAccumulator listed below was never merged!")
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
                if isinstance(propval,PublicToolHandleArray):
                    ths = [th.getFullName() for th in propval]
                    propstr = "PublicToolHandleArray([ {0} ])".format(', '.join(ths))
                elif isinstance(propval,PrivateToolHandleArray):
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

                self._msg.info( " "*nestLevel +"\\__ "+ seq.name() +" (seq: %s %s)" %(  "SEQ" if __prop("Sequential") else "PAR", "OR" if __prop("ModeOR") else "AND"  ) )
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
        if parentName is None:
            parent=self._sequence
        else:
            parent = findSubSequence(self._sequence, parentName )
            if parent is None:
                raise ConfigurationError("Missing sequence %s to add new sequence to" % parentName )

        parent += newseq
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

            existingAlg = findAlgorithm(seq, algo.getName())
            if existingAlg:
                self._deduplicateComponent(algo, existingAlg)
            else:
                seq+=algo #TODO: Deduplication necessary?
            pass
        return None


    def getEventAlgo(self,name=None,seqName=None):
        if name is None:
            algs = self.getEventAlgos(seqName)
            if len(algs) == 1:
                return algs[0]
            raise ConfigurationError("Number of algorithms returned by getEventAlgo %d which is != 1 expected by this API" % len(algs) )
                
        if seqName is None:
            seq=self._sequence
        else:
            seq = findSubSequence(self._sequence, seqName )
        

        algo = findAlgorithm( seq, name )
        if algo is None:
            raise ConfigurationError("Can not find an algorithm of name %s "% name)
        return algo

    def getEventAlgos(self,seqName=None):
        if seqName is None:
            seq=self._sequence
        else:
            seq = findSubSequence(self._sequence, seqName )
        return list( set( sum( flatSequencers( seq ).values(), []) ) )

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
        except Exception:
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
                # both are defined but with distinct type
                if type(oldprop) != type(newprop):
                    raise DeduplicationFailed("Property  '%s' of component '%s' defined multiple times with conflicting types %s and %s" % \
                                                  (prop,comp.getJobOptName(),type(oldprop),type(newprop)))

                propid = "%s.%s" % (comp.getType(), str(prop))

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

                        if matchProperty(propid):
                            mergeprop = unifyProperty(propid, oldprop, newprop)
                            setattr(comp, prop, mergeprop)
                            continue

                        for newTool in newprop:
                            self._deduplicate(newTool,oldprop)
                        pass
                    elif isinstance(oldprop,list): #if properties are mergeable, do that!
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

    def __getOne(self, allcomps, name=None, typename="???"):
        selcomps = allcomps if name is None else [ t for t in allcomps if t.getName() == name ]
        if len( selcomps ) == 1:
            return selcomps[0]            
        raise ConfigurationError("Number of %s available %d which is != 1 expected by this API" % (typename, len(selcomps)) )
        
    def getPublicTools(self):
        return self._publicTools

    def getPublicTool(self, name=None):        
        """ Returns single public tool, exception if either not found or to many found"""
        return self.__getOne( self._publicTools, name, "PublicTools")

    def getServices(self):
        return self._services

    def getService(self, name=None):        
        """ Returns single service, exception if either not found or to many found"""
        return self.__getOne( self._services, name, "Services")
    
    def addPrivateTool(self, newTool):
        if not isinstance(newTool,ConfigurableAlgTool):
            raise TypeError("Attempt to add wrong type: %s as private AlgTool" % type( newTool ).__name__)
        self._deduplicate(newTool,self._privateTools)

    def getPrivateTools(self):
        return self._privateTools

    def getPrivateTool(self, name=None):        
        """ Returns single private tool, exception if either not found or to many found"""
        return self.__getOne( self._privateTools, name, "PrivateTools")
        

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


    def mergeAll(self,others, sequenceName=None):
        if isinstance(others,ComponentAccumulator):
            return self.merge(others)

        elif isinstance(others,collections.Sequence):
            for other in others:
                if isinstance (other,ComponentAccumulator):
                    self.merge(other)
                elif isinstance (other,ConfigurableService):
                    self.addService(other)
                elif isinstance(other,ConfigurableAlgorithm):
                    self.addEventAlgorithm(other, sequenceName=sequenceName)
                    #FIXME: At this point we can't distingush event algos from conditions algos.
                    #Might become possible with new Gaudi configurables
                elif isinstance(other,ConfigurableAlgTool):
                    self.addPublicTool(other)
                else:
                    raise RuntimeError("mergeAll called with unexpected parameter of type %s" % type(other))

        else:
            raise RuntimeError("mergeAll called with unexpected parameter")

    def merge(self,other, sequenceName=None):
        """ Merging in the other accumulator """
        if other is None:
            raise RuntimeError("merge called on object of type None: did you forget to return a CA from a config function?")

        if isinstance(other,collections.Sequence):
            self._msg.error("Merge called with a: %s "  % str(type(other)) + " of length: %d " % len(other))
            self._msg.error("where elements are of type : " + ", ".join([ str(type(x).__name__) for x in other]) )
            if len(other) > 1 and isinstance(other[0], ComponentAccumulator):
                self._msg.error("Try calling mergeAll")
                raise RuntimeError("Merge can not handle a sequence: " + ", ".join([ str(type(x).__name__) for x in other]) +", call mergeAll instead" )
            else:
                raise RuntimeError("Merge can not handle a sequence: " + ", ".join([ str(type(x).__name__) for x in other]) +"" )

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
                        if existingAlg != c:
                            self._deduplicate(c, existingAlg)
                    else: # absent, adding
                        self._msg.debug("  Merging algorithm %s to a sequence %s", c.name(), dest.name() )
                        dest += c

            checkSequenceConsistency(self._sequence)
                        

        #Merge sequences:
        #if (self._sequence.getName()==other._sequence.getName()):
        if sequenceName:
            destSeq = self.getSequence( sequenceName )
        else:
            destSeq=findSubSequence(self._sequence,other._sequence.name()) or self._sequence
        mergeSequences(destSeq,other._sequence)



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
            bsfilename = "./"+localbs[0]

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
            # part of the sequence may come from the bootstrap, we need to retain the content, that is done here
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
            print svc.getFullName()
            extSvc+=[svc.getFullName(),]
            if svc.getJobOptName() in _servicesToCreate:
                svcToCreate+=[svc.getFullName(),]

        #print self._services
        #print extSvc
        #print svcToCreate
        app.setProperty("ExtSvc",str(extSvc))
        app.setProperty("CreateSvc",str(svcToCreate))

        print "CONFIGURE STEP"
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
        for seqName, algoList in flatSequencers( self._sequence ).iteritems():
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



def CAtoGlobalWrapper(cfgmethod,flags):
     Configurable.configurableRun3Behavior+=1
     result=cfgmethod(flags)
     Configurable.configurableRun3Behavior-=1

     result.appendToGlobals()
     return


class PropSetterProxy(object):
   __compPaths = {}
   __scannedCA = None

   def __init__(self, ca, path):      
      self.__path = path
      self.__findComponents( ca )

      
   def __setattr__(self, name, value):
       if name.startswith("_PropSetterProxy"):
           return super(PropSetterProxy, self).__setattr__(name, value)
       
       msg = logging.getLogger('foreach_component')
       import fnmatch
       for component_path, component in PropSetterProxy.__compPaths.iteritems():
           if fnmatch.fnmatch( component_path, self.__path ):
               if name in component.getProperties():
                   try:
                       setattr( component, name, value )
                       msg.debug( "Set property: %s to value %s of component %s because it matched %s " % ( name, str(value), component_path, self.__path )   )
                   except Exception, ex:
                       msg.warning( "Failed to set property: %s to value %s of component %s because it matched %s, reason: %s" % ( name, str(value), component_path, self.__path, str(ex) )   )
                       pass
               else:
                   msg.warning( "No such a property: %s in component %s, tried to set it because it matched %s" % ( name, component_path, self.__path )   )


   def __findComponents(self, ca):
       if ca is not PropSetterProxy.__scannedCA:
           PropSetterProxy.__scannedCA = ca
           PropSetterProxy.__compPaths = {}
           def __add(path, comp):
               if comp.getName() == "":
                   return
               PropSetterProxy.__compPaths[ path ] = comp


           for svc in ca._services:
               PropSetterProxy.__compPaths['SvcMgr/'+svc.getFullName()] = svc
           for t in ca._publicTools:
               PropSetterProxy.__compPaths['ToolSvc/'+t.getFullName()] = t
           
           def __nestAlg(startpath, comp): # it actually dives inside the algorithms and (sub) tools               
               if comp.getName() == "":
                   return
               for name, value in comp.getProperties().iteritems():
                   if isinstance( value, ConfigurableAlgTool ) or isinstance( value, PrivateToolHandle ):
                       __add( startpath+"/"+name+"/"+value.getFullName(), value )
                       __nestAlg( startpath+"/"+name+"/"+value.getName(), value )
                   if isinstance( value, PrivateToolHandleArray):
                       for toolIndex,t in enumerate(value):
                           __add( startpath+"/"+name+"/"+t.getFullName(), t )
                           __nestAlg( startpath+"/"+name+"/"+t.getName(), value[toolIndex] )
                           
               
           def __nestSeq( startpath, seq ):
               for c in seq.getChildren():
                   if isSequence(c):
                       __nestSeq( startpath+"/"+c.getName(), c )                       
                   else: # the algorithm or tool
                       __add( startpath+"/"+c.getFullName(),  c )
                       __nestAlg( startpath+"/"+c.getFullName(), c )

           __nestSeq("", ca._sequence)
            
            
def foreach_component(componentAccumulator, path):
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
   return PropSetterProxy(componentAccumulator, path)




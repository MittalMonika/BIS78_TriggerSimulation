import sys, re, copy
from AthenaCommon.Logging import logging
from AthenaCommon.Constants import VERBOSE,INFO,DEBUG
log = logging.getLogger('MenuComponents')
log.setLevel( VERBOSE )
logLevel=DEBUG

from DecisionHandling.DecisionHandlingConf import RoRSeqFilter

class Node():
    """ base class representing one Alg + inputs + outputs, to be used to Draw dot diagrams and connect objects"""
    def __init__(self, Alg):
        self.name = ("%sNode")%( Alg.name())
        self.Alg=Alg
        self.algname = Alg.name()
        self.inputs=[]
        self.outputs=[]
        
    def addOutput(self, name):
        self.outputs.append(name)

    def addInput(self, name):
        self.inputs.append(name)

    def getOutputList(self):        
        return self.outputs

    def getInputList(self):        
        return self.inputs

    def __str__(self):
        return "Node::%s  [%s] -> [%s]"%(self.algname, ' '.join(map(str, self.getInputList())), ' '.join(map(str, self.getOutputList())))


    
class AlgNode(Node):
    """Node class that connects inputs and outputs to basic alg. properties """
    def __init__(self, Alg, inputProp, outputProp):
        Node.__init__(self, Alg)
        self.outputProp=outputProp
        self.inputProp=inputProp

    def addDefaultOutput(self):
        if self.outputProp is not '':
            self.addOutput(("%s_%s"%(self.algname,self.outputProp)))
        
    def setPar(self, prop, name):
        cval = self.Alg.getProperties()[prop]
        try:
            if type(cval) == type(list()):                
                cval.append(name)
                return setattr(self.Alg, prop, cval)
            else:
                return setattr(self.Alg, prop, name)
        except:
            pass

    def getPar(self, prop):
        try:
            return getattr(self.Alg, prop)
        except:
            return self.Alg.getDefaultProperty(prop)
        raise "Error in reading property " + prop + " from " + self.Alg

    def addOutput(self, name):
        if self.outputProp is not '':
            return self.setPar(self.outputProp,name) 
    
    def getOutputList(self):
        outputs = []
        cval = self.getPar(self.outputProp)
        if cval == '':
            return outputs
        if type(cval) == type(list()):  
            outputs.extend(cval)
        else:
            outputs.append(cval)
        return outputs
  
    def addInput(self, name):
        if self.inputProp is not '':
            return self.setPar(self.inputProp,name)
    
    def getInputList(self):
        inputs = []
        cval = self.getPar(self.inputProp)
        if cval =='':
            return inputs
        if type(cval) == type(list()):  
            inputs.extend(cval)
        else:
            inputs.append(cval)
        return inputs

    def __str__(self):
        return "Alg::%s  [%s] -> [%s]"%(self.algname, ' '.join(map(str, self.getInputList())), ' '.join(map(str, self.getOutputList())))
    

class HypoToolConf():
    """ Class to group info on hypotools"""
    def __init__(self, HypoToolClassName):
        self.className =HypoToolClassName
        self.name=''
        self.conf=''

    def setName(self, name):
        self.name=name

    def setConf(self, conf):
        self.conf=conf

        
class HypoAlgNode(AlgNode):
    """Node for HypoAlgs"""
    def __init__(self, Alg):
        assert isHypoBase(Alg), "Error in creating HypoAlgNode from Alg "  + Alg.name()
        AlgNode.__init__(self, Alg, 'HypoInputDecisions', 'HypoOutputDecisions')
        self.addDefaultOutput()
        self.tools = []
        self.previous=[]

    def addHypoTool (self, HypoToolConf):
        from TrigUpgradeTest.MenuHypoTools import createHypoTool        
        tools = self.Alg.HypoTools
        if HypoToolConf.name not in self.tools:
            ## HypoTools are private, so need to be created when added to the Alg
            self.Alg.HypoTools = tools+[createHypoTool(HypoToolConf.className, HypoToolConf.name, HypoToolConf.conf)]
        self.tools.append(HypoToolConf.name)            
        return True
           

    def setPreviousDecision(self,prev):        
        self.previous.append(prev)
        return self.addInput(prev)

    def __str__(self):
        return "HypoAlg::%s  [%s] -> [%s], previous = [%s], HypoTools=[%s]"%(self.Alg.name(),' '.join(map(str, self.getInputList())),
                                                                                 ' '.join(map(str, self.getOutputList())),
                                                                                 ' '.join(map(str, self.previous)),
                                                                                 ' '.join(map(str, self.tools)))




class SequenceFilterNode(AlgNode):
    """Node for any kind of sequence filter"""
    def __init__(self, Alg, inputProp, outputProp):
        AlgNode.__init__(self,  Alg, inputProp, outputProp)
        self.seeds = []
              
    def setChains(self, name):
        if log.isEnabledFor(logging.DEBUG):
            log.debug("Adding Chain %s to filter %s"%(name, self.name))
        return self.setPar("Chains", name)
    
    def getChains(self):
        return self.getPar("Chains")

    def addSeed(self, seed):
        self.seeds.append(seed)

    def __str__(self):
        return "SequenceFilter::%s  [%s] -> [%s], seeds=[%s], chains=%s"%(self.Alg.name(),' '.join(map(str, self.getInputList())),' '.join(map(str, self.getOutputList())), ' '.join(map(str, self.seeds)), self.getChains()) 




class RoRSequenceFilterNode(SequenceFilterNode):
    def __init__(self, name):
        Alg= RoRSeqFilter(name, OutputLevel = logLevel)
        SequenceFilterNode.__init__(self,  Alg, 'Input', 'Output')



class InputMakerNode(AlgNode):
    def __init__(self, Alg):
        assert isInputMakerBase(Alg), "Error in creating InputMakerNode from Alg "  + Alg.name()
        AlgNode.__init__(self,  Alg, 'InputMakerInputDecisions', 'InputMakerOutputDecisions')
        Alg.OutputLevel = logLevel


from DecisionHandling.DecisionHandlingConf import ComboHypo
class ComboMaker(AlgNode):
    def __init__(self, name):
        Alg = ComboHypo(name)
        if log.isEnabledFor(logging.DEBUG):
            log.debug("Making combo Alg %s"%name)
        AlgNode.__init__(self,  Alg, 'HypoInputDecisions', 'HypoOutputDecisions')
        Alg.OutputLevel = logLevel
        self.prop="MultiplicitiesMap"


    def addChain(self, chain):
        if log.isEnabledFor(logging.DEBUG):
            log.debug("ComboMaker %s adding chain %s"%(self.algname,chain))
        from MenuChains import getConfFromChainName
        confs=getConfFromChainName(chain)
        for conf in confs:
            seed=conf.replace("HLT_", "")
            integers = map(int, re.findall(r'^\d+', seed))
            multi=0
            if len(integers)== 0:
                multi=1
            elif len(integers)==1:
                multi=integers[0]
                re.sub('^\d+',"",seed) #remove the multiplicity form the string
            else:
                sys.exit("ERROR in decoding combined chain %s"%(chain))

            newdict={chain:[multi]}
            cval = self.Alg.getProperties()[self.prop]            
            if type(cval) == type(dict()):
                if chain in cval.keys():
                    cval[chain].append(multi)
                else:
                    cval[chain]=[multi]
            else:
                cval=newdict
            setattr(self.Alg, self.prop, cval)
            


#########################################################
# USEFULL TOOLS
#########################################################

def isHypoBase(alg):
    if  'HypoInputDecisions'  in alg.__class__.__dict__:
        return True
    prop = alg.__class__.__dict__.get('_properties')
    return  ('HypoInputDecisions'  in prop)

def isInputMakerBase(alg):
    return  ('InputMakerInputDecisions'  in alg.__class__.__dict__)

def isFilterAlg(alg):
    return isinstance(alg, RoRSeqFilter)
        
##########################################################
# NOW sequences and chains
##########################################################

        
class MenuSequence():
    """ Class to group reco sequences with the Hypo"""
    def __init__(self, Sequence, Maker,  Hypo, HypoToolClassName):
        from AthenaCommon.AlgSequence import AthSequencer
        self.name = "S_%s"%(Hypo.name()) # sequence name is based on hypo name
        self.sequence     = Node( Alg=Sequence)
        self.maker        = InputMakerNode( Alg=Maker)
        self.hypoToolConf = HypoToolConf(HypoToolClassName)
        self.hypo         = HypoAlgNode( Alg=Hypo)
        self.inputs=[]
        self.outputs=[]
        self.seed=''
        self.reuse = False # flag to draw dot diagrmas

    def replaceHypoForCombo(self,HypoAlg):
        if log.isEnabledFor(logging.DEBUG):
            log.debug("set new Hypo %s for combo sequence %s "%(HypoAlg.name(), self.name))
        self.hypo= HypoAlgNode( Alg=HypoAlg)

    def connectToFilter(self,sfilter):
        filter_output = sfilter.getOutputList()
        inputs=[filter_output[i] for i,fseed in enumerate (sfilter.seeds) if self.seed in fseed  ]
        if log.isEnabledFor(logging.DEBUG):
            log.debug("connectToFilter: found %d inputs to sequence::%s from Filter::%s (from seed %s)",
                          len(inputs), self.name, sfilter.algname, self.seed)
            for i in inputs: log.debug("- "+i)
        if len(inputs) == 0:
                sys.exit("ERROR, no inputs to sequence are set!")
        for inp in inputs:
            new_output = "%s_from_%s"%(self.hypo.algname, inp)
            self.hypo.addOutput(new_output)
            self.outputs.append( new_output)
        
        #### Connect the InputMaker
        for i in inputs:
            self.maker.addInput(i)
            self.inputs.append(i)
        input_maker_output=["%s_from_%s"%(self.maker.algname,i)  for i in inputs  ]
        if len(input_maker_output) == 0:
            sys.exit("ERROR, no outputs to sequence are set!")
        for out in input_maker_output:
            self.maker.addOutput(out) 
            self.hypo.setPreviousDecision(out)
        if log.isEnabledFor(logging.DEBUG):
            log.debug("connectToFilter: connecting InputMaker to HypoAlg: adding %d output to InputMaker::%s and sending to HypoAlg::%s",
                          len(inputs), self.maker.algname, self.hypo.algname)
        for i in inputs: log.debug(i)

    def __str__(self):
        return "MenuSequence::%s \n Hypo::%s \n Maker::%s \n Sequence::%s"%(self.name, self.hypo, self.maker, self.sequence)

    
class Chain:
    """Basic class to define the trigger menu """
    def __init__(self, name, Seed, ChainSteps=[]):
        self.name = name
        self.seed=Seed
        self.steps=ChainSteps         
        self.vseeds = Seed.strip().split("_")        
        self.vseeds.pop(0) #remove first L1 string
        # group_seed is used to se tthe seed type (EM, MU,JET), removing the actual threshold
        self.group_seed  = ["L1"+filter(lambda x: x.isalpha(), stri) for stri in self.vseeds]
        self.setSeedsToSequences() # save seed of each menuseq
        if log.isEnabledFor(logging.DEBUG):
            log.debug("Chain " + name + " with seeds: %s "%str( self.vseeds))

        for step in self.steps:
            if step.isCombo:
                step.combo.addChain(self.name)

        
    def setSeedsToSequences(self):
        # set the seed to the menusequences
        sequences1=self.steps[0].sequences
        tot_seq=len(sequences1)
        tot_seed=len(self.vseeds)
        if tot_seq==tot_seed:
            nseq=0
            for seed in self.vseeds:
                for step in self.steps:
                    seq=step.sequences[nseq]
                    seq.seed ="L1"+filter(lambda x: x.isalpha(), seed)
                    if log.isEnabledFor(logging.DEBUG):
                        log.debug( "Chain %s adding seed %s to sequence %d in step %s"%(self.name, seq.seed, nseq, step.name))
                nseq+=1
           
        else:
            log.error("found %d sequences in this chain and %d seeds. What to do??", tot_seq, tot_seed)
            sys.exit("ERROR, in chain configuration") 
        
    def decodeHypoToolConfs(self):
        """ This is extrapolating the hypotool configuration from the (combined) chain name"""
        from MenuChains import getConfFromChainName
        signatures = getConfFromChainName(self.name)
        for step in self.steps:
            if len(signatures) != len(step.sequences):
                log.error("Error in step %s: found %d signatures and %d sequences"%(step.name, len(signatures), len(step.sequences)))
                sys.exit("ERROR, in chain configuration")
            nseq=0
            for seq in step.sequences:
                seq.hypoToolConf.setConf(signatures[nseq])
                seq.hypoToolConf.setName(self.name)
                seq.hypo.addHypoTool(seq.hypoToolConf)
                nseq +=1
                

class CFSequence():
    """Class to describe the ChainStep + filter"""
    def __init__(self, ChainStep, FilterAlg):
        self.name = ChainStep.name   # not needed?     
        self.filter = FilterAlg
        self.step=ChainStep
        self.connect()

    def connect(self):
        """Connect filter to ChainStep (all its sequences)"""
        for seq in self.step.sequences:
            seq.connectToFilter(self.filter)

        if self.step.isCombo: self.connectCombo()

    def connectCombo(self):
        # reset sequence output, they will b ereplaced by new combo outputs
        for seq in self.step.sequences:
            seq.outputs=[]
            
        for seq in self.step.sequences:
            combo_input=seq.hypo.getOutputList()[0]
            self.step.combo.addInput(combo_input)
            log.debug("Adding inputs %s to combo %s"%(combo_input, self.step.combo.algname))
            # inputs are the output decisions of the hypos of the sequences
            # outputs are the modified name of input deciions that need to be connected to the next filter
            combo_output="combo_%s"%combo_input
            self.step.combo.addOutput(combo_output)
            seq.outputs.append(combo_output)
            log.debug("Adding outputs %s to combo %s"%(combo_output, self.step.combo.algname))
        
    
    def __str__(self):
        return "--- CFSequence %s ---\n + Filter: %s \n +  %s \n "%(self.name,\
            self.filter, self.step )
          




class ChainStep:
    """Class to describe one step of a chain; if more than one menuSequence, then the step is combo"""
    def __init__(self, name,  Sequences=[]):
        self.name = name
        self.sequences=[]
        self.isCombo=len(Sequences)>1
        self.combo=None
        if self.isCombo:            
            self.makeCombo(Sequences)
        else:
            self.sequences = Sequences


    def replaceSequence(self, old, new):
        # maybe obsolete?
        idx=self.sequences.index(old) # this raise exception
        self.sequences.pop(idx)
        self.sequences.insert(idx, new)

    def makeCombo(self, Sequences):        
        # For combo sequences, duplicate the Hypo with a Combo_name and create the comboHypoAlg
        self.combo = ComboMaker("ComboHypo_%s"%self.name)
        for sequence in Sequences:
            new_sequence=copy.deepcopy(sequence)
            new_sequence.name=("%s_Combo"%sequence.name)
            oldhypo=sequence.hypo.Alg
            new_hypoAlg=oldhypo.clone("%s_Combo"%oldhypo.name())
            new_sequence.replaceHypoForCombo(new_hypoAlg)
            self.sequences.append(new_sequence)

    def __str__(self):
        return "--- ChainStep %s ---\n + isCombo: %d \n +  %s \n "%(self.name, self.isCombo, ' '.join(map(str, self.sequences) ))


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
class InEventReco( ComponentAccumulator ):
    """ Class to handle in-event reco """
    pass


class InViewReco( ComponentAccumulator ):
    """ Class to handle in-view reco, sets up the View maker if not provided and exposes InputMaker so that more inputs to it can be added in the process of assembling the menu """
    def __init__(self, name, viewMaker=None):
        super( InViewReco, self ).__init__()
        self.name = name
        from AthenaCommon.CFElements import parOR, seqAND
        self.mainSeq = seqAND( name )
        self.addSequence( self.mainSeq )

        from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
        if viewMaker:
            self.viewMakerAlg = viewMaker 
        else:
            from AthenaCommon.Constants import DEBUG
            self.viewMakerAlg = EventViewCreatorAlgorithm(name+'ViewsMaker',
                                                          ViewFallThrough = True,
                                                          RoIsLink        = 'initialRoI', # -||-
                                                          InViewRoIs      = name+'RoIs',
                                                          Views           = name+'Views',
                                                          ViewNodeName    = name+"InView",
                                                          OutputLevel=DEBUG)

        self.addEventAlgo( self.viewMakerAlg, self.mainSeq.name() )
        self.viewsSeq = parOR( self.viewMakerAlg.ViewNodeName )        
        self.addSequence( self.viewsSeq, self.mainSeq.name() )
        
    def addInputFromFilter(self, filterAlg ):
        assert len(filterAlg.Output) == 1, "Can only oprate on filter algs with one configured output, use addInput to setup specific inputs"
        self.addInput( filterAlg.Output[0], "Reco_"+( filterAlg.Output[0].replace("Filtered_", "") ) )

    def addInput(self, inKey, outKey ):
        """Adds input (DecisionsContainer) from which the views should be created """
        self.viewMakerAlg.InputMakerInputDecisions += [ inKey ]
        self.viewMakerAlg.InputMakerOutputDecisions += [ outKey ]

    def addRecoAlg( self, alg ):
        """Reconstruction alg to be run per view"""
        self.addEventAlgo( alg, self.viewsSeq.name() )

    def sequence( self ):
        return self.mainSeq

    def inputMaker( self ):
        return self.viewMakerAlg 


class RecoFragmentsPool:
    """ Class to host all the reco fragments that need to be reused """
    fragments = {}
    @classmethod
    def retrieve( cls,  creator, flags, **kwargs ):
        """ create, or return created earlier reco fragment

        Reco fragment is uniquelly identified by the function and set og **kwargs. 
        The flags are not part of unique identifier as creation of new reco fragments should not be caused by difference in the unrelated flags.
        """
        
        from AthenaCommon.Logging import logging
        requestHash = hash( ( creator, tuple(kwargs.keys()), tuple(kwargs.values()) ) ) 
        if requestHash not in cls.fragments:
            recoFragment = creator( flags, **kwargs )
            cls.fragments[requestHash] = recoFragment
            log.debug( "created reconstruction fragment" )
            return recoFragment
        else:
            log.debug( "reconstruction fragment from the cache" )
            return cls.fragments[requestHash]

    # @classmethod
    # def mergeTo( cls, acc ):
    #     """ Adds algorithms in the reco fragments and all related services to the accumulator """
    #     for frag in cls.fragments.values():
    #         acc.merge( frag )

    

class FilterHypoSequence(  ComponentAccumulator ):    
    def __init__( self, name ):
        super( FilterHypoSequence, self ).__init__()
        from AthenaCommon.CFElements import seqAND
        self.name = name
        self.mainSeq = seqAND( name )
        self.addSequence( self.mainSeq )
        self.filterAlg =  None
        self.reco = None
        self.hypoAlg = None

    def addReco( self, reco ):
        """ Adds reconstruction sequence, has to be one of InViewReco or InEventReco objects """
        assert self.filterAlg, "The filter alg has to be setup first"
        self.reco = reco
        reco.addInputFromFilter( self.filterAlg )
        self.addSequence( reco.sequence(), self.mainSeq.name() )
        self.merge( reco )

    def addFilter( self, chains, inKey ):
        """ adds filter alg of type RoRSeqFilter, output key is predefined by the sequence name as: Filtered + the name"""
        from DecisionHandling.DecisionHandlingConf import RoRSeqFilter
        self.filterAlg = RoRSeqFilter( self.name+'Filter' )
        self.filterAlg.Input       = [ inKey ]
        self.filterAlg.Output      = [ 'Filtered_'+self.name ]
        self.filterAlg.Chains      = chains
        from AthenaCommon.Constants import DEBUG
        self.filterAlg.OutputLevel = DEBUG
        self.addEventAlgo( self.filterAlg, self.mainSeq.name() )

    def hypoDecisions( self ):
        return "Decisions_"+self.name

    def addHypo( self, hypo ):
        self.hypoAlg = hypo
        self.hypoAlg.HypoInputDecisions = self.reco.inputMaker().InputMakerOutputDecisions[-1]
        self.hypoAlg.HypoOutputDecisions = self.hypoDecisions()
        self.addEventAlgo( hypo, self.mainSeq.name() )

    def sequence( self ):
        return self.mainSeq

    def filter( self ):
        return self.filterAlg

    def check( self ):
        assert self.filterAlg, "Filter not configured"
        assert self.hypoAlg, "Hypo alg is not configured"


class HLTMenuAccumulator( ComponentAccumulator ):
    def __init__( self ):
        super( HLTMenuAccumulator, self ).__init__()
        from AthenaCommon.CFElements import seqAND
        HLTSteps =  seqAND( "HLTSteps" )
        self.addSequence( HLTSteps )

    def __getOrMakeStepSequence(self, step, isFilter = False ):
       """ Constructs sequence for the step, the filtering step or, reco step will be created depending on isFilter flags
       
       The function assures that all previous steps are aready in place i.e. HLTStep_1_filter, HLTStep_1 ... until HLTStep_N-1 are in place.
       Therefore the steps can be added in any order (not yet sure if that will but better assure robustness now)
       """
       from AthenaCommon.CFElements import parOR
       name = "HLTStep_%d%s" % (step, "_filters" if isFilter else "" )
        
       s  = self.getSequence( name )
       if s:
           return s
       # make sure that sequences for previous steps are in place
       for p in range( 1, step ):
           self.__getOrMakeStepSequence( p, isFilter = True )
           self.__getOrMakeStepSequence( p, isFilter = False )
       # make sure that filtering steps is placed before reco steps
       if not isFilter:
           self.__getOrMakeStepSequence( step, isFilter = True )
       s = parOR( name )
       self.addSequence( s, parentName="HLTSteps")
       return s


    def setupSteps( self, steps ):        
        """ The main menu function, it is responsible for crateion of step sequences and placing slice specific sequencers there.
                
        It would rely on the MenuSeq object API in two aspects, to obtain filter alg and to obtain the reco sequence
        The MenuSeq should already contain all the chains that are needed for the menu setup (chains info can be accessed via flags passed when steps were created)
        """
        for stepNo, fhSeq in enumerate( steps, 1 ):
            filterStep = self.__getOrMakeStepSequence( stepNo, isFilter = True )
            filterStep += fhSeq.filter() # FilterHypoSequence API

            recoStep = self.__getOrMakeStepSequence( stepNo, isFilter = False )            
            self.addSequence( fhSeq.sequence(), recoStep.name() )

    
    def steps( self ):
        """ returns step seqeuncers """
        return self.getSequence("HLTSteps")

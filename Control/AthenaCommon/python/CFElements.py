from AthenaCommon.AlgSequence import AthSequencer


def parOR(name, subs=[]):
    """ parallel OR sequencer """
    seq = AthSequencer( name )
    seq.ModeOR = True
    seq.Sequential = False
    seq.StopOverride = True
    for s in subs:
        seq += s
    return seq
    
def seqAND(name, subs=[]):
    """ sequential AND sequencer """
    seq = AthSequencer( name )
    seq.ModeOR = False
    seq.Sequential = True
    seq.StopOverride = False
    for s in subs:
        seq += s
    return seq

def seqOR(name, subs=[]):
    """ sequential OR sequencer, used when a barrier needs to be set but all subs reached irrespective of the decision """
    seq = AthSequencer( name )
    seq.ModeOR = True
    seq.Sequential = True
    seq.StopOverride = True
    for s in subs:
        seq += s
    return seq

def stepSeq(name, filterAlg, rest):
    """ elementary HLT step sequencer, filterAlg is gating, rest is anything that needs to happen within the step """
    stepReco = parOR(name+"_reco", rest)
    stepAnd = seqAND(name, [ filterAlg, stepReco ])
    return stepAnd

def isSequence( obj ):
    return 'AthSequence' in type( obj ).__name__
    
def findSubSequence( start, nameToLookFor ):
    """ Traverse sequences tree to find a sequence of a given name. The first one is returned. """
    if start.name() == nameToLookFor:
        return start
    for c in start.getChildren():
        #print type(c), c.name()
        if isSequence( c ):
            if  c.name() == nameToLookFor:
                return c
            found = findSubSequence( c, nameToLookFor )
            if found:
                return found
    return None

def findOwningSequence( start, nameToLookFor ):
    """ find sequence that owns the sequence nameTooLookFor"""
    for c in start.getChildren():
        if  c.name() == nameToLookFor:
            return start
        if isSequence( c ):
            found = findOwningSequence( c, nameToLookFor )
            if found:
                return found
    return None

def findAlgorithm( startSequence, nameToLookFor, depth = 1000000 ):
    """ Traverse sequences tree to find the algorithm of given name. The first encountered is returned. 

    The name() method is used to obtain the algorithm name, that one has to match to the request.
    Depth of the search can be controlled by the depth parameter. 
    Typical use is to limit search to the startSequence with depth parameter set to 1
    """
    for c in startSequence.getChildren():
        if not isSequence( c ):
            if  c.name() == nameToLookFor:
                return c
        else:
            if depth > 1:
                found = findAlgorithm( c, nameToLookFor, depth-1 )
                if found:
                    return found                

    return None

def flatAlgorithmSequences( start ):
    """ Converts tree like structure of sequences into dictionary 
    keyed by top/start sequence name containing lists of of algorithms & sequences."""

    def __inner( seq, collector ):
        for c in seq.getChildren():        
            collector[seq.name()].append( c )
            if isSequence( c ):                        
                __inner( c, collector )
                
    from collections import defaultdict
    c = defaultdict(list)
    __inner(start, c)
    return c

def flatSequencers( start ):
    """ Flattens sequences """
    
    def __inner( seq, collector ):
        for c in seq.getChildren():        
            collector[seq.name()].append( c )
            if isSequence( c ):            
                __inner( c, collector )

    from collections import defaultdict
    c = defaultdict(list)
    __inner(start, c)
    return c


# self test
if __name__ == "__main__":
    from AthenaCommon.Configurable import ConfigurablePyAlgorithm
    top = parOR("top")
    top += parOR("nest1")
    top += seqAND("nest2")
    top += ConfigurablePyAlgorithm("SomeAlg0")

    f = findSubSequence(top, "top")
    assert f, "Can not find sequence at start"
    assert f.name() == "top", "Wrong sequence"
    # a one level deep search
    nest2 = findSubSequence( top, "nest2" )
    assert nest2, "Can not find sub sequence" 
    assert nest2.name() == "nest2", "Sub sequence incorrect"

    nest2 += parOR("deep_nest1")
    nest2 += parOR("deep_nest2")
    
    nest2 += ConfigurablePyAlgorithm("SomeAlg1")
    nest2 += ConfigurablePyAlgorithm("SomeAlg2")

    # deeper search
    d = findSubSequence(top, "deep_nest2")
    assert d, "Deep searching for sub seqeunce fails"
    assert d.name() == "deep_nest2", "Wrong sub sequence in deep search"

    d += ConfigurablePyAlgorithm("SomeAlg3")

    # algorithm is not a sequence
    d = findSubSequence(top, "SomeAlg1") 
    assert d == None, "Algorithm confused as a sequence"
    
    # no on demand creation
    inexistent = findSubSequence(top, "not_there")
    assert inexistent == None, "ERROR, found sub sequence that does not relay exist %s" % inexistent.name()
    
    # owner finding
    inexistent = findOwningSequence(top, "not_there")
    assert inexistent == None, "ERROR, found owner of inexistent sequence %s"% inexistent.name()

    owner = findOwningSequence(top, "deep_nest1")
    assert owner.name() == "nest2", "Wrong owner %s" % owner.name()

    owner = findOwningSequence(top, "deep_nest2")
    assert owner.name() == "nest2", "Wrong owner %s" % owner.name()

    owner = findOwningSequence(top, "SomeAlg1")
    assert owner.name() == "nest2", "Wrong owner %s" % owner.name()

    owner = findOwningSequence(top, "SomeAlg0")
    assert owner.name() == "top", "Wrong owner %s" % owner.name()

    flat = flatAlgorithmSequences( top )
    expected = [ "top", "nest2", "deep_nest2" ]
    assert set( flat.keys() ) == set( expected ), "To many or to few sequences in flat structure, present %s expected %s "% ( " ".join( flat.keys() ), " ".join( expected ) )    

    flat = flatSequencers(top)
    assert set( flat.keys() ) == set( expected ), "To many or to few sequences in flat structure, present %s expected %s "% ( " ".join( flat.keys() ), " ".join( expected ) )    

    a1 = findAlgorithm( top, "SomeAlg0" )
    assert a1, "Can't find algorithm present in sequence"
    a1 = findAlgorithm( top, "SomeAlg1" )
    assert a1, "Can't find nested algorithm "

    a1 = findAlgorithm( nest2, "SomeAlg0" )
    assert a1 == None, "Finding algorithm that is in the upper sequence"
    
    a1 = findAlgorithm( nest2, "NonexistentAlg" )
    assert a1 == None, "Finding algorithm that is does not exist"
             
    a1 = findAlgorithm( top, "SomeAlg0", 1)
    assert a1, "Could not find algorithm within the required nesting depth == 1"

    a1 = findAlgorithm( top, "SomeAlg1", 1)
    assert a1 == None, "Could find algorithm even if it is deep in sequences structure"

    a1 = findAlgorithm( top, "SomeAlg1", 2)
    assert a1, "Could not find algorithm within the required nesting depth == 2"

    a1 = findAlgorithm( top, "SomeAlg3", 2)
    assert a1 == None, "Could find algorithm evn if it is deep in sequences structure"    


    print ("All OK")

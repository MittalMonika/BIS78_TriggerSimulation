# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, addPrivateTool

def makeMetAnalysisSequence( dataType, metSuffix, jetContainer, jetSystematics,
                             components ):
    """Create a met analysis algorithm sequence

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
    """

    if not dataType in ["data", "mc", "afii"] :
        raise ValueError ("invalid data type: " + dataType)

    # if dataType == "data" :
    #     metName = "AnalysisMet_%SYS%"
    #     pass
    # else :
    #     metName = "PreAnalysisMet_%SYS%"
    #     pass
    metName = "AnalysisMet_%SYS%"
    metSys = "(^$)"

    # Create the analysis algorithm sequence object:
    seq = AlgSequence( "MetAnalysisSequence" )


    # Set up the met maker algorithm:
    alg = createAlgorithm( 'CP::MetMakerAlg',
                           'MetMakerAlg' )
    addPrivateTool( alg, 'makerTool',
                    'met::METMaker' )
    alg.makerTool.DoPFlow = "PFlow" in metSuffix
    alg.metCore = "MET_Core_" + metSuffix
    alg.metAssociation = "METAssoc_" + metSuffix
    alg.jets = jetContainer
    alg.jetsRegex = jetSystematics
    particles = []
    particlesRegex = []
    particlesType = []
    particlesKey = []
    print "components: ", components
    for part in components :
        particles.append (part["containerName"])
        particlesRegex.append (part["regex"])
        particlesType.append (part["type"])
        particlesKey.append (part["termName"])
        metSys += "|" + part["regex"]
        pass
    alg.systematicsRegex = "(^MET_.*)"
    alg.particles = particles
    alg.particlesRegex = particlesRegex
    alg.particlesType = particlesType
    alg.particlesKey = particlesKey
    alg.met = metName
    seq += alg

    if dataType != "data" :
        alg = createAlgorithm( 'CP::MetSystematicsAlg',
                               'MetSystematicsAlg' )
        addPrivateTool( alg, 'systematicsTool',
                        'met::METSystematicsTool' )
        alg.met = metName
        #metName = "AnalysisMet_%SYS"
        #alg.metOut = metName
        alg.metRegex = metSys
        alg.systematicsRegex = "(^MET_.*)"
        metSys += "|" + alg.systematicsRegex
        seq += alg
        pass


    # Set up the met builder algorithm:
    alg = createAlgorithm( 'CP::MetBuilderAlg',
                           'MetBuilderAlg' )
    alg.met = metName
    alg.metRegex = metSys
    seq += alg


    # Set up the met significance algorithm:
    alg = createAlgorithm( 'CP::MetSignificanceAlg',
                           'MetSignificanceAlg' )
    addPrivateTool( alg, 'significanceTool',
                    'met::METSignificance' )
    alg.significanceTool.SoftTermParam = 0
    alg.significanceTool.TreatPUJets = True
    alg.met = metName
    alg.metRegex = metSys
    seq += alg

    # Return the sequence:
    return seq

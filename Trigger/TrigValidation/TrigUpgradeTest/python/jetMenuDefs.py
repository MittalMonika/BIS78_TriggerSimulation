#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLTMenuConfig.Jet.JetMenuSequences import jetMenuSequence, jetRecoSequence

def jetDictFromString(jet_def_string):
    """ Function to create the jet Menu Sequence"""
   
    # Translate the definition string into an approximation
    # of the "recoParts" in the jet chainParts.
    jetRecoDict = {}
    # Insert values from string
    # Python names are more descriptive. May want to sync
    # these names with the SignatureDict, needs coordination with
    # menu group when they start to implement this
    jetalg, inputtype, clusterscale, jetcalib = jet_def_string.split('_')
    jetRecoDict = {
        "recoAlg":  jetalg,
        "dataType": inputtype,
        "calib":    clusterscale,
        "jetCalib": jetcalib
    }
    return jetRecoDict

def jetRecoSequenceFromString(jet_def_string):
    print "CACCA ", jet_def_string
    jetRecoDict=jetDictFromString(jet_def_string)
    return jetRecoSequence(None,**jetRecoDict)	
    
 
def jetMenuSequenceFromString(jet_def_string):
    jetRecoDict=jetDictFromString(jet_def_string)
    return jetMenuSequence(None,**jetRecoDict) # First arg dummy flags for RecoFragmentsPool

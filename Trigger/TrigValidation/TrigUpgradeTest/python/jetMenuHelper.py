#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLTMenuConfig.Jet.JetMenuSequences import jetMenuSequence, jetCFSequence

def jetDictFromString(jet_def_string):
    """ Function to retrieve the jet dictionaory from string"""
   
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

def jetCFSequenceFromString(jet_def_string):
    """ Function to retrieve jet Reco sequence from string"""
    jetRecoDict=jetDictFromString(jet_def_string)
    return jetCFSequence(None,**jetRecoDict)	
    
 
def jetMenuSequenceFromString(jet_def_string):
    """ Function to retrieve jet menu sequence from string"""
    jetRecoDict=jetDictFromString(jet_def_string)
    return jetMenuSequence(None,**jetRecoDict) # First arg dummy flags for RecoFragmentsPool
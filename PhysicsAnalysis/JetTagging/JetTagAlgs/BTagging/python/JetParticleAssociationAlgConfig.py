# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from BTagging.JetParticleAssociationConfig import JetParticleAssociationCfg
from BTagging.BTagTrackToJetAssociatorConfig import BTagTrackToJetAssociatorCfg

# import the JetBTaggerAlg configurable
from BTagging.BTaggingConf import Analysis__JetParticleAssociationAlg

def JetParticleAssociationAlgCfg(ConfigFlags, JetCollection="", ParticleCollection="", AssociationName="", **options):

    acc=ComponentAccumulator()
    jetcol = JetCollection
    partcol = ParticleCollection

    
    release = options.get('Release', None)

    optionAssoc = {} 
    if release == '21':
      if 'BB' in AssociationName: # Improve this test (maybe not used for Run3)
        release += 'bb'
        optionAssoc = {'shareTracks': False,
                       'useVariableSizedTrackCone' : True,
                       'coneSizeFitPar1' : 3.15265e-01,
                       'coneSizeFitPar2' : -3.66502e-01,
                       'coneSizeFitPar3' : -1.56387e-05}

    # setup the associator
    options['JetCollectionName'] = jetcol + 'Jets'
    options['TrackParticleCollectionName'] = partcol
    options['TrackToJetAssociatorName'] = AssociationName
    options['TrackToJetAssociator'] = acc.popToolsAndMerge(BTagTrackToJetAssociatorCfg(ConfigFlags, AssociationName.split('.')[1], options = optionAssoc))
    options['Associator'] = acc.popToolsAndMerge(JetParticleAssociationCfg(ConfigFlags))
    options['name'] = (jetcol + '_assoc'+release).lower()
    
    # -- create the association algorithm
    acc.addEventAlgo(Analysis__JetParticleAssociationAlg(**options))
            
    return acc

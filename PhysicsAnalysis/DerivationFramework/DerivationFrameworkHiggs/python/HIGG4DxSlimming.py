# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

#################################################
# Common code used for the HIGG4 slimming       #
# Z.Zinonos                                     #
# zenon@cern.ch                                 #
# Nov 2015                                      #
#################################################


def setup(HIGG4DxName, HIGG4DxStream, HIGG4DxSlimmingHelper):
    from AthenaCommon.GlobalFlags import globalflags
    DFisMC = (globalflags.DataSource()=='geant4')
    
    #smart slimming
    if HIGG4DxName == 'HIGG4D4':
        HIGG4DxSlimmingHelper.SmartCollections = ["Electrons",
                                                  "Muons",
                                                  "TauJets",
                                                  "MET_Reference_AntiKt4EMTopo",
                                                  "AntiKt4EMTopoJets",
                                                  "BTagging_AntiKt4EMTopo",
                                                  "InDetTrackParticles",
                                                  "PrimaryVertices"]
    elif HIGG4DxName == 'HIGG4D5':
        HIGG4DxSlimmingHelper.SmartCollections = ["Electrons",
                                                  "Photons",
                                                  "Muons",
                                                  "TauJets",
                                                  "MET_Reference_AntiKt4EMTopo",
                                                  "AntiKt4EMTopoJets",
                                                  "BTagging_AntiKt4EMTopo",
                                                  "InDetTrackParticles",
                                                  "PrimaryVertices"]
    else:
        HIGG4DxSlimmingHelper.SmartCollections = ["Electrons",
                                                  "Photons",
                                                  "Muons",
                                                  "TauJets",
                                                  "MET_Reference_AntiKt4EMTopo",
                                                  "MET_Reference_AntiKt4LCTopo",
                                                  "AntiKt4EMTopoJets",
                                                  "AntiKt4LCTopoJets",
                                                  "BTagging_AntiKt4EMTopo",
                                                  "BTagging_AntiKt4LCTopo",
                                                  "InDetTrackParticles",
                                                  "PrimaryVertices"]

    if HIGG4DxName in ['HIGG4D2', 'HIGG4D3']:
        HIGG4DxSlimmingHelper.SmartCollections += ["BTagging_AntiKt2Track"]

    #extra variables added to the smart slimming content
    ExtraContentElectrons=[
        "Electrons."
        "LHLoose."
        "LHMedium."
        "LHTight."
        "LHValue"
        ]

    ExtraElectronsTruth=[
        "Electrons."
        "truthOrigin."
        "truthType."
        "truthParticleLink"
        ]

    ExtraContentMuons=[
        "Muons."
        "quality."
        "m"
        ]

    ExtraMuonsTruth=[
        "MuonTruthParticles."
        "truthOrigin."
        "truthType"
        ]

    ExtraContentTaus=[
        "TauJets."
        "pantau_CellBasedInput_isPanTauCandidate."
        "pantau_CellBasedInput_DecayMode."
        "ptPanTauCellBased."
        "etaPanTauCellBased."
        "phiPanTauCellBased."
        "mPanTauCellBased."
        "pantau_CellBasedInput_BDTValue_1p0n_vs_1p1n."
        "pantau_CellBasedInput_BDTValue_1p1n_vs_1pXn."
        "pantau_CellBasedInput_BDTValue_3p0n_vs_3pXn."
        "ele_match_lhscore."
        "ele_olr_pass."
        "electronLink"
        ,
        "TauNeutralParticleFlowObjects."
        "pt."
        "eta."
        "phi."
        "m."
        "e."
        "rapidity."
        "bdtPi0Score"
        ]

    ExtraTausTruth = [
        "TauJets.IsTruthMatched.truthParticleLink.truthJetLink"
        ]
    

    if HIGG4DxName == 'HIGG4D3':
        ExtraContentVtx=["PrimaryVertices.x.y.z.vertexType.TauRefittedPVLink.chiSquared.numberDoF"]
    else:
        ExtraContentVtx=["PrimaryVertices.x.y.z.vertexType"]

    HIGG4DxSlimmingHelper.ExtraVariables = ExtraContentElectrons + ExtraContentMuons + ExtraContentTaus + ExtraContentVtx

    if DFisMC:
        HIGG4DxSlimmingHelper.ExtraVariables += ExtraElectronsTruth + ExtraMuonsTruth + ExtraTausTruth

    #extra containers
    if HIGG4DxName in ['HIGG4D2', 'HIGG4D3', 'HIGG4D4', 'HIGG4D5']:
        HIGG4DxSlimmingHelper.AllVariables += ["LVL1JetRoIs"]

    if HIGG4DxName in ['HIGG4D2', 'HIGG4D3']:
        HIGG4DxSlimmingHelper.AllVariables += ["AntiKt10LCTopoJets"]

    if HIGG4DxName in ['HIGG4D2', 'HIGG4D3']:
       HIGG4DxSlimmingHelper.AllVariables += ["DiTauJets"]

    if DFisMC:
        HIGG4DxSlimmingHelper.AllVariables += ["TruthEvents", 
                                               "TruthParticles", 
                                               "TruthVertices", 
                                               "AntiKt4TruthJets",
                                               "AntiKt4TruthWZJets"]

    if HIGG4DxName == 'HIGG4D3':
        HIGG4DxSlimmingHelper.AppendToDictionary = {
              "AntiKtVR30Rmax4Rmin02TrackJets"               :   "xAOD::JetContainer"        ,
              "AntiKtVR30Rmax4Rmin02TrackJetsAux"            :   "xAOD::JetAuxContainer"     ,
              "BTagging_AntiKtVR30Rmax4Rmin02Track"          :   "xAOD::BTaggingContainer"   ,
              "BTagging_AntiKtVR30Rmax4Rmin02TrackAux"       :   "xAOD::BTaggingAuxContainer",
              }
              
        HIGG4DxSlimmingHelper.AllVariables += ["AntiKtVR30Rmax4Rmin02TrackJets", "BTagging_AntiKtVR30Rmax4Rmin02Track"]
    
    # common for all formats
    HIGG4DxSlimmingHelper.AllVariables +=  [ "MET_Track" ]  # this is needed for the forward JVT
    
    
    #derivation truth
    if DFisMC:

        from DerivationFrameworkMCTruth.MCTruthCommon import *

        HIGG4DxSlimmingHelper.StaticContent = ["xAOD::TruthParticleContainer#TruthMuons",
                                               "xAOD::TruthParticleAuxContainer#TruthMuonsAux.",
                                               "xAOD::TruthParticleContainer#TruthElectrons",
                                               "xAOD::TruthParticleAuxContainer#TruthElectronsAux.",
                                               #"xAOD::TruthParticleContainer#TruthPhotons",
                                               #"xAOD::TruthParticleAuxContainer#TruthPhotonsAux.",
                                               "xAOD::TruthParticleContainer#TruthNeutrinos",
                                               "xAOD::TruthParticleAuxContainer#TruthNeutrinosAux.",
                                               "xAOD::TruthParticleContainer#TruthTaus",
                                               "xAOD::TruthParticleAuxContainer#TruthTausAux."]



    #trigger content
    if HIGG4DxName == 'HIGG4D1':
        HIGG4DxSlimmingHelper.IncludeMuonTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeEGammaTriggerContent = True
    elif HIGG4DxName == 'HIGG4D2':
        HIGG4DxSlimmingHelper.IncludeMuonTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeEGammaTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeTauTriggerContent = True
    elif HIGG4DxName == 'HIGG4D3':
        HIGG4DxSlimmingHelper.IncludeTauTriggerContent = True
    elif HIGG4DxName == 'HIGG4D4':
        HIGG4DxSlimmingHelper.IncludeJetTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeTauTriggerContent = True
    elif HIGG4DxName == "HIGG4D5":
        HIGG4DxSlimmingHelper.IncludeJetTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeTauTriggerContent = True
        HIGG4DxSlimmingHelper.IncludeEtMissTriggerContent = True
    else:
        assert False, "HIGG4DxSlimming: Unknown derivation stream '{}'".format(HIGG4DxName)

    if HIGG4DxName in ['HIGG4D2', 'HIGG4D3']:
        from DerivationFrameworkJetEtMiss.JetCommon import *
        addJetOutputs(HIGG4DxSlimmingHelper, [HIGG4DxName+"Jets"])

    # the very last line in job options
    HIGG4DxSlimmingHelper.AppendContentToStream(HIGG4DxStream)

    #add tau PV refitted results
    if HIGG4DxName == 'HIGG4D3':
        HIGG4DxStream.AddItem(['xAOD::VertexContainer#TauRefittedPrimaryVertices'])
        HIGG4DxStream.AddItem(['xAOD::VertexAuxContainer#TauRefittedPrimaryVerticesAux.-vxTrackAtVertex'])



    

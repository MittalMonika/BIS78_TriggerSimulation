# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

#################################################
# Common code used for the HIGG4 augmentation          #
# Z.Zinonos                                        #
# zenon@cern.ch                                        #
# Nov 2015                                        #
#################################################

from AthenaCommon.GlobalFlags import globalflags
# running on data or MC
DFisMC = (globalflags.DataSource()=='geant4')

## add LHE3 weights metadata
from DerivationFrameworkCore.LHE3WeightMetadata import *

## jet tag nonprompt lepton
import JetTagNonPromptLepton.JetTagNonPromptLeptonConfig as JetTagConfig

def setup(HIGG4DxName, ToolSvc):

    augmentationTools=[]

    # Isolation Tool
    #from DerivationFrameworkMuons.TrackIsolationDecorator import MUON1IDTrackDecorator
    #ToolSvc += MUON1IDTrackDecorator
    #augmentationTools.append(MUON1IDTrackDecorator)

    # DELTA R TOOL
    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__DeltaRTool
    HIGG4DxTauJetsElectronsDeltaRTool = DerivationFramework__DeltaRTool(  name                    = "HIGG4DxTauJetsElectronsDeltaRTool",
                                                                          ContainerName           = "Electrons",
                                                                          SecondContainerName     = "TauJets",
                                                                          StoreGateEntryName      = "HIGG4DxTauJetsElectronsDeltaR")
    ToolSvc += HIGG4DxTauJetsElectronsDeltaRTool
    augmentationTools.append(HIGG4DxTauJetsElectronsDeltaRTool)

    # TAU SELECTOR TOOL
    from DerivationFrameworkTau.DerivationFrameworkTauConf import DerivationFramework__TauSelectionWrapper
    HIGG4DxJetBDTSigMediumSelectionWrapper = DerivationFramework__TauSelectionWrapper( name                    = "HIGG4DxJetBDTSigMediumSelectionWrapper",
                                                                                       #IsTauFlag              = IsTauFlag.JetBDTSigMedium,
                                                                                       IsTauFlag               = 20,
                                                                                       CollectionName          = "TauJets",
                                                                                       StoreGateEntryName      = "HIGG4DxJetBDTSigMedium")
    ToolSvc += HIGG4DxJetBDTSigMediumSelectionWrapper
    augmentationTools.append(HIGG4DxJetBDTSigMediumSelectionWrapper)

    HIGG4DxJetBDTSigLooseSelectionWrapper = DerivationFramework__TauSelectionWrapper( name                   = "HIGG4DxJetBDTSigLooseSelectionWrapper",
                                                                                     IsTauFlag               = 19,
                                                                                     CollectionName          = "TauJets",
                                                                                     StoreGateEntryName      = "HIGG4DxJetBDTSigLoose")
    ToolSvc += HIGG4DxJetBDTSigLooseSelectionWrapper
    augmentationTools.append(HIGG4DxJetBDTSigLooseSelectionWrapper)



    # Tau primary vertex refit
    if HIGG4DxName == "HIGG4D3":
        from DerivationFrameworkTau.DerivationFrameworkTauConf import DerivationFramework__TauPVTrkSelectionTool
        HIGG4DxTauPVTrkSelectionTool = DerivationFramework__TauPVTrkSelectionTool( name = "HIGG4DxTauPVTrkSelectionTool",
                                                                                   #minPt = 15000,
                                                                                   #m_maxDeltaR = 0.2,
#                                                                                   UseTrueTracks = DFisMC,
                                                                                   UseTrueTracks = False,  # TauTruthTrackMatchingTool not working ATM. Needs protection against thinned truth info in the xAOD
                                                                                   TauContainerName = "TauJets",
                                                                                   TauPVTracksContainerName = "TauPVTracks")
        ToolSvc += HIGG4DxTauPVTrkSelectionTool
        augmentationTools.append(HIGG4DxTauPVTrkSelectionTool)

        from DerivationFrameworkTau.DerivationFrameworkTauConf import DerivationFramework__TauPVRefitTool
        HIGG4DxTauPVRefittingTool = DerivationFramework__TauPVRefitTool( name = "HIGG4DxTauPVRefittingTool",
                                                                         PVLinkName = "PVLink",
                                                                         RefittedLinkName = "TauRefittedPVLink",
                                                                         PVContainerName = "PrimaryVertices",
                                                                         TauTrkContainerName = "TauPVTracks",
                                                                         PVRefContainerName = "TauRefittedPrimaryVertices")
        ToolSvc += HIGG4DxTauPVRefittingTool
        augmentationTools.append(HIGG4DxTauPVRefittingTool)

    if DFisMC:
        # Tau truth matching - using the tau tools
        from DerivationFrameworkTau.TauTruthCommon import scheduleTauTruthTools
        scheduleTauTruthTools()

    #Tau Overlapping Electron LLH Decorator
    from TauAnalysisTools.TauAnalysisToolsConf import TauAnalysisTools__TauOverlappingElectronLLHDecorator
    HIGG4DxTauOrlElLLHDec = TauAnalysisTools__TauOverlappingElectronLLHDecorator(name="HIGG4DxTauOrlElLLHDec",
                                                                                 ElectronContainerName = "Electrons")
    
    ToolSvc += HIGG4DxTauOrlElLLHDec
        
    #Tau Overlapping Electron LLH Decorator Wrapper
    from DerivationFrameworkTau.DerivationFrameworkTauConf import DerivationFramework__TauOverlappingElectronLLHDecoratorWrapper
    HIGG4DxTauOrlElLLHDecWrapper = DerivationFramework__TauOverlappingElectronLLHDecoratorWrapper( name = "HIGG4DxTauOrlElLLHDecWrapper",
                                                                                                   TauOverlappingElectronLLHDecorator = HIGG4DxTauOrlElLLHDec,
                                                                                                   TauContainerName     = "TauJets")
    
    ToolSvc += HIGG4DxTauOrlElLLHDecWrapper
    augmentationTools.append(HIGG4DxTauOrlElLLHDecWrapper)

    #=============
    # return tools
    #=============
    return augmentationTools

#=====================================
# variable-R jets reconstruction + btagging
#=====================================
def addVRJetsAndBTagging(HIGG4DxName, sequence):
    
    # Create variable-R trackjets and dress AntiKt10LCTopo with ghost VR-trkjet
    
    from DerivationFrameworkFlavourTag.HbbCommon import addVRJets
    
    # removed for now. Does not work in rel 21 anymore
    addVRJets(sequence)
    
    # Run b-tagging
    from BTagging.BTaggingFlags import BTaggingFlags
    
    # alias for VR
    BTaggingFlags.CalibrationChannelAliases += ["AntiKtVR30Rmax4Rmin02Track->AntiKtVR30Rmax4Rmin02Track,AntiKt4EMTopo"]
    
#====================================================================
# JetTagNonPromptLepton decorations. Called from HIGG4D1.py
#====================================================================
def addJetTagNonPromptLepton(HIGG4DxName, sequence):
    
    # Build AntiKt4PV0TrackJets and run b-tagging
    JetTagConfig.ConfigureAntiKt4PV0TrackJets(sequence, HIGG4DxName)
    
    # Add BDT decoration algs
    sequence += JetTagConfig.GetDecoratePromptLeptonAlgs()
    sequence += JetTagConfig.GetDecoratePromptTauAlgs()

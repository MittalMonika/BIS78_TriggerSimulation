# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

HIGG5D1Content = [
### Event info
"xAOD::EventInfo_v1#EventInfo",
"xAOD::EventAuxInfo_v1#EventInfoAux.",
"PileUpEventInfo#McEventInfo",
### Electron
"xAOD::ElectronContainer_v1#ElectronCollection",
"xAOD::ElectronAuxContainer#ElectronCollectionAux.",
"xAOD::ElectronContainer_v1#FwdElectrons",        ### may not need
"xAOD::ElectronAuxContainer#FwdElectronsAux.", ### may not need
### Muon
"xAOD::MuonContainer_v1#Muons",
"xAOD::MuonAuxContainer_v1#MuonsAux.",
### Tau
"xAOD::TauJetContainer_v1#TauRecContainer",
"xAOD::TauJetAuxContainer_v1#TauRecContainerAux.",
### Photon
#"xAOD::PhotonContainer_v1#*",
#"xAOD::PhotonAuxContainer_v1#*",
### Jet
"xAOD::JetContainer_v1#AntiKt4EMTopoJets",
"xAOD::JetAuxContainer_v1#AntiKt4EMTopoJetsAux.",
"xAOD::JetContainer_v1#AntiKt4LCTopoJets",
"xAOD::JetAuxContainer_v1#AntiKt4LCTopoJetsAux.",
"xAOD::JetContainer_v1#AntiKt4TruthJets",
"xAOD::JetAuxContainer_v1#AntiKt4TruthJetsAux.",
"xAOD::JetContainer_v1#AntiKt4TruthWZJets",
"xAOD::JetAuxContainer_v1#AntiKt4TruthWZJetsAux.",
"xAOD::JetContainer_v1#AntiKt10LCTopoJets",
"xAOD::JetAuxContainer_v1#AntiKt10LCTopoJetsAux.",
"xAOD::JetContainer_v1#AntiKt10TruthJets",
"xAOD::JetAuxContainer_v1#AntiKt10TruthJetsAux.",
"xAOD::JetContainer_v1#AntiKt10TruthWZJets",
"xAOD::JetAuxContainer_v1#AntiKt10TruthWZJetsAux.",
"xAOD::JetContainer_v1#CamKt12LCTopoJets",
"xAOD::JetAuxContainer_v1#CamKt12LCTopoJetsAux.",
"xAOD::JetContainer_v1#CamKt12TruthJets",
"xAOD::JetAuxContainer_v1#CamKt12TruthJetsAux.",
"xAOD::JetContainer_v1#CamKt12TruthWZJets",
"xAOD::JetAuxContainer_v1#CamKt12TruthWZJetsAux.",
"xAOD::JetContainer_v1#AntiKt3PV0TrackJets",
"xAOD::JetAuxContainer_v1#AntiKt3PV0TrackJetsAux.",
"xAOD::EventShape_v1#*",
"xAOD::EventShapeAuxInfo_v1#*",
### Calo cluster 
#"xAOD::CaloClusterContainer_v1#CaloCalTopoCluster",
#"xAOD::CaloClusterAuxContainer_v1#CaloCalTopoClusterAux.",
"xAOD::CaloClusterContainer_v1#LArClusterEMFrwd",
"xAOD::CaloClusterAuxContainer_v1#LArClusterEMFrwdAux.",
"xAOD::CaloClusterContainer_v1#egClusterCollection",
"xAOD::CaloClusterAuxContainer_v1#egClusterCollectionAux.",
### Track
"xAOD::TrackParticleContainer_v1#CombinedMuonTrackParticles",
"xAOD::TrackParticleContainer_v1#ExtrapolatedMuonTrackParticles",
"xAOD::TrackParticleContainer_v1#GSFTrackParticles",
"xAOD::TrackParticleContainer_v1#InDetTrackParticles",
"xAOD::TrackParticleContainer_v1#InDetTrackParticlesForward",
"xAOD::TrackParticleAuxContainer_v1#CombinedMuonTrackParticlesAux.",
"xAOD::TrackParticleAuxContainer_v1#ExtrapolatedMuonTrackParticlesAux.",
"xAOD::TrackParticleAuxContainer_v1#GSFTrackParticlesAux.",
"xAOD::TrackParticleAuxContainer_v1#InDetTrackParticlesAux.",
"xAOD::TrackParticleAuxContainer_v1#InDetTrackParticlesForwardAux.",
### Missing ET
"xAOD::MissingETContainer_v1#MET_EMJets",
"xAOD::MissingETAuxContainer_v1#MET_EMJetsAux.",
"xAOD::MissingETContainer_v1#MET_EleLHMedEMJets",
"xAOD::MissingETAuxContainer_v1#MET_EleLHMedEMJetsAux.",
#"xAOD::MissingETContainer_v1#MET_RefFinal",
#"xAOD::MissingETAuxContainer_v1#MET_RefFinalAux.",
"xAOD::MissingETContainer_v1#MET_RefFinalFix",
"xAOD::MissingETAuxContainer_v1#MET_RefFinalFixAux.",
"xAOD::MissingETContainer_v1#MET_Truth",
"xAOD::MissingETAuxContainer_v1#MET_TruthAux.",
#"xAOD::MissingETContainer_v1#MET_Track",
#"xAOD::MissingETAuxContainer_v1#MET_TrackAux.",
"xAOD::MissingETContainer_v1#MET_TrackFix", 
"xAOD::MissingETAuxContainer_v1#MET_TrackFixAux.", 
"xAOD::MissingETComponentMap_v1#METMap_EMJets", 
"xAOD::MissingETAuxComponentMap_v1#METMap_EMJetsAux.",
"xAOD::MissingETComponentMap_v1#METMap_EleLHMedEMJets",
"xAOD::MissingETAuxComponentMap_v1#METMap_EleLHMedEMJetsAux.",
"xAOD::MissingETComponentMap_v1#METMap_RefFinalFix",
"xAOD::MissingETAuxComponentMap_v1#METMap_RefFinalFixAux.",
"xAOD::MissingETComponentMap_v1#METMap_Truth", 
"xAOD::MissingETAuxComponentMap_v1#METMap_TruthAux.", 
"xAOD::MissingETComponentMap_v1#METMap_TrackFix", 
"xAOD::MissingETAuxComponentMap_v1#METMap_TrackFixAux.", 
### Vertex
"xAOD::VertexContainer_v1#*",
"xAOD::VertexAuxContainer_v1#*",
### B-tag
"xAOD::BTaggingContainer_v1#BTagging_AntiKt4EMTopo",
"xAOD::BTaggingAuxContainer_v1#BTagging_AntiKt4EMTopoAux.",
"xAOD::BTaggingContainer_v1#BTagging_AntiKt4LCTopo",
"xAOD::BTaggingAuxContainer_v1#BTagging_AntiKt4LCTopoAux.",
"xAOD::BTaggingContainer_v1#BTagging_AntiKt4Truth",
"xAOD::BTaggingAuxContainer_v1#BTagging_AntiKt4TruthAux.",
"xAOD::BTaggingContainer_v1#BTagging_AntiKt4TruthWZ",
"xAOD::BTaggingAuxContainer_v1#BTagging_AntiKt4TruthWZAux.",
"xAOD::BTaggingContainer_v1#BTagging_AntiKt10LCTopo",
"xAOD::BTaggingAuxContainer_v1#BTagging_AntiKt10LCTopoAux.",
"xAOD::BTaggingContainer_v1#BTagging_AntiKt10Truth",
"xAOD::BTaggingAuxContainer_v1#BTagging_AntiKt10TruthAux.",
"xAOD::BTaggingContainer_v1#BTagging_AntiKt10TruthWZ",
"xAOD::BTaggingAuxContainer_v1#BTagging_AntiKt10TruthWZAux.",
### Trigger 
"xAOD::TrigDecision_v1#xTrigDecision",
"xAOD::TrigDecisionAuxInfo_v1#xTrigDecisionAux.",
"xAOD::TriggerMenuContainer_v1#TriggerMenu",
"xAOD::TriggerMenuAuxContainer_v1#TriggerMenuAux.",
"HLT::HLTResult#HLTResult_HLT",
"HLT::HLTResult#HLTResult_L2",
"HLT::HLTResult#HLTResult_EF",
"TrigDec::TrigDecision#TrigDecision",
### Truth
"xAOD::TruthVertexContainer_v1#TruthVertex",
"xAOD::TruthVertexAuxContainer_v1#TruthVertexAux.",
"xAOD::TruthParticleContainer_v1#TruthParticle",
"xAOD::TruthParticleContainer_v1#MuonTruthParticle",
"xAOD::TruthParticleAuxContainer_v1#TruthParticleAux.",
"xAOD::TruthParticleAuxContainer_v1#MuonTruthParticleAux.",
"xAOD::TruthEventContainer_v1#TruthEvent",
"xAOD::TruthEventAuxContainer_v1#TruthEventAux.",
### Tau
"xAOD::PFOContainer_v1#TauPi0ChargedPFOContainer",
"xAOD::PFOAuxContainer_v1#TauPi0ChargedPFOContainerAux.",
"xAOD::PFOContainer_v1#TauPi0NeutralPFOContainer",
"xAOD::PFOAuxContainer_v1#TauPi0NeutralPFOContainerAux.",
"xAOD::PFOContainer_v1#TauShotPFOContainer",
"xAOD::PFOAuxContainer_v1#TauShotPFOContainerAux.",
"xAOD::PFOContainer_v1#chargedTauPFO_eflowRec",
"xAOD::PFOAuxContainer_v1#chargedTauPFO_eflowRecAux.",
"xAOD::PFOContainer_v1#neutralTauPFO_eflowRec",
"xAOD::PFOAuxContainer_v1#neutralTauPFO_eflowRecAux."
]

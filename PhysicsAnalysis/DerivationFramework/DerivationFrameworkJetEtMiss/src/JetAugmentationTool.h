////////////////////-*- C++ -*-////////////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// JetAugmentationTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_JETAUGMENTATIONTOOL_H
#define DERIVATIONFRAMEWORK_JETAUGMENTATIONTOOL_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "JetInterface/IJetModifier.h"
#include "JetInterface/IJetUpdateJvt.h"
#include "JetJvtEfficiency/IJetJvtEfficiency.h"
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"
#include "xAODJet/JetContainer.h"

using namespace std;
//namespace InDet { class IInDetTrackSelectionTool; }
//namespace InDet { class InDetTrackSelectionTool; }
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
//#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
using namespace InDet;

namespace DerivationFramework {

  class JetAugmentationTool : public AthAlgTool, public IAugmentationTool {
  public:
    JetAugmentationTool(const std::string& t, const std::string& n, const IInterface* p);

    StatusCode initialize();
    StatusCode finalize();
    virtual StatusCode addBranches() const;

  private:
    std::string m_momentPrefix;
    std::string m_containerName;
    //
    // implement augmentations explicitly to avoid need to parse lists of moments to copy
    //
    
    // calibration
    SG::AuxElement::Decorator<float>* dec_calibpt;
    SG::AuxElement::Decorator<float>* dec_calibeta;
    SG::AuxElement::Decorator<float>* dec_calibphi;
    SG::AuxElement::Decorator<float>* dec_calibm;
    ToolHandle<IJetModifier> m_jetCalibTool;
    std::string m_calibMomentKey;
    bool m_docalib;

    // JVT
    SG::AuxElement::Decorator<float>* dec_jvt;
    SG::AuxElement::Decorator<char>* dec_passJvt;
    ToolHandle<IJetUpdateJvt> m_jvtTool;
    ToolHandle<CP::IJetJvtEfficiency> m_jetJvtEfficiencyTool; //!
    std::string m_jvtMomentKey;
    bool m_dojvt;

    // b-tagging       @author tripiana@cern.ch
    std::vector<SG::AuxElement::Decorator<float>*> dec_btag;
    std::vector<std::string> m_btagWP;
    bool m_dobtag;
    /// Athena configured tools
    ToolHandleArray<IBTaggingSelectionTool> m_btagSelTools;

    //TrackSumMass and TrackSumPt for calo-jets built in Tier-0
    //@author: nurfikri.bin.norjoharuddeen@cern.ch
    ToolHandle<IJetModifier> m_jetTrackSumMomentsTool;
    bool m_decoratetracksum;
    SG::AuxElement::Decorator<float>* dec_tracksummass;
    SG::AuxElement::Decorator<float>* dec_tracksumpt;

    //OriginCorrection 
    ToolHandle<IJetModifier> m_jetOriginCorrectionTool;
    bool m_decorateorigincorrection;
    SG::AuxElement::Decorator<ElementLink<xAOD::VertexContainer>>* dec_origincorrection;
    SG::AuxElement::Decorator<float>* dec_originpt;
    SG::AuxElement::Decorator<float>* dec_origineta;
    SG::AuxElement::Decorator<float>* dec_originphi;
    SG::AuxElement::Decorator<float>* dec_originm;
    
    // GhostTruthAssociation for derivations, @author jeff.dandoy@cern.ch
    ToolHandle<IJetModifier> m_jetPtAssociationTool;
    bool m_decorateptassociation;
    SG::AuxElement::Decorator<float>* dec_GhostTruthAssociationFraction;
    SG::AuxElement::Decorator< ElementLink<xAOD::JetContainer> >* dec_GhostTruthAssociationLink;

    // Ntracks for QG tagging --- antonio ---
    bool m_decoratentracks;
    SG::AuxElement::Decorator<int>* dec_AssociatedNtracks;
    ToolHandle<InDet::IInDetTrackSelectionTool> m_trkSelectionTool;
    //InDet::IInDetTrackSelectionTool *m_trkSelectionTool;

    SG::AuxElement::Decorator<vector<float>>* dec_Track_pt;
    SG::AuxElement::Decorator<vector<float>>* dec_Track_eta;
    SG::AuxElement::Decorator<vector<float>>* dec_Track_phi;
    SG::AuxElement::Decorator<vector<float>>* dec_Track_E;
    SG::AuxElement::Decorator<vector<int>>* dec_Track_passCount1;
    SG::AuxElement::Decorator<vector<int>>* dec_Track_passCount2;
    SG::AuxElement::Decorator<vector<int>>* dec_Track_passCount3;

  };
}

#endif // DERIVATIONFRAMEWORK_JETAUGMENTATIONTOOL_H

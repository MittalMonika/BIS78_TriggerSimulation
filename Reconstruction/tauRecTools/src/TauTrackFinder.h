/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS
#ifndef TAUREC_TAUTRACKFINDER_H
#define TAUREC_TAUTRACKFINDER_H

#include "tauRecTools/TauRecToolBase.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkCaloExtension/CaloExtensionCollection.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "VxVertex/RecVertex.h"

// xAOD Tracking Tool
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

namespace Trk {
  class ITrackSelectorTool;
  class IParticleCaloExtensionTool;
}

/////////////////////////////////////////////////////////////////////////////

/** 
 * @brief Associate tracks to the tau candidate.
 * 
 *  The tracks have to pass dedicated quality criteria and have to 
 *  match to a primary vertex consistent with the tau origin.
 * 
 * @author KG Tan <Kong.Guan.Tan@cern.ch>
 * @author Felix Friedrich
 */

class TauTrackFinder : public TauRecToolBase {
public:
    //-------------------------------------------------------------
    //! Constructor and Destructor
    //-------------------------------------------------------------
    TauTrackFinder(const std::string& name);
    ASG_TOOL_CLASS2(TauTrackFinder, TauRecToolBase, ITauToolBase);
    ~TauTrackFinder();

    //-------------------------------------------------------------
    //! Enumerator defining type of tau track
    //-------------------------------------------------------------
    enum TauTrackType
    {
        TauTrackCore  = 0,
        TauTrackWide  = 1,
        TauTrackOther = 2,
        NotTauTrack   = 3
    };

    //-------------------------------------------------------------
    //! Algorithm functions
    //-------------------------------------------------------------
    virtual StatusCode initialize() override;
    virtual StatusCode eventInitialize() override;
    virtual StatusCode execute(xAOD::TauJet& pTau) override;
    virtual StatusCode eventFinalize() override;
    virtual StatusCode finalize() override;
    
    
    //-------------------------------------------------------------
    //! Extrapolate track eta and phi to the calorimeter middle surface
    //-------------------------------------------------------------
    StatusCode extrapolateToCaloSurface(xAOD::TauJet& pTau);

    TauTrackType tauTrackType( const xAOD::TauJet& tauJet,
    		const xAOD::TrackParticle& trackParticle,
    		const xAOD::Vertex* primaryVertex);

    void getTauTracksFromPV( const xAOD::TauJet& tauJet,
    		const xAOD::TrackParticleContainer& trackParticleCont,
    		const xAOD::Vertex* primaryVertex,
    		std::vector<const xAOD::TrackParticle*> &tauTracks,
    		std::vector<const xAOD::TrackParticle*> &wideTracks,
    		std::vector<const xAOD::TrackParticle*> &otherTracks);

    // new xAOD version
    void removeOffsideTracksWrtLeadTrk(std::vector<const xAOD::TrackParticle*> &tauTracks,
                                           std::vector<const xAOD::TrackParticle*> &wideTracks,
                                           std::vector<const xAOD::TrackParticle*> &otherTracks,
                                           const xAOD::Vertex* tauOrigin,
                                           double maxDeltaZ0);

    void  getDeltaZ0Values(std::vector<float>& vDeltaZ0coreTrks, std::vector<float>& vDeltaZ0wideTrks);
    void  resetDeltaZ0Cache();

private:
    //-------------------------------------------------------------
    //! Some internally used functions
    //-------------------------------------------------------------
    float getZ0(const xAOD::TrackParticle* track, const xAOD::Vertex* vertex);   //xAOD version

    //-------------------------------------------------------------
    //! tools
    //-------------------------------------------------------------
    ToolHandle< Trk::IParticleCaloExtensionTool >  m_caloExtensionTool;
    ToolHandle<Trk::ITrackSelectorTool> m_trackSelectorTool_tau;
    ToolHandle<Reco::ITrackToVertex> m_trackToVertexTool;
    //output particle calo extension collection
    SG::ReadHandleKey<CaloExtensionCollection>  m_ParticleCacheKey{this,
      "tauParticleCache", "ParticleCaloExtension", "Name of the particle measurement extrapolation cache for TauTrackFinder"};
    
    //-------------------------------------------------------------
    //! Input parameters for algorithm
    //-------------------------------------------------------------
    double  m_maxJetDr_tau;
    double  m_maxJetDr_wide;
    
    //-------------------------------------------------------------
    // z0 cuts
    //-------------------------------------------------------------
    float m_z0maxDelta;
    bool m_applyZ0cut;
    bool m_storeInOtherTrks;
    bool m_removeDuplicateCoreTracks;
    std::vector<float> m_vDeltaZ0coreTrks;
    std::vector<float> m_vDeltaZ0wideTrks;

    //-------------------------------------------------------------
    // Bypass TrackSelectorTool / Extrapolation
    //-------------------------------------------------------------

    bool m_bypassSelector;
    bool m_bypassExtrapolator;
    Gaudi::Property<bool> m_useOldCalo{this,"useOldCalo",false,"If true, it uses the CaloExtensionTool for calculating track extrapolation. Otherwise, it allows the code to read from the cache created by CaloExtensionBuilderalg."};

    //-------------------------------------------------------------
    // Sets of EM/Had samplings for track extrapolation 
    //-------------------------------------------------------------
    std::set<CaloSampling::CaloSample> m_EMSamplings;
    std::set<CaloSampling::CaloSample> m_HadSamplings;

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackPartInputContainer{this,"Key_trackPartInputContainer", "InDetTrackParticles", "input track particle container key"};

};

#endif //TAUREC_TAUTRACKFINDER_H
#endif //XAOD_ANALYSIS

# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

########################################################################
#                                                                      #
# JetModConfig: A helper module for configuring tools that support     #
# jet reconstruction                                                   #
# Author: TJ Khoo                                                      #
#                                                                      #
########################################################################

from AthenaCommon import Logging
jrtlog = Logging.logging.getLogger('JetRecToolsConfig')

# Package configurable imports
from InDetTrackSelectionTool import InDetTrackSelectionToolConf
from TrackVertexAssociationTool import TrackVertexAssociationToolConf
from JetRecTools import JetRecToolsConf

# May need to specify non-standard tracking collections, e.g. for trigger
# Testing code -- move to another module and perhaps allow extensions
# e.g. in a dedicated trigger collections module to keep online/offline
# code more factorised
trackcollectionmap = {
    # Offline track collections
    "": {
        "Tracks":           "InDetTrackParticles",
        "JetTracks":        "JetSelectedTracks",
        "Vertices":         "PrimaryVertices",
        "TVA":              "JetTrackVtxAssoc",
        "GhostTracks":      "PseudoJetGhostTrack",
        "GhostTracksLabel": "GhostTrack",
    }
}

def getTrackSelTool(trkopt=""):
    jettracksname = "JetSelectedTracks"
    if trkopt: jettracksname += "_{}".format("trkopt")

    # Track selector needs its own hierarchical config getter in JetRecTools?
    from InDetTrackSelectionTool import InDetTrackSelectionToolConf
    idtrackselloose = InDetTrackSelectionToolConf.InDet__InDetTrackSelectionTool(
        "idtrackselloose",
        CutLevel         = "Loose",
        minPt            = 500,
        UseTrkTrackTools = False,
        Extrapolator     = "",
        TrackSummaryTool = ""
    )
    jettrackselloose = JetRecToolsConf.JetTrackSelectionTool(
        "jettrackselloose",
        InputContainer  = trackcollectionmap[trkopt]["Tracks"],
        OutputContainer = jettracksname,
        Selector        = idtrackselloose
    )
    return jettrackselloose

def getTrackVertexAssocTool(trkopt=""):
    if trkopt: "_{}".format(trkopt)
    # Track-vertex association
    from TrackVertexAssociationTool import TrackVertexAssociationToolConf
    idtvassoc = TrackVertexAssociationToolConf.CP__TrackVertexAssociationTool(
        "idloosetvassoc",
        VertexContainer         = trackcollectionmap[trkopt]["Vertices"],
    )

    jettvassoc = JetRecToolsConf.TrackVertexAssociationTool(
        "jettvassoc",
        TrackParticleContainer  = trackcollectionmap[trkopt]["Tracks"],
        TrackVertexAssociation  = trackcollectionmap[trkopt]["TVA"],
        VertexContainer         = trackcollectionmap[trkopt]["Vertices"],
        TrackVertexAssoTool     = idtvassoc,
    )
    return jettvassoc


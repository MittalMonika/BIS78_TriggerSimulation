#
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#


from AthenaCommon.AppMgr import ServiceMgr as svcMgr
from AthenaCommon.Constants import VERBOSE,DEBUG,INFO
from AthenaCommon.CFElements import parOR, seqAND, seqOR, stepSeq


def jetAthSequence(ConfigFlags):
    from TrigT2CaloCommon.CaloDef import clusterFSInputMaker
    InputMakerAlg= clusterFSInputMaker()

    dataOrMC = "data"
    #if ConfigFlags.Input.isMC:
    #    dataOrMC = "mc"
    # want to make this automatic.

    if ConfigFlags.jetdef=="EMTopoSubJESIS":
    	(recoSequence, sequenceOut) = jetRecoSequenceEMTopo("Calib:TrigEMSubJESIS:"+dataOrMC)
    elif ConfigFlags.jetdef=="EMTopoSubJES":
    	(recoSequence, sequenceOut) = jetRecoSequenceEMTopo("Calib:TrigEMSubJES:"+dataOrMC)

    JetAthSequence =  seqAND("jetAthSequence",[InputMakerAlg, recoSequence ])
    return (JetAthSequence, InputMakerAlg, sequenceOut)

    
def jetRecoSequenceEMTopo(calibString, RoIs = 'FSJETRoI'):

    from TrigT2CaloCommon.CaloDef import HLTFSTopoRecoSequence
    (caloMakerSequence, caloclusters) = HLTFSTopoRecoSequence(RoIs)

    from JetRecConfig.JetDefinition import JetConstit, JetDefinition, xAODType
  
    #hardcoded jet collection for now 
    # chosen jet collection
    jetsFullName = "TrigAntiKt4EMTopoSubJES"
    clustermods = ["ECPSFrac","ClusterMoments"]
    #clustermods = []
    trigMinPt = 7e3
    TrigEMTopo = JetConstit( xAODType.CaloCluster, ["EM"])
    TrigEMTopo.ptmin = 2e3
    TrigEMTopo.ptminfilter = 15e3
    TrigAntiKt4EMTopoSubJES = JetDefinition( "AntiKt", 0.4, TrigEMTopo, ptmin=trigMinPt,ptminfilter=trigMinPt)
    TrigAntiKt4EMTopoSubJES.modifiers = [calibString,"Sort"] + clustermods 
    #TrigAntiKt4EMTopoSubJES.ghostdefs = []
    #TrigAntiKt4EMTopoSubJES.isTrigger = True
    jetDefinition = TrigAntiKt4EMTopoSubJES


    from JetRecConfig import JetRecConfig
    deps = JetRecConfig.resolveDependencies( jetDefinition )

    print("resolved dependencies: ", deps)

    inputdeps = deps["inputs"]
    constit = inputdeps[0]
    constit.rawname = caloclusters
    #constit.inputname = caloclusters
    print("constit.rawname = ", constit.rawname )
    print("constit.inputname = ", constit.inputname )
    constitAlg = getConstitAlg( constit )
    jetRecoSequence = seqAND( "JetRecoSeq", [constitAlg])

    constitPJAlg = getConstitPJGAlg( constit )
    constitPJKey = constitPJAlg.PJGetter.OutputContainer

    pjs = [constitPJKey]

    jetRecoSequence += constitPJAlg 
        
    if "JetSelectedTracks" in inputdeps or "JetTrackVtxAssoc" in inputdeps:
        jetTrkPrepAlg = getTrackPrepAlg( "trigjetalg_TrackPrep")
        jetRecoSequence += jetTrkPrepAlg
        
    eventShapeAlg = getEventShapeAlg( constit, constitPJKey )
    jetRecoSequence += eventShapeAlg                    

    # Schedule the ghost PseudoJetGetterAlgs
    for ghostdef in deps["ghosts"]:
        print("ghostdef = ", ghostdef)
        ghostPJAlg = getGhostPJGAlg( ghostdef )
        jetRecoSequence += ghostPJAlg
        ghostPJKey = ghostPJAlg.PJGetter.OutputContainer
        pjs.append( ghostPJKey )

    # Generate a JetAlgorithm to run the jet finding and modifiers
    # (via a JetRecTool instance).
    jetRecAlg = JetRecConfig.getJetAlgorithm(jetsFullName, jetDefinition, pjs, deps["mods"])

    jetRecoSequence += jetRecAlg

    #sequenceOut = 'HLTJets_EMSubJES'
    sequenceOut = jetsFullName

    caloMakerSequence += jetRecoSequence

    #jetRecoFullSequence = seqAND("fullJetRecoSequence", [caloMakerSequence, jetRecoSequence]) 
    jetRecoFullSequence = caloMakerSequence

    return (jetRecoFullSequence,sequenceOut)

def getConstitAlg( constit ):

        from JetRecConfig import ConstModHelpers
        constitalg = ConstModHelpers.getConstitModAlg(constit)
        return constitalg

def getConstitPJGAlg(basedef):

    from JetRec import JetRecConf
    getter = JetRecConf.PseudoJetGetter("pjg_"+basedef.label,
        InputContainer = basedef.inputname,
        OutputContainer = "PseudoJet"+basedef.label,
        Label = basedef.label,
        SkipNegativeEnergy=True,
        GhostScale=0.
        )

    pjgalg = JetRecConf.PseudoJetAlgorithm(
        "pjgalg_"+basedef.label,
        PJGetter = getter
        )
    return pjgalg

def getGhostPJGAlg(ghostdef):

    label = "Ghost"+ghostdef.inputtype
    kwargs = {
        "OutputContainer":    "PseudoJet"+label,
        "Label":              label,
        "SkipNegativeEnergy": True,
        "GhostScale":         1e-40
        }

    from JetRec import JetRecConf
    pjgclass = JetRecConf.PseudoJetGetter
    if ghostdef.inputtype=="MuonSegment":
        # Muon segments have a specialised type
        pjgclass = JetRecConf.MuonSegmentPseudoJetGetter
        kwargs = {
            "InputContainer":"MuonSegments",
            "OutputContainer":"PseudoJet"+label,
            "Label":label,
            "Pt":1e-20
            }
    elif ghostdef.inputtype=="Track":
        kwargs["InputContainer"] = "JetSelectedTracks"
    elif ghostdef.inputtype.startswith("TruthLabel"):
        truthsuffix = ghostdef.inputtype[5:]
        kwargs["InputContainer"] = "TruthLabel"+truthsuffix
    elif ghostdef.inputtype == "Truth":
        kwargs["InputContainer"] = "JetInputTruthParticles"
    else:
        raise ValueError("Unhandled ghost type {0} received!".format(ghostdef.inputtype))

    getter = pjgclass("pjg_"+label, **kwargs)

    pjgalg = JetRecConf.PseudoJetAlgorithm(
        "pjgalg_"+label,
        PJGetter = getter
        )
    return pjgalg

def getTrackPrepAlg( trackPrepAlgName ):

        from JetRecTools import JetRecToolsConfig
        from JetRec import JetRecConf
        # Jet track selection
        jettrackselloose = JetRecToolsConfig.getTrackSelTool()
        jettvassoc = JetRecToolsConfig.getTrackVertexAssocTool()

        jettrkprepalg = JetRecConf.JetAlgorithm(trackPrepAlgName)
        jettrkprepalg.Tools = [ jettrackselloose, jettvassoc ]

        return jettrkprepalg

def getEventShapeAlg( constit, constitpjkey ):

        rhokey = "Kt4"+constit.label+"EventShape"
        rhotoolname = "EventDensity_Kt4"+constit.label
        
        from EventShapeTools import EventShapeToolsConf
        rhotool = EventShapeToolsConf.EventDensityTool(rhotoolname)
        rhotool.InputContainer = constitpjkey
        rhotool.OutputContainer = rhokey
        
        eventshapealg = EventShapeToolsConf.EventDensityAthAlg("{0}Alg".format(rhotoolname))
        eventshapealg.EventDensityTool = rhotool

        return eventshapealg


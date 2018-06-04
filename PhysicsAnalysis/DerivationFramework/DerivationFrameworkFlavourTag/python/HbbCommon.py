# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration


# import Common Algs
from DerivationFrameworkJetEtMiss.JetCommon import DFJetAlgs

# Import star stuff (it was like that when I got here)
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
from JetRec.JetRecConf import JetAlgorithm

#===================================================================
# ExKt/CoM Subjets
#===================================================================

# make exkt subjet finding tool
def buildExclusiveSubjets(ToolSvc, JetCollectionName, subjet_mode, nsubjet, doTrackSubJet, ExGhostLabels=None):
    from JetRec.JetRecStandard import jtm

    subjetlabel = "Ex%s%iSubJets" % (subjet_mode, nsubjet)
    doCoM = False
    algj = "Kt"
    if subjet_mode == "CoM" :
      doCoM = True
      #
      #supported algorithms: Reconstruction/Jet/JetSubStructureUtils/Root/SubjetFinder.cxx
      #algorithms:"ee_kt" or "EEKt" for EE_Kt
      #           "cambridge" or "CamKt" for Cambridge
      #           "FastJetPlugin" for EECambridge plugin 
      algj = "ee_kt"

    SubjetContainerName = "%sEx%s%iSubJets" % (JetCollectionName.replace("Jets", ""), subjet_mode, nsubjet)
    ExKtbbTagToolName = str( "Ex%sbbTagTool%i_%s" % (subjet_mode, nsubjet, JetCollectionName) )
    if hasattr(jtm, ExKtbbTagToolName):
        ExKtbbTagToolInstance = jtm[ ExKtbbTagToolName ]
        print " ExKtbbTagTool ", ExKtbbTagToolName, "is alredy in jtm.tools"
    else:
        print " building ExKtbbTagTool ", ExKtbbTagToolName

        from JetSubStructureMomentTools.JetSubStructureMomentToolsConf import SubjetFinderTool
        from JetSubStructureMomentTools.JetSubStructureMomentToolsConf import SubjetRecorderTool
        if ExGhostLabels == None:
          if subjet_mode == "CoM":
            ExGhostLabels = []
          else:
            ExGhostLabels = ["GhostBHadronsFinal", "GhostBHadronsInitial", "GhostBQuarksFinal", "GhostCHadronsFinal", "GhostCHadronsInitial", "GhostCQuarksFinal", "GhostHBosons", "GhostPartons", "GhostTQuarksFinal", "GhostTausFinal", "GhostTruth"]
            if "Track" not in JetCollectionName:
              ExGhostLabels += ["GhostTrack"]


        subjetrecorder = SubjetRecorderTool("subjetrecorder_%s%i_%s" % (subjet_mode, nsubjet, JetCollectionName))
        ToolSvc += subjetrecorder
        subjetrecorder.SubjetLabel = subjetlabel
        subjetrecorder.SubjetContainerName = SubjetContainerName

        if not hasattr(ToolSvc, "ExKtTrackSelectorLoose"):
          from InDetTrackSelectionTool.InDetTrackSelectionToolConf import InDet__InDetTrackSelectionTool
          ToolSvc += InDet__InDetTrackSelectionTool("ExKtTrackSelectorLoose",
                                                    CutLevel = "Loose",
                                                   )

        from JetTagTools.JetTagToolsConf import Analysis__ExKtbbTagTool
        ExKtbbTagToolInstance = Analysis__ExKtbbTagTool(
          name = ExKtbbTagToolName, 
          JetAlgorithm = algj,
          JetRadius = 10.0,
          PtMin = 1000,
          ExclusiveNJets = nsubjet,
          doTrack = doTrackSubJet,
          InputJetContainerName = JetCollectionName,
          SubjetContainerName = SubjetContainerName,
          SubjetRecorder = subjetrecorder,
          SubjetLabel = subjetlabel,
          SubjetAlgorithm_BTAG = "AntiKt",
          SubjetRadius_BTAG = 0.4,
          TrackSelectionTool = ToolSvc.ExKtTrackSelectorLoose,
          PrimaryVtxContainerName = "PrimaryVertices",
          SubjetBoostConstituent = doCoM,
          GhostLabels = ",".join(ExGhostLabels)
        )
        jtm.add( ExKtbbTagToolInstance )
        ToolSvc += ExKtbbTagToolInstance

    return (ExKtbbTagToolInstance, SubjetContainerName)

#===================================================================
# Build ExKt Subjets
#===================================================================
def addExKt(sequence, ToolSvc, ExKtJetCollection__FatJet, nSubjets, doTrackSubJet):
    from JetRec.JetRecStandard import jtm
    ExKtJetCollection__SubJet = []
    for JetCollectionExKt in ExKtJetCollection__FatJet:
        # build ExKtbbTagTool instance
        (ExKtbbTagToolInstance, SubjetContainerName) = buildExclusiveSubjets(ToolSvc, JetCollectionExKt, "Kt", nSubjets, doTrackSubJet)
        ExKtJetCollection__SubJet += [SubjetContainerName]

        exktAlgName = "jfind_%s" % (SubjetContainerName)
        exktJetRecToolName = "%s" % (SubjetContainerName)
        exktBTagName = "BTagging_%s" % (SubjetContainerName.replace("Jets", ""))

        if exktAlgName in DFJetAlgs:
            print " Algorithm ExKt ", exktAlgName, "already built before"
            if hasattr(sequence, exktAlgName):
                print " sequence ExKt ", sequence, "already has an instance of algorithm", exktAlgName
            else:
                print " Add algorithm ExKt ", exktAlgName, "to sequence", sequence
                sequence += DFJetAlgs[exktAlgName]
        else:
            print " Algorithm ExKt ", exktAlgName, " to be built sequence ", sequence
            if hasattr(jtm, exktJetRecToolName):
                jetrec = jtm [ exktJetRecToolName ]
                print " ExKtJetRecTool ", exktJetRecToolName, " is found in jtm.tools sequence ", sequence
            else:
                # build subjet collection through JetRecTool
                from JetRec.JetRecConf import JetRecTool
                jetrec = JetRecTool(
                                     name = exktJetRecToolName,
                                     OutputContainer = JetCollectionExKt,
                                     InputContainer = JetCollectionExKt,
                                     JetModifiers = [ExKtbbTagToolInstance],
                                   )
                jtm.add( jetrec )
                ToolSvc += jetrec
                print " ExKtJetRecTool ", exktJetRecToolName, " is now built, sequence ", sequence

            exktJetRecBTagToolName = str( "%s_sub"%exktJetRecToolName )
            if hasattr(jtm, exktJetRecBTagToolName):
                print " ExKtJetRecBTagTool ", exktJetRecBTagToolName, " is already built, sequence ", sequence
                jetrec_btagging = jtm [ exktJetRecBTagToolName ]
            else:
                from BTagging.BTaggingFlags import BTaggingFlags
                btag_exkt = ConfInst.setupJetBTaggerTool(ToolSvc, JetCollection=exktJetRecToolName.replace("Jets", ""), AddToToolSvc=True, Verbose=True,
                             options={"name"         : exktBTagName.lower(), 
                                      "BTagName"     : exktBTagName, 
                                      "BTagJFVtxName": "JFVtx",                  
                                      "BTagSVName"   : "SecVtx",                 
                                      },
                             SetupScheme = "",
                             TaggerList = BTaggingFlags.StandardTaggers,
                             )
                from BTagging.BTaggingConfiguration import defaultTrackAssoc, defaultMuonAssoc
                mods = [defaultTrackAssoc, defaultMuonAssoc, btag_exkt]
                jetrec_btagging = JetRecTool( name = exktJetRecBTagToolName,
                                          InputContainer  = SubjetContainerName,
                                          OutputContainer = SubjetContainerName,
                                          JetModifiers    = mods)
                jtm.add( jetrec_btagging )
                ToolSvc += jetrec_btagging
                print " ExKtJetRecBTagTool ", exktJetRecBTagToolName, " is now built, sequence ", sequence


            jetalg_exkt = JetAlgorithm(
                                     name = exktAlgName,
                                     Tools = [jetrec,jetrec_btagging],
                                    )
            sequence += jetalg_exkt
            DFJetAlgs[exktAlgName] = jetalg_exkt

    return ExKtJetCollection__SubJet


#===================================================================
# Build CoM Subjets
#===================================================================
def addExCoM(sequence, ToolSvc, ExKtJetCollection__FatJet, nSubjets, doTrackSubJet):
    from JetRec.JetRecStandard import jtm
    ExCoMJetCollection__SubJet = []
    for JetCollectionExCoM in ExKtJetCollection__FatJet:
        (ExCoMbbTagToolInstance, SubjetContainerName) = buildExclusiveSubjets(ToolSvc, JetCollectionExCoM, "CoM", nSubjets, doTrackSubJet)
        ExCoMJetCollection__SubJet += [SubjetContainerName]


        excomELresetName = "ELreset_subjet_%s" % (SubjetContainerName.replace("Jets", ""))
        excomAlgName = "jfind_%s" % (SubjetContainerName)
        excomJetRecToolName = "%s" % (SubjetContainerName)
        excomBTagName = "BTagging_%s" % (SubjetContainerName.replace("Jets", ""))

        if excomAlgName in DFJetAlgs:
            print " Algorithm ExCoM ", excomAlgName, "already built before sequence ", sequence

            if hasattr(sequence, excomAlgName):
                print " sequence ExCoM ", sequence, "already has an instance of algorithm", excomAlgName
            else:
                print " Add algorithm ExCoM ", excomAlgName, "to sequence", sequence
                sequence += DFJetAlgs[excomAlgName]
                sequence += CfgMgr.xAODMaker__ElementLinkResetAlg(excomELresetName, SGKeys=[SubjetContainerName+"Aux."])
                sequence += DFJetAlgs[excomAlgName+"_btag"]
        else:
            print " Algorithm ExCoM ", excomAlgName, " to be built sequence ", sequence
            if hasattr(jtm, excomJetRecToolName):
                print " ExCoMJetRecTool ", excomJetRecToolName, " already built sequence ",  sequence
                jetrec = jtm [ excomJetRecToolName ]
            else:
                print " ExCoM tool ", excomJetRecToolName, " to be built sequence ", sequence
                from JetRec.JetRecConf import JetRecTool
                jetrec = JetRecTool(
                                     name = excomJetRecToolName,
                                     OutputContainer = JetCollectionExCoM,
                                     InputContainer = JetCollectionExCoM,
                                     JetModifiers = [ExCoMbbTagToolInstance],
                                   )
                jtm.add( jetrec )
                ToolSvc += jetrec

            excomJetRecBTagToolName = str( "%s_sub"%excomJetRecToolName )
            if hasattr(jtm, excomJetRecBTagToolName):
                print " ExCoMJetRecBTagTool ", excomJetRecBTagToolName, " already built sequence ",  sequence
                jetrec_btagging = jtm [ excomJetRecBTagToolName ]
            else:
                print " ExCoMJetRecBTagTool ", excomJetRecBTagToolName, " to be built sequence ",  sequence

                #make the btagging tool for excom jets
                from BTagging.BTaggingFlags import BTaggingFlags
                btag_excom = ConfInst.setupJetBTaggerTool(ToolSvc, JetCollection=excomJetRecToolName.replace("Jets", ""), AddToToolSvc=True, Verbose=True,
                             options={"name"         : excomBTagName.lower(), 
                                      "BTagName"     : excomBTagName, 
                                      "BTagJFVtxName": "JFVtx",                  
                                      "BTagSVName"   : "SecVtx",                 
                                      },
                             SetupScheme = "",
                             TaggerList = BTaggingFlags.StandardTaggers,
                             )
                # running association + b-tagging on subjets now
                from BTagging.BTaggingConfiguration import comTrackAssoc, comMuonAssoc
                mods = [comTrackAssoc, comMuonAssoc, btag_excom]

                jetrec_btagging = JetRecTool( name = excomJetRecBTagToolName,
                                          InputContainer  = SubjetContainerName,
                                          OutputContainer = SubjetContainerName,
                                          JetModifiers    = mods)
                jtm.add( jetrec_btagging )
                ToolSvc += jetrec_btagging

            jetalg_excom = JetAlgorithm(
                                     name = excomAlgName,
                                     Tools = [jetrec],
                                    )
            sequence += jetalg_excom
            DFJetAlgs[excomAlgName] = jetalg_excom

            # Reset EL for ExCoM subjets after all of them are built
            # Otherwise crashing for CoM during TrackToJetAssociation due to failure of finding the large-R parent jet.
            sequence += CfgMgr.xAODMaker__ElementLinkResetAlg(excomELresetName, SGKeys=[SubjetContainerName+"Aux."])

            jetalg_excom_btag = JetAlgorithm(
                                     name = excomAlgName+"_btag",
                                     Tools = [jetrec_btagging],
                                    )
            sequence += jetalg_excom_btag
            DFJetAlgs[excomAlgName+"_btag"] = jetalg_excom_btag

    return ExCoMJetCollection__SubJet

##################################################################
# Build variable-R subjets, recluster AntiKt10LCTopojet with ghost VR and copy ghost link to AntiKt10LCTopo 
##################################################################
def addVRJets(sequence, *pos_opts, **opts):
    from JetRec.JetRecStandard import jtm
    from AthenaCommon import Logging

    if 'logger' not in opts:
        logger = Logging.logging.getLogger('VRLogger')
    else:
        logger = opts['logger']

    # define constants here, since we don't want every derivaiton
    # deciding for themselves what a VR jet is. If we do want this
    # flexibility, this code will need some rewriting to ensure that
    # there are no issues with train safety.
    if opts or pos_opts:
        logger.warning('Options specified for VR jets, they will be ignored')

    VRJetName="AntiKtVR30Rmax4Rmin02Track"
    VRGhostLabel="GhostVR30Rmax4Rmin02TrackJet"
    VRJetAlg="AntiKt"
    VRJetRadius=0.4
    VRJetInputs='pv0track'
    VRJetOptions = dict(
        ghostArea = 0 , ptmin = 2000, ptminFilter = 7000,
        variableRMinRadius = 0.02, variableRMassScale = 30000,
        calibOpt = "none")


    #==========================================================
    # Build VR jets
    #==========================================================

    VRJetAlgName = "jfind_%sJets" % (VRJetName)
    VRJetRecToolName = "%sJets" % (VRJetName)
    VRJetBTagName = "BTagging_%s" % (VRJetName)

    from AthenaCommon.AppMgr import ToolSvc

    #make the btagging tool for VR jets
    btag_vrjets = ConfInst.setupJetBTaggerTool(ToolSvc, JetCollection=VRJetRecToolName, AddToToolSvc=True, Verbose=True,
                 options={"name"         : VRJetBTagName.lower(), 
                          "BTagName"     : VRJetBTagName, 
                          "BTagJFVtxName": "JFVtx",                  
                          "BTagSVName"   : "SecVtx",                 
                          },
                 SetupScheme = "",
                 TaggerList = ['IP2D', 'IP3D', 'MultiSVbb1',  'MultiSVbb2', 'SV1', 'JetFitterNN', 'SoftMu', 
                               'MV2c10', 'MV2c10mu', 'MV2c10rnn', 'JetVertexCharge', 'MV2cl100' , 'MVb', 'DL1', 'DL1rnn', 'DL1mu', 'RNNIP']
                 )

    from BTagging.BTaggingConfiguration import defaultTrackAssoc, defaultMuonAssoc

    pseudoJetGetters = jtm.gettersMap[VRJetInputs]

    # We want to include ghost associated tracks in the pv0 tracks so that
    # we can use the looser ghost association criteria for b-tagging.
    if VRJetInputs == "pv0track":
        pseudoJetGetters = pseudoJetGetters + ["gtrackget"]

    if VRJetAlgName in DFJetAlgs:
        print "Algorithm", VRJetAlgName, "already built before"

        if hasattr(sequence, VRJetAlgName):
            print "   Sequence", sequence, "already has an instance of algorithm", VRJetAlgName
        else:
            print "   Add algorithm", VRJetAlgName, "to sequence", sequence
            sequence += DFJetAlgs[VRJetAlgName]
    else:
        print "Create algorithm", VRJetAlgName

        if hasattr(jtm, VRJetRecToolName):
            print "   JetRecTool", VRJetRecToolName, "is alredy in jtm.tools in sequence ", sequence
        else:
            print "   Create JetRecTool", VRJetRecToolName
            #can only run trackjetdrlabeler with truth labels, so MC only

            mods = [defaultTrackAssoc, defaultMuonAssoc, btag_vrjets]

            if globalflags.DataSource()!='data': 
                mods.append(jtm.trackjetdrlabeler)

            jtm.addJetFinder(
                VRJetRecToolName,
                VRJetAlg,
                VRJetRadius,
                pseudoJetGetters,
                modifiersin=mods,
                ivtxin=0,
                **VRJetOptions)

        from JetRec.JetRecConf import JetAlgorithm
        jetalg_smallvr30_track = JetAlgorithm(VRJetAlgName, Tools = [ jtm[VRJetRecToolName] ])
        sequence += jetalg_smallvr30_track
        DFJetAlgs[VRJetAlgName] = jetalg_smallvr30_track

    #==========================================================
    # Build PseudoJet Getter
    #==========================================================

    pjgettername = VRGhostLabel.lower()

    if hasattr(jtm, pjgettername):
        print "Found", pjgettername, "in jtm in sequence", sequence
    else:
        print "Add", pjgettername, "to jtm in sequence", sequence

        from JetRec.JetRecConf import PseudoJetGetter
        jtm += PseudoJetGetter(
          pjgettername,                                                          # give a unique name
          InputContainer = jetFlags.containerNamePrefix() + VRJetName + "Jets",  # SG key
          Label = VRGhostLabel,                                                  # this is the name you'll use to retrieve ghost associated VR track jets
          OutputContainer = "PseudoJet" + VRGhostLabel,
          SkipNegativeEnergy = True,
          GhostScale = 1.e-20,                                                   # this makes the PseudoJet Ghosts, and thus the reco flow will treat them as such
        )

    #==========================================================
    # Re-cluster large-R jet with VR ghost associated on it 
    # AntiKt10LCTopo hard-coded for now
    #==========================================================
    LargeRJetAlg     = "jfind_akt10lctopo_%s" %(VRJetName.lower())
    LargeRJets       = "AKt10LCTopo_%sJets"   %(VRJetName)
    LargeRJetPrefix  = "AKt10LCTopo_%s"       %(VRJetName)
    newLCTopo        = "lctopo_%s"            %(VRJetName.lower())
    LinkTransferAlg  = "LinkTransfer_%s"     %(VRJetName)

    if LargeRJetAlg in DFJetAlgs:
        print "  Found ", LargeRJetAlg," in DFJetAlgs in", sequence
        if hasattr(sequence, LargeRJetAlg):
            print "   Algsequence", sequence, "already has an instance of", LargeRJetAlg
        else:
            print "   Added ", LargeRJetAlg," to sequence ", sequence
            sequence += DFJetAlgs[LargeRJetAlg]

    else:
        if hasattr(jtm, LargeRJets):
            print  LargeRJets, " is alredy in jtm.tools in sequence ", sequence
        else:
            print "  Create new ", LargeRJets,"in", sequence
            OutputJets.setdefault("CustomJets" , [] ).append(LargeRJets)
            jtm.gettersMap[newLCTopo] = list(jtm.gettersMap["lctopo"])
            jtm.gettersMap[newLCTopo] += [ jtm[pjgettername] ] 
            jtm.addJetFinder(LargeRJets, "AntiKt", 1.0, newLCTopo , "lctopo_ungroomed",
                             ghostArea = 0 , ptmin = 40000, ptminFilter = 50000,
                             calibOpt = "none")

        jetalg_largefr10_lctopo = JetAlgorithm(LargeRJetAlg, Tools = [ jtm[LargeRJets] ])
        sequence += jetalg_largefr10_lctopo
        DFJetAlgs[LargeRJetAlg] = jetalg_largefr10_lctopo

    #==========================================================
    # Transfer the link from re-clustered jet to original jet
    # AntiKt10LCTopo hard-coded for now
    # Issue here: If addVRJets() is called more than once for different VR, only link to first VR would be transferred
    # Solution: decouple the following part with parts above
    #==========================================================

    jetassoctool = getJetExternalAssocTool('AntiKt10LCTopo', LargeRJetPrefix, MomentPrefix='', ListOfOldLinkNames=[VRGhostLabel])
    applyJetAugmentation('AntiKt10LCTopo', LinkTransferAlg, sequence, jetassoctool)

##################################################################
# Build variable-R subjets, recluster AntiKt10TCCjet with ghost VR and copy ghost link to AntiKt10TCC - Copy and pasting this defintion is an ugly temporary solution. We need to properly rewrite this for a more general use and remove the hardcoded jet collection. 
##################################################################
def addVRJetsTCC(sequence, VRJetName, VRGhostLabel, VRJetAlg="AntiKt", VRJetRadius=0.4, VRJetInputs="pv0track", **VRJetOptions):
    from JetRec.JetRecStandard import jtm

    from AthenaCommon.AppMgr import ToolSvc
    #==========================================================
    # Build VR jets
    #==========================================================

    VRJetAlgName = "jfind_%sJets" % (VRJetName)
    VRJetRecToolName = "%sJets" % (VRJetName)
    VRJetBTagName = "BTagging_%s" % (VRJetName)

    #make the btagging tool for VR jets
    btag_vrjets = ConfInst.setupJetBTaggerTool(ToolSvc, JetCollection=VRJetRecToolName, AddToToolSvc=True, Verbose=True,
                 options={"name"         : VRJetBTagName.lower(), 
                          "BTagName"     : VRJetBTagName, 
                          "BTagJFVtxName": "JFVtx",                  
                          "BTagSVName"   : "SecVtx",                 
                          },
                 SetupScheme = "",
                 TaggerList = ['IP2D', 'IP3D', 'MultiSVbb1',  'MultiSVbb2', 'SV1', 'JetFitterNN', 'SoftMu', 
                               'MV2c10', 'MV2c10mu', 'MV2c10rnn', 'JetVertexCharge', 'MV2cl100' , 'MVb', 'DL1', 'DL1rnn', 'DL1mu', 'RNNIP', 'MV2c10Flip']
                 )

    if VRJetAlgName in DFJetAlgs:
        print "Algorithm", VRJetAlgName, "already built before"

        if hasattr(sequence, VRJetAlgName):
            print "   Sequence", sequence, "already has an instance of algorithm", VRJetAlgName
        else:
            print "   Add algorithm", VRJetAlgName, "to sequence", sequence
            sequence += DFJetAlgs[VRJetAlgName]
    else:
        print "Create algorithm", VRJetAlgName

        if hasattr(jtm, VRJetRecToolName):
            print "   JetRecTool", VRJetRecToolName, "is alredy in jtm.tools in sequence ", sequence
        else:
            print "   Create JetRecTool", VRJetRecToolName
            #can only run trackjetdrlabeler with truth labels, so MC only
            if globalflags.DataSource()!='data': 
                jtm.addJetFinder(VRJetRecToolName, VRJetAlg, VRJetRadius, VRJetInputs, modifiersin=[trackassoc, muonassc, btag_vrjets,jtm.trackjetdrlabeler], **VRJetOptions) 
            else:
                jtm.addJetFinder(VRJetRecToolName, VRJetAlg, VRJetRadius, VRJetInputs, modifiersin=[trackassoc, muonassoc, btag_vrjets], **VRJetOptions)

        from JetRec.JetRecConf import JetAlgorithm
        jetalg_smallvr30_track = JetAlgorithm(VRJetAlgName, Tools = [ jtm[VRJetRecToolName] ])
        sequence += jetalg_smallvr30_track
        DFJetAlgs[VRJetAlgName] = jetalg_smallvr30_track

    #==========================================================
    # Build PseudoJet Getter
    #==========================================================

    pjgettername = VRGhostLabel.lower()

    if hasattr(jtm, pjgettername):
        print "Found", pjgettername, "in jtm in sequence", sequence
    else:
        print "Add", pjgettername, "to jtm in sequence", sequence

        from JetRec.JetRecConf import PseudoJetGetter
        jtm += PseudoJetGetter(
          pjgettername,                                                          # give a unique name
          InputContainer = jetFlags.containerNamePrefix() + VRJetName + "Jets",  # SG key
          Label = VRGhostLabel,                                                  # this is the name you'll use to retrieve ghost associated VR track jets
          OutputContainer = "PseudoJet" + VRGhostLabel,
          SkipNegativeEnergy = True,
          GhostScale = 1.e-20,                                                   # this makes the PseudoJet Ghosts, and thus the reco flow will treat them as such
        )

    #==========================================================
    # Re-cluster large-R jet with VR ghost associated on it 
    # AntiKt10TCC hard-coded for now
    #==========================================================
    LargeRJetAlg     = "jfind_akt10trackcalocluster_%s" %(VRJetName.lower())
    LargeRJets       = "AKt10TrackCaloCluster_%sJets"   %(VRJetName)
    LargeRJetPrefix  = "AKt10TrackCaloCluster_%s"       %(VRJetName)
    newLCTopo        = "tcc_%s"            %(VRJetName.lower())
    LinkTransferAlg  = "LinkTransfer_%s"     %(VRJetName)

    if LargeRJetAlg in DFJetAlgs:
        print "  Found ", LargeRJetAlg," in DFJetAlgs in", sequence
        if hasattr(sequence, LargeRJetAlg):
            print "   Algsequence", sequence, "already has an instance of", LargeRJetAlg
        else:
            print "   Added ", LargeRJetAlg," to sequence ", sequence
            sequence += DFJetAlgs[LargeRJetAlg]

    else:
        if hasattr(jtm, LargeRJets):
            print  LargeRJets, " is alredy in jtm.tools in sequence ", sequence
        else:
            print "  Create new ", LargeRJets,"in", sequence
            OutputJets.setdefault("CustomJets" , [] ).append(LargeRJets)
            jtm.gettersMap[newLCTopo] = list(jtm.gettersMap["tcc"])
            jtm.gettersMap[newLCTopo] += [ jtm[pjgettername] ] 
            jtm.addJetFinder(LargeRJets, "AntiKt", 1.0, newLCTopo , "tcc_ungroomed",
                             ghostArea = 0 , ptmin = 40000, ptminFilter = 50000,
                             calibOpt = "none")
        from JetRec.JetRecConf import JetAlgorithm
        jetalg_largefr10_tcc = JetAlgorithm(LargeRJetAlg, Tools = [ jtm[LargeRJets] ])
        sequence += jetalg_largefr10_tcc
        DFJetAlgs[LargeRJetAlg] = jetalg_largefr10_tcc

    #==========================================================
    # Transfer the link from re-clustered jet to original jet
    # AntiKt10TCC hard-coded for now
    # Issue here: If addVRJets() is called more than once for different VR, only link to first VR would be transferred
    # Solution: decouple the following part with parts above
    #==========================================================

    jetassoctool = getJetExternalAssocTool('AntiKt10TrackCaloCluster', LargeRJetPrefix, MomentPrefix='', ListOfOldLinkNames=[VRGhostLabel])
    applyJetAugmentation('AntiKt10TrackCaloCluster', LinkTransferAlg, sequence, jetassoctool)
    
#===================================================================
# Utils: Copy Jets
#===================================================================
def addCopyJet(FTAG5Seq, ToolSvc, InputJetCollectionName, OutputJetCollectionName, **options):
  doShallowCopy = options.pop("ShallowCopy", False)

  from JetRec.JetRecConf import JetRecTool
  jetrec = JetRecTool(
                       name = "JetRecTool_CopyJet_%s_%s" % (InputJetCollectionName, OutputJetCollectionName),
                       OutputContainer = OutputJetCollectionName,
                       InputContainer = InputJetCollectionName,

                       ShallowCopy = doShallowCopy,
                       **options
                     )

  ToolSvc += jetrec
  FTAG5Seq += JetAlgorithm(
                            name = "JetAlgorithm_CopyJet_%s_%s" % (InputJetCollectionName, OutputJetCollectionName),
                            Tools = [jetrec],
                          )

  return OutputJetCollectionName


#========================================================================
# Hbb Tagger
#========================================================================
def addHbbTagger(sequence, ToolSvc, logger=None,
                 output_level=WARNING,
                 jet_collection="AntiKt10LCTopoTrimmedPtFrac5SmallR20"):
    if logger is None:
        logger = Logging.logging.getLogger('HbbTaggerLog')

    fat_calibrator_name = "HbbCalibrator"
    is_data = not DerivationFrameworkIsMonteCarlo
    if not hasattr(ToolSvc, fat_calibrator_name):
        fatCalib = CfgMgr.JetCalibrationTool(
            fat_calibrator_name,
            OutputLevel=output_level,
            JetCollection=jet_collection,
            ConfigFile="JES_MC16recommendation_FatJet_JMS_comb_19Jan2018.config",
            CalibSequence="EtaJES_JMS",
            CalibArea="00-04-81",
            IsData=is_data)
        ToolSvc += fatCalib
        logger.info('set up {}'.format(fatCalib))
    else:
        logger.info('took {} from tool svc'.format(fat_calibrator_name))

    hbb_tagger_name = "HbbTagger"
    config_file_name = (
        "BoostedJetTaggers/HbbTagger/Summer2018/Apr13HbbNetwork.json")
    if not hasattr(ToolSvc, hbb_tagger_name):
        hbbTagger = CfgMgr.HbbTaggerDNN(
            hbb_tagger_name,
            OutputLevel=output_level,
            neuralNetworkFile=config_file_name)
        ToolSvc += hbbTagger
        logger.info('set up {}'.format(hbbTagger))
    else:
        logger.info('took {} from tool svc'.format(hbb_tagger_name))

    tagger_alg_name = "HbbTaggerAlg"
    if not hasattr(sequence, tagger_alg_name):
        if tagger_alg_name in DFJetAlgs:
            sequence += DFJetAlgs[tagger_alg_name]
            logger.info('took {} from jet algs'.format(tagger_alg_name))
        else:
            tagger_alg = CfgMgr.HbbTaggingAlgorithm(
                tagger_alg_name,
                OutputLevel=output_level,
                jetCollectionName=(jet_collection + "Jets"),
                decorationName="HbbScore",
                minPt=250e3,
                maxEta=2.0,
                tagger=hbbTagger,
                calibrationTool=fatCalib)
            DFJetAlgs[tagger_alg_name] = tagger_alg
            sequence += tagger_alg
            logger.info('set up {}'.format(tagger_alg))
    else:
        logger.info('{} already scheduled for {}'.format(
            tagger_alg_name, jet_collection))


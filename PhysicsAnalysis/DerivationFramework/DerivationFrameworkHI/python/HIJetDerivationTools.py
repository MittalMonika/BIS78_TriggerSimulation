# Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration

def addJetThinningTool(collection="AntiKt4HIJets",deriv="HION7",jet_pt_threshold=15) :
    from AthenaCommon.AppMgr import ToolSvc
    from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__GenericObjectThinning
    jet_tool = DerivationFramework__GenericObjectThinning(name                    = "%sJetThinningTool_%s" % (deriv,collection),
                                                          ThinningService         = "%sThinningSvc" % deriv,
                                                          ContainerName           = collection,
                                                          SelectionString         = "%s.pt  > (%d* GeV)" % (collection,jet_pt_threshold),
                                                          ApplyAnd                = False)
                                                                 

    ToolSvc+=jet_tool
    return jet_tool
    
def addJetClusterThinningTool(collection="AntiKt4HIJets",deriv="HION7",pt_cut=25) :
    from AthenaCommon.AppMgr import ToolSvc
    from DerivationFrameworkCalo.DerivationFrameworkCaloConf import DerivationFramework__JetCaloClusterThinning
    jet_cl_tool = DerivationFramework__JetCaloClusterThinning(name                   = "%sJetClusterThinningTool_%s" % (deriv,collection),
                                                              ThinningService        = "%sThinningSvc" % deriv,
                                                              SGKey                  = collection,
                                                              TopoClCollectionSGKey  = "HIClusters",
                                                              SelectionString        = "%s.pt > %f*GeV" % (collection,pt_cut))
                                                              #ConeSize               = 0
								

    ToolSvc+=jet_cl_tool
    return jet_cl_tool


def addJetTrackThinningTool(collection="AntiKt4HIJets",deriv="HION7") :
    from AthenaCommon.AppMgr import ToolSvc
    from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__JetTrackParticleThinning
    jet_track_tool = DerivationFramework__JetTrackParticleThinning(name                    = "%sJetTrackThinningTool_%s" % (deriv,collection),
                                                                   ThinningService         = "%sThinningSvc" % deriv,
                                                                   JetKey                  = collection,
                                                                   InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                   ApplyAnd                = True)
                                                                 

    ToolSvc+=jet_track_tool
    return jet_track_tool

#Not used
def addTrackThinningTool(deriv="HION7", track_pt_threshold=4) :
    from AthenaCommon.AppMgr import ToolSvc
    from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
    TPThinningTool = DerivationFramework__TrackParticleThinning( name                    = "%sTPThinningTool" % deriv,
                                                                 ThinningService         = "%sThinningSvc" % deriv,
                                                                 SelectionString         = "InDetTrackParticles.pt > (%d* GeV)" %  track_pt_threshold,
                                                                 InDetTrackParticlesKey  = "InDetTrackParticles")
    ToolSvc += TPThinningTool
    return TPThinningTool
#Not used
def addTrackThinningToolTight(deriv="HION7", track_pt_threshold=4) :
    from AthenaCommon.AppMgr import ToolSvc
    from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
    TPThinningTool = DerivationFramework__TrackParticleThinning( name                    = "%sTPThinningTool" % deriv,
                                                                 ThinningService         = "%sThinningSvc" % deriv,
                                                                 SelectionString         = "InDetTrackParticles.pt > (%f* GeV) && (abs(InDetTrackParticles.eta) < 2.5) && ((InDetTrackParticles.numberOfSCTHits + InDetTrackParticles.numberOfPixelHits) > 8 ) && (InDetTrackParticles.numberOfPixelHoles < 1) && (InDetTrackParticles.numberOfSCTHoles < 3)" %  track_pt_threshold,
                                                                 InDetTrackParticlesKey  = "InDetTrackParticles")
    ToolSvc += TPThinningTool
    return TPThinningTool


HIJetTriggerVars=["HLT_xAOD__JetContainer_a4ionemsubjesISFS.pt"
                  "HLT_xAOD__JetContainer_a4ionemsubjesISFS.eta",
                  "HLT_xAOD__JetContainer_a4ionemsubjesISFS.phi",
                  "HLT_xAOD__JetContainer_a4ionemsubjesISFS.m"]

HIClusterVars=["HIClusters.eta0",
               "HIClusters.phi0",
               "HIClusters.altE",
               "HIClusters.altEta",
               "HIClusters.altPhi",
               "HIClusters.altM",
               "HIClusters.rawE",
               "HIClusters.rawEta",
               "HIClusters.rawPhi",
               "HIClusters.rawM",
               "HIClusters.HIEtaPhiWeight",
               "HIClusters.HISubtractedE"]
#               "HIClusters.e_sampl",
#               "HIClusters.time"]

HISeedBranches=["pt","eta","phi","m"]

def makeHIJetBranchList() :
    state_vars=["pt","eta","phi","m"]
    c=list(state_vars)
    states=["JetUnsubtractedScaleMomentum","JetSubtractedScaleMomentum"]
    for s in states:
        for v in state_vars:
            c.append(s+'_'+v)
    c+=['ConstituentScale',
        'constituentLinks',
        'constituentWeights',
        'AverageLArQF',
        'EMFrac',
        'FracSamplingMax',
        'FracSamplingMaxIndex',
        'HECFrac',
        'HECQuality',
        'LArQuality',
        'N90Constituents',
        'NegativeE',
        'Timing',
        'BchCorrCell',
        'LArBadHVEnergyFrac',
        'LArBadHVNCell',
        'EnergyPerSampling',
        'GhostAntiKt4HITrackJets',
        'GhostAntiKt4HITrackJetsCount',
        'GhostAntiKt4HITrackJetsPt',
        'GhostMuonSegmentCount',
        'GhostTrack',
        'NumTrkPt4000',
        'SumPtTrkPt4000',
        'TrackWidthPt4000',
        'Width',
        'MaxConstituentET',
        'MaxOverMean']
    return c


HIJetBranches=makeHIJetBranchList()

##DROPPED CONTENT##
##misc##
#'JetSubtractedScaleNoIterationMomentum_pt',
#'JetSubtractedScaleV2OnlyMomentum_pt',
# 'InputType',
# 'AlgorithmType',
# 'SizeParameter',
# 'DetectorEta',
# 'ConstituentScale',
# 'OriginCorrected',
# 'PileupCorrected',
# 'JetGhostArea',
# 'OriginVertex',
# 'OriginVertex.m_persKey',
# 'OriginVertex.m_persIndex',
# 'OotFracClusters10',
# 'OotFracClusters5',
##associations##
#'GhostMuonSegment',
#'GhostTrackCount',
#'GhostTrackPt',
#'HIClusters_DR8Assoc',
##JVF/JVT##
#'HighestJVFVtx',
#'HighestJVFVtx.m_persKey',
#'HighestJVFVtx.m_persIndex',
#'JVF',
#'Jvt',
#'JvtJvfcorr',
#'JvtRpt',
##tracks in jets
#'NumTrkPt1000',
#'NumTrkPt2000',
#'NumTrkPt500',
#'SumPtTrkPt1000',
#'SumPtTrkPt2000',
#'SumPtTrkPt500',
#'TrackWidthPt1000',
#'TrackWidthPt2000',
#'TrackWidthPt500',
##moments##
#'CentroidR',
#'WidthPhi'

from JetRec.JetRecStandard import jtm
from JetRec.JetRecConf import JetAlgorithm

from JetRec.JetRecFlags import jetFlags
from DerivationFrameworkHI.HIDerivationFlags import HIDerivationFlags
if HIDerivationFlags.isSimulation() : jetFlags.useTruth.set_Value_and_Lock(True)
#topo_upc_mods = jtm.modifiersMap["calib_topo_ungroomed"]
#print topo_upc_mods
#if jetFlags.useTruth():
#    truth_upc_mods = jtm.modifiersMap["truth_ungroomed"]
#    print truth_upc_mods
#skipmods = ["ktdr","nsubjettiness","ktsplitter","angularity","dipolarity","planarflow","ktmassdrop","encorr","comshapes"]
#for mod in skipmods:
#    print "remove", mod
#    topo_upc_mods.remove(jtm.tools[mod])
#    if jetFlags.useTruth(): truth_upc_mods.remove(jtm.tools[mod])
#jtm.modifiersMap["topo_upc"] = topo_upc_mods
#if jetFlags.useTruth(): jtm.modifiersMap["truth_upc"] = truth_upc_mods

OutputJetsUPC = {}
def addStandardJetsUPC(jetalg, rsize, inputtype, ptmin=2000, ptminFilter=5000,
                       mods="calib", calibOpt="none", ghostArea=0.01,
                       algseq=None, outputGroup="CustomJets"):
    jetnamebase = "{0}{1}{2}".format(jetalg,int(rsize*10),inputtype)
    jetname = jetnamebase+"Jets"
    algname = "jetalg"+jetnamebase
    OutputJetsUPC.setdefault(outputGroup , [] ).append(jetname)

    if not jetname in jtm.tools:
        # Set default for the arguments to be passd to addJetFinder
        finderArgs = dict( modifiersin= [], consumers = [])
        finderArgs['ptmin'] = ptmin
        finderArgs['ptminFilter'] = ptminFilter
        finderArgs['ghostArea'] = ghostArea
        # no container exist. simply build a new one.
        if inputtype=="LCTopo" or inputtype=="EMTopo" or inputtype == "EMPFlow" or inputtype == "EMCPFlow":
            finderArgs['modifiersin'] = mods
            finderArgs['calibOpt'] = "none"
        #finderArgs.pop('modifiersin') # leave the default modifiers.
    
        # map the input to the jtm code for PseudoJetGetter
        getterMap = dict( LCTopo = 'lctopo', EMTopo = 'emtopo', EMPFlow = 'empflow', EMCPFlow = 'emcpflow', Truth='truth', TruthWZ='truthwz', PV0Track='pv0track')
        # create the finder for the temporary collection.
        finderTool= jtm.addJetFinder(jetname, jetalg, rsize, getterMap[inputtype],**finderArgs)

        from JetRec.JetRecConf import JetAlgorithm
        alg = JetAlgorithm(algname, Tools = [finderTool])
        print "   Added", algname, "to sequence", algseq
        algseq += alg

def addUPCJets(jetalg,radius,inputtype,sequence,outputlist):
    jetname = "{0}{1}{2}Jets".format(jetalg,int(radius*10),inputtype)
    algname = "jetalg"+jetname
    
    if not hasattr(sequence,algname):
        if inputtype == "Truth": 
            addStandardJetsUPC(jetalg, radius, "Truth", mods="truth_upc", ptmin=5000, algseq=sequence, outputGroup=outputlist)
        else:
            addStandardJetsUPC(jetalg, radius, inputtype, mods="topo_upc",
                               ghostArea=0.01, ptmin=2000, ptminFilter=5000, calibOpt="o", algseq=sequence, outputGroup=outputlist)

#99% trigger efficiency points for different items
HITriggerDict = {'HLT_j50_ion_L1TE20':  68, 'HLT_j60_ion_L1TE50': 79, 'HLT_j75_ion_L1TE50': 89}
ppTriggerDict = {'HLT_j30_L1TE5': 34,'HLT_j40_L1TE10':  44, 'HLT_j50_L1J12': 59, 'HLT_j60_L1J15': 70, 'HLT_j75_L1J20': 79, 'HLT_j85': 89}

HI18TriggerDict = {
'HLT_mb_sptrk_L1ZDC_A_C_VTE50': 20,
'HLT_noalg_pc_L1TE50_VTE600.0ETA49': 20,
'HLT_noalg_cc_L1TE600.0ETA49': 20,
'HLT_mb_sptrk': 20,
'HLT_noalg_mb_L1TE50': 20,
'HLT_j50_ion_L1J12': 50,
'HLT_j60_ion_L1J15': 60,
'HLT_j60_ion_L1J20': 60,
'HLT_j75_ion_L1J20': 75,
'HLT_j75_ion_L1J30': 75,
'HLT_j85_ion_L1J20': 85,
'HLT_j85_ion_L1J30': 85,
'HLT_j100_ion_L1J20': 100,
'HLT_j100_ion_L1J30': 100,
'HLT_j110_ion_L1J30': 110,
'HLT_j110_ion_L1J50': 110,
'HLT_j120_ion_L1J30': 120,
'HLT_j120_ion_L1J50': 120,
'HLT_j130_ion_L1J30': 130,
'HLT_j150_ion_L1J30': 150,
'HLT_j150_ion_L1J50': 150,
'HLT_j180_ion_L1J50': 180,
'HLT_j200_ion_L1J50': 200}










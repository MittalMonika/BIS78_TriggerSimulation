#
#  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import INFO, DEBUG,VERBOSE

### Output Name ###
muFastInfo = "MuonL2SAInfo"

# Get Rpc data decoder for MuFast data preparator 
def RpcDataPreparatorCfg( flags, roisKey ):

    acc = ComponentAccumulator()

    # Get BS decoder 
    from MuonConfig.MuonBytestreamDecodeConfig import RpcBytestreamDecodeCfg
    rpcAcc = RpcBytestreamDecodeCfg( flags, forTrigger=True )
    rpcAcc.getEventAlgo("RpcRawDataProvider").RoIs = roisKey
    acc.merge( rpcAcc )

    # Get BS->RDO convertor
    from MuonConfig.MuonRdoDecodeConfig import RpcRDODecodeCfg    
    rpcAcc = RpcRDODecodeCfg( flags, forTrigger=True )
    rpcAcc.getEventAlgo("RpcRdoToRpcPrepData").RoIs = roisKey
    acc.merge( rpcAcc )

    # Set Rpc data preparator for MuFast data preparator
    from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__RpcDataPreparator
    RpcDataPreparator = TrigL2MuonSA__RpcDataPreparator( RpcPrepDataProvider  = acc.getPublicTool( "RpcRdoToRpcPrepDataTool" ),
                                                         RpcRawDataProvider   = acc.getPublicTool( "RPC_RawDataProviderTool" ),
                                                         #DecodeBS = DetFlags.readRDOBS.RPC_on() ) # This should be used flags
                                                         DecodeBS = True )
    acc.addPublicTool( RpcDataPreparator, primary=True ) # Now this is needed, but should be removed
 
    return acc, RpcDataPreparator

# Get Tgc data decoder for MuFast data preparator 
def TgcDataPreparatorCfg( flags, roisKey ):

    acc = ComponentAccumulator()

    # Get BS decoder 
    from MuonConfig.MuonBytestreamDecodeConfig import TgcBytestreamDecodeCfg
    tgcAcc = TgcBytestreamDecodeCfg( flags, forTrigger=True )
    #TgcRawDataProvider.RoIs = roisKey
    acc.merge( tgcAcc )

    # Get BS->RDO convertor
    from MuonConfig.MuonRdoDecodeConfig import TgcRDODecodeCfg    
    tgcAcc = TgcRDODecodeCfg( flags, forTrigger=True )
    tgcAcc.getEventAlgo("TgcRdoToTgcPrepData").RoIs = roisKey
    acc.merge( tgcAcc )

    # Set Tgc data preparator for MuFast data preparator
    from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__TgcDataPreparator
    TgcDataPreparator = TrigL2MuonSA__TgcDataPreparator( TgcPrepDataProvider  = acc.getPublicTool( "TgcRdoToTgcPrepDataTool" ) )
                                                         #DecodeBS = DetFlags.readRDOBS.TGC_on() ) # This should be used flags
    acc.addPublicTool( TgcDataPreparator, primary=True ) # This should be removed
 
    return acc, TgcDataPreparator

# Get Mdt data decoder for MuFast data preparator 
def MdtDataPreparatorCfg( flags, roisKey ):

    acc = ComponentAccumulator()

    # Get BS decoder 
    from MuonConfig.MuonBytestreamDecodeConfig import MdtBytestreamDecodeCfg
    mdtAcc = MdtBytestreamDecodeCfg( flags, forTrigger=True )
    #MdtRawDataProvider.RoIs = roisKey
    acc.merge( mdtAcc )

    # Get BS->RDO convertor
    from MuonConfig.MuonRdoDecodeConfig import MdtRDODecodeCfg    
    mdtAcc = MdtRDODecodeCfg( flags, forTrigger=True )
    mdtAcc.getEventAlgo("MdtRdoToMdtPrepData").RoIs = roisKey
    acc.merge( mdtAcc )

    # Set Mdt data preparator for MuFast data preparator
    from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__MdtDataPreparator
    MdtDataPreparator = TrigL2MuonSA__MdtDataPreparator( MdtPrepDataProvider  = acc.getPublicTool( "MdtRdoToMdtPrepDataTool" ),
                                                         MDT_RawDataProvider   = acc.getPublicTool( "MDT_RawDataProviderTool" ),
                                                         #DecodeBS = DetFlags.readRDOBS.MDT_on() ) # This should be used flags
                                                         DecodeBS = True )
    acc.addPublicTool( MdtDataPreparator, primary=True ) # This should be removed
 
    return acc, MdtDataPreparator

# Get Csc data decoder for MuFast data preparator 
def CscDataPreparatorCfg( flags, roisKey ):

    acc = ComponentAccumulator()

    # Get BS decoder 
    from MuonConfig.MuonBytestreamDecodeConfig import CscBytestreamDecodeCfg
    cscAcc = CscBytestreamDecodeCfg( flags, forTrigger=True )
    #CscRawDataProvider.RoIs = roisKey
    acc.merge( cscAcc )

    # Get BS->RDO convertor
    from MuonConfig.MuonRdoDecodeConfig import CscRDODecodeCfg    
    cscAcc = CscRDODecodeCfg( flags, forTrigger=True )
    cscAcc.getEventAlgo("CscRdoToCscPrepData").RoIs = roisKey
    acc.merge( cscAcc )

    # Get cluster builder
    from MuonConfig.MuonRdoDecodeConfig import CscClusterBuildCfg
    cscAcc = CscClusterBuildCfg( flags, forTrigger=True )
    acc.merge( cscAcc )

    # Set Csc data preparator for MuFast data preparator
    from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__CscDataPreparator
    CscDataPreparator = TrigL2MuonSA__CscDataPreparator( CscPrepDataProvider  = acc.getPublicTool( "CscRdoToCscPrepDataTool" ),
                                                         CscClusterProvider   = acc.getPublicTool( "CscThesholdClusterBuilderTool" ) )
                                                         #DecodeBS = DetFlags.readRDOBS.CSC_on() ) # This should be used flags
    acc.addPublicTool( CscDataPreparator, primary=True ) # This should be removed
 
    return acc, CscDataPreparator

# Configure Reco alg of muFast step
def muFastSteeringCfg( flags, roisKey ):

    acc = ComponentAccumulator()

    # Get RPC decoder
    rpcAcc, RpcDataPreparator = RpcDataPreparatorCfg( flags, roisKey )
    acc.merge( rpcAcc )

    # Get TGC decoder
    tgcAcc, TgcDataPreparator = TgcDataPreparatorCfg( flags, roisKey )
    acc.merge( tgcAcc )

    # Get MDT decoder
    mdtAcc, MdtDataPreparator = MdtDataPreparatorCfg( flags, roisKey )
    acc.merge( mdtAcc )

    # Get CSC decoder
    cscAcc, CscDataPreparator = CscDataPreparatorCfg( flags, roisKey )
    acc.merge( cscAcc )

    # Set MuFast data preparator
    from TrigL2MuonSA.TrigL2MuonSAConf import TrigL2MuonSA__MuFastDataPreparator
    MuFastDataPreparator = TrigL2MuonSA__MuFastDataPreparator( CSCDataPreparator = CscDataPreparator,
                                                               MDTDataPreparator = MdtDataPreparator,
                                                               RPCDataPreparator = RpcDataPreparator,
                                                               TGCDataPreparator = TgcDataPreparator )

    # Set Reco alg of muFast step
    from TrigL2MuonSA.TrigL2MuonSAConf import MuFastSteering
    muFastAlg = MuFastSteering( name                          = "MuFastSteering_Muon",
                                DataPreparator                = MuFastDataPreparator,
                                R_WIDTH_TGC_FAILED            = 200,
                                R_WIDTH_RPC_FAILED            = 400,
                                DoCalibrationStream           = False,
                                USE_ROIBASEDACCESS_CSC        = True,
                                RpcErrToDebugStream           = True,
                                Timing                        = False,
                                MonTool                       = "",
                                # TriggerFlags.run2Config != '2016'
                                UseEndcapInnerFromBarrel      = True, 
                                # not 900 GeV
                                WinPt                         = 6.0,
                                Scale_Road_BarrelInner        = 1,
                                Scale_Road_BarrelMiddle       = 1,
                                Scale_Road_BarrelOuter        = 1 )  
    return acc, muFastAlg

def PtBarrelLUTSvcCfg( flags ):

    acc = ComponentAccumulator()
    from TrigL2MuonSA.TrigL2MuonSAConfig import PtBarrelLUTSvc
    ptBarrelLUTSvc = PtBarrelLUTSvc()
    acc.addService( ptBarrelLUTSvc )

    return acc, ptBarrelLUTSvc

def PtBarrelLUTSvcCfg_MC( flags ):

    acc = ComponentAccumulator()
    from TrigL2MuonSA.TrigL2MuonSAConfig import PtBarrelLUTSvc_MC
    ptBarrelLUTSvc_MC = PtBarrelLUTSvc_MC() 
    acc.addService( ptBarrelLUTSvc_MC )

    return acc, ptBarrelLUTSvc_MC

def PtEndcapLUTSvcCfg( flags ):

    acc = ComponentAccumulator()
    from TrigL2MuonSA.TrigL2MuonSAConfig import PtEndcapLUTSvc
    ptEndcapLUTSvc = PtEndcapLUTSvc()
    acc.addService( ptEndcapLUTSvc )

    return acc, ptEndcapLUTSvc

def PtEndcapLUTSvcCfg_MC( flags ):

    acc = ComponentAccumulator()
    from TrigL2MuonSA.TrigL2MuonSAConfig import PtEndcapLUTSvc_MC
    ptEndcapLUTSvc_MC = PtEndcapLUTSvc_MC() 
    acc.addService( ptEndcapLUTSvc_MC )

    return acc, ptEndcapLUTSvc_MC


def AlignmentBarrelLUTSvcCfg( flags ):

    acc = ComponentAccumulator()
    from TrigL2MuonSA.TrigL2MuonSAConfig import AlignmentBarrelLUTSvc
    alignmentBarrelLUTSvc = AlignmentBarrelLUTSvc()
    acc.addService( alignmentBarrelLUTSvc )

    return acc, alignmentBarrelLUTSvc

# In the future, above functions should be moved to TrigL2MuonSA package(?)

def l2MuFastAlgCfg( flags, roisKey="MURoIs" ):

    acc = ComponentAccumulator()

    # Get Reco alg of muFast step
    muFastAcc, muFastFex = muFastSteeringCfg( flags, roisKey )  
    muFastFex.MuRoIs = roisKey
    muFastFex.RecMuonRoI = "RecMURoIs"
    muFastFex.MuonL2SAInfo = muFastInfo
    muFastFex.forID = "forID"
    muFastFex.forMS = "forMS"
    muFastFex.OutputLevel = DEBUG
    acc.merge( muFastAcc )   

    # Get services of the Reco alg
    acc.merge( PtBarrelLUTSvcCfg(flags)[0] )   
    acc.merge( PtBarrelLUTSvcCfg_MC(flags)[0] )   
    acc.merge( PtEndcapLUTSvcCfg(flags)[0] )   
    acc.merge( PtEndcapLUTSvcCfg_MC(flags)[0] )   
    acc.merge( AlignmentBarrelLUTSvcCfg(flags)[0] )

    return acc, muFastFex


def l2MuFastRecoCfg( flags ):

    # Set EventViews for muFast step
    from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import InViewReco
    reco = InViewReco("L2MuFastReco")

    # Get Reco alg of muFast Step in order to set into the view
    algAcc, alg = l2MuFastAlgCfg( flags, roisKey=reco.name+"RoIs")

    reco.addRecoAlg( alg )
    reco.merge( algAcc )

    return reco

def l2MuFastHypoCfg( flags, name="UNSPECIFIED", muFastInfo="UNSPECIFIED" ):

    from TrigMuonHypoMT.TrigMuonHypoMTConf import TrigMufastHypoAlg
    muFastHypo = TrigMufastHypoAlg( name )
    muFastHypo.MuonL2SAInfoFromMuFastAlg = muFastInfo 

    return muFastHypo
 

def generateMuonsCfg( flags ):

    acc = ComponentAccumulator()
    
    from TriggerMenuMT.HLTMenuConfig.Menu.MenuComponents import MenuSequence, ChainStep, Chain, RecoFragmentsPool

    # Set Hypo alg of muFast step
    from TrigMuonHypoMT.TrigMuonHypoMTConf import TrigMufastHypoAlg
    from TrigMuonHypoMT.TrigMuonHypoMTConfig import TrigMufastHypoToolFromName
    muFastHypo = TrigMufastHypoAlg("TrigL2MuFastHypo")
    muFastHypo.MuonL2SAInfoFromMuFastAlg = muFastInfo

    # Set Reco alg of muFast step
    l2MuFastReco = RecoFragmentsPool.retrieve( l2MuFastRecoCfg, flags )
    acc.merge( l2MuFastReco )

    # Set muFast step
    muFastSeq = MenuSequence( Sequence    = l2MuFastReco.sequence(),
                              Maker       = l2MuFastReco.inputMaker(),
                              Hypo        = muFastHypo,
                              HypoToolGen = TrigMufastHypoToolFromName )

    muFastStep = ChainStep("Muon_muFastStep1", [muFastSeq])

    chains = [ Chain(c.split()[0], c.split()[1], [muFastStep]) for c in flags.Trigger.menu.muon ]

    return acc, chains


from AthenaCommon.Constants import ERROR, DEBUG
from AthenaCommon.CFElements import seqAND, parOR
from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm

def setMinimalCaloSetup() :
  from AthenaCommon.AppMgr import ServiceMgr as svcMgr
  if not hasattr(svcMgr,'TrigCaloDataAccessSvc'):
    from TrigT2CaloCommon.TrigT2CaloCommonConf import TrigCaloDataAccessSvc
    svcMgr+=TrigCaloDataAccessSvc()
    svcMgr.TrigCaloDataAccessSvc.OutputLevel=ERROR


########################
## ALGORITHMS
# defined as private within this module, so that they can be configured only in functions in this module
########################

def _algoHLTCaloCell(name="HLTCaloCellMaker", inputEDM='FSRoI', outputEDM='CellsClusters', RoIMode=True, OutputLevel=ERROR) :
   setMinimalCaloSetup();
   from AthenaCommon.AppMgr import ServiceMgr as svcMgr
   from TrigCaloRec.TrigCaloRecConfig import HLTCaloCellMaker
   algo=HLTCaloCellMaker(name)
   #"HLTCaloCellMaker"
   algo.RoIs=inputEDM
   algo.TrigDataAccessMT=svcMgr.TrigCaloDataAccessSvc
   algo.OutputLevel=OutputLevel
   algo.CellsName=outputEDM
   return algo;

def _algoHLTTopoCluster(inputEDM="CellsClusters", OutputLevel=ERROR) :
   from TrigCaloRec.TrigCaloRecConfig import TrigCaloClusterMakerMT_topo
   algo = TrigCaloClusterMakerMT_topo(doMoments=True, doLC=False, cells=inputEDM)
   algo.CaloClusters="caloclusters"
   algo.OutputLevel=OutputLevel
   return algo

def _algoL2Egamma(inputEDM="EMRoIs",OutputLevel=ERROR):
    setMinimalCaloSetup();
    from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import T2CaloEgamma_ReFastAlgo
    algo=T2CaloEgamma_ReFastAlgo("FastCaloL2EgammaAlg")
    algo.RoIs=inputEDM
    algo.ClustersName="L2CaloClusters" # defalut value, added for debugging
    algo.OutputLevel=OutputLevel
    return algo


####################################
##### SEQUENCES
####################################

def fastCaloRecoSequence(InViewRoIs):
    fastCaloAlg = _algoL2Egamma(OutputLevel=DEBUG,inputEDM=InViewRoIs)
    fastCaloInViewSequence = seqAND( 'fastCaloInViewSequence', [fastCaloAlg] )
    sequenceOut = fastCaloAlg.ClustersName
    return (fastCaloInViewSequence, sequenceOut)


def fastCaloEVCreator():   
    InViewRoIs="EMCaloRoIs"     
    fastCaloViewsMaker = EventViewCreatorAlgorithm( "fastCaloViewsMaker", OutputLevel=DEBUG)
    fastCaloViewsMaker.ViewFallThrough = True
    fastCaloViewsMaker.RoIsLink = "initialRoI"
    fastCaloViewsMaker.InViewRoIs = InViewRoIs
    fastCaloViewsMaker.Views = "EMCaloViews"
    fastCaloViewsMaker.ViewNodeName = "fastCaloInViewSequence"
    return (fastCaloViewsMaker, InViewRoIs)


def createFastCaloSequence(EMRoIDecisions):
    """Used for standalone testing"""
    (fastCaloViewsMaker, InViewRoIs) = fastCaloEVCreator()
    # connect to RoIs
    fastCaloViewsMaker.InputMakerInputDecisions =  [ EMRoIDecisions ]         
    fastCaloViewsMaker.InputMakerOutputDecisions = [ EMRoIDecisions + "IMOUTPUT"]

    (fastCaloInViewSequence, sequenceOut) = fastCaloRecoSequence(InViewRoIs)
     
    fastCaloSequence = seqAND("fastCaloSequence", [fastCaloViewsMaker, fastCaloInViewSequence ])
    return (fastCaloSequence, sequenceOut)

##################################
# cluster maker functions
###################################

def clusterFSInputMaker( ):
  """ Creates the inputMaker for FS in menu"""
  RoIs = 'FSJETRoI'
  from DecisionHandling.DecisionHandlingConf import InputMakerForRoI
  InputMakerAlg = InputMakerForRoI("clusterFSInputMaker", OutputLevel = DEBUG, RoIsLink="initialRoI")
  InputMakerAlg.RoIs=RoIs
  return InputMakerAlg


def HLTCellMaker(RoIs='FSJETRoI'):
    # in standalone mode: FSRoI
    CellsClusters = 'CaloCells'
    # setting this value does not work: 'CellsClusters'
    cellMakerAlgo = _algoHLTCaloCell(name="HLTCaloCellMaker", inputEDM=RoIs, outputEDM=CellsClusters, RoIMode=True, OutputLevel=DEBUG)
    return cellMakerAlgo

def HLTFSCellMakerRecoSequence(RoIs='FSJETRoI'):
    cellMake = HLTCellMaker(RoIs)
    RecoSequence = parOR("ClusterRecoSequence", [cellMake])
    return (RecoSequence, cellMake.CellsName)
   
 
def HLTFSTopoRecoSequence(RoIs='FSJETRoI'):
    cellMake = HLTCellMaker(RoIs)
    topoClusterMaker = _algoHLTTopoCluster(inputEDM = cellMake.CellsName, OutputLevel=DEBUG) 
    RecoSequence = parOR("TopoClusterRecoSequence", [cellMake, topoClusterMaker])
    print topoClusterMaker
    for tool in topoClusterMaker.ClusterMakerTools:
        print tool

    return (RecoSequence, topoClusterMaker.CaloClusters)

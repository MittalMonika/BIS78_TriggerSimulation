# ------------------------------------------------------------
#
# ----------- loading the selection tools for Calo Seeded Brem
#
# ------------------------------------------------------------

#
# --- get the builder tool
#
from InDetCaloClusterROIBuilder.InDetCaloClusterROIBuilderConf import InDet__CaloClusterROI_Builder
InDetCaloClusterROIBuilder = InDet__CaloClusterROI_Builder(name = "InDetCaloClusterROIBuilder")

if (InDetFlags.doPrintConfigurables()):
    print InDetCaloClusterROIBuilder

#
# --- now load the algorithm
#
from InDetCaloClusterROISelector.InDetCaloClusterROISelectorConf import InDet__CaloClusterROI_Selector
InDetHadCaloClusterROISelector = InDet__CaloClusterROI_Selector (name                         = "InDetHadCaloClusterROISelector",
                                                              InputClusterContainerName    = InDetKeys.HadCaloClusterContainer(),    # "LArClusterEM"
                                                              OutputClusterContainerName   = InDetKeys.HadCaloClusterROIContainer(), # "InDetCaloClusterROIs"
                                                              ClusterEtCut                 = 25000,
                                                              CaloClusterROIBuilder        = InDetCaloClusterROIBuilder)

topSequence += InDetHadCaloClusterROISelector
if (InDetFlags.doPrintConfigurables()):
    print InDetHadCaloClusterROISelector


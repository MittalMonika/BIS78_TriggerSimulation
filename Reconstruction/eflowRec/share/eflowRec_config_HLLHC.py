from eflowRec.eflowRecFlags import jobproperties

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

#Jet/MET algorithms - EM input and LC output

#Same as for Run2 so can use EM version
from eflowRec.eflowPreparationGetter import eflowPreparationGetter
CaloObjectBuilderGetter = eflowPreparationGetter()

#Need HLLHC specific items, so use HLLHC version
from eflowRec.eflowCaloObjectBuilderGetter import eflowCaloObjectBuilderGetter_HLLHC
ObjectBuilderToolsGetter = eflowCaloObjectBuilderGetter()

#Need HLLHC specific items, so use HLLHC version
from eflowRec.eflowObjectBuilderGetter import eflowObjectBuilderGetter_HLLHC
ObjectBuilderGetter = eflowObjectBuilderGetter()

if not jobproperties.eflowRecFlags.eflowAlgType == "EOverP":
    from eflowRec.eflowRecConf import eflowVertexInformationSetter
    eflowVertexInformationSetter = eflowVertexInformationSetter("eflowVertextInformationSetter_EM")
    topSequence += eflowVertexInformationSetter

    from eflowRec.eflowRecConf import eflowOverlapRemoval
    eflowOverlapRemoval = eflowOverlapRemoval("eflowOverlapRemoval_EM")
    topSequence += eflowOverlapRemoval

# $Id$
#
# @file egammaD3PDMaker/share/ClusterCorrectionD3PD_topOptions.py
# @author scott snyder <snyder@bnl.gov>
# @date Oct 2012
# @brief Top-level JO for making cluster correction d3pd outside of reco_trf.
#


###################################################################3
# Define the input file here.
#

from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.FilesInput= ["AOD.pool.root"]


###################################################################3
# Define the output file here.
#

if not globals().get('tupleFileOutput'):
    tupleFileOutput = 'clusters.root'

from D3PDMakerConfig.D3PDProdFlags import prodFlags
prodFlags.WriteClusterCorrectionD3PD = True
prodFlags.WriteClusterCorrectionD3PD.FileName = tupleFileOutput
prodFlags.WriteClusterCorrectionD3PD.lock()

###################################################################3
# Define other job options here.
#

athenaCommonFlags.EvtMax = -1

from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
D3PDMakerFlags.DoTrigger = False


###################################################################
# Configure RecExCommon.
#

from RecExConfig.RecFlags import rec
rec.DPDMakerScripts.append( "egammaD3PDMaker/ClusterCorrectionD3PD_prodJobOFragment.py" )
rec.doCBNT.set_Value_and_Lock( False )
rec.doWriteTAG.set_Value_and_Lock( False )

# Block loading conditions folders we won't need.
blocked_folders = [
    '/CALO/Identifier/CaloTTOnAttrIdMapAtlas',
    '/CALO/Identifier/CaloTTOnOffIdMapAtlas',
    '/CALO/Identifier/CaloTTPpmRxIdMapAtlas',
    '/LAR/ElecCalibMC/AutoCorr',
    '/LAR/ElecCalibMC/DAC2uA',
    '/LAR/ElecCalibMC/HVScaleCorr',
    '/LAR/ElecCalibMC/MinBias',
    '/LAR/ElecCalibMC/MinBiasAverage',
    '/LAR/ElecCalibMC/MphysOverMcal',
    '/LAR/ElecCalibMC/Noise',
    '/LAR/ElecCalibMC/Pedestal',
    '/LAR/ElecCalibMC/Ramp',
    '/LAR/ElecCalibMC/Shape',
    '/LAR/ElecCalibMC/fSampl',
    '/LAR/ElecCalibMC/uA2MeV',
    '/LAR/Identifier/LArTTCellMapAtlas',
    '/MDT/DCS/DROPPEDCH',
    '/MDT/DCS/PSLVCHSTATE',
    '/PIXEL/PixdEdx',
    '/TILE/OFL02/NOISE/AUTOCR',
    '/TILE/OFL02/PULSESHAPE/PHY',
    '/TILE/ONL01/FILTER/OF2/PHY',
    '/TILE/ONL01/NOISE/OFNI',
    '/TRT/Calib/HTCalib',
    '/TRT/Calib/PID',
    '/TRT/Calib/PID_RToT',
    '/TRT/Calib/PID_RToTver_New',
    '/TRT/Calib/PIDver_New',
    '/TRT/Calib/ToTCalib',
    '/TRT/Cond/Status',
    '/TRT/Cond/StatusPermanent',
    '/CALO/H1Weights/H1WeightsKt4Topo',
    '/Indet/PixelDist',
    '/Indet/TrkErrorScaling',
    ]
from IOVDbSvc.CondDB import conddb
for f in blocked_folders:
    conddb.blockFolder (f)


include( "RecExCommon/RecExCommon_topOptions.py" )

include.block ("MuonByteStreamCnvTest/jobOptions_MuonRDOToDigit.py")

from AthenaCommon.CfgGetter import getPublicTool

from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags
from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags

## configure the tool

from MuonByteStreamCnvTest.MuonByteStreamCnvTestConf import MuonRdoToMuonDigitTool
MuonRdoToMuonDigitTool = MuonRdoToMuonDigitTool (
                         DecodeMdtRDO = True,
                         DecodeRpcRDO = True,
                         DecodeTgcRDO = True,
                         DecodeCscRDO = MuonGeometryFlags.hasCSC(),
                         DecodeSTGC_RDO = (CommonGeometryFlags.Run() in ["RUN3", "RUN4"]),
                         DecodeMM_RDO = (CommonGeometryFlags.Run() in ["RUN3", "RUN4"]),
                         cscRdoDecoderTool=("Muon::CscRDO_Decoder" if MuonGeometryFlags.hasCSC() else ""),
                         stgcRdoDecoderTool=("Muon::STGC_RDO_Decoder" if (CommonGeometryFlags.Run() in ["RUN3", "RUN4"]) else ""),
                         mmRdoDecoderTool=("Muon::MM_RDO_Decoder" if (CommonGeometryFlags.Run() in ["RUN3", "RUN4"]) else "")
                         )
			 
if MuonGeometryFlags.hasCSC(): MuonRdoToMuonDigitTool.cscCalibTool = getPublicTool("CscCalibTool")

ToolSvc += MuonRdoToMuonDigitTool

## configure the algorithm
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from MuonByteStreamCnvTest.MuonByteStreamCnvTestConf import MuonRdoToMuonDigit
topSequence += MuonRdoToMuonDigit( "MuonRdoToMuonDigit" )
topSequence.MuonRdoToMuonDigit.MuonRdoToMuonDigitTool = MuonRdoToMuonDigitTool


/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "ALFA_Ntuple/ALFA_Ntuple.h"
#include "AthenaKernel/errorcheck.h"
#include <typeinfo>
#include <numeric>

void BLM::clear()
{
	iTimeIOVstart = 0;
	iTimeIOVstop  = 0;
	memset(&fBLM, 0.0, sizeof(fBLM));
}

void HVCHANNEL::clear()
{
	iTimeIOVstart = 0;
	iTimeIOVstop  = 0;
	memset(&fActualVMeas, 0.0, sizeof(fActualVMeas));
	memset(&fActualIMeas, 0.0, sizeof(fActualIMeas));
}

void LOCALMONITORING::clear()
{
	iTimeIOVstart = 0;
	iTimeIOVstop  = 0;
	memset(&fTempSensor1, 0.0, sizeof(fTempSensor1));
	memset(&fTempSensor2, 0.0, sizeof(fTempSensor2));
	memset(&fTempSensor3, 0.0, sizeof(fTempSensor3));
	memset(&fTempSensor4, 0.0, sizeof(fTempSensor4));
	memset(&fTempSensor5, 0.0, sizeof(fTempSensor5));
}

void MOVEMENT::clear()
{
	iTimeIOVstart = 0;
	iTimeIOVstop  = 0;
	memset(&fPosLVDT, 0.0, sizeof(fPosLVDT));
	memset(&fPosMotor, 0.0, sizeof(fPosMotor));
}

void RADMON::clear()
{
	iTimeIOVstart = 0;
	iTimeIOVstop  = 0;
	memset(&fDose, 0.0, sizeof(fDose));
	memset(&fFluence, 0.0, sizeof(fFluence));
	memset(&fTemp, 0.0, sizeof(fTemp));
}

void TRIGGERRATE::clear()
{
	iTimeIOVstart = 0;
	iTimeIOVstop  = 0;
	memset(&iTRate, 0.0, sizeof(iTRate));
}

void BPMALFA::clear()
{
	iTimeIOVstart = 0;
	iTimeIOVstop  = 0;
	bpmr_r_x_pos  = 0.0;
	bpmr_r_y_pos  = 0.0;
	bpmr_r_x_err  = 0.0;
	bpmr_r_y_err  = 0.0;
	bpmr_l_x_pos  = 0.0;
	bpmr_l_y_pos  = 0.0;
	bpmr_l_x_err  = 0.0;
	bpmr_l_y_err  = 0.0;
	bpmsa_r_x_pos = 0.0;
	bpmsa_r_y_pos = 0.0;
	bpmsa_r_x_err = 0.0;
	bpmsa_r_y_err = 0.0;
	bpmsa_l_x_pos = 0.0;
	bpmsa_l_y_pos = 0.0;
	bpmsa_l_x_err = 0.0;
	bpmsa_l_y_err = 0.0;
	bpmwb_r_x_pos = 0.0;
	bpmwb_r_y_pos = 0.0;
	bpmwb_r_x_err = 0.0;
	bpmwb_r_y_err = 0.0;
	bpmya_l_x_pos = 0.0;
	bpmya_l_y_pos = 0.0;
	bpmya_l_x_err = 0.0;
	bpmya_l_y_err = 0.0;
}

void ALGOTREEMD::clear()
{
	iAlgoID   = 0;
	iNumTrack = 0;
	memset(&iDetector, 0, sizeof(iDetector));
	memset(&bRecFlag, false, sizeof(bRecFlag));
	fill_n(&fXDet[0][0], sizeof(fXDet)/sizeof(Double_t), -9999.0);
	fill_n(&fYDet[0][0], sizeof(fYDet)/sizeof(Double_t), -9999.0);
	fill_n(&fXPot[0][0], sizeof(fXPot)/sizeof(Double_t), -9999.0);
	fill_n(&fYPot[0][0], sizeof(fYPot)/sizeof(Double_t), -9999.0);
	fill_n(&fXStat[0][0], sizeof(fXStat)/sizeof(Double_t), -9999.0);
	fill_n(&fYStat[0][0], sizeof(fYStat)/sizeof(Double_t), -9999.0);
	fill_n(&fXLHC[0][0], sizeof(fXLHC)/sizeof(Double_t), -9999.0);
	fill_n(&fYLHC[0][0], sizeof(fYLHC)/sizeof(Double_t), -9999.0);
	fill_n(&fZLHC[0][0], sizeof(fZLHC)/sizeof(Double_t), -9999.0);
	fill_n(&fXBeam[0][0], sizeof(fXBeam)/sizeof(Double_t), -9999.0);
	fill_n(&fYBeam[0][0], sizeof(fYBeam)/sizeof(Double_t), -9999.0);
	fill_n(&iFibSelMD[0][0][0], sizeof(iFibSelMD)/sizeof(Int_t), -9999);
	fill_n(&fInterceptDet[0][0], sizeof(fInterceptDet)/sizeof(Double_t), -9999.0);
	fill_n(&fSlopeDet[0][0], sizeof(fSlopeDet)/sizeof(Double_t), -9999.0);
	fill_n(&fOverU[0][0], sizeof(fOverU)/sizeof(Double_t), -9999.0);
	fill_n(&fOverV[0][0], sizeof(fOverV)/sizeof(Double_t), -9999.0);
	fill_n(&iNumU[0][0], sizeof(iNumU)/sizeof(Int_t), -9999);
	fill_n(&iNumV[0][0], sizeof(iNumV)/sizeof(Int_t), -9999);

}

void ALGOTREEOD::clear()
{
	iAlgoID   = 0;
	iNumTrack = 0;
	memset(&iDetector, 0, sizeof(iDetector));
	memset(&bRecFlag, false, sizeof(bRecFlag));
	fill_n(&iFibSelOD[0][0][0], sizeof(iFibSelOD)/sizeof(Int_t), -9999);
	fill_n(&fXDet[0][0], sizeof(fXDet)/sizeof(Double_t), -9999.0);
	fill_n(&fYDet[0][0], sizeof(fYDet)/sizeof(Double_t), -9999.0);
	fill_n(&fXPot[0][0], sizeof(fXPot)/sizeof(Double_t), -9999.0);
	fill_n(&fYPot[0][0], sizeof(fYPot)/sizeof(Double_t), -9999.0);
	fill_n(&fXStat[0][0], sizeof(fXStat)/sizeof(Double_t), -9999.0);
	fill_n(&fYStat[0][0], sizeof(fYStat)/sizeof(Double_t), -9999.0);
	fill_n(&fXLHC[0][0], sizeof(fXLHC)/sizeof(Double_t), -9999.0);
	fill_n(&fYLHC[0][0], sizeof(fYLHC)/sizeof(Double_t), -9999.0);
	fill_n(&fZLHC[0][0], sizeof(fZLHC)/sizeof(Double_t), -9999.0);
	fill_n(&fXBeam[0][0], sizeof(fXBeam)/sizeof(Double_t), -9999.0);
	fill_n(&fYBeam[0][0], sizeof(fYBeam)/sizeof(Double_t), -9999.0);
	fill_n(&fInterceptDet[0][0], sizeof(fInterceptDet)/sizeof(Double_t), -9999.0);
	fill_n(&fSlopeDet[0][0], sizeof(fSlopeDet)/sizeof(Double_t), -9999.0);
	fill_n(&fOverY[0][0], sizeof(fOverY)/sizeof(Double_t), -9999.0);
	fill_n(&iNumY[0][0], sizeof(iNumY)/sizeof(Int_t), -9999);
}

ALFA_Ntuple::ALFA_Ntuple(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator), m_iovSvc( "IOVDbSvc", name ),
m_pDetStore(0)
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::ALFA_Ntuple()");
	LogStream << MSG::DEBUG << "begin ALFA_Ntuple::ALFA_Ntuple()" << endreq;

	m_bCoolData = true;
	declareProperty("CoolData", m_bCoolData);

	m_strCollectionName       = "ALFA_DigitCollection";
	m_strODCollectionName     = "ALFA_ODDigitCollection";
	m_strCollectionName       = "ALFA_DigitCollection";
	m_strODCollectionName     = "ALFA_ODDigitCollection";
	m_strTruthCollectionName  = "TruthEvent";
	m_strKeyRawDataCollection = "ALFA_RawData";

	declareProperty("McEventCollectionName", m_strTruthCollectionName, "McEventCollection name");

	// ALFA_LocRec collection - returns reconstructed positions in Det CS
	m_strLocRecCollectionName   = "ALFA_LocRecEvCollection";
	m_strLocRecODCollectionName = "ALFA_LocRecODEvCollection";

	// ALFA_LocRecCorr collection - returns reconstructed positions in ATLAS CS
	m_strLocRecCorrCollectionName   = "ALFA_LocRecCorrEvCollection";
	m_strLocRecCorrODCollectionName = "ALFA_LocRecCorrODEvCollection";

	// ALFA_LocRec collection - returns reconstructed positions in Det CS
	m_strGloRecCollectionName   = "ALFA_GloRecEvCollection";

	declareProperty("outputLocalRootFile", m_rootOutputFileName = "ALFA_Ntuple.root", "output local root file name");


	// data type using in the local reconstruction
	// for the simulation data the value is 0, for the real data the value is 1. Unset value is -1
	declareProperty("DataType", m_iDataType=1, "data type using in the local reconstruction");
	declareProperty("GeneratorType", m_iGeneratorType=0, "generator type using in the simulation");

	declareProperty("listCoolFolders", m_CoolFolders, "a vector of an active COOL folders");

	// for a multiple algorithm reconstruction ------------------------------------------
	m_vecListAlgoMD.clear();
	declareProperty("ListAlgoMD", m_vecListAlgoMD);
	m_vecListAlgoOD.clear();
	declareProperty("ListAlgoOD", m_vecListAlgoOD);

	m_strMainAlgoMD = "MDMultiple";
	m_strMainAlgoOD = "ODTracking";


	m_vecMainDetGainMode = NULL;
	m_vecMainDetGainMode = new vector<string>();

	m_TreeEventHeader   = NULL;
	m_TreeGlobalTracks  = NULL;
	m_TreeRunHeader     = NULL;
	m_TreeTrackingData  = NULL;
	m_rootOutput        = NULL;
	m_storeGate         = NULL;
	m_TreeBLM           = NULL;
	m_TreeBPMALFA       = NULL;
	m_TreeHVChannel     = NULL;
	m_TreeLocalMon      = NULL;
	m_TreeMovement      = NULL;
	m_TreeRadmon        = NULL;
	m_TreeTriggerRate   = NULL;
	m_Tree_Vtx_Kin_Info = NULL;
	m_bVtxKinFillFlag   = false;
	m_fProtonMomentum   = 0.0;
	m_ft_13             = 0.0;
	m_ft_24             = 0.0;
	m_iBCId             = 0;
	m_iEvent            = 0;
	m_iEvtFromZero      = 0;
	m_iFillNum          = 0;
	m_iFlagBLM          = 0;
	m_iFlagBPM          = 0;
	m_iFlagPotPos       = 0;
	m_iFlagRadDose      = 0;
	m_iFlagTRate        = 0;
	m_iFlagTemperature  = 0;
	m_iFlagVolt         = 0;
	m_iLumBlock         = 0;
	m_iNumGloTrack      = 0;
	m_iNumTrack         = 0;
	m_iRunNum           = 0;
	m_iTimeStamp        = 0;
	m_iTimeStamp_ns     = 0;
	memset(&m_bDQFlag, false, sizeof(m_bDQFlag));
	memset(&m_bHLTrigSig, false, sizeof(m_bHLTrigSig));
	memset(&m_bLVL1TrigSig, false, sizeof(m_bLVL1TrigSig));
	memset(&m_bLVL2TrigSig, false, sizeof(m_bLVL2TrigSig));
	memset(&m_fp_beam2_i, 0.0, sizeof(m_fp_beam2_i));
	memset(&m_fVtx_A, 0.0, sizeof(m_fVtx_A));
	memset(&m_fVtx_C, 0.0, sizeof(m_fVtx_C));
	memset(&m_fVtx_g, 0.0, sizeof(m_fVtx_g));
	memset(&m_fp_A, 0.0, sizeof(m_fp_A));
	memset(&m_fp_C, 0.0, sizeof(m_fp_C));
	memset(&m_fp_beam1_f, 0.0, sizeof(m_fp_beam1_f));
	memset(&m_fp_beam1_i, 0.0, sizeof(m_fp_beam1_i));
	memset(&m_fp_beam2_f, 0.0, sizeof(m_fp_beam2_f));
	memset(&m_fp_beam2_i, 0.0, sizeof(m_fp_beam2_i));

	memset(&m_fvtx_beam1_f, 0.0, sizeof(m_fvtx_beam1_f));
	memset(&m_fvtx_beam2_f, 0.0, sizeof(m_fvtx_beam2_f));
	
	memset(&m_iScaler, 0, sizeof(m_iScaler));
	memset(&m_iArmGlo, 0, sizeof(m_iArmGlo));
	fill_n(&m_fxPosGlo[0], sizeof(m_fxPosGlo)/sizeof(Double_t), -9999.0);
	fill_n(&m_fyPosGlo[0], sizeof(m_fyPosGlo)/sizeof(Double_t), -9999.0);
	fill_n(&m_fxSlopeGlo[0], sizeof(m_fxSlopeGlo)/sizeof(Double_t), -9999.0);
	fill_n(&m_fySlopeGlo[0], sizeof(m_fySlopeGlo)/sizeof(Double_t), -9999.0);
	fill_n(&m_fOverU[0][0], sizeof(m_fOverU)/sizeof(Double_t), -9999.0);
	fill_n(&m_fOverV[0][0], sizeof(m_fOverV)/sizeof(Double_t), -9999.0);
	fill_n(&m_fOverY[0][0], sizeof(m_fOverY)/sizeof(Double_t), -9999.0);
	fill_n(&m_iNumU[0][0], sizeof(m_iNumU)/sizeof(Int_t), -9999);
	fill_n(&m_iNumV[0][0], sizeof(m_iNumV)/sizeof(Int_t), -9999);
	fill_n(&m_iNumY[0][0], sizeof(m_iNumY)/sizeof(Int_t), -9999);


	m_BPMALFA.clear();
	m_BeamLossMonitor.clear();
	m_HVChannel.clear();
	m_LocalMonitoring.clear();
	m_Movement.clear();
	m_Radmon.clear();
	m_TriggerRate.clear();

	LogStream << MSG::DEBUG << "end ALFA_Ntuple::ALFA_Ntuple()" << endreq;
}

// Destructor - check up memory allocation
// delete any memory allocation on the heap
ALFA_Ntuple::~ALFA_Ntuple()
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::~ALFA_Ntuple()");
	LogStream << MSG::DEBUG << "begin ALFA_Ntuple::~ALFA_Ntuple()" << endreq;

	if (m_vecMainDetGainMode!=NULL)
	{
		delete m_vecMainDetGainMode;
		m_vecMainDetGainMode = NULL;
	}

	LogStream << MSG::DEBUG << "end ALFA_Ntuple::~ALFA_Ntuple()" << endreq;
}

StatusCode ALFA_Ntuple::initialize()
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::initialize()");
	LogStream << MSG::DEBUG << "begin ALFA_Ntuple::initialize()" << endreq;

	StatusCode sc;

	sc = service("StoreGateSvc",m_storeGate);
	if(sc.isFailure())
	{
		LogStream << MSG::ERROR << "Unable to retrieve pointer to StoreGateSvc" << endreq;
		return sc;
	}

	if (m_bCoolData)
	{
		if (StatusCode::SUCCESS!=service("DetectorStore",m_pDetStore))
		{
			LogStream << MSG::FATAL << "Detector store not found" << endreq;
			return StatusCode::FAILURE;
		}
	}

	m_MapAlgoNameToID.clear();
	m_MapAlgoNameToID.insert(pair<string, int>("ODTracking", 1));
	m_MapAlgoNameToID.insert(pair<string, int>("MDTracking", 2));
	m_MapAlgoNameToID.insert(pair<string, int>("MDOverlap", 3));
	m_MapAlgoNameToID.insert(pair<string, int>("CenterGravity", 4));
	m_MapAlgoNameToID.insert(pair<string, int>("MDMultiple", 5));
	m_MapAlgoNameToID.insert(pair<string, int>("HalfReco", 6));
	m_MapAlgoNameToID.insert(pair<string, int>("MDGap", 7));

	// output local root file -----------------------------------------------------------
	m_rootOutput = new TFile(m_rootOutputFileName.c_str(),"RECREATE");
	if (m_bCoolData)
	{
		TDirectory *dirCOOL = m_rootOutput->mkdir("COOL");
		dirCOOL->cd();

		m_TreeBLM = new TTree("BeamLossMonitor","BeamLossMonitor");
		m_TreeBLM->Branch("BLM", &m_BeamLossMonitor, "TimeIOVStart/l:TimeIOVStop:blm1310ms[6]/D");

		m_TreeHVChannel = new TTree("HVChannel", "HVChannel");
		m_TreeHVChannel->Branch("HVChan", &m_HVChannel, "TimeIOVStart/l:TimeIOVStop:Actual_VMeas[216]/D:Actual_IMeas[216]");

		m_TreeLocalMon = new TTree("LocalMonitoring", "LocalMonitoring");
		m_TreeLocalMon->Branch("LocalMon", &m_LocalMonitoring, "TimeIOVStart/l:TimeIOVStop:Temperature_sensor1[8]/D:Temperature_sensor2[8]:Temperature_sensor3[8]:Temperature_sensor4[8]:Temperature_sensor5[8]");

		m_TreeMovement = new TTree("Movement", "Movement");
		m_TreeMovement->Branch("Movement", &m_Movement, "TimeIOVStart/l:TimeIOVStop:positionRO_lvdt[8]/D:positionRO_motor[8]");

		m_TreeRadmon = new TTree("Radmon", "Radmon");
		m_TreeRadmon->Branch("Radm", &m_Radmon, "TimeIOVStart/l:TimeIOVStop:dose_value[4]/D:fluence_value[4]:temperature_value[4]");

		m_TreeTriggerRate = new TTree("TriggerRate", "TriggerRate");
		m_TreeTriggerRate->Branch("TrigRate", &m_TriggerRate, "TimeIOVStart/l:TimeIOVStop:Trate[8]");

		m_TreeBPMALFA = new TTree("BPMALFA", "BPMALFA");
		m_TreeBPMALFA->Branch("BPMALFA", &m_BPMALFA, "TimeIOVStart/l:TimeIOVStop:bpmr_r_x_pos/D:bpmr_r_y_pos:bpmr_r_x_err:bpmr_r_y_err:bpmr_l_x_pos:bpmr_l_y_pos:bpmr_l_x_err:bpmr_l_y_err:bpmsa_r_x_pos:bpmsa_r_y_pos:bpmsa_r_x_err:bpmsa_r_y_err:bpmsa_l_x_pos:bpmsa_l_y_pos:bpmsa_l_x_err:bpmsa_l_y_err:bpmwb_r_x_pos:bpmwb_r_y_pos:bpmwb_r_x_err:bpmwb_r_y_err:bpmya_l_x_pos:bpmya_l_y_pos:bpmya_l_x_err:bpmya_l_y_err");
	}

	m_rootOutput->cd();
	m_TreeRunHeader = new TTree("RunHeader", "RunHeader");
	m_TreeRunHeader->Branch("Fill_num", &m_iFillNum, "Fill_num/I");
	m_TreeRunHeader->Branch("Run_num", &m_iRunNum, "Run_num/I");
	if (m_bCoolData)
	{
		m_TreeRunHeader->Branch("Trigger_set", &m_bTriggerSet, "Trigger_set[8][6]/O");
		m_TreeRunHeader->Branch("Latency", &m_iLatency, "Latency[8][3]/I");
		m_TreeRunHeader->Branch("mainDet_gainMode", &m_vecMainDetGainMode);
		m_TreeRunHeader->Branch("Threshold", m_iThreshold, "Threshold[8][3]/I");
		m_TreeRunHeader->Branch("Gain", m_iGain, "Gain[8][5]/I");
	}
	m_TreeRunHeader->Branch("TransformDetRP", &m_fTransformDetRP, "TransformDetRP[8][4][4]/D");
	m_TreeRunHeader->Branch("Proton_momentum", &m_fProtonMomentum, "Proton_momentum/D");

	m_TreeEventHeader = new TTree("EventHeader", "EventHeader");
	m_TreeEventHeader->Branch("Evt_num", &m_iEvent, "Evt_num/i");
	if (m_iDataType==1)	// a part only for measured data
	{
		m_TreeEventHeader->Branch("BCId", &m_iBCId, "BCId/I");
		m_TreeEventHeader->Branch("TimeStp", &m_iTimeStamp, "TimeStp/I");
		m_TreeEventHeader->Branch("TimeStp_ns", &m_iTimeStamp_ns, "TimeStp_ns/I");
		m_TreeEventHeader->Branch("Scaler", &m_iScaler, "Scaler[8]/I");
		m_TreeEventHeader->Branch("Lum_block", &m_iLumBlock, "Lum_block/i");
		m_TreeEventHeader->Branch("TrigPat", &m_bTrigPat, "TrigPat[8][16]/O");
		m_TreeEventHeader->Branch("QDC_Trig", &m_iQDCTrig, "QDC_Trig[8][2]/I");
		m_TreeEventHeader->Branch("LVL1TrigSig", &m_bLVL1TrigSig, "LVL1TrigSig[256]/O");
		m_TreeEventHeader->Branch("LVL2TrigSig", &m_bLVL2TrigSig, "LVL2TrigSig[256]/O");
		m_TreeEventHeader->Branch("HLTrigSig", &m_bHLTrigSig, "HLTrigSig[256]/O");
		m_TreeEventHeader->Branch("DQFlag", &m_bDQFlag, "DQFlag[25]/O");
	}
	m_TreeEventHeader->Branch("FiberHitsMD", &m_bFibHitsMD, "FiberHitsMD[8][20][64]/O");
	m_TreeEventHeader->Branch("FiberHitsODPos", &m_bFibHitsODPos, "FiberHitsODPos[8][3][30]/O");
	m_TreeEventHeader->Branch("FiberHitsODNeg", &m_bFibHitsODNeg, "FiberHitsODNeg[8][3][30]/O");
	if (m_bCoolData)
	{
		//these are flag variables to merge eventHeader with DCS data
		m_TreeEventHeader->Branch("PotPos", &m_iFlagPotPos, "PotPos/I");
		m_TreeEventHeader->Branch("Temperature", &m_iFlagTemperature, "Temperature/I");
		m_TreeEventHeader->Branch("Volt", &m_iFlagVolt, "Volt/I");
		m_TreeEventHeader->Branch("RadDose", &m_iFlagRadDose, "RadDose/I");
		m_TreeEventHeader->Branch("BeamLossMon", &m_iFlagBLM, "BeamLossMon/I");
		m_TreeEventHeader->Branch("TriggerRate", &m_iFlagTRate, "TriggerRate/I");

		//vymazat? ne, implementovat BPMs z COOL DB
		m_TreeEventHeader->Branch("BPM", &m_iFlagBPM, "BPM/I");
	}
	m_TreeEventHeader->Branch("MultiMD", &m_iMultiMD, "MultiMD[8][20]/I");
	m_TreeEventHeader->Branch("MultiODPos", &m_iMultiODPos, "MultiODPos[8][3]/I");
	m_TreeEventHeader->Branch("MultiODNeg", &m_iMultiODNeg, "MultiODNeg[8][3]/I");

	m_TreeTrackingData = new TTree("TrackingData", "TrackingData");
	m_TreeTrackingData->Branch("Evt_num", &m_iEvent, "Evt_num/i");
	m_TreeTrackingData->Branch("NumTrack", &m_iNumTrack, "NumTrack/I");
	m_TreeTrackingData->Branch("RecFlag", &m_bRecFlag, "RecFlag[NumTrack][8]/O");
	m_TreeTrackingData->Branch("Detector", &m_iDetector, "Detector[NumTrack][8]/I");
	m_TreeTrackingData->Branch("x_Det", &m_fXDet, "x_Det[NumTrack][8]/D");
	m_TreeTrackingData->Branch("y_Det", &m_fYDet, "y_Det[NumTrack][8]/D");
	m_TreeTrackingData->Branch("x_Pot", &m_fXPot, "x_Pot[NumTrack][8]/D");
	m_TreeTrackingData->Branch("y_Pot", &m_fYPot, "y_Pot[NumTrack][8]/D");
	m_TreeTrackingData->Branch("x_Stat", &m_fXStat, "x_Stat[NumTrack][8]/D");
	m_TreeTrackingData->Branch("y_Stat", &m_fYStat, "y_Stat[NumTrack][8]/D");
	m_TreeTrackingData->Branch("x_LHC", &m_fXLHC, "x_LHC[NumTrack][8]/D");
	m_TreeTrackingData->Branch("y_LHC", &m_fYLHC, "y_LHC[NumTrack][8]/D");
	m_TreeTrackingData->Branch("z_LHC", &m_fZLHC, "z_LHC[NumTrack][8]/D");
	m_TreeTrackingData->Branch("x_Beam", &m_fXBeam, "x_Beam[NumTrack][8]/D");
	m_TreeTrackingData->Branch("y_Beam", &m_fYBeam, "y_Beam[NumTrack][8]/D");
	m_TreeTrackingData->Branch("OverU", &m_fOverU, "OverU[NumTrack][8]/D");
	m_TreeTrackingData->Branch("OverV", &m_fOverV, "OverV[NumTrack][8]/D");
	m_TreeTrackingData->Branch("OverY", &m_fOverY, "OverY[NumTrack][8]/D");
	m_TreeTrackingData->Branch("NU", &m_iNumU, "NU[NumTrack][8]/I");
	m_TreeTrackingData->Branch("NV", &m_iNumV, "NV[NumTrack][8]/I");
	m_TreeTrackingData->Branch("NY", &m_iNumY, "NY[NumTrack][8]/I");
	m_TreeTrackingData->Branch("Fib_SelMD", &m_iFibSelMD, "Fib_SelMD[NumTrack][8][20]/I");
	m_TreeTrackingData->Branch("Fib_SelOD", &m_iFibSelOD, "Fib_SelOD[NumTrack][8][3]/I");
	m_TreeTrackingData->Branch("intercept_Det", &m_fInterceptDet, "intercept_Det[NumTrack][8]/D");
	m_TreeTrackingData->Branch("slope_Det", &m_fSlopeDet, "slope_Det[NumTrack][8]/D");

	if (m_iDataType==0)	// a part only for simulation
	{
		m_Tree_Vtx_Kin_Info = new TTree("Vertex_and_IP_kinematics", "Vertex_and_IP_kinematics");
		m_Tree_Vtx_Kin_Info->Branch("vtx_g" , &m_fVtx_g,  "vtx_g[4]/D");
		m_Tree_Vtx_Kin_Info->Branch("vtx_A" , &m_fVtx_A,  "vtx_A[4]/D");
		m_Tree_Vtx_Kin_Info->Branch("vtx_C" , &m_fVtx_C,  "vtx_C[4]/D");
		m_Tree_Vtx_Kin_Info->Branch("p_beam1_i" , &m_fp_beam1_i,  "p_beam1_i[4]/D");
		m_Tree_Vtx_Kin_Info->Branch("p_beam2_i" , &m_fp_beam2_i,  "p_beam2_i[4]/D");
		m_Tree_Vtx_Kin_Info->Branch("p_beam1_f" , &m_fp_beam1_f,  "p_beam1_f[4]/D");
		m_Tree_Vtx_Kin_Info->Branch("p_beam2_f" , &m_fp_beam2_f,  "p_beam2_f[4]/D");
		m_Tree_Vtx_Kin_Info->Branch("p_A" , &m_fp_A,  "p_A[4]/D");
		m_Tree_Vtx_Kin_Info->Branch("p_C" , &m_fp_C,  "p_C[4]/D");
//		m_Tree_Vtx_Kin_Info->Branch("t_13"  , &ft_13,   "t_13/D");
//		m_Tree_Vtx_Kin_Info->Branch("t_24"  , &ft_24,   "t_24/D");
		m_Tree_Vtx_Kin_Info->Branch("vtx_beam1_f" , &m_fvtx_beam1_f,  "vtx_beam1_f[4]/D");
		m_Tree_Vtx_Kin_Info->Branch("vtx_beam2_f" , &m_fvtx_beam2_f,  "vtx_beam2_f[4]/D");
	}

	m_TreeGlobalTracks = new TTree("GlobalTracks", "GlobalTracks");
	m_TreeGlobalTracks->Branch("NumGloTrack", &m_iNumGloTrack, "NumGloTrack/I");
	m_TreeGlobalTracks->Branch("arm", &m_iArmGlo, "arm[NumGloTrack]/I");
	m_TreeGlobalTracks->Branch("x_pos", &m_fxPosGlo, "x_pos[NumGloTrack]/D");
	m_TreeGlobalTracks->Branch("y_pos", &m_fyPosGlo, "y_pos[NumGloTrack]/D");
	m_TreeGlobalTracks->Branch("x_slope", &m_fxSlopeGlo, "x_slope[NumGloTrack]/D");
	m_TreeGlobalTracks->Branch("y_slope", &m_fySlopeGlo, "y_slope[NumGloTrack]/D");


	// the trees for the other algorithms -------------------------------
	string strAlgoMD;
	m_MapTreeOtherAlgo.clear();
	m_MapAlgoTreeMD.clear();

	for(unsigned int i=0; i<m_vecListAlgoMD.size(); i++)
	{
		strAlgoMD = m_vecListAlgoMD[i];
		if (strAlgoMD == m_strMainAlgoMD) continue;

		LogStream << MSG::DEBUG << "Algo [name, ID] = " << strAlgoMD << ", " << m_MapAlgoNameToID[strAlgoMD] << endreq;

		if(m_MapAlgoTreeMD.find(m_MapAlgoNameToID[strAlgoMD])==m_MapAlgoTreeMD.end())
		{
			m_AlgoTreeMD.clear();
			m_MapAlgoTreeMD.insert(pair<int, ALGOTREEMD>(m_MapAlgoNameToID[strAlgoMD], m_AlgoTreeMD));
		}

		if(m_MapTreeOtherAlgo.find(m_MapAlgoNameToID[strAlgoMD])==m_MapTreeOtherAlgo.end())
		{
			m_treeOtherAlgo = new TTree(strAlgoMD.c_str(), strAlgoMD.c_str());
			m_treeOtherAlgo->Branch("algoID", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].iAlgoID, "algoID/I");
			m_treeOtherAlgo->Branch("Evt_num", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].iEvent, "Evt_num/i");
			m_treeOtherAlgo->Branch("NumTrack", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].iNumTrack, "NumTrack/I");
			m_treeOtherAlgo->Branch("RecFlag", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].bRecFlag, "RecFlag[NumTrack][8]/O");
			m_treeOtherAlgo->Branch("Detector", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].iDetector, "Detector[NumTrack][8]/I");
			m_treeOtherAlgo->Branch("x_Det", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fXDet, "x_Det[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("y_Det", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fYDet, "y_Det[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("x_Pot", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fXPot, "x_Pot[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("y_Pot", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fYPot, "y_Pot[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("x_Stat", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fXStat, "x_Stat[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("y_Stat", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fYStat, "y_Stat[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("x_LHC", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fXLHC, "x_LHC[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("y_LHC", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fYLHC, "y_LHC[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("z_LHC", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fZLHC, "z_LHC[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("x_Beam", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fXBeam, "x_Beam[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("y_Beam", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fYBeam, "y_Beam[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("OverU", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fOverU, "OverU[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("OverV", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fOverV, "OverV[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("NU", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].iNumU, "NU[NumTrack][8]/I");
			m_treeOtherAlgo->Branch("NV", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].iNumV, "NV[NumTrack][8]/I");
			m_treeOtherAlgo->Branch("Fib_SelMD", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].iFibSelMD, "Fib_SelMD[NumTrack][8][20]/I");
			m_treeOtherAlgo->Branch("intercept_Det", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fInterceptDet, "intercept_Det[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("slope_Det", &m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].fSlopeDet, "slope_Det[NumTrack][8]/D");

			m_MapTreeOtherAlgo.insert(pair<int, TTree*>(m_MapAlgoNameToID[strAlgoMD], m_treeOtherAlgo));
		}
	}

	string strAlgoOD;
	m_MapAlgoTreeOD.clear();
	for (unsigned int i=0; i<m_vecListAlgoOD.size(); i++)
	{
		strAlgoOD = m_vecListAlgoOD[i];
		if (strAlgoOD == m_strMainAlgoOD) continue;

		LogStream << MSG::DEBUG << "Algo [name, ID] = " << strAlgoOD << ", " << m_MapAlgoNameToID[strAlgoOD] << endreq;

		if(m_MapAlgoTreeOD.find(m_MapAlgoNameToID[strAlgoOD])==m_MapAlgoTreeOD.end())
		{
			m_AlgoTreeOD.clear();
			m_MapAlgoTreeOD.insert(pair<int, ALGOTREEOD>(m_MapAlgoNameToID[strAlgoOD], m_AlgoTreeOD));
		}

		if(m_MapTreeOtherAlgo.find(m_MapAlgoNameToID[strAlgoOD])==m_MapTreeOtherAlgo.end())
		{
			m_treeOtherAlgo = new TTree(strAlgoOD.c_str(), strAlgoOD.c_str());
			m_treeOtherAlgo->Branch("algoID", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].iAlgoID, "algoID/I");
			m_treeOtherAlgo->Branch("Evt_num", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].iEvent, "Evt_num/i");
			m_treeOtherAlgo->Branch("NumTrack", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].iNumTrack, "NumTrack/I");
			m_treeOtherAlgo->Branch("RecFlag", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].bRecFlag, "RecFlag[NumTrack][8]/O");
			m_treeOtherAlgo->Branch("Detector", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].iDetector, "Detector[NumTrack][8]/I");
			m_treeOtherAlgo->Branch("x_Det", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fXDet, "x_Det[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("y_Det", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fYDet, "y_Det[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("x_Pot", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fXPot, "x_Pot[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("y_Pot", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fYPot, "y_Pot[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("x_Stat", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fXStat, "x_Stat[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("y_Stat", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fYStat, "y_Stat[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("x_LHC", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fXLHC, "x_LHC[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("y_LHC", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fYLHC, "y_LHC[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("z_LHC", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fZLHC, "z_LHC[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("x_Beam", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fXBeam, "x_Beam[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("y_Beam", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fYBeam, "y_Beam[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("OverY", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fOverY, "OverY[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("NY", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].iNumY, "NY[NumTrack][8]/I");
			m_treeOtherAlgo->Branch("Fib_SelOD", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].iFibSelOD, "Fib_SelOD[NumTrack][8][3]/I");
			m_treeOtherAlgo->Branch("intercept_Det", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fInterceptDet, "intercept_Det[NumTrack][8]/D");
			m_treeOtherAlgo->Branch("slope_Det", &m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].fSlopeDet, "slope_Det[NumTrack][8]/D");

			m_MapTreeOtherAlgo.insert(pair<int, TTree*>(m_MapAlgoNameToID[strAlgoOD], m_treeOtherAlgo));
		}
	}

	//default values for RunHeader tree -------------------------------------------------
	m_iFillNum = -1;
	m_iRunNum  = -1;
	m_vecMainDetGainMode->clear();
	m_fProtonMomentum = -9999.0;
	memset(&m_bTriggerSet, false, sizeof(m_bTriggerSet));
	fill_n(&m_iLatency[0][0], sizeof(m_iLatency)/sizeof(Int_t), -1);
	fill_n(&m_iGain[0][0], sizeof(m_iGain)/sizeof(Int_t), -1);
	fill_n(&m_iThreshold[0][0], sizeof(m_iThreshold)/sizeof(Int_t), -1);
	fill_n(&m_fTransformDetRP[0][0][0], sizeof(m_fTransformDetRP)/sizeof(Double_t), -9999.0);

	m_iEvtFromZero     =  0;
	m_iFlagBLM         = -1;
	m_iFlagPotPos      = -1;
	m_iFlagRadDose     = -1;
	m_iFlagTemperature = -1;
	m_iFlagTRate       = -1;
	m_iFlagVolt        = -1;
	m_iFlagBPM         = -1;


	//clear COOL directory --------------------------------------------------------------
	fill_n(&m_BeamLossMonitor.fBLM[0], sizeof(m_BeamLossMonitor.fBLM)/sizeof(Double_t), -9999.0);
	fill_n(&m_HVChannel.fActualVMeas[0], sizeof(m_HVChannel.fActualVMeas)/sizeof(Double_t), -9999.0);
	fill_n(&m_HVChannel.fActualIMeas[0], sizeof(m_HVChannel.fActualIMeas)/sizeof(Double_t), -9999.0);
	fill_n(&m_LocalMonitoring.fTempSensor1[0], sizeof(m_LocalMonitoring.fTempSensor1)/sizeof(Double_t), -9999.0);
	fill_n(&m_LocalMonitoring.fTempSensor2[0], sizeof(m_LocalMonitoring.fTempSensor2)/sizeof(Double_t), -9999.0);
	fill_n(&m_LocalMonitoring.fTempSensor3[0], sizeof(m_LocalMonitoring.fTempSensor3)/sizeof(Double_t), -9999.0);
	fill_n(&m_LocalMonitoring.fTempSensor4[0], sizeof(m_LocalMonitoring.fTempSensor4)/sizeof(Double_t), -9999.0);
	fill_n(&m_LocalMonitoring.fTempSensor5[0], sizeof(m_LocalMonitoring.fTempSensor5)/sizeof(Double_t), -9999.0);
	fill_n(&m_Movement.fPosLVDT[0], sizeof(m_Movement.fPosLVDT)/sizeof(Double_t), -9999.0);
	fill_n(&m_Movement.fPosMotor[0], sizeof(m_Movement.fPosMotor)/sizeof(Double_t), -9999.0);
	fill_n(&m_Radmon.fDose[0], sizeof(m_Radmon.fDose)/sizeof(Double_t), -9999.0);
	fill_n(&m_Radmon.fFluence[0], sizeof(m_Radmon.fFluence)/sizeof(Double_t), -9999.0);
	fill_n(&m_Radmon.fTemp[0], sizeof(m_Radmon.fTemp)/sizeof(Double_t), -9999.0);
	fill_n(&m_TriggerRate.iTRate[0], sizeof(m_TriggerRate.iTRate)/sizeof(Int_t), -1);

	if (m_bCoolData)
	{
		//register IOV/COOL callbacks
//		CHECK(AddCOOLFolderCallback(COOLFOLDER_BLM));
//		CHECK(AddCOOLFolderCallback(COOLFOLDER_HVCHANNEL));
//		CHECK(AddCOOLFolderCallback(COOLFOLDER_LOCALMONITORING));
//		CHECK(AddCOOLFolderCallback(COOLFOLDER_MOVEMENT));
//		CHECK(AddCOOLFolderCallback(COOLFOLDER_RADMON));
//		CHECK(AddCOOLFolderCallback(COOLFOLDER_TRIGGERRATES));
//		CHECK(AddCOOLFolderCallback(COOLFOLDER_FECONFIGURATION));
//		CHECK(AddCOOLFolderCallback(COOLFOLDER_TRIGGERSETTINGS));
//		CHECK(AddCOOLFolderCallback(COOLFOLDER_LHC_FILLSTATE));
//		CHECK(AddCOOLFolderCallbackAthenaAttributeList(COOLFOLDER_OLC_ALFA));

		if (m_CoolFolders.size()==0) LogStream << MSG::DEBUG << "No COOL folders were specified in jobOption file(s), no ones were readout and stored in the ALFA n-tuple file." << endreq;

		for (unsigned int i=0; i<m_CoolFolders.size(); i++)
		{
			LogStream << MSG::DEBUG << "m_CoolFolders[" << i << "] = " << m_CoolFolders[i] << endreq;
			if      (m_CoolFolders[i] == COOLFOLDER_BLM)             CHECK(AddCOOLFolderCallback(COOLFOLDER_BLM));
			else if (m_CoolFolders[i] == COOLFOLDER_HVCHANNEL)       CHECK(AddCOOLFolderCallback(COOLFOLDER_HVCHANNEL));
			else if (m_CoolFolders[i] == COOLFOLDER_LOCALMONITORING) CHECK(AddCOOLFolderCallback(COOLFOLDER_LOCALMONITORING));
			else if (m_CoolFolders[i] == COOLFOLDER_MOVEMENT)        CHECK(AddCOOLFolderCallback(COOLFOLDER_MOVEMENT));
			else if (m_CoolFolders[i] == COOLFOLDER_RADMON)          CHECK(AddCOOLFolderCallback(COOLFOLDER_RADMON));
			else if (m_CoolFolders[i] == COOLFOLDER_TRIGGERRATES)    CHECK(AddCOOLFolderCallback(COOLFOLDER_TRIGGERRATES));
			else if (m_CoolFolders[i] == COOLFOLDER_FECONFIGURATION) CHECK(AddCOOLFolderCallback(COOLFOLDER_FECONFIGURATION));
			else if (m_CoolFolders[i] == COOLFOLDER_TRIGGERSETTINGS) CHECK(AddCOOLFolderCallback(COOLFOLDER_TRIGGERSETTINGS));
			else if (m_CoolFolders[i] == COOLFOLDER_LHC_FILLSTATE)   CHECK(AddCOOLFolderCallback(COOLFOLDER_LHC_FILLSTATE));
			else if (m_CoolFolders[i] == COOLFOLDER_OLC_ALFA)        CHECK(AddCOOLFolderCallbackAthenaAttributeList(COOLFOLDER_OLC_ALFA));
		}
	}


	LogStream << MSG::DEBUG << "end ALFA_Ntuple::initialize()" << endreq;

	return StatusCode::SUCCESS;
}

StatusCode ALFA_Ntuple::execute()
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::execute()");
	LogStream << MSG::DEBUG << "begin ALFA_Ntuple::execute()" << endreq;

	StatusCode sc;

	//default values for TrackingData tree ----------------------------------------------
	m_iEvent        =  0;
	m_iNumTrack     =  1;
	m_iLumBlock     = -1;
	m_iBCId         = -1;
	m_iTimeStamp    = -1;
	m_iTimeStamp_ns = -1;
	memset(&m_bTrigPat, false, sizeof(m_bTrigPat));
	memset(&m_bLVL1TrigSig, false, sizeof(m_bLVL1TrigSig));
	memset(&m_bLVL2TrigSig, false, sizeof(m_bLVL2TrigSig));
	memset(&m_bHLTrigSig, false, sizeof(m_bHLTrigSig));
	memset(&m_bFibHitsMD, false, sizeof(m_bFibHitsMD));
	memset(&m_bFibHitsODPos, false, sizeof(m_bFibHitsODPos));
	memset(&m_bFibHitsODNeg, false, sizeof(m_bFibHitsODNeg));
	memset(&m_bDQFlag, false, sizeof(m_bDQFlag));
	memset(&m_iMultiMD, 0, sizeof(m_iMultiMD));
	memset(&m_iMultiODPos, 0, sizeof(m_iMultiODPos));
	memset(&m_iMultiODNeg, 0, sizeof(m_iMultiODNeg));
	memset(&m_iDetector, 0, sizeof(m_iDetector));
	memset(&m_bRecFlag, false, sizeof(m_bRecFlag));

	fill_n(&m_iScaler[0], sizeof(m_iScaler)/sizeof(Int_t), -1);
	fill_n(&m_iQDCTrig[0][0], sizeof(m_iQDCTrig)/sizeof(Int_t), -1);
	fill_n(&m_fXDet[0][0], sizeof(m_fXDet)/sizeof(Double_t), -9999.0);
	fill_n(&m_fYDet[0][0], sizeof(m_fYDet)/sizeof(Double_t), -9999.0);
	fill_n(&m_fXPot[0][0], sizeof(m_fXPot)/sizeof(Double_t), -9999.0);
	fill_n(&m_fYPot[0][0], sizeof(m_fYPot)/sizeof(Double_t), -9999.0);
	fill_n(&m_fXStat[0][0], sizeof(m_fXStat)/sizeof(Double_t), -9999.0);
	fill_n(&m_fYStat[0][0], sizeof(m_fYStat)/sizeof(Double_t), -9999.0);
	fill_n(&m_fXLHC[0][0], sizeof(m_fXLHC)/sizeof(Double_t), -9999.0);
	fill_n(&m_fYLHC[0][0], sizeof(m_fYLHC)/sizeof(Double_t), -9999.0);
	fill_n(&m_fZLHC[0][0], sizeof(m_fZLHC)/sizeof(Double_t), -9999.0);
	fill_n(&m_fXBeam[0][0], sizeof(m_fXBeam)/sizeof(Double_t), -9999.0);
	fill_n(&m_fYBeam[0][0], sizeof(m_fYBeam)/sizeof(Double_t), -9999.0);
	fill_n(&m_iFibSelMD[0][0][0], sizeof(m_iFibSelMD)/sizeof(Int_t), -9999);
	fill_n(&m_iFibSelOD[0][0][0], sizeof(m_iFibSelOD)/sizeof(Int_t), -9999);
	fill_n(&m_fInterceptDet[0][0], sizeof(m_fInterceptDet)/sizeof(Double_t), -9999.0);
	fill_n(&m_fSlopeDet[0][0], sizeof(m_fSlopeDet)/sizeof(Double_t), -9999.0);

	fill_n(&m_fOverU[0][0], sizeof(m_fOverU)/sizeof(Double_t), -9999.0);
	fill_n(&m_fOverV[0][0], sizeof(m_fOverV)/sizeof(Double_t), -9999.0);
	fill_n(&m_fOverY[0][0], sizeof(m_fOverY)/sizeof(Double_t), -9999.0);
	fill_n(&m_iNumU[0][0], sizeof(m_iNumU)/sizeof(Int_t), -9999);
	fill_n(&m_iNumV[0][0], sizeof(m_iNumV)/sizeof(Int_t), -9999);
	fill_n(&m_iNumY[0][0], sizeof(m_iNumY)/sizeof(Int_t), -9999);


	//default values for Vertex_and_IP_kinematics tree ----------------------------------
	if (m_iDataType==0)
	{
		memset(&m_fVtx_g, 0, sizeof(m_fVtx_g));
		memset(&m_fVtx_A, 0, sizeof(m_fVtx_A));
		memset(&m_fVtx_C, 0, sizeof(m_fVtx_C));
		memset(&m_fp_beam1_i, 0, sizeof(m_fp_beam1_i));
		memset(&m_fp_beam1_f, 0, sizeof(m_fp_beam1_f));
		memset(&m_fp_beam2_i, 0, sizeof(m_fp_beam2_i));
		memset(&m_fp_beam2_f, 0, sizeof(m_fp_beam2_f));
		memset(&m_fp_A, 0, sizeof(m_fp_A));
		memset(&m_fp_C, 0, sizeof(m_fp_C));
		memset(&m_fvtx_beam1_f, 0, sizeof(m_fvtx_beam1_f));
		memset(&m_fvtx_beam2_f, 0, sizeof(m_fvtx_beam2_f));

		m_ft_13 = 0;
		m_ft_24 = 0;
	}

	//default values for GlobalTracks tree ----------------------------------------------
	m_iNumGloTrack = 0;
	memset(&m_iArmGlo, 0, sizeof(m_iArmGlo));
	fill_n(&m_fxPosGlo[0], sizeof(m_fxPosGlo)/sizeof(Double_t), -9999.0);
	fill_n(&m_fyPosGlo[0], sizeof(m_fyPosGlo)/sizeof(Double_t), -9999.0);
	fill_n(&m_fxSlopeGlo[0], sizeof(m_fxSlopeGlo)/sizeof(Double_t), -9999.0);
	fill_n(&m_fySlopeGlo[0], sizeof(m_fySlopeGlo)/sizeof(Double_t), -9999.0);

	const EventInfo* eventInfo;
	sc = m_storeGate->retrieve(eventInfo);
	if (sc.isFailure())
	{
		LogStream << MSG::ERROR << "ALFA_Ntuple: Cannot get event info." << endreq;
		return sc;
	}

	// current event number, run number and luminosity block
	m_iEvent = eventInfo->event_ID()->event_number();
	m_iRunNum = eventInfo->event_ID()->run_number();
	m_iLumBlock = eventInfo->event_ID()->lumi_block();

	// for other algorithms -------------------------------------------------------------
	string strAlgoMD;
	for(unsigned int i=0; i<m_vecListAlgoMD.size(); i++)
	{
		strAlgoMD = m_vecListAlgoMD[i];
		m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].clear();

		m_MapAlgoTreeMD[m_MapAlgoNameToID[strAlgoMD]].iEvent = m_iEvent;
	}

	string strAlgoOD;
	for(unsigned int i=0; i<m_vecListAlgoOD.size(); i++)
	{
		strAlgoOD = m_vecListAlgoOD[i];
		m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].clear();

		m_MapAlgoTreeOD[m_MapAlgoNameToID[strAlgoOD]].iEvent = m_iEvent;
	}

	// ----------------------------------------------------------------------------------
	if (m_iDataType==1)
	{
		// retrieve the RawData RDO container from the TDS ------------------------------
		sc = GetRawDataCollection();
		if (sc.isFailure())
		{
			LogStream << MSG::WARNING << "Failed to read ALFA raw data collection." << endreq;
		}
	}

	if(m_iDataType==0)
	{
		sc = TruthInfo();
		if (sc.isFailure())
		{
			LogStream << MSG::FATAL << "TruthInfo Failed" << endreq;
			return sc;
		}
	}

	sc = GetLocRecData();
	if (sc.isFailure())
	{
		LogStream << MSG::WARNING << "Failed to read ALFA locRec data collections." << endreq;
	}

	sc = GetGloRecData();
	if (sc.isFailure())
	{
		LogStream << MSG::WARNING << "Failed to read ALFA gloRec data collections." << endreq;
	}

	// from here NumTrack value is known ------------------------------------------------


	sc = ALFACollectionReading();
	if (sc.isFailure())
	{
		LogStream << MSG::WARNING << "Failed to read ALFA digit data collections." << endreq;
	}

	sc = GetLocRecCorrData();
	if (sc.isFailure())
	{
		LogStream << MSG::WARNING << "Failed to read ALFA locRecCorr data collections." << endreq;
	}

	// ----------------------------------------------------------------------------------
	LogStream << MSG::DEBUG << "filling of the EventHeader tree" << endreq;
	m_TreeEventHeader->Fill();
	LogStream << MSG::DEBUG << "filling of the TrackingData tree" << endreq;
	m_TreeTrackingData->Fill();
	LogStream << MSG::DEBUG << "filling of the GlobalTracks tree" << endreq;
	m_TreeGlobalTracks->Fill();


	map<int, TTree*>::iterator itTree;
	for (itTree=m_MapTreeOtherAlgo.begin(); itTree!=m_MapTreeOtherAlgo.end(); itTree++)
	{
		LogStream << MSG::DEBUG << "filling of the " << itTree->first << " tree" << endreq;
		m_MapTreeOtherAlgo[itTree->first]->Fill();
	}

	LogStream << MSG::DEBUG << "filling of the vtx info tree" << endreq;
	if (m_bVtxKinFillFlag || (m_iDataType==0 && (m_iGeneratorType==0 || m_iGeneratorType==2))) {m_Tree_Vtx_Kin_Info->Fill();}
//	if (m_iDataType==0) {m_Tree_Vtx_Kin_Info->Fill();}

	//-----------------------------------------------------------------------------------
//	cout << "iTimeStamp, iTimeStampns" << endl;
//	cout << "id:  " << eventInfo->event_ID()->time_stamp() << ", " << eventInfo->event_ID()->time_stamp_ns_offset() << endl;
//	cout << "bsc: " << m_iTimeStamp << ", " << m_iTimeStamp_ns << endl;


	m_iEvtFromZero++;

	LogStream << MSG::DEBUG << "end ALFA_Ntuple::execute()" << endreq;

	return StatusCode::SUCCESS;
}

StatusCode ALFA_Ntuple::finalize()
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::finalize()");
	LogStream << MSG::DEBUG << "begin ALFA_Ntuple::finalize()" << endreq;

	//fill RunHeader tree ---------------------------------------------------------------
	m_TreeRunHeader->Fill();

	m_rootOutput->cd();
	m_rootOutput->Write();
	m_rootOutput->Close();

	LogStream << MSG::DEBUG << "end ALFA_Ntuple::finalize()" << endreq;

	return StatusCode::SUCCESS;
}

StatusCode ALFA_Ntuple::TruthInfo()
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::TruthInfo()");
	LogStream << MSG::DEBUG << "begin ALFA_Ntuple::TruthInfo()" << endreq;

	const McEventCollection* mcTru = 0;
	double px, py, pz, E, m;
	int coll_counter = 0;
	int vtx_counter[8];
	memset(&vtx_counter, 0, sizeof(vtx_counter));

	HepMC::GenParticle* pi1=0;
	HepMC::GenParticle* pi2=0;
	HepMC::GenParticle* p1=0;
	HepMC::GenParticle* p2=0;

	int pint = 0;
	int pcount = 0;

	m_bVtxKinFillFlag = false;

	StatusCode sc = m_storeGate->retrieve(mcTru, m_strTruthCollectionName);
	if(sc.isFailure() || !mcTru)
	{
		LogStream << MSG::ERROR << "Container "<< m_strTruthCollectionName <<" NOT FOUND !!!!!!!" <<endreq;
		return sc;
	}

	McEventCollection::const_iterator mcTruBeg = mcTru->begin(); // = (*mcTru).begin()
	McEventCollection::const_iterator mcTruEnd = mcTru->end();

	//loop over collection (container)
	for(;mcTruBeg!=mcTruEnd;++mcTruBeg)
	{
		coll_counter++;

		if (coll_counter > 1 )
		{
			LogStream << MSG::ERROR << "problem, too many collections " << endreq;
			break;
		}

		if (coll_counter == 0 )
		{
			LogStream << MSG::DEBUG << "no collection for the event " << endreq;
			return sc;
		}

		HepMC::GenEvent::vertex_const_iterator begGenVtxItr = (**mcTruBeg).vertices_begin();
		HepMC::GenEvent::vertex_const_iterator endGenVtxItr = (**mcTruBeg).vertices_end();

		//loop over verteces belonging to one event
		for(;begGenVtxItr!=endGenVtxItr;++begGenVtxItr)
		{
//			std::cout << "coll_counter-1 = " << coll_counter-1 << endl;

			vtx_counter[coll_counter-1]++;

//			std::cout << " vtx_counter - 1 = " << vtx_counter[coll_counter-1]-1 << endl;

			LogStream << MSG::DEBUG << " * vertex no: " << vtx_counter[coll_counter-1] << endreq;
			LogStream << MSG::DEBUG << " * position x = " << (**begGenVtxItr).position().x() << ", y = " << (**begGenVtxItr).position().y()<< ", z =" << (**begGenVtxItr).position().z() << endreq;

			HepMC::GenVertex::particle_iterator child;
			child = (*begGenVtxItr)->particles_begin(HepMC::family);
			HepMC::GenVertex::particle_iterator child_end;
			child_end = (*begGenVtxItr)->particles_end(HepMC::family);

			for(; child != child_end; ++child)
			{
//				if ((*child == genEvt->beam_particles().first) || (*child == genEvt->beam_particles().second))
//				{
					px = (*child)->momentum().px();
					py = (*child)->momentum().py();
					pz = (*child)->momentum().pz();
					E  = (*child)->momentum().e();
					m  = (*child)->momentum().m();

					LogStream << MSG::DEBUG << std::setprecision(12) << endreq;
					LogStream << MSG::DEBUG << "particle barcode = " << (*child)->barcode() << endreq;
					LogStream << MSG::DEBUG << "particle pdg = " << (*child)->pdg_id() << endreq;
					LogStream << MSG::DEBUG << "particle status = " << (*child)->status() << endreq;
					LogStream << MSG::DEBUG << " *  px = " << px << ", py = " << py << ", pz = " << pz <<  ", E = " << E << ", m = " << m << endreq;
					LogStream << MSG::DEBUG << " " << endreq;

					// incoming protons at the interaction point
					if( ((*child)->status() == 4) && (pint < 2))
					{
						if( (*child)->pdg_id() == 2212)
						{
							pint++;

//							std::cout << "pint = " << pint << endl;

							if(pz > 0)
							{
								pi1=(*child);

								// assuming no bugs in generation - vertex shoudl be just 1 ;-)
								m_fVtx_g[0] = 0.;
								m_fVtx_g[1] = (**begGenVtxItr).position().x();
								m_fVtx_g[2] = (**begGenVtxItr).position().y();
								m_fVtx_g[3] = (**begGenVtxItr).position().z();
							}
							if(pz < 0)
							{
								pi2=(*child);
							}
//							if(pint > 2){LogStream << MSG::DEBUG << "Strange: More than two incoming protons in this event!" << endreq;}
						}
					}

//					std::cout << "pip1" << endl;
					// outgoing protons at the interaction point
					if( (*child)->status() == 201 && (pcount < 2))
//					if( (*child)->status() == 212 && (pcount < 2))
					{
						if( (*child)->pdg_id() == 2212)
						{
							pcount++;

//							std::cout << "pip2" << endl;

							if(pz > 0)
							{
								p1=(*child);

								m_fvtx_beam1_f[0] = 0.;
								m_fvtx_beam1_f[1] = (**begGenVtxItr).position().x();
								m_fvtx_beam1_f[2] = (**begGenVtxItr).position().y();
								m_fvtx_beam1_f[3] = (**begGenVtxItr).position().z();
							}
							if(pz < 0)
							{
								p2=(*child);

								m_fvtx_beam2_f[0] = 0.;
								m_fvtx_beam2_f[1] = (**begGenVtxItr).position().x();
								m_fvtx_beam2_f[2] = (**begGenVtxItr).position().y();
								m_fvtx_beam2_f[3] = (**begGenVtxItr).position().z();
							}
//							if(pcount > 2){LogStream << MSG::DEBUG << "Strange: More than two outcoming protons in this event (elastic scaterring)!" << endreq;}
						}
					}

					if( ((*child)->status() == 1))
					{
						if( (*child)->pdg_id() == 2212){

//							std::cout << "pip" << endl;

							if(pz > 0)
							{
								m_fVtx_A[0] = 0.;
								m_fVtx_A[1] = (**begGenVtxItr).position().x();
								m_fVtx_A[2] = (**begGenVtxItr).position().y();
								m_fVtx_A[3] = (**begGenVtxItr).position().z();

								m_fp_A[0] = E;
								m_fp_A[1] = px;
								m_fp_A[2] = py;
								m_fp_A[3] = pz;
							}
							if(pz < 0)
							{
								m_fVtx_C[0] = 0.;
								m_fVtx_C[1] = (**begGenVtxItr).position().x();
								m_fVtx_C[2] = (**begGenVtxItr).position().y();
								m_fVtx_C[3] = (**begGenVtxItr).position().z();

								m_fp_C[0] = E;
								m_fp_C[1] = px;
								m_fp_C[2] = py;
								m_fp_C[3] = pz;
							}
//							if(pint > 2){LogStream << MSG::DEBUG << "Strange: More than two incoming protons in this event!" << endreq;}
						}
					}
//					std::cout << "pip3" << endl;
//				}
			}
		}

		std::cout << "pi1, pi2, p1, p2 = " << pi1 << "  " << pi2 << "  " << p1 << "  " << p2 << std::endl;
		//if ((pi1 != 0) && (pi2 != 0) && (p1 != 0) && (p2 != 0))
		if ((p1 != 0) && (p2 != 0))
		{
			m_bVtxKinFillFlag = true;

// 			HepMC::FourVector pv1 = (pi1->momentum());
// 			HepMC::FourVector pv2 = (pi2->momentum());
// 			HepMC::FourVector pv3 = (p1->momentum());
// 			HepMC::FourVector pv4 = (p2->momentum());

// 			CLHEP::HepLorentzVector hp1(pv1.px(),pv1.py(),pv1.pz(),pv1.e());
// 			CLHEP::HepLorentzVector hp2(pv2.px(),pv2.py(),pv2.pz(),pv2.e());
// 			CLHEP::HepLorentzVector hp3(pv3.px(),pv3.py(),pv3.pz(),pv3.e());
// 			CLHEP::HepLorentzVector hp4(pv4.px(),pv4.py(),pv4.pz(),pv4.e());

// 			m_ft_13 = (hp1-hp3).m2();
// 			m_ft_24 = (hp2-hp4).m2();

// 			LogStream << MSG::DEBUG << " ******************************************************* " << endreq;
// 			LogStream << MSG::DEBUG << " " << endreq;
// 			LogStream << MSG::DEBUG << " t_13 = " << m_ft_13 << endreq;
// 			LogStream << MSG::DEBUG << " t_24 = " << m_ft_24 << endreq;
// 			LogStream << MSG::DEBUG << " ***************** " << endreq;

// 			m_fp_beam1_i[0] = pi1->momentum().e();
// 			m_fp_beam1_i[1] = pi1->momentum().px();
// 			m_fp_beam1_i[2] = pi1->momentum().py();
// 			m_fp_beam1_i[3] = pi1->momentum().pz();

			LogStream << MSG::DEBUG << "initial particle 1: px = " << m_fp_beam1_i[1] << ", py = " << m_fp_beam1_i[2] << ", pz = " << m_fp_beam1_i[3] << ", E = " << m_fp_beam1_i[0] << endreq;
			LogStream << MSG::DEBUG << " ** " << endreq;

// 			m_fp_beam2_i[0] = pi2->momentum().e();
// 			m_fp_beam2_i[1] = pi2->momentum().px();
// 			m_fp_beam2_i[2] = pi2->momentum().py();
// 			m_fp_beam2_i[3] = pi2->momentum().pz();

			LogStream << MSG::DEBUG << "initial particle 2: px = " << m_fp_beam2_i[1] << ", py = " << m_fp_beam2_i[2] << ", pz = " << m_fp_beam2_i[3] << ", E = " << m_fp_beam2_i[0] << endreq;
			LogStream << MSG::DEBUG << " ** " << endreq;

			m_fp_beam1_f[0] = p1->momentum().e();
			m_fp_beam1_f[1] = p1->momentum().px();
			m_fp_beam1_f[2] = p1->momentum().py();
			m_fp_beam1_f[3] = p1->momentum().pz();

			LogStream << MSG::DEBUG << "final particle 1: px = " << m_fp_beam1_f[1] << ", py = " << m_fp_beam1_f[2] << ", pz = " << m_fp_beam1_f[3] << ", E = " << m_fp_beam1_f[0] << endreq;
			LogStream << MSG::DEBUG << " ** " << endreq;

			m_fp_beam2_f[0] = p2->momentum().e();
			m_fp_beam2_f[1] = p2->momentum().px();
			m_fp_beam2_f[2] = p2->momentum().py();
			m_fp_beam2_f[3] = p2->momentum().pz();

			LogStream << MSG::DEBUG << "final particle 2: px = " << m_fp_beam2_f[1] << ", py = " << m_fp_beam2_f[2] << ", pz = " << m_fp_beam2_f[3] << ", E = " << m_fp_beam2_f[0] << endreq;
			LogStream << MSG::DEBUG << " ** " << endreq;

			LogStream << MSG::DEBUG << " " << endreq;
			LogStream << MSG::DEBUG << " ******************************************************* " << endreq;
		}
	}

	LogStream << MSG::DEBUG << "end ALFA_Ntuple::TruthInfo()" << endreq;

	return StatusCode::SUCCESS;
}

StatusCode ALFA_Ntuple::GetRawDataCollection()
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_LocRec::GetRawDataCollection()");
	LogStream << MSG::DEBUG << "begin ALFA_LocRec::GetRawDataCollection()" << endreq;

	StatusCode sc = StatusCode::SUCCESS;

	// retrieve the RawData RDO container from the TDS ----------------------------------
	const ALFA_RawDataContainer* container = 0;
	sc = m_storeGate->retrieve(container, m_strKeyRawDataCollection);
	if (sc.isFailure())
	{
		LogStream << MSG::WARNING << "Container "<< m_strKeyRawDataCollection <<" is not available !!!" << endreq;
		return StatusCode::FAILURE;
	}

	// loop over container start --------------------------------------------------------
	ALFA_RawDataContainer::const_iterator RawData_Collection_Beg = container->begin();
	ALFA_RawDataContainer::const_iterator RawData_Collection_End = container->end();

	int iMBId;

	fill_n(&m_iScaler[0], sizeof(m_iScaler)/sizeof(Int_t), -1);
	m_iBCId = -1;
	m_iTimeStamp = -1;
	m_iTimeStamp_ns = -1;

	vector<bool> bTrigPat, bLvl1TrigPat;
	int iTrigPatSize=0, iLvl1TrigPatSize = 0;

	m_iTimeStamp = container->GetTimeStamp();
	m_iTimeStamp_ns = container->GetTimeStampns();
	m_iBCId = container->GetBCId();

	iLvl1TrigPatSize = container->GetLvl1Pattern_POT().size();
	bLvl1TrigPat = container->GetLvl1Pattern_POT();
	for (int i=0; i<iLvl1TrigPatSize; i++) m_bLVL1TrigSig[i] = bLvl1TrigPat[i];

	//loop over collection (container) with hits (i.e. over 1 event) --------------------
	for(;RawData_Collection_Beg!=RawData_Collection_End; ++RawData_Collection_Beg)
	{
		iMBId = (*RawData_Collection_Beg)->GetMBId_POT();
		m_iScaler[iMBId-1] = (*RawData_Collection_Beg)->Get_scaler_POT();

		m_iQDCTrig[iMBId-1][0] = (*RawData_Collection_Beg)->Get_ADC1_POT();
		m_iQDCTrig[iMBId-1][1] = (*RawData_Collection_Beg)->Get_ADC2_POT();

		bTrigPat.clear();
		bTrigPat = (*RawData_Collection_Beg)->Get_pattern_POT();
		iTrigPatSize = bTrigPat.size();
		for (int i=0; i<iTrigPatSize; i++) m_bTrigPat[iMBId-1][i] = bTrigPat[iTrigPatSize-(i+1)];
	}

	LogStream << MSG::DEBUG << "end ALFA_Ntuple::GetRawDataCollection()" << endreq;

	return StatusCode::SUCCESS;
}

StatusCode ALFA_Ntuple::ALFACollectionReading()
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_LocRec::ALFA_Collection_Reading()");
	LogStream << MSG::DEBUG << "begin ALFA_LocRec::ALFA_Collection_Reading()" << endreq;

	StatusCode sc;
	Int_t iRPot, iPlate, iFiber, iSide;

	const ALFA_DigitCollection* mcGen = 0;
	const ALFA_ODDigitCollection* mcODGen = 0;

	sc = m_storeGate->retrieve(mcGen,m_strCollectionName);
	if(sc.isFailure() || !mcGen)
	{
		LogStream << MSG::WARNING << "Container "<< m_strCollectionName <<" is not available !!!" << endreq;
		return StatusCode::FAILURE;
	}

	ALFA_DigitCollection::const_iterator mcGenBeg = mcGen->begin();
	ALFA_DigitCollection::const_iterator mcGenEnd = mcGen->end();

	for(;mcGenBeg!=mcGenEnd;++mcGenBeg)
	{
		iRPot  = ((*mcGenBeg)->getStation());
		iPlate = ((*mcGenBeg)->getPlate());
		iFiber = ((*mcGenBeg)->getFiber());

		m_bFibHitsMD[iRPot][iPlate][iFiber] = true;
		m_iMultiMD[iRPot][iPlate]++;
	}

	sc = m_storeGate->retrieve(mcODGen,m_strODCollectionName);
	if(sc.isFailure() || !mcODGen)
	{
		LogStream << MSG::WARNING << "Container "<< m_strODCollectionName <<" is not available !!!" << endreq;
		return StatusCode::FAILURE;
	}

	ALFA_ODDigitCollection::const_iterator mcODGenBeg = mcODGen->begin();
	ALFA_ODDigitCollection::const_iterator mcODGenEnd = mcODGen->end();

	for(;mcODGenBeg!=mcODGenEnd;++mcODGenBeg)
	{
		iRPot  = ((*mcODGenBeg)->getStation());
		iPlate = ((*mcODGenBeg)->getPlate());
		iSide  = ((*mcODGenBeg)->getSide());
		iFiber = ((*mcODGenBeg)->getFiber());

		if (iSide==0) m_bFibHitsODNeg[iRPot][iPlate][iFiber] = true;	//right side
		else m_bFibHitsODPos[iRPot][iPlate][iFiber] = true;

		if (iSide==0) m_iMultiODNeg[iRPot][iPlate]++;	//right side
		else m_iMultiODPos[iRPot][iPlate]++;
	}

	LogStream << MSG::DEBUG << "end ALFA_Ntuple::ALFACollectionReading()" << endreq;

	return StatusCode::SUCCESS;
}

StatusCode ALFA_Ntuple::GetLocRecData()
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::GetLocRecData()");
	LogStream << MSG::DEBUG << "begin ALFA_Ntuple::GetLocRecData()" << endreq;

	StatusCode sc;
	Int_t iRPot, iSide, iNumTrackPot[RPOTSCNT];
	Int_t iNumU, iNumV;
	Double_t fOverU, fOverV;
	Double_t fRecPosX, fRecPosY;

	memset(&iNumTrackPot, 0, sizeof(iNumTrackPot));

	vector<Int_t> vNumTrackPotAll;
	vNumTrackPotAll.assign(8, 0);


	const ALFA_LocRecEvCollection* pLocRecCol = 0;
	const ALFA_LocRecODEvCollection* pLocRecODCol = 0;

	sc = m_storeGate->retrieve(pLocRecCol, m_strLocRecCollectionName);
	if(sc.isFailure() || !pLocRecCol)
	{
		LogStream << MSG::WARNING << "Container "<< m_strLocRecCollectionName <<" is not available !!!" << endreq;
		return StatusCode::FAILURE;
	}

	ALFA_LocRecEvCollection::const_iterator mcColBeg = pLocRecCol->begin();
	ALFA_LocRecEvCollection::const_iterator mcColEnd = pLocRecCol->end();

	map<int, vector<Int_t> > mapNumTrackPotMD;
	map<int, vector<Int_t> > mapNumTrackPotOD;
	mapNumTrackPotMD.clear();
	mapNumTrackPotOD.clear();

	vector<int> vecFibSel;
	vecFibSel.clear();
	int iAlgoID = 0;
	int iTrackTmp = 0;
	for(; mcColBeg!=mcColEnd; ++mcColBeg)
	{
		iAlgoID   = (*mcColBeg)->getAlgoNum();
		iRPot     = (*mcColBeg)->getPotNum();
		fRecPosX  = (*mcColBeg)->getXposition();
		fRecPosY  = (*mcColBeg)->getYposition();
		fOverU    = (*mcColBeg)->getOverU();
		fOverV    = (*mcColBeg)->getOverV();
		iNumU     = (*mcColBeg)->getNumU();
		iNumV     = (*mcColBeg)->getNumV();
		vecFibSel = (*mcColBeg)->getFibSel();

		if (mapNumTrackPotMD.find(iAlgoID)==mapNumTrackPotMD.end())
		{
			mapNumTrackPotMD.insert(pair<int, vector<Int_t> >(iAlgoID, vNumTrackPotAll));
		}


		if (iAlgoID==m_MapAlgoNameToID[m_strMainAlgoMD])
		{
			m_fXDet[iNumTrackPot[iRPot]][iRPot]  = fRecPosX;
			m_fYDet[iNumTrackPot[iRPot]][iRPot]  = fRecPosY;
			m_fOverU[iNumTrackPot[iRPot]][iRPot] = fOverU;
			m_fOverV[iNumTrackPot[iRPot]][iRPot] = fOverV;
			m_iNumU[iNumTrackPot[iRPot]][iRPot]  = iNumU;
			m_iNumV[iNumTrackPot[iRPot]][iRPot]  = iNumV;

			for (unsigned int i=0; i<vecFibSel.size(); i++)
			{
				m_iFibSelMD[iNumTrackPot[iRPot]][iRPot][i] = vecFibSel[i];
			}

			m_bRecFlag[iNumTrackPot[iRPot]][iRPot] = true;
			m_iDetector[iNumTrackPot[iRPot]][iRPot] = 1;

			iNumTrackPot[iRPot]++;
		}

		// for other MD algorithms than the main one ------------------------------------
//		if (iAlgoID!=m_MapAlgoNameToID["MDMultiple"])
		{
			iTrackTmp = mapNumTrackPotMD[iAlgoID][iRPot];

			m_MapAlgoTreeMD[iAlgoID].iAlgoID                  = iAlgoID;
			m_MapAlgoTreeMD[iAlgoID].fXDet[iTrackTmp][iRPot]  = fRecPosX;
			m_MapAlgoTreeMD[iAlgoID].fYDet[iTrackTmp][iRPot]  = fRecPosY;
			m_MapAlgoTreeMD[iAlgoID].fOverU[iTrackTmp][iRPot] = fOverU;
			m_MapAlgoTreeMD[iAlgoID].fOverV[iTrackTmp][iRPot] = fOverV;
			m_MapAlgoTreeMD[iAlgoID].iNumU[iTrackTmp][iRPot]  = iNumU;
			m_MapAlgoTreeMD[iAlgoID].iNumV[iTrackTmp][iRPot]  = iNumV;

			for (unsigned int i=0; i<vecFibSel.size(); i++)
			{
				m_MapAlgoTreeMD[iAlgoID].iFibSelMD[iTrackTmp][iRPot][i] = vecFibSel[i];
			}

			m_MapAlgoTreeMD[iAlgoID].bRecFlag[iTrackTmp][iRPot] = true;
			m_MapAlgoTreeMD[iAlgoID].iDetector[iTrackTmp][iRPot] = 1;

			mapNumTrackPotMD[iAlgoID][iRPot]++;
		}


//		cout << "iAlgoID, iRPot, iNumTrackPot, iNumTrackPotMD << " << iAlgoID << ", " << iRPot << ", " << iNumTrackPot[iRPot] << ", " << mapNumTrackPotMD[iAlgoID][iRPot] << endl;

//		cout << "iRPot, m_iDetector, fRecPosX = " << iRPot << ", " << m_iDetector << ", " << fRecPosX << endl;
	}


	sc = m_storeGate->retrieve(pLocRecODCol, m_strLocRecODCollectionName);
	if(sc.isFailure() || !pLocRecODCol)
	{
		LogStream << MSG::WARNING << "Container "<< m_strLocRecODCollectionName <<" is not available !!!" << endreq;
		return StatusCode::FAILURE;
	}

	ALFA_LocRecODEvCollection::const_iterator mcColODBeg = pLocRecODCol->begin();
	ALFA_LocRecODEvCollection::const_iterator mcColODEnd = pLocRecODCol->end();

	Int_t iSign=0;
	vecFibSel.clear();
	Double_t fOverY;
	Int_t iNumY;
	iTrackTmp = 0;
	vNumTrackPotAll.clear();
	vNumTrackPotAll.assign(8, 0);
	for(; mcColODBeg!=mcColODEnd; ++mcColODBeg)
	{
		iAlgoID  = (*mcColODBeg)->getAlgoNum();
		iRPot    = (*mcColODBeg)->getPotNum();
		iSide    = (*mcColODBeg)->getSide();
		iSign    = (iSide==0)? -1 : 1;
		fRecPosX = iSign*22;
		fRecPosY = (*mcColODBeg)->getYposition();
		fOverY   = (*mcColODBeg)->getOverY();
		iNumY    = (*mcColODBeg)->getNumY();

//		cout << "iPot, algo, yOD = " << iRPot << ", " << iAlgoID << ", " << fRecPosY << endl;

		if (mapNumTrackPotOD.find(iAlgoID)==mapNumTrackPotOD.end())
		{
			mapNumTrackPotOD.insert(pair<int, vector<Int_t> >(iAlgoID, vNumTrackPotAll));
		}

		if (iAlgoID==m_MapAlgoNameToID[m_strMainAlgoOD])
		{
			m_fXDet[iNumTrackPot[iRPot]][iRPot]  = fRecPosX;
			m_fYDet[iNumTrackPot[iRPot]][iRPot]  = fRecPosY;
			m_fOverY[iNumTrackPot[iRPot]][iRPot] = fOverY;
			m_iNumY[iNumTrackPot[iRPot]][iRPot]  = iNumY;

			vecFibSel = (*mcColODBeg)->getFibSel();
			for (unsigned int i=0; i<vecFibSel.size(); i++)
			{
				m_iFibSelOD[iNumTrackPot[iRPot]][iRPot][i] = vecFibSel[i];
			}

			m_bRecFlag[iNumTrackPot[iRPot]][iRPot] = true;
			//right OD = 3 (side=0), left OD = 2 (side=1).
			m_iDetector[iNumTrackPot[iRPot]][iRPot] = (iSide==0)? 3 : 2;
			iNumTrackPot[iRPot]++;
		}

		// for other OD algorithms than the main one ---------------------------------------
//		if (iAlgoID!=m_MapAlgoNameToID["MDMultiple"])
		{
			iTrackTmp = mapNumTrackPotOD[iAlgoID][iRPot];

			m_MapAlgoTreeOD[iAlgoID].iAlgoID                  = iAlgoID;
			m_MapAlgoTreeOD[iAlgoID].fXDet[iTrackTmp][iRPot]  = fRecPosX;
			m_MapAlgoTreeOD[iAlgoID].fYDet[iTrackTmp][iRPot]  = fRecPosY;
			m_MapAlgoTreeOD[iAlgoID].fOverY[iTrackTmp][iRPot] = fOverY;
			m_MapAlgoTreeOD[iAlgoID].iNumY[iTrackTmp][iRPot]  = iNumY;

			for (unsigned int i=0; i<vecFibSel.size(); i++)
			{
				m_MapAlgoTreeOD[iAlgoID].iFibSelOD[iTrackTmp][iRPot][i] = vecFibSel[i];
			}

			m_MapAlgoTreeOD[iAlgoID].bRecFlag[iTrackTmp][iRPot] = true;
			m_MapAlgoTreeOD[iAlgoID].iDetector[iTrackTmp][iRPot] = (iSide==0)? 3 : 2;

			mapNumTrackPotOD[iAlgoID][iRPot]++;
		}
//		cout << "iAlgoID, iRPot, iNumTrackPot, iNumTrackPotOD << " << iAlgoID << ", " << iRPot << ", " << iNumTrackPot[iRPot] << ", " << mapNumTrackPotOD[iAlgoID][iRPot] << endl;
	}

	Int_t iNumTrackPotMax = 1;
	for (int i=0; i<RPOTSCNT; i++)
	{
		if (iNumTrackPot[i]>iNumTrackPotMax) iNumTrackPotMax = iNumTrackPot[i];
	}
	m_iNumTrack = iNumTrackPotMax;


	//for the other MD algorithms ----------------------------------------------------------
	map<int, int> mapNumTrackPotMax;
	mapNumTrackPotMax.clear();
	map<int, vector<Int_t> >::iterator it;
	for (it=mapNumTrackPotMD.begin(); it!=mapNumTrackPotMD.end(); it++)
	{
		for (int iPot=0; iPot<RPOTSCNT; iPot++)
		{
//			cout << "first, second = " << it->first << ", " << it->second[iPot] << endl;
			if (mapNumTrackPotMax.find(it->first)==mapNumTrackPotMax.end()) mapNumTrackPotMax.insert(pair<int, int>(it->first, 0));

			if (it->second[iPot] > mapNumTrackPotMax[it->first]) mapNumTrackPotMax[it->first] = it->second[iPot];
		}
	}

	map<int, int >::iterator itNum;
	for (itNum=mapNumTrackPotMax.begin(); itNum!=mapNumTrackPotMax.end(); itNum++)
	{
//		cout << "first, second = " << itNum->first << ", " << itNum->second << endl;
		m_MapAlgoTreeMD[itNum->first].iNumTrack = itNum->second;

	}

	//for the other OD algorithms ----------------------------------------------------------
	mapNumTrackPotMax.clear();
	for (it=mapNumTrackPotOD.begin(); it!=mapNumTrackPotOD.end(); it++)
	{
		for (int iPot=0; iPot<RPOTSCNT; iPot++)
		{
//			cout << "first, second = " << it->first << ", " << it->second[iPot] << endl;
			if (mapNumTrackPotMax.find(it->first)==mapNumTrackPotMax.end()) mapNumTrackPotMax.insert(pair<int, int>(it->first, 0));

			if (it->second[iPot] > mapNumTrackPotMax[it->first]) mapNumTrackPotMax[it->first] = it->second[iPot];
		}
	}

	for (itNum=mapNumTrackPotMax.begin(); itNum!=mapNumTrackPotMax.end(); itNum++)
	{
//		cout << "first, second = " << itNum->first << ", " << itNum->second << endl;
		m_MapAlgoTreeOD[itNum->first].iNumTrack = itNum->second;
	}


	LogStream << MSG::DEBUG << "end ALFA_Ntuple::GetLocRecData()" << endreq;

	return StatusCode::SUCCESS;
}

StatusCode ALFA_Ntuple::GetLocRecCorrData()
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::GetLocRecCorrData()");
	LogStream << MSG::DEBUG << "begin ALFA_Ntuple::GetLocRecCorrData()" << endreq;

	StatusCode sc;
	Int_t iRPot, iSide, iNumTrackPot[RPOTSCNT], iSign;
	int iAlgoID = 0, iTrackTmp = 0;
	Double_t fxLHC, fyLHC, fzLHC;
	Double_t fxPot, fyPot;
	Double_t fxStat, fyStat;
	Double_t fxBeam, fyBeam;


	memset(&iNumTrackPot, 0, sizeof(iNumTrackPot));

	vector<Int_t> vNumTrackPotAll;
	vNumTrackPotAll.assign(8, 0);

	map<int, vector<Int_t> > mapNumTrackPot;
	mapNumTrackPot.clear();

	const ALFA_LocRecCorrEvCollection* pLocRecCorrCol = 0;
	const ALFA_LocRecCorrODEvCollection* pLocRecCorrODCol = 0;

	sc = m_storeGate->retrieve(pLocRecCorrCol, m_strLocRecCorrCollectionName);
	if(sc.isFailure() || !pLocRecCorrCol)
	{
		LogStream << MSG::WARNING << "Container " << m_strLocRecCorrCollectionName << " is not available !!!" << endreq;
		return StatusCode::FAILURE;
	}

	ALFA_LocRecCorrEvCollection::const_iterator mcColBeg = pLocRecCorrCol->begin();
	ALFA_LocRecCorrEvCollection::const_iterator mcColEnd = pLocRecCorrCol->end();

	//loop over collection (container) with hits (i.e. over 1 event)
	for(; mcColBeg!=mcColEnd; ++mcColBeg)
	{
		iAlgoID = (*mcColBeg)->getAlgoNum();
		iRPot   = (*mcColBeg)->getPotNum();
		fxLHC   = (*mcColBeg)->getXpositionLHC();
		fyLHC   = (*mcColBeg)->getYpositionLHC();
		fzLHC   = (*mcColBeg)->getZpositionLHC();
		fxPot   = (*mcColBeg)->getXpositionPot();
		fyPot   = (*mcColBeg)->getYpositionPot();
		fxStat  = (*mcColBeg)->getXpositionStat();
		fyStat  = (*mcColBeg)->getYpositionStat();
		fxBeam  = (*mcColBeg)->getXpositionBeam();
		fyBeam  = (*mcColBeg)->getYpositionBeam();

		if (mapNumTrackPot.find(iAlgoID)==mapNumTrackPot.end())
		{
			mapNumTrackPot.insert(pair<int, vector<Int_t> >(iAlgoID, vNumTrackPotAll));
		}

		if (iAlgoID==m_MapAlgoNameToID[m_strMainAlgoMD])
		{
			m_fXLHC[iNumTrackPot[iRPot]][iRPot]  = fxLHC;
			m_fYLHC[iNumTrackPot[iRPot]][iRPot]  = fyLHC;
			m_fZLHC[iNumTrackPot[iRPot]][iRPot]  = fzLHC;
			m_fXPot[iNumTrackPot[iRPot]][iRPot]  = fxPot;
			m_fYPot[iNumTrackPot[iRPot]][iRPot]  = fyPot;
			m_fXStat[iNumTrackPot[iRPot]][iRPot] = fxStat;
			m_fYStat[iNumTrackPot[iRPot]][iRPot] = fyStat;
			m_fXBeam[iNumTrackPot[iRPot]][iRPot] = fxBeam;
			m_fYBeam[iNumTrackPot[iRPot]][iRPot] = fyBeam;

			iNumTrackPot[iRPot]++;
		}



		// for other MD algorithms than the main one ---------------------------------------
//		if (iAlgoID!=m_MapAlgoNameToID["MDMultiple"])
		{
			iTrackTmp = mapNumTrackPot[iAlgoID][iRPot];

			m_MapAlgoTreeMD[iAlgoID].fXLHC[iTrackTmp][iRPot]  = fxLHC;
			m_MapAlgoTreeMD[iAlgoID].fYLHC[iTrackTmp][iRPot]  = fyLHC;
			m_MapAlgoTreeMD[iAlgoID].fZLHC[iTrackTmp][iRPot]  = fzLHC;
			m_MapAlgoTreeMD[iAlgoID].fXPot[iTrackTmp][iRPot]  = fxPot;
			m_MapAlgoTreeMD[iAlgoID].fYPot[iTrackTmp][iRPot]  = fyPot;
			m_MapAlgoTreeMD[iAlgoID].fXStat[iTrackTmp][iRPot] = fxStat;
			m_MapAlgoTreeMD[iAlgoID].fYStat[iTrackTmp][iRPot] = fyStat;
			m_MapAlgoTreeMD[iAlgoID].fXBeam[iTrackTmp][iRPot] = fxBeam;
			m_MapAlgoTreeMD[iAlgoID].fYBeam[iTrackTmp][iRPot] = fyBeam;

			mapNumTrackPot[iAlgoID][iRPot]++;
		}


//		cout << "iRPot, iNumTrackPot, fRecPosX = " << iRPot << ", " << iNumTrackPot[iRPot]-1 << ", " << fRecPosX << endl;
	}

	sc = m_storeGate->retrieve(pLocRecCorrODCol, m_strLocRecCorrODCollectionName);
	if(sc.isFailure() || !pLocRecCorrODCol)
	{
		LogStream << MSG::WARNING << "Container " << m_strLocRecCorrODCollectionName << " is not available !!!" << endreq;
		return StatusCode::FAILURE;
	}

	ALFA_LocRecCorrODEvCollection::const_iterator mcColODBeg = pLocRecCorrODCol->begin();
	ALFA_LocRecCorrODEvCollection::const_iterator mcColODEnd = pLocRecCorrODCol->end();

	iTrackTmp = 0;
	vNumTrackPotAll.clear();
	vNumTrackPotAll.assign(8, 0);
	mapNumTrackPot.clear();
	//loop over collection (container) with hits (i.e. over 1 event)
	for(; mcColODBeg!=mcColODEnd; ++mcColODBeg)
	{
		iAlgoID = (*mcColODBeg)->getAlgoNum();
		iRPot   = (*mcColODBeg)->getPotNum();
		iSide   = (*mcColODBeg)->getSide();
		iSign   = (iSide==0)? -1 : 1;
		fxLHC   = iSign*22;
		fyLHC   = (*mcColODBeg)->getYpositionLHC();
		fzLHC   = (*mcColODBeg)->getZpositionLHC();
		fxPot   = iSign*22;
		fyPot   = (*mcColODBeg)->getYpositionPot();
		fxStat  = iSign*22;
		fyStat  = (*mcColODBeg)->getYpositionStat();
		fxBeam  = iSign*22;
		fyBeam  = (*mcColODBeg)->getYpositionBeam();

//		cout << "LocRecCorr = " << fxLHC << ", " << fyLHC << ", " << fzLHC << endl;

		if (mapNumTrackPot.find(iAlgoID)==mapNumTrackPot.end())
		{
			mapNumTrackPot.insert(pair<int, vector<Int_t> >(iAlgoID, vNumTrackPotAll));
		}

		if (iAlgoID==m_MapAlgoNameToID[m_strMainAlgoOD])
		{
			m_fXLHC[iNumTrackPot[iRPot]][iRPot]  = fxLHC;
			m_fYLHC[iNumTrackPot[iRPot]][iRPot]  = fyLHC;
			m_fZLHC[iNumTrackPot[iRPot]][iRPot]  = fzLHC;
			m_fXPot[iNumTrackPot[iRPot]][iRPot]  = fxPot;
			m_fYPot[iNumTrackPot[iRPot]][iRPot]  = fyPot;
			m_fXStat[iNumTrackPot[iRPot]][iRPot] = fxStat;
			m_fYStat[iNumTrackPot[iRPot]][iRPot] = fyStat;
			m_fXBeam[iNumTrackPot[iRPot]][iRPot] = fxBeam;
			m_fYBeam[iNumTrackPot[iRPot]][iRPot] = fyBeam;

			iNumTrackPot[iRPot]++;
		}

		// for other OD algorithms than the main one ---------------------------------------
//		if (iAlgoID!=m_MapAlgoNameToID["MDMultiple"])
		{
			iTrackTmp = mapNumTrackPot[iAlgoID][iRPot];

			m_MapAlgoTreeOD[iAlgoID].fXLHC[iTrackTmp][iRPot]  = fxLHC;
			m_MapAlgoTreeOD[iAlgoID].fYLHC[iTrackTmp][iRPot]  = fyLHC;
			m_MapAlgoTreeOD[iAlgoID].fZLHC[iTrackTmp][iRPot]  = fzLHC;
			m_MapAlgoTreeOD[iAlgoID].fXPot[iTrackTmp][iRPot]  = fxPot;
			m_MapAlgoTreeOD[iAlgoID].fYPot[iTrackTmp][iRPot]  = fyPot;
			m_MapAlgoTreeOD[iAlgoID].fXStat[iTrackTmp][iRPot] = fxStat;
			m_MapAlgoTreeOD[iAlgoID].fYStat[iTrackTmp][iRPot] = fyStat;
			m_MapAlgoTreeOD[iAlgoID].fXBeam[iTrackTmp][iRPot] = fxBeam;
			m_MapAlgoTreeOD[iAlgoID].fYBeam[iTrackTmp][iRPot] = fyBeam;

			mapNumTrackPot[iAlgoID][iRPot]++;
		}
	}

	LogStream << MSG::DEBUG << "end ALFA_Ntuple::GetLocRecCorrData()" << endreq;

	return StatusCode::SUCCESS;
}

StatusCode ALFA_Ntuple::GetGloRecData()
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::GetGloRecData()");
	LogStream << MSG::DEBUG << "begin ALFA_Ntuple::GetGloRecData()" << endreq;

	StatusCode sc;
	Int_t iArm, iNumGloTrack = 0;
	Double_t fxPos, fyPos, fxSlope, fySlope;

	const ALFA_GloRecEvCollection* pGloRecCol = 0;
	sc = m_storeGate->retrieve(pGloRecCol, m_strGloRecCollectionName);
	if(sc.isFailure() || !pGloRecCol)
	{
		LogStream << MSG::WARNING << "Container "<< m_strGloRecCollectionName <<" is not available !!!" << endreq;
		return StatusCode::FAILURE;
	}

	ALFA_GloRecEvCollection::const_iterator mcColBeg = pGloRecCol->begin();
	ALFA_GloRecEvCollection::const_iterator mcColEnd = pGloRecCol->end();

	for(; mcColBeg!=mcColEnd; ++mcColBeg)
	{
		iArm    = (*mcColBeg)->getArmNum();
		fxPos   = (*mcColBeg)->getXposition();
		fyPos   = (*mcColBeg)->getYposition();
		fxSlope = (*mcColBeg)->getXslope();
		fySlope = (*mcColBeg)->getYslope();

		m_iArmGlo[iNumGloTrack]    = iArm;
		m_fxPosGlo[iNumGloTrack]   = fxPos;
		m_fyPosGlo[iNumGloTrack]   = fyPos;
		m_fxSlopeGlo[iNumGloTrack] = fxSlope;
		m_fySlopeGlo[iNumGloTrack] = fySlope;

		iNumGloTrack++;

//		cout << "NumGloTrack, Arm, x, y, x_slope, y_slope = " << iNumGloTrack << ", " << iArm << ", " << fxPos << ", " << fyPos << ", " << fxSlope << ", " << fySlope << endl;
	}

	m_iNumGloTrack = iNumGloTrack;

	LogStream << MSG::DEBUG << "end ALFA_Ntuple::GetGloRecData()" << endreq;
	return StatusCode::SUCCESS;
}

StatusCode ALFA_Ntuple::AddCOOLFolderCallback(const string& Folder)
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::AddCOOLFolderCallback()");
	StatusCode sc=StatusCode::FAILURE;

	const DataHandle<CondAttrListCollection> DataPtr;
	sc=m_pDetStore->regFcn(&ALFA_Ntuple::COOLUpdate, this, DataPtr, Folder, true);
	if(sc!=StatusCode::SUCCESS)
	{
		LogStream << MSG::ERROR << "Cannot register COOL callback for folder '"<<Folder<<"'" << endreq;
	}

	return sc;
}

StatusCode ALFA_Ntuple::AddCOOLFolderCallbackAthenaAttributeList(const string& Folder)
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::AddCOOLFolderCallbackAthenaAttributeList()");
	StatusCode sc=StatusCode::FAILURE;

	const DataHandle<AthenaAttributeList> DataPtr;
	sc=m_pDetStore->regFcn(&ALFA_Ntuple::COOLUpdate, this, DataPtr, Folder, false);
	if(sc!=StatusCode::SUCCESS)
	{
		LogStream << MSG::ERROR << "Cannot register COOL callback for folder '"<<Folder<<"'" << endreq;
	}

	return sc;
}

StatusCode ALFA_Ntuple::COOLUpdate(IOVSVC_CALLBACK_ARGS_P(/*I*/, keys))
{
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_Ntuple::COOLUpdate()");
	list<string>::const_iterator iter;
	const CondAttrListCollection* atrlistcol;
	const AthenaAttributeList* atrlist;

	uint64_t iTimeIOVStart=0, iTimeIOVStop=0;

	unsigned int iChannel = 0;
	for(iter=keys.begin();iter!=keys.end();iter++)
	{
		if((*iter)==COOLFOLDER_BLM)
		{
			if (StatusCode::SUCCESS==m_pDetStore->retrieve(atrlistcol,COOLFOLDER_BLM))
			{
				for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr)
				{
					iChannel = (*citr).first-1;
					m_BeamLossMonitor.fBLM[iChannel] = (((*citr).second)[0]).data<float>();
				}

				if (COOLIOVRange(COOLFOLDER_BLM, iTimeIOVStart, iTimeIOVStop).isFailure()) LogStream << MSG::ERROR << "Couldn't get IOV data about folder: " << COOLFOLDER_BLM << endreq;

				m_BeamLossMonitor.iTimeIOVstart = iTimeIOVStart;
				m_BeamLossMonitor.iTimeIOVstop  = iTimeIOVStop;

				m_TreeBLM->Fill();
				m_iFlagBLM = m_TreeBLM->GetEntries()-1;
			}
			else LogStream << MSG::WARNING << "Could not retrieve CondAttrListCollection " << COOLFOLDER_BLM << endreq;
		}
		else if((*iter)==COOLFOLDER_HVCHANNEL)
		{
			if (StatusCode::SUCCESS==m_pDetStore->retrieve(atrlistcol,COOLFOLDER_HVCHANNEL))
			{
				for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr)
				{
					iChannel = (*citr).first-1;
					m_HVChannel.fActualVMeas[iChannel] = (((*citr).second)[0]).data<float>();
					m_HVChannel.fActualIMeas[iChannel] = (((*citr).second)[1]).data<float>();
				}

				if (COOLIOVRange(COOLFOLDER_HVCHANNEL, iTimeIOVStart, iTimeIOVStop).isFailure()) LogStream << MSG::ERROR << "Couldn't get IOV data about folder: " << COOLFOLDER_HVCHANNEL << endreq;

				m_HVChannel.iTimeIOVstart = iTimeIOVStart;
				m_HVChannel.iTimeIOVstop  = iTimeIOVStop;

				m_TreeHVChannel->Fill();
				m_iFlagVolt = m_TreeHVChannel->GetEntries()-1;
			}
			else LogStream << MSG::WARNING << "Could not retrieve CondAttrListCollection " << COOLFOLDER_HVCHANNEL << endreq;
		}
		else if((*iter)==COOLFOLDER_LOCALMONITORING)
		{
			if (StatusCode::SUCCESS==m_pDetStore->retrieve(atrlistcol,COOLFOLDER_LOCALMONITORING))
			{
				for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr)
				{
					iChannel = (*citr).first-1;
					m_LocalMonitoring.fTempSensor1[iChannel] = (((*citr).second)[0]).data<float>();
					m_LocalMonitoring.fTempSensor2[iChannel] = (((*citr).second)[1]).data<float>();
					m_LocalMonitoring.fTempSensor3[iChannel] = (((*citr).second)[2]).data<float>();
					m_LocalMonitoring.fTempSensor4[iChannel] = (((*citr).second)[3]).data<float>();
					m_LocalMonitoring.fTempSensor5[iChannel] = (((*citr).second)[4]).data<float>();
				}

				if (COOLIOVRange(COOLFOLDER_LOCALMONITORING, iTimeIOVStart, iTimeIOVStop).isFailure()) LogStream << MSG::ERROR << "Couldn't get IOV data about folder: " << COOLFOLDER_LOCALMONITORING << endreq;

				m_LocalMonitoring.iTimeIOVstart = iTimeIOVStart;
				m_LocalMonitoring.iTimeIOVstop  = iTimeIOVStop;

				m_TreeLocalMon->Fill();
				m_iFlagTemperature = m_TreeLocalMon->GetEntries()-1;
			}
			else LogStream << MSG::WARNING << "Could not retrieve CondAttrListCollection " << COOLFOLDER_LOCALMONITORING << endreq;
		}
		else if((*iter)==COOLFOLDER_MOVEMENT)
		{
			if (StatusCode::SUCCESS==m_pDetStore->retrieve(atrlistcol,COOLFOLDER_MOVEMENT))
			{
				for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr)
				{
					iChannel = (*citr).first-1;
					m_Movement.fPosLVDT[iChannel]  = (((*citr).second)[0]).data<float>();
					m_Movement.fPosMotor[iChannel] = (((*citr).second)[1]).data<float>();
				}

				if (COOLIOVRange(COOLFOLDER_MOVEMENT, iTimeIOVStart, iTimeIOVStop).isFailure()) LogStream << MSG::ERROR << "Couldn't get IOV data about folder: " << COOLFOLDER_MOVEMENT << endreq;

				m_Movement.iTimeIOVstart = iTimeIOVStart;
				m_Movement.iTimeIOVstop  = iTimeIOVStop;

				m_TreeMovement->Fill();
				m_iFlagPotPos = m_TreeMovement->GetEntries()-1;
			}
			else LogStream << MSG::WARNING << "Could not retrieve CondAttrListCollection " << COOLFOLDER_MOVEMENT << endreq;
		}
		else if((*iter)==COOLFOLDER_RADMON)
		{
			if (StatusCode::SUCCESS==m_pDetStore->retrieve(atrlistcol,COOLFOLDER_RADMON))
			{
				for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr)
				{
					iChannel = (*citr).first-1;
					m_Radmon.fDose[iChannel]    = (((*citr).second)[0]).data<float>();
					m_Radmon.fFluence[iChannel] = (((*citr).second)[1]).data<float>();
					m_Radmon.fTemp[iChannel]    = (((*citr).second)[2]).data<float>();
				}

				if (COOLIOVRange(COOLFOLDER_RADMON, iTimeIOVStart, iTimeIOVStop).isFailure()) LogStream << MSG::ERROR << "Couldn't get IOV data about folder: " << COOLFOLDER_RADMON << endreq;

				m_Radmon.iTimeIOVstart = iTimeIOVStart;
				m_Radmon.iTimeIOVstop  = iTimeIOVStop;

				m_TreeRadmon->Fill();
				m_iFlagRadDose = m_TreeRadmon->GetEntries()-1;
			}
			else LogStream << MSG::WARNING << "Could not retrieve CondAttrListCollection " << COOLFOLDER_RADMON << endreq;
		}
		else if((*iter)==COOLFOLDER_TRIGGERRATES)
		{
			if (StatusCode::SUCCESS==m_pDetStore->retrieve(atrlistcol,COOLFOLDER_TRIGGERRATES))
			{
				for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr)
				{
					iChannel = (*citr).first-1;
					m_TriggerRate.iTRate[iChannel] = (Int_t)(((*citr).second)[0]).data<unsigned int>();
				}

				if (COOLIOVRange(COOLFOLDER_TRIGGERRATES, iTimeIOVStart, iTimeIOVStop).isFailure()) LogStream << MSG::ERROR << "Couldn't get IOV data about folder: " << COOLFOLDER_TRIGGERRATES << endreq;

				m_TriggerRate.iTimeIOVstart = iTimeIOVStart;
				m_TriggerRate.iTimeIOVstop  = iTimeIOVStop;

				m_TreeTriggerRate->Fill();
				m_iFlagTRate = m_TreeTriggerRate->GetEntries()-1;
			}
			else LogStream << MSG::WARNING << "Could not retrieve CondAttrListCollection " << COOLFOLDER_TRIGGERRATES << endreq;
		}
		else if((*iter)==COOLFOLDER_FECONFIGURATION)
		{
			if (StatusCode::SUCCESS==m_pDetStore->retrieve(atrlistcol,COOLFOLDER_FECONFIGURATION))
			{
				for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr)
				{
					iChannel = (*citr).first-1;
					m_iThreshold[iChannel][0] = (((*citr).second)[0]).data<int>();
					m_iThreshold[iChannel][1] = (((*citr).second)[5]).data<int>();
					m_iThreshold[iChannel][2] = (((*citr).second)[8]).data<int>();
					m_iGain[iChannel][0]      = (((*citr).second)[1]).data<int>();
					m_iGain[iChannel][1]      = (((*citr).second)[3]).data<int>();
					m_iGain[iChannel][2]      = (((*citr).second)[4]).data<int>();
					m_iGain[iChannel][3]      = (((*citr).second)[6]).data<int>();
					m_iGain[iChannel][4]      = (((*citr).second)[7]).data<int>();
					m_vecMainDetGainMode->push_back((((*citr).second)[2]).data<string>());
				}
			}
			else LogStream << MSG::WARNING << "Could not retrieve CondAttrListCollection " << COOLFOLDER_FECONFIGURATION << endreq;
		}
		else if((*iter)==COOLFOLDER_TRIGGERSETTINGS)
		{
			if (StatusCode::SUCCESS==m_pDetStore->retrieve(atrlistcol,COOLFOLDER_TRIGGERSETTINGS))
			{
				for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr)
				{
					iChannel = (*citr).first-1;
					m_bTriggerSet[iChannel][0] = (((*citr).second)[0]).data<bool>();
					m_bTriggerSet[iChannel][1] = (((*citr).second)[1]).data<bool>();
					m_bTriggerSet[iChannel][2] = (((*citr).second)[2]).data<bool>();
					m_bTriggerSet[iChannel][3] = (((*citr).second)[3]).data<bool>();
					m_bTriggerSet[iChannel][4] = (((*citr).second)[4]).data<bool>();
					m_bTriggerSet[iChannel][5] = (((*citr).second)[5]).data<bool>();
					m_iLatency[iChannel][0]    = (Int_t)(((*citr).second)[6]).data<unsigned int>();
					m_iLatency[iChannel][1]    = (Int_t)(((*citr).second)[7]).data<unsigned int>();
					m_iLatency[iChannel][2]    = (Int_t)(((*citr).second)[8]).data<unsigned int>();
				}
			}
			else LogStream << MSG::WARNING << "Could not retrieve CondAttrListCollection " << COOLFOLDER_TRIGGERSETTINGS << endreq;
		}
		else if((*iter)==COOLFOLDER_LHC_FILLSTATE)
		{
			if (StatusCode::SUCCESS==m_pDetStore->retrieve(atrlistcol,COOLFOLDER_LHC_FILLSTATE))
			{
				for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr)
				{
					m_iFillNum = (((*citr).second)[0]).data<int>();
				}
			}
			else
			{
				LogStream << MSG::WARNING << "Could not retrieve CondAttrListCollection " << COOLFOLDER_LHC_FILLSTATE << endreq;
			}
		}
		else if((*iter)==COOLFOLDER_OLC_ALFA)
		{
			if (StatusCode::SUCCESS==m_pDetStore->retrieve(atrlist,COOLFOLDER_OLC_ALFA))
			{
				// the following code dumps the attribute list into a string for printing
				// to access individual elements by name, use e.g.
				// float var1=(*atrlist)["T04"].data<float>();
				// to get the value of a float column called T04 into var1
//				std::ostringstream atrstring;
//				atrlist->print(atrstring);
//				cout<< "Values for folder /TDAQ/ALFA" << endl;
//				cout << atrstring.str() << endl;

				m_BPMALFA.bpmr_r_x_pos  = (*atrlist)["bpmr_r_x_pos"].data<float>();
				m_BPMALFA.bpmr_r_y_pos  = (*atrlist)["bpmr_r_y_pos"].data<float>();
				m_BPMALFA.bpmr_r_x_err  = (*atrlist)["bpmr_r_x_err"].data<float>();
				m_BPMALFA.bpmr_r_y_err  = (*atrlist)["bpmr_r_y_err"].data<float>();
				m_BPMALFA.bpmr_l_x_pos  = (*atrlist)["bpmr_l_x_pos"].data<float>();
				m_BPMALFA.bpmr_l_y_pos  = (*atrlist)["bpmr_l_y_pos"].data<float>();
				m_BPMALFA.bpmr_l_x_err  = (*atrlist)["bpmr_l_x_err"].data<float>();
				m_BPMALFA.bpmr_l_y_err  = (*atrlist)["bpmr_l_y_err"].data<float>();
				m_BPMALFA.bpmsa_r_x_pos = (*atrlist)["bpmsa_r_x_pos"].data<float>();
				m_BPMALFA.bpmsa_r_y_pos = (*atrlist)["bpmsa_r_y_pos"].data<float>();
				m_BPMALFA.bpmsa_r_x_err = (*atrlist)["bpmsa_r_x_err"].data<float>();
				m_BPMALFA.bpmsa_r_y_err = (*atrlist)["bpmsa_r_y_err"].data<float>();
				m_BPMALFA.bpmsa_l_x_pos = (*atrlist)["bpmsa_l_x_pos"].data<float>();
				m_BPMALFA.bpmsa_l_y_pos = (*atrlist)["bpmsa_l_y_pos"].data<float>();
				m_BPMALFA.bpmsa_l_x_err = (*atrlist)["bpmsa_l_x_err"].data<float>();
				m_BPMALFA.bpmsa_l_y_err = (*atrlist)["bpmsa_l_y_err"].data<float>();
				m_BPMALFA.bpmwb_r_x_pos = (*atrlist)["bpmwb_r_x_pos"].data<float>();
				m_BPMALFA.bpmwb_r_y_pos = (*atrlist)["bpmwb_r_y_pos"].data<float>();
				m_BPMALFA.bpmwb_r_x_err = (*atrlist)["bpmwb_r_x_err"].data<float>();
				m_BPMALFA.bpmwb_r_y_err = (*atrlist)["bpmwb_r_y_err"].data<float>();
				m_BPMALFA.bpmya_l_x_pos = (*atrlist)["bpmya_l_x_pos"].data<float>();
				m_BPMALFA.bpmya_l_y_pos = (*atrlist)["bpmya_l_y_pos"].data<float>();
				m_BPMALFA.bpmya_l_x_err = (*atrlist)["bpmya_l_x_err"].data<float>();
				m_BPMALFA.bpmya_l_y_err = (*atrlist)["bpmya_l_y_err"].data<float>();

				if (COOLIOVRange(COOLFOLDER_OLC_ALFA, iTimeIOVStart, iTimeIOVStop).isFailure()) LogStream << MSG::ERROR << "Couldn't get IOV data about folder: " << COOLFOLDER_OLC_ALFA << endreq;

				m_BPMALFA.iTimeIOVstart = iTimeIOVStart;
				m_BPMALFA.iTimeIOVstop  = iTimeIOVStop;

				m_TreeBPMALFA->Fill();
				m_iFlagBPM = m_TreeBPMALFA->GetEntries()-1;
			}
			else
			{
				LogStream << MSG::WARNING << "Could not retrieve AthenaAttributeList " << COOLFOLDER_OLC_ALFA << endreq;
			}
		}
	}

	return StatusCode::SUCCESS;
}

StatusCode ALFA_Ntuple::COOLIOVRange(string strFolder, uint64_t &iTimeIOVStart, uint64_t &iTimeIOVStop)
{
	string strFoldername, strTag;
	IOVRange Range;
	bool bRetrieved;
	unsigned long long ullBytesRead;
	float fReadTime;

	if(!m_iovSvc->getKeyInfo(strFolder, strFoldername, strTag, Range, bRetrieved, ullBytesRead, fReadTime)) return StatusCode::FAILURE;

	IOVTime timeIOVstart = Range.start();
	IOVTime timeIOVstop  = Range.stop();


	if(timeIOVstart.isRunEvent() && timeIOVstop.isRunEvent())
	{
		iTimeIOVStart = static_cast<uint64_t>(((timeIOVstart.run()&0xffff)<<16)|(timeIOVstart.event()&0xffff));
		iTimeIOVStop  = static_cast<uint64_t>(((timeIOVstop.run()&0xffff)<<16)|(timeIOVstop.event()&0xffff));
	}
	else if(timeIOVstart.isTimestamp() && timeIOVstop.isTimestamp())
	{
		iTimeIOVStart = static_cast<unsigned long long>(timeIOVstart.timestamp());
		iTimeIOVStop  = static_cast<unsigned long long>(timeIOVstop.timestamp());
	}
	else
	{
		iTimeIOVStart = 0;
		iTimeIOVStop  = 0;
	}

	return StatusCode::SUCCESS;
}

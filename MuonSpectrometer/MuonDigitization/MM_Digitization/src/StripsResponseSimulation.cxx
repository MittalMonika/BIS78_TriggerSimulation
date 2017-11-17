/*
Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// PROJECTS
#include "MM_Digitization/StripsResponseSimulation.h"
#include "GaudiKernel/MsgStream.h"
#include "MM_Digitization/MM_IonizationCluster.h"
#include "MM_Digitization/MM_StripResponse.h"
#include "MM_Digitization/GarfieldGas.h"
#include "PathResolver/PathResolver.h"

#include<map>
#include<algorithm>

using namespace std;

/*

To Do List:

Check Lorentz Angle is in correct direction...

*/


/*******************************************************************************/
StripsResponseSimulation::StripsResponseSimulation():

	// Variables that should be set externally (MmDigitizationTool)
	m_qThreshold(0),                 // 0.001
	m_transverseDiffusionSigma(0),   // 0.360/10.
	m_longitudinalDiffusionSigma(0), // 0.190/10.
	m_pitch(0.500),
	m_crossTalk1(0),                 // 0.1
	m_crossTalk2(0),                 // 0.03
	m_driftGapWidth(0),              // 5.128
	m_driftVelocity(0),              // 0.047

	// Other variables
	m_avalancheGain(1.16e4),
	m_maxPrimaryIons(300),
	m_interactionDensityMean( 16.15 / 5. ),  //   16.15 interactions per 5 mm traversed
	m_interactionDensitySigma( 4.04 / 5. ),  //   Spread in this number.

	// Function Pointers
	m_polyaFunction(0),
	m_lorentzAngleFunction(0),
	m_longitudinalDiffusionFunction(0),
	m_transverseDiffusionFunction(0),
	m_interactionDensityFunction(0),
	m_random(0),
	m_writeOutputFile(true),
	m_outputFile(0)
	{
	}
/*******************************************************************************/
void StripsResponseSimulation::initHistos()
{
	if(m_writeOutputFile){
		m_outputFile = new TFile("MM_StripsResponse_Plots.root","RECREATE");
		if(m_outputFile) m_outputFile->cd();
	}

	m_mapOfHistograms["nInteractions"] = new TH1F("nInteractions","nInteractions",100,0,100);
	m_mapOfHistograms["nElectrons"] = new TH1F("nElectrons","nElectrons",200,0,200);
	m_mapOfHistograms["lorentzAngle"] = new TH1F("lorentzAngle","lorentzAngle",100,0,3);
	m_mapOfHistograms["effectiveCharge"] = new TH1F("effectiveCharge","effectiveCharge",100,0,2e5);

	m_mapOf2DHistograms["lorentzAngleVsTheta"] = new TH2F("lorentzAngleVsTheta","lorentzAngleVsTheta",  100,-3,3,  100,-3,3);
	m_mapOf2DHistograms["lorentzAngleVsBy"] = new TH2F("lorentzAngleVsBy","lorentzAngleVsBy",  100,-3,3,  100,-2.,2.);


}
/*******************************************************************************/
void StripsResponseSimulation::writeHistos()
{

	if(m_outputFile){

		m_outputFile->cd();
		// for (auto & tmpHist : m_mapOfHistograms){
		// 	tmpHist.second->Write();
		// }
		m_outputFile->Write();
	}
}
/*******************************************************************************/
void StripsResponseSimulation::initFunctions()
{

	// Probability of having an interaction (per unit length traversed) is sampled from a gaussian provided by G. Iakovidis
	m_interactionDensityFunction = new TF1("interactionDensityFunction","gaus", 0., 10.);
	m_interactionDensityFunction->SetParameter(0, 1.0); // normalization
    m_interactionDensityFunction->SetParameter(1, m_interactionDensityMean ); // mean
    m_interactionDensityFunction->SetParameter(2, m_interactionDensitySigma); // width

    // Polya function for gain per primary electron. Fit and parameters from G. Iakovidis
	m_polyaFunction = new TF1("m_polyaFunction","[2]*(TMath::Power([0]+1,[0]+1)/TMath::Gamma([0]+1))*TMath::Power(x/[1],[0])*TMath::Exp(-([0]+1)*x/[1])",0,50000);
    m_polyaFunction->SetParameter(0, 1.765); // polya theta
    m_polyaFunction->SetParameter(1, m_avalancheGain ); // mean gain
    m_polyaFunction->SetParameter(2, 104.9); // constant

	// Functional form fit to agree with Garfield simulations. Fit and parameters from G. Iakovidis
	m_lorentzAngleFunction = new TF1("lorentzAngleFunction","[0] + [1]*x + [2]*x*x + [3]*x*x*x + [4]*x*x*x*x",0,2);
	m_lorentzAngleFunction->SetParameters(0,58.87, -2.983, -10.62, 2.818);

	m_longitudinalDiffusionFunction = new TF1("longdiff","gaus", 0., 5.);

	m_transverseDiffusionFunction = new TF1("transdiff", "1.*TMath::Exp(-TMath::Power(x,2.)/(2.*[0]*[0])) + 0.001*TMath::Exp(-TMath::Power(x,2)/(2.*[1]*[1]))", -1., 1.);

	m_random = new TRandom3(0);

	Athena::MsgStreamMember log("StripsResponseSimulation::initFunctions");
	log << MSG::DEBUG << "StripsResponseSimulation::initFunctions DONE" << endmsg;

}
/*******************************************************************************/
void StripsResponseSimulation::clearValues()
{

	if (m_polyaFunction                  != 0 ) delete m_polyaFunction                  ;
	if (m_lorentzAngleFunction           != 0 ) delete m_lorentzAngleFunction           ;
	if (m_longitudinalDiffusionFunction  != 0 ) delete m_longitudinalDiffusionFunction  ;
	if (m_transverseDiffusionFunction    != 0 ) delete m_transverseDiffusionFunction    ;
	if (m_interactionDensityFunction     != 0 ) delete m_interactionDensityFunction     ;
	if (m_random                         != 0 ) delete m_random                         ;

}

/*******************************************************************************/
void StripsResponseSimulation::initialize()
{

	initHistos ();
	initFunctions();

	Athena::MsgStreamMember log("StripsResponseSimulation::initializationFrom");
	log << MSG::DEBUG << "StripsResponseSimulation::initializationFrom set values" << endmsg;

}

/*******************************************************************************/
MmStripToolOutput StripsResponseSimulation::GetResponseFrom(const MmDigitToolInput & digiInput)
{

	Athena::MsgStreamMember log("StripsResponseSimulation::GetResponseFrom");
	log << MSG::DEBUG << "\t \t StripsResponseSimulation::GetResponseFrom start " << endmsg;

	IonizationClusters.clear();
	finalNumberofStrip.clear();
	finalqStrip.clear();
	finaltStrip.clear();

	whichStrips(digiInput.positionWithinStrip(), digiInput.stripIDLocal(), digiInput.incomingAngle(), digiInput.stripMaxID(), digiInput);

	log << MSG::DEBUG << "\t \t StripsResponseSimulation::GetResponseFrom creating MmDigitToolOutput object " << endmsg;

	MmStripToolOutput tmpStripToolOutput(finalNumberofStrip, finalqStrip, finaltStrip);

	return tmpStripToolOutput;

}

//__________________________________________________________________________________________________
//==================================================================================================
// calculate the strips that got fired, charge and time. Assume any angle thetaDegrees....
//==================================================================================================
void StripsResponseSimulation::whichStrips(const float & hitx, const int & stripID, const float & thetaDegrees, const int & stripMaxID, const MmDigitToolInput & digiInput)
{

	Athena::MsgStreamMember msglog("StripsResponseSimulation::whichStrips");
	msglog << MSG::DEBUG << "\t \t StripsResponseSimulation::whichStrips start " << endmsg;

	float eventTime = digiInput.eventTime();
	float theta = thetaDegrees*TMath::Pi()/180.0;

	int nPrimaryIons = 0;

	m_random->SetSeed((int)fabs(hitx*10000));

	Amg::Vector3D b = digiInput.magneticField() * 1000.;

	// Still need to understand which sign is which... But I think this is correct...

	float lorentzAngle = ( b.y()>0. ?  1.  :  -1. ) * m_lorentzAngleFunction->Eval( fabs( b.y() ) ) * TMath::DegToRad() ; // in radians

	m_mapOf2DHistograms["lorentzAngleVsTheta"]->Fill(lorentzAngle,theta);
	m_mapOf2DHistograms["lorentzAngleVsBy"]->Fill(lorentzAngle,b.y());


	msglog << MSG::DEBUG << "StripsResponseSimulation::lorentzAngle vs theta: " << lorentzAngle << " " << theta << endmsg;
	msglog << MSG::DEBUG << "StripsResponseSimulation::function pointer points to " << m_interactionDensityFunction << endmsg;

	float pathLengthTraveled = ( 1. / m_interactionDensityFunction->GetRandom() ) * -1. * log( m_random->Uniform() );

	float pathLength         = m_driftGapWidth/TMath::Cos(theta);

	while (pathLengthTraveled < pathLength){

		MM_IonizationCluster IonizationCluster(hitx, pathLengthTraveled*sin(theta), pathLengthTraveled*cos(theta));
		IonizationCluster.createElectrons(m_random);
		//    IonizationCluster.diffuseElectrons(longitudinalDiffusSigma, diffusSigma, m_random);
		//---
		TVector2 initialPosition = IonizationCluster.getIonizationStart();

		msglog << MSG::DEBUG << "StripsResponseSimulation:: New interaction starting at x,y, pathLengthTraveled: " << initialPosition.X() << " " << initialPosition.Y() << " " << pathLengthTraveled<< endmsg;

		for (auto& Electron : IonizationCluster.getElectrons()){

			m_longitudinalDiffusionFunction->SetParameters(1.0, initialPosition.Y(), initialPosition.Y()*m_longitudinalDiffusionSigma);

			if ( m_longitudinalDiffusionSigma == 0 || m_transverseDiffusionSigma == 0) {

				m_transverseDiffusionFunction->SetParameters( initialPosition.Y()*m_transverseDiffusionSigma , 0.0);

			} else {

				m_transverseDiffusionFunction->SetParameters( initialPosition.Y()*m_transverseDiffusionSigma , 1.0);

			}

			Electron->setOffsetPosition(m_transverseDiffusionFunction->GetRandom(), m_longitudinalDiffusionFunction->GetRandom());

		}

		IonizationCluster.propagateElectrons( lorentzAngle , m_driftVelocity);

		//---
		for (auto& Electron : IonizationCluster.getElectrons()){

			float effectiveCharge = m_polyaFunction->GetRandom();

			Electron->setCharge( effectiveCharge );
			msglog << MSG::DEBUG << "StripsResponseSimulation::Electron's effective charge is  " << effectiveCharge << endmsg;

			// Add eventTime in Electron time
			Electron->setTime(Electron->getTime() + eventTime);

			m_mapOfHistograms["effectiveCharge"]->Fill( effectiveCharge );

		}
		//---
		IonizationClusters.push_back(IonizationCluster);

		pathLengthTraveled +=  (1. / m_interactionDensityFunction->GetRandom() ) * -1. * log( m_random->Uniform() );

		msglog << MSG::DEBUG << "StripsResponseSimulation:: path length traveled: " << pathLengthTraveled << endmsg;

		nPrimaryIons++;
		if (nPrimaryIons >= m_maxPrimaryIons) break; //don't create more than "MaxPrimaryIons" along a track....

	} //end of clusters loop

	float timeresolution = 0.01; //ns

	MM_StripResponse StripResponse(IonizationClusters, timeresolution, m_pitch, stripID, stripMaxID);
	StripResponse.timeOrderElectrons();
	StripResponse.calculateTimeSeries(thetaDegrees, digiInput.gasgap());
	StripResponse.simulateCrossTalk( m_crossTalk1,  m_crossTalk2);
	StripResponse.calculateSummaries( m_qThreshold );

	//Connect the output with the rest of the existing code
	finalNumberofStrip = StripResponse.getStripVec();
	finalqStrip = StripResponse.getTotalChargeVec();
	finaltStrip = StripResponse.getTimeThresholdVec();
	tStripElectronicsAbThr = StripResponse.getTimeMaxChargeVec();
	qStripElectronics = StripResponse.getMaxChargeVec();

	m_mapOfHistograms["nInteractions"]->Fill(nPrimaryIons);
	m_mapOfHistograms["nElectrons"]->Fill( StripResponse.getNElectrons() );
	m_mapOfHistograms["lorentzAngle"]->Fill( lorentzAngle );

} // end of whichStrips()

/*******************************************************************************/
StripsResponseSimulation::~StripsResponseSimulation()
{
	if(m_outputFile){
		writeHistos();
		delete m_outputFile;
	}
	clearValues();
}
/*******************************************************************************/

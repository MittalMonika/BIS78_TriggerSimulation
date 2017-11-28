#include "fastjet/internal/base.hh"
#include "fastjet/PseudoJet.hh"

#ifndef JETRECTOOLS_PUPPI_h
#define JETRECTOOLS_PUPPI_h

using namespace std;
using namespace fastjet;

// variables (other than alpha) should always be defined so that PU takes small values and HS takes large values - JS
class PuppiUserInfo: public fastjet::PseudoJet::UserInfoBase{
 public:
	PuppiUserInfo();
	PuppiUserInfo(std::vector<double>);
	PuppiUserInfo(double);

	std::vector<double> otherChi2Vec;
};

//=================================================================================================================================

class Puppi{

 public:
	// default ctor
	Puppi(double R0, double Rmin, double beta, double centralPTCutOffset, double centralPTCutSlope, double forwardPTCutOffset, double forwardPTCutSlope, double etaBoundary, bool PUPenalty);

	void setParticles(const std::vector<fastjet::PseudoJet> chargedHS, const std::vector<fastjet::PseudoJet> chargedPU, const std::vector<fastjet::PseudoJet> neutral, const std::vector<fastjet::PseudoJet> forward, int nPU);

	// Returns the weight that would be applied to a particle (assuming it is neutral or forward)
	double getWeight(const PseudoJet pfo);

	// Returns the value of alpha for this particle
	double getAlpha(const PseudoJet pfo);

	double getMedian();
	double getRMS();

 private:

	void fillUserInfo(PseudoJet& pfo);
	void calculateAllAlphas();
	void findAlphaMedianAndRMS();

	double getChi2(const PseudoJet& pfo);

	std::vector<PseudoJet> m_chargedHS;
	std::vector<PseudoJet> m_chargedPU;
	std::vector<PseudoJet> m_neutral;
	std::vector<PseudoJet> m_forward;

	std::vector< std::vector< PseudoJet >* > m_allParticles;

	double m_R0;
	double m_Rmin;
	double m_beta;
	double m_centralPTCutOffset;
	double m_centralPTCutSlope;
	double m_forwardPTCutOffset;
	double m_forwardPTCutSlope;
	double m_etaBoundary;
	bool m_PUPenalty;
	bool m_includeCentralNeutralsInAlpha;

	double m_median;
	double m_rms;

	double m_nPU;

	double m_nDF;

};

#endif 
//FASTJET_END_NAMESPACE

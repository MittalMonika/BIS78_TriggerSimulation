/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// --------------------------------------------------
// 
// File:  GeneratorFilters/LeadingDiBjetFilter.h

// AuthorList:
// Tim Barklow
//
// Based on Filter DiBjetFilter written by:
// Stephen Bieniek
//


#ifndef GENERATORFILTERSLEADINGDIBJETFILTER_H
#define GENERATORFILTERSLEADINGDIBJETFILTER_H

#include "GeneratorModules/GenFilter.h"

//Random number generator required for accepting light jets
class TRandom3;

class LeadingDiBjetFilter:public GenFilter {
public:
        LeadingDiBjetFilter(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~LeadingDiBjetFilter();
        virtual StatusCode filterInitialize();
        virtual StatusCode filterFinalize();
        virtual StatusCode filterEvent();

private:
	// Setable Properties:-

	/* Variables for heavy flavour filtering */
        double m_bottomPtMin;
        double m_bottomEtaMax;
        double m_deltaRFromTruth;
        double m_jetPtMin;
        double m_jetEtaMax;
	/* Variables for cutting sample into pt slices */
	double m_leadJet_ptMin;
	double m_leadJet_ptMax;
        std::string m_TruthJetContainerName;
	/* Variables for light jet suppression */
	double m_LightJetSuppressionFactor;
        bool   m_AcceptSomeLightEvents;

	// Local Member Data:-
        int    m_NPass;
        int    m_Nevt;
        double m_SumOfWeigths_Pass;
        double m_SumOfWeigths_Evt;
	TRandom3* m_ranNumGen;

        bool isBwithWeakDK(const int pID) const;


};

#endif



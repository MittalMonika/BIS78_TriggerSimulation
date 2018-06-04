/*
Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigBtagValidationTest_H
#define TrigBtagValidationTest_H

#ifndef XAOD_ANALYSIS
#include "AthenaBaseComps/AthAlgorithm.h"
#else
#include "AthAnalysisBaseComps/AthAnalysisAlgorithm.h"
#endif

#include "GaudiKernel/ToolHandle.h"

#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrigBtagEmulationTool/ITrigBtagEmulationTool.h"

#include "xAODBTagging/BTaggingContainer.h"
#include "xAODBTagging/BTaggingAuxContainer.h"

#include <string>
#include <map>
#include <tuple>

class IExecuteTool;

namespace Trig{

  class TrigBtagValidationTest : 
#ifndef XAOD_ANALYSIS
  public AthAlgorithm 
#else
  public AthAnalysisAlgorithm
#endif
{ 
        public: 
            /// Constructor with parameters: 
            TrigBtagValidationTest(const std::string& name, ISvcLocator* pSvcLocator);

            /// Destructor: 
	    ~TrigBtagValidationTest();

            /// Athena algorithm's Hooks
            StatusCode  initialize();
            StatusCode  execute();
            StatusCode  finalize();

        private:
	    /// Default constructor: 
	    TrigBtagValidationTest();

	    template<typename T> 
	      bool getFromCombo(std::vector<const T*>&,const Trig::Combination&,std::string key="");


        private:
            ToolHandle<Trig::TrigDecisionTool> m_trigdec;
            ToolHandle<ITrigBtagEmulationTool> m_emulationTool;

            std::vector<std::string>   m_triggerList;
	    StoreGateSvc              *m_storeGate;

	    bool m_errorAtMismatch;
	    bool m_retrieveRetaggedJets;
	    std::string m_chain;

	    long int m_eventCount;
	    long int m_min_eventCount;
	    long int m_max_eventCount;

        private:
	    std::vector< std::string > m_toBeEmulatedTriggers;

	    enum { EMUL, TDT, mismatchesTOT , mismatchesTDT1EMUL0, mismatchesTDT0EMUL1 };
	    std::map< std::string,std::tuple<long int,long int,long int,long int,long int> > m_counters;
    }; 
}
#endif

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////////
// Package : MuonDQAMonitoring
// Author:   N. Benekos(Illinois)
// March 2008
//
// DESCRIPTION:
// Subject: MDT-->Offline Muon Data Quality
///////////////////////////////////////////////////////////////////////////

#ifndef MuonEventInfoMonTool_H
#define MuonEventInfoMonTool_H

#include <sstream>
#include <string.h>

#include "StoreGate/StoreGateSvc.h" 
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
 
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

#include "AthenaMonitoring/AthenaMonManager.h"
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "MuonDQAMonitoring/MuonDQAEvent.h"
 
class TFile;
template <class ConcreteAlgorithm> class AlgFactory;

namespace MuonDQA {
  class MuonDQAEventInfo;
  //}
  /////////////////////////////////////////////////////////////////////////////

  class MuonEventInfoMonTool: public ManagedMonitorToolBase 
    {
    public:

 
      MuonEventInfoMonTool( const std::string & type, const std::string & name, const IInterface* parent ); 
 
      virtual ~MuonEventInfoMonTool();
      StatusCode initialize();
      virtual StatusCode bookHistograms();
      virtual StatusCode fillHistograms();
      virtual StatusCode procHistograms();


    private: 

      /** retrieve event information from storegate and convert to MuonRawDQAEventInfo */
      const MuonDQAEvent* retrieveEvent() const; 
      /** retrieve event information from storegate and convert to MuonCalibEventInfo */
      MuonDQAEventInfo retrieveEventInfo() const;
  
      StoreGateSvc* m_eventStore;
  
      //Declare Properties
 
      int m_lastEvent;
      bool m_TriggerTagAdd;             //!< property toggling if the Trigger type should be added to the EventTag 

      std::string  m_eventTag;          //!< tag of the event, describing reconstruction mechanism 
    
      TH1F* m_hTriggerType; 

    };
}

#endif  

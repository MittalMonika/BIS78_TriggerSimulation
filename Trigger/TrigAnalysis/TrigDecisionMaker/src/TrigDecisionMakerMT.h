// -*- C++ -*-

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/**************************************************************************
 **
 **   File:         TrigDecisionMakerMT.h
 **
 **   Description:  - Re-entrant algorithm-derived class to run after the Steering
 **                 - Finds the list of defined chains at initialization
 **                 - For each event:
 **                   * Creates a xAODTriggerDecision object
 **                   * Fills it with the outcome of each defined chain
 **                   * Output to StoreGate via writehandle written to ESD/AOD
 **
 * @author Tim Martin      <Tim.Martin@cern.ch>      - University of Warwick
 **
 **   Created:      16 July 2018
 **
 **************************************************************************/
#ifndef TrigDecisionMaker_TrigDecisionMakerMT_H
#define TrigDecisionMaker_TrigDecisionMakerMT_H

// Base class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"

#include "xAODEventInfo/EventInfo.h"

// trigger/configuration stuff
#include "xAODTrigger/TrigCompositeContainer.h"
#include "xAODTrigger/TrigDecision.h"
#include "DecisionHandling/TrigCompositeUtils.h"
#include "TrigT1Result/RoIBResult.h"

// containers
#include <vector>
#include <string>
#include <map>

namespace HLT {
  class ILvl1ResultAccessTool;
}

namespace LVL1CTP {
  class Lvl1Result;
}

namespace TrigConf {
  class ITrigConfigSvc;
}

namespace TrigDec {

  /**
   * @class TrigDecisionMakerMT
   *  - Re-entrant algorithm-derived class to run after the MT Trigger
   *  - Finds the list of defined chains at initialization
   *  - For each event:
   *    * Creates a xAODTriggerDecision object
   *    * Fills it with the outcome of each defined chain & L1 data
   *    * Output to StoreGate via writehandle written to ESD/AOD
   *    * Based on TrigDecisionMaker.h
   *
   * @author Tim Martin      <Tim.Martin@cern.ch>      - University of Warwick
   */
  class TrigDecisionMakerMT : public ::AthReentrantAlgorithm
  {

  public:

    TrigDecisionMakerMT(const std::string &name, ISvcLocator *pSvcLocator); //!< std Gaudi Algorithm constructor
    ~TrigDecisionMakerMT();                                                 //!< std  deconstructor

    // IAlgorithm virtual methods to implement
    virtual StatusCode initialize() override; //!< std Gaudi initialize method -> read-in trigger configuration
    virtual StatusCode execute_r( const EventContext& context ) const override; //!< Re-entrant execute to create the xAOD::TrigDecision
    virtual StatusCode finalize() override;    //!< std Gaudi finalize method -> print out statistics

    StatusCode getL1Result (const LVL1CTP::Lvl1Result*& result, const EventContext& context) const; //!< retrieve LVL1 result (called in execute)
    StatusCode getHLTResult(const TrigCompositeUtils::DecisionContainer*& result, const EventContext& context) const; //!< retrieve HLT results (called in execute)

    char getBGByte(int BCId) const; //!< to get the BG byte encoded for a given BC

  private:

    Gaudi::Property<bool> m_doL1{this, "doL1",  true, "Read L1 trigger information"};
    Gaudi::Property<bool> m_doHLT{this, "doHLT", true, "Read HLT trigger information"};

    // Tools & services
    ServiceHandle<TrigConf::ITrigConfigSvc> m_trigConfigSvc; //!< handle to the full (L1 & HLT) trigger config service
    ToolHandle<HLT::ILvl1ResultAccessTool> m_lvl1Tool;  //!< tool to ease the access to the L1 results (RoIs, items, etc)

    // Input keys configuration
    SG::ReadHandleKey<TrigCompositeUtils::DecisionContainer> m_HLTSummaryKeyIn {this, "HLTSummary", "HLTSummary", "HLT summary container Key"};
    SG::ReadHandleKey<LVL1CTP::Lvl1Result> m_L1ResultKeyIn {this, "Lvl1Result", "Lvl1Result", "Lvl1 Result Object Key"};
    SG::ReadHandleKey<ROIB::RoIBResult> m_ROIBResultKeyIn {this, "RoIBResult", "RoIBResult", "RoIB Result Object Key"};
    SG::ReadHandleKey<xAOD::EventInfo> m_EventInfoKeyIn {this, "EventInfo", "EventInfo", "Event Info Object Key"};

    // Output configuration
    SG::WriteHandleKey<xAOD::TrigDecision> m_trigDecisionKeyOut {this, "TrigDecisionKey", "xTrigDecision", "Output trigger decision object key"};

    // For statistics
    mutable std::atomic<uint32_t> m_nEvents{0}, m_l1Passed{0}, m_hltPassed{0};
  };
}

#endif

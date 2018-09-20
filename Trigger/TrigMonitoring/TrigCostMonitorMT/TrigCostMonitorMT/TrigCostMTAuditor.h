/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCOSTMONITORMT_TRIGCOSTMTAUDITOR_H
#define TRIGCOSTMONITORMT_TRIGCOSTMTAUDITOR_H 1

#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaBaseComps/AthMessaging.h"

#include "ITrigCostMTSvc.h"

/**
 * @class TrigCostMTAuditor
 * @brief Gaudi Auditor implementation to hook algorithm executions and notify the Trigger Cost Service
 *
 * Only monitors the Execute event type from the available StandardEventTypes: 
 * {Initialize, ReInitialize, Execute, BeginRun, EndRun, Finalize, Start, Stop, ReStart}
 */
class TrigCostMTAuditor : virtual public Auditor, public AthMessaging {

  using AthMessaging::msg;

  public:

  /**
   * @brief Standard Gaudi Auditor constructor
   * @param[in] name The algorithm object's name
   * @param[in] svcloc A pointer to a service location service
   */
  TrigCostMTAuditor( const std::string& name, ISvcLocator* svcloc );

  /**
   * @brief Initialise auditor. Return handle to Trigger Cost Service
   * @return Success if service handle obtained
   */
  StatusCode initialize() override;

  /**
   * @brief Does nothing
   * @return Success
   */
  StatusCode finalize() override;

  /**
   * @brief Audit before an algorithm standard event type is called
   * @param[in] evt The event type. Only Execute is monitored
   * @param[in] caller The name of the calling algorithm
   */
  void before(StandardEventType evt, const std::string& caller) override;

  /**
   * @brief Audit after an algorithm standard event type is called
   * @param[in] evt The event type. Only Execute is monitored
   * @param[in] caller The name of the calling algorithm
   */
  void after(StandardEventType evt, const std::string&, const StatusCode& sc) override;

  /**
   * @brief Redirects to the const std::string& interface
   */
  void before(StandardEventType evt, INamedInterface* caller) override;

  /**
   * @brief Redirects to the const std::string& interface
   */
  void after(StandardEventType evt, INamedInterface* caller, const StatusCode& sc) override;

  /**
   * @brief Not used
   */
  void before(CustomEventTypeRef, const std::string&) override {}

  /**
   * @brief Not used
   */
  void after(CustomEventTypeRef, const std::string&, const StatusCode&) override {}

  /**
   * @brief Not used
   */
  void before(CustomEventTypeRef, INamedInterface*) override {}

  /**
   * @brief Not used
   */
  void after(CustomEventTypeRef, INamedInterface*, const StatusCode&) override {}

private:

  /**
   * @brief Performs internal call to the trigger cost service
   * @param[in] caller Name of algorithm being audited
   * @param[in] type AuditType::Before or AuditType::After depending on if the start or stop of execution
   */
  void callService(const std::string& caller, ITrigCostMTSvc::AuditType type);

  ServiceHandle<ITrigCostMTSvc> m_trigCostSvcHandle { this, "TrigCostMTSvc", "TrigCostMTSvc", 
    "The trigger cost service to pass audit information to" };

};

inline void TrigCostMTAuditor::before(StandardEventType evt, INamedInterface* caller) {
  return this->before(evt, caller->name());
}

inline void TrigCostMTAuditor::after(StandardEventType evt, INamedInterface* caller, const StatusCode& sc) {
  return this->after(evt, caller->name(), sc);
}

#endif // TRIGCOSTMONITORMT_TRIGCOSTMTAUDITOR_H

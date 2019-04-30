/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "LArHV/FCALHVLine.h"
#include "LArHV/FCALHVModule.h"
#include "LArHV/FCALHVManager.h"
#include "FCALHVPayload.h"

class FCALHVLine::Clockwork {
public:
  Clockwork(const FCALHVModule* modulePtr
	    , unsigned int line)
    : module(modulePtr)
    , iLine(line)
  {}

  const FCALHVModule *module;
  unsigned int iLine;
};

FCALHVLine::FCALHVLine(const FCALHVModule* module
		       , unsigned int iLine)
  : m_c(new Clockwork(module,iLine))
{
}

FCALHVLine::~FCALHVLine()
{
  delete m_c;
}

const FCALHVModule& FCALHVLine::getModule() const
{
  return *(m_c->module);
}

unsigned int FCALHVLine::getLineIndex() const
{
  return m_c->iLine;
}

bool FCALHVLine::hvOn() const
{
  FCALHVPayload *payload = m_c->module->getManager().getPayload(*this);
  return (payload->voltage>=-9999);
}

double FCALHVLine::voltage() const {
  FCALHVPayload *payload = m_c->module->getManager().getPayload(*this);
  return payload->voltage;
}

double FCALHVLine::current() const {
  FCALHVPayload *payload = m_c->module->getManager().getPayload(*this);
  return payload->current;
}

void FCALHVLine::voltage_current(double& voltage, double&current) const {
 FCALHVPayload *payload = m_c->module->getManager().getPayload(*this);
 voltage = payload->voltage;
 current = payload->current;
}


int FCALHVLine::hvLineNo() const {
  FCALHVPayload *payload = m_c->module->getManager().getPayload(*this);
  return payload->hvLineNo;
}

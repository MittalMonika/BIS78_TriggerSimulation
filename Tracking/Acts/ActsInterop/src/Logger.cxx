/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsInterop/Logger.h"

#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/CommonMessaging.h"

#include "Acts/Utilities/Logger.hpp"

#include <boost/optional.hpp>

#include <iostream>
#include <string>

void
ActsAthenaPrintPolicy::flush(const Acts::Logging::Level& lvl, const std::ostringstream& input)
{
  const std::vector<MSG::Level> athLevelVector{
    MSG::VERBOSE, 
    MSG::DEBUG,
    MSG::INFO, 
    MSG::WARNING, 
    MSG::ERROR, 
    MSG::FATAL
  };

  MSG::Level athLevel = athLevelVector[lvl];
  (*m_msg) << athLevel << input.str() << endmsg;
}
  

bool
ActsAthenaFilterPolicy::doPrint(const Acts::Logging::Level& lvl) const 
{
  const std::array<MSG::Level, 6> athLevelVector{
    MSG::VERBOSE, 
    MSG::DEBUG,
    MSG::INFO, 
    MSG::WARNING, 
    MSG::ERROR, 
    MSG::FATAL
  };

  MSG::Level athLevel = athLevelVector[lvl];
  return m_msg->level() <= athLevel;
}


std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(IMessageSvc *svc, const std::string& name, int level, boost::optional<std::string> parent_name)
{
  using namespace Acts::Logging;

  std::string full_name = name;
  if (parent_name) {
    full_name = *parent_name + "." + full_name;
  }

  auto msg = std::make_shared<MsgStream>(svc, full_name);
  msg->setLevel(level);
  auto filter = std::make_unique<ActsAthenaFilterPolicy>(msg);
  auto print = std::make_unique<ActsAthenaPrintPolicy>(msg);
  return std::make_unique<const Acts::Logger>(std::move(print), std::move(filter));
}

std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(CommonMessagingBase* parent, const std::string& name)
{
  // no explicit name, get from component
  INamedInterface *inamed = dynamic_cast<INamedInterface*>(parent);
  boost::optional<std::string> parent_name = boost::none;
  // this will not prefix if parent is not named (which it should be)
  if (inamed == nullptr) {
    throw std::invalid_argument("parent needs to be INamedInterface");
  }
  parent_name = inamed->name();
  return makeActsAthenaLogger(parent, name, parent_name);
}

std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(CommonMessagingBase* parent, const std::string& name, boost::optional<std::string> parent_name)
{
  int level = 0;
  INamedInterface *inamed = dynamic_cast<INamedInterface*>(parent);
  if (inamed != nullptr) {
    level = parent->msgSvc()->outputLevel(inamed->name());
  }
  return makeActsAthenaLogger(parent->msgSvc().get(), name, level, parent_name);
}

std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(CommonMessagingBase* parent, const std::string& name, const std::string& parent_name)
{
  
  return makeActsAthenaLogger(parent, name, boost::optional<std::string>(parent_name));
}

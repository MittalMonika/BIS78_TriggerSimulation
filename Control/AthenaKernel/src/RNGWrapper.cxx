#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include <functional>

#include "GaudiKernel/EventContext.h"
ATHRNG::RNGWrapper::RNGWrapper(CLHEP::HepRandomEngine* eng):m_engine(eng),m_evNum(0),m_runNum(0){
}
ATHRNG::RNGWrapper::~RNGWrapper(){}

bool ATHRNG::RNGWrapper::setSeed(const std::string& algName,const EventContext& ctx){
  if(!ctx.valid()){
    return false;
  }
  if ((m_evNum!=ctx.eventID().event_number())||(m_runNum!=ctx.eventID().run_number())){
    m_evNum=ctx.eventID().event_number();
    m_runNum=ctx.eventID().run_number();
    auto algHash=std::hash<std::string>{}(algName);
    auto evHash=std::hash<decltype(m_evNum)>{}(m_evNum);
    auto runHash=std::hash<decltype(m_runNum)>{}(m_evNum);
    auto hsh=evHash^(runHash+(evHash<<6)+(evHash>>2));
    hsh=hsh^(algHash+(hsh<<6)+(hsh>>2));
    m_engine->setSeed(hsh,0);
    return true;
  }
  return false;
}

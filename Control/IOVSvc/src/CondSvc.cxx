/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
#include "IOVSvc/CondSvc.h"
#include "AthenaKernel/CondCont.h"
#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/SvcFactory.h"

//---------------------------------------------------------------------------

CondSvc::CondSvc( const std::string& name, ISvcLocator* svcLoc ):
  base_class(name,svcLoc),
  m_sgs("StoreGateSvc/ConditionStore", name)
{
}

//---------------------------------------------------------------------------

CondSvc::~CondSvc() {
}

//---------------------------------------------------------------------------

StatusCode
CondSvc::initialize() {

  // Initialise mother class in order to print DEBUG messages during initialize()
  StatusCode sc(AthService::initialize());
  msg().setLevel( m_outputLevel.value() );

  if (!sc.isSuccess()) {
    warning () << "Base class could not be initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  if (!m_sgs.isValid()) {
    error() << "could not get ConditionStore" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;

}
//---------------------------------------------------------------------------

void
CondSvc::dump() const {

  std::ostringstream ost;
  dump(ost);

  info() << ost.str() << endmsg;

}


//---------------------------------------------------------------------------

void
CondSvc::dump(std::ostringstream& ost) const {

  std::lock_guard<std::recursive_mutex> lock(m_lock);

  ost << "CondSvc::dump()";

  ost << "\ndumping id->alg map\n";
  for (auto ent : m_idMap) {
    ost << std::endl << " + " << ent.first << " : ";
    for (auto a : ent.second) {
      ost << " " << a->name();
    }
  }

  ost << "\n\ndumping alg->id map\n";
  for (auto ent : m_algMap) {
    ost << std::endl << " + " << ent.first->name() << " : ";
    for (auto a : ent.second) {
      ost << " " << a;
    }
  }
  ost << "\n\ndumping ConditionStore:\n\n";

  SG::ConstIterator<CondContBase> cib,cie;
  if (m_sgs->retrieve(cib,cie).isSuccess()) {
    while (cib != cie) {
      ost << " + ";
      cib->list(ost);
      ++cib;
    }
  }

  ost << "\n";
    
}

//---------------------------------------------------------------------------

StatusCode
CondSvc::finalize() {

  ATH_MSG_DEBUG( "CondSvc::finalize()" );

  if (msgLvl(MSG::DEBUG)) {
    std::ostringstream ost;
    dump(ost);
    
    ATH_MSG_DEBUG( ost.str() );
  }

  return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------

StatusCode 
CondSvc::regHandle(IAlgorithm* alg, const Gaudi::DataHandle& dh, 
                   const std::string& key) {

  std::lock_guard<std::recursive_mutex> lock(m_lock);

  ATH_MSG_DEBUG( "regHandle: alg: " << alg->name() << "  id: " << dh.fullKey()
                 << "  dBkey: " << key );

  if (dh.mode() != Gaudi::DataHandle::Writer) {
    info() << dh.fullKey() << " is a ReadHandle. no need to register"
           << endmsg;
    return StatusCode::SUCCESS;
  }

  id_map_t::iterator itd2 = m_idMap.find(dh.fullKey());
  if (itd2 != m_idMap.end()) {
    IAlgorithm *ia = *(itd2->second.begin());
    if (ia->name() != alg->name()) {
      error() << "WriteCondHandle " << dh.fullKey() 
              << " is already registered against a different Algorithm " 
              << ia->name()
              << ". This is not allowed."
              << endmsg;
      return StatusCode::FAILURE;
    }
    // FIXME : so why is it a set<IAlgorithm*> ??
  }

  m_keyMap[key] = dh.fullKey();
  m_condAlgs.insert(alg);

  m_condIDs.emplace( dh.fullKey() );

  //  id_map_t::iterator itd2 = m_idMap.find(dh.fullKey());
  if (itd2 != m_idMap.end()) {
    itd2->second.insert( alg );
  } else {
    m_idMap[dh.fullKey()] = IAlgHashSet { alg };
  }

  alg_map_t::iterator ita2 = m_algMap.find(alg);
  if (ita2 != m_algMap.end()) {
    ita2->second.insert( dh.fullKey() );
  } else {
    m_algMap[alg] = DataObjIDColl { dh.fullKey() };
  }

  return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------

bool
CondSvc::getInvalidIDs(const EventContext& ctx, DataObjIDColl& invalidIDs) {
  std::lock_guard<std::recursive_mutex> lock(m_lock);

  EventIDBase now(ctx.eventID().run_number(), ctx.eventID().event_number());

  std::ostringstream ost;
  ost << "getInvalidIDS " << ctx.eventID() 
      << ": retrieving all ConstIterator<CondContBase>";
  SG::ConstIterator<CondContBase> cib, cie;
  if (m_sgs->retrieve(cib,cie).isSuccess()) {
    while(cib != cie) {
      ost << std::endl << "  + " << cib.key() << "  " << cib->valid(now) 
          << "  id: "  << cib->id();
      
      if (! (cib->valid(now)) ) {
        invalidIDs.insert( cib->id() );
      }

      ++cib;      
    }
  } else {
    ATH_MSG_DEBUG(ost.str());
    ATH_MSG_DEBUG("ConditionStore is empty");
    return false;
  }

  ost << std::endl << "  -> found " << invalidIDs.size() << " invalid IDs";

  ATH_MSG_DEBUG( ost.str() );

  return true;
}

//---------------------------------------------------------------------------

bool
CondSvc::getIDValidity(const EventContext& ctx, DataObjIDColl& validIDs,
                       DataObjIDColl& invalidIDs) {
  std::lock_guard<std::recursive_mutex> lock(m_lock);

  EventIDBase now(ctx.eventID());

  std::ostringstream ost;
  ost << "getValidIDS " << ctx.eventID() 
      << ": retrieving all ConstIterator<CondContBase>";
  SG::ConstIterator<CondContBase> cib, cie;
  if (m_sgs->retrieve(cib,cie).isSuccess()) {
    while(cib != cie) {
      ost << std::endl << "  + " << cib.key() << "  v: " << cib->valid(now) 
          << "  id: "  << cib->id();
      
      if ( cib->valid(now) ) {
        validIDs.insert( cib->id() );
      } else {
        invalidIDs.insert( cib->id() );
      }

      ++cib;      
    }
  } else {
    ATH_MSG_DEBUG( ost.str() );
    ATH_MSG_DEBUG( "Condition store empty." );
    return false;
  }

  ost  << std::endl << "  -> found " << validIDs.size() << " valid, "
       << invalidIDs.size() << " invalid IDs";

  ATH_MSG_DEBUG( ost.str() );

  return true;
}

//---------------------------------------------------------------------------

bool
CondSvc::getValidIDs(const EventContext& ctx, DataObjIDColl& validIDs) {
  std::lock_guard<std::recursive_mutex> lock(m_lock);

  EventIDBase now(ctx.eventID());

  std::ostringstream ost;
  ost << "getValidIDS " << ctx.eventID() 
      << ": retrieving all ConstIterator<CondContBase>";
  SG::ConstIterator<CondContBase> cib, cie;
  if (m_sgs->retrieve(cib,cie).isSuccess()) {
    while(cib != cie) {
      ost << std::endl << "  + " << cib.key() << "  v: " << cib->valid(now) 
          << "  id: "  << cib->id();
      
      if ( cib->valid(now) ) {
        validIDs.insert( cib->id() );
      }

      ++cib;      
    }
  } else {
    ATH_MSG_DEBUG( ost.str() );
    ATH_MSG_DEBUG( "Condition store empty." );
    return false;
  }

  ost  << std::endl << "  -> found " << validIDs.size() << " valid IDs";

  ATH_MSG_DEBUG( ost.str() );

  return true;
}

//---------------------------------------------------------------------------

bool
CondSvc::isValidID(const EventContext& ctx, const DataObjID& id) const {
  std::lock_guard<std::recursive_mutex> lock(m_lock);

  EventIDBase now(ctx.eventID());

  CondContBase *cib;
  if (m_sgs->retrieve(cib, id.key()).isSuccess()) {
    return cib->valid(now);
  }

  return false;

}

//---------------------------------------------------------------------------

bool
CondSvc::isRegistered(const DataObjID& id) const {

  return (m_condIDs.find(id) != m_condIDs.end());

}

//---------------------------------------------------------------------------

const DataObjIDColl&
CondSvc::conditionIDs() const {

  return m_condIDs;

}


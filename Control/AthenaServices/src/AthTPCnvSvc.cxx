///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// AthTPCnvSvc.cxx 
// Implementation file for class AthTPCnvSvc
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

#include "AthTPCnvSvc.h"
#include "Gaudi/PluginService.h"


// Constructors
////////////////
AthTPCnvSvc::AthTPCnvSvc( const std::string& name, 
                          ISvcLocator* pSvcLocator ) : 
  base_class ( name, pSvcLocator ),
  m_clidSvc("ClassIDSvc/ClassIDSvc", name)
{
}

// Destructor
///////////////
AthTPCnvSvc::~AthTPCnvSvc()
{
  for (auto p : m_cnvs)
    delete p;
}


/** @brief load the T/P converter class named `cls`
 *  return NULL on failure.
 *  the converter is OWNED by the T/P converter service
 */
ITPCnvBase* 
AthTPCnvSvc::load_tpcnv(const std::string& cls)
{
  ITPCnvBase* cnv = ITPCnvBase::Factory::create (cls);
  if (cnv == 0) {
    ATH_MSG_INFO("could not load class [" << cls
                 << "] via Reflex::PluginService");
  }
  else {
    m_cnvs.push_back (cnv);
  }
  return cnv;
}

/** @brief return the T/P converter for a transient class (NULL if failure)
 *         `ITPCnvSvc` owns the `ITPCnvBase` pointer
 */ 
ITPCnvBase* 
AthTPCnvSvc::t2p_cnv(const std::string& transClassName)
{
  ITPCnvBase* cnv = ITPCnvBase::Factory::create ("_TRANS_" + transClassName);
  if (cnv == 0) {
    ATH_MSG_WARNING("Could not load converter for transient class ["
                    << transClassName << "]");
  }
  else {
    m_cnvs.push_back (cnv);
  }
  return cnv;
}

/** @brief return the T/P converter for a transient class (NULL if failure)
 *         `ITPCnvSvc` owns the `ITPCnvBase` pointer
 */ 
ITPCnvBase* 
AthTPCnvSvc::t2p_cnv(const CLID& transClid)
{
  std::string trans_type;
  if (!m_clidSvc->getTypeNameOfID(transClid, trans_type).isSuccess()) {
    ATH_MSG_INFO("could not get a type-name for clid [" << transClid << "]");
    return 0;
  }

  ITPCnvBase* cnv = ITPCnvBase::Factory::create ("_TRANS_" + trans_type);
  if (cnv == 0) {
    // try a typeinfo-name before bailing out...
    if (!m_clidSvc->getTypeInfoNameOfID(transClid, trans_type).isSuccess()) {
      ATH_MSG_INFO("could not get a typeinfo-name for clid ["
                   << transClid << "]");
      return 0;
    }
    cnv = ITPCnvBase::Factory::create ("_TRANS_" + trans_type);
  }
  if (cnv == 0) {
    ATH_MSG_WARNING("could not load converter for transient CLID ["
                    << transClid << "] (" << trans_type << ")");
  }
  else {
    m_cnvs.push_back (cnv);
  }
  return cnv;
}

/** @brief return the T/P converter for a persistent class (NULL if failure)
 *         `ITPCnvSvc` owns the `ITPCnvBase` pointer
 */ 
ITPCnvBase* 
AthTPCnvSvc::p2t_cnv(const std::string& persClassName)
{
  ITPCnvBase* cnv = ITPCnvBase::Factory::create ("_PERS_" + persClassName);
  if (cnv == 0) {
    ATH_MSG_WARNING("Could not load converter for persistent class ["
                    << persClassName << "]");
  }
  else {
    m_cnvs.push_back (cnv);
  }
  return cnv;
}

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file  src/xAODTestRead.cxx
 * @author snyder@bnl.gov
 * @date May 2014
 * @brief Algorithm to test reading xAOD data.
 */


#include "xAODTestRead.h"
#include "DataModelTestDataCommon/CVec.h"
#include "DataModelTestDataCommon/CVecWithData.h"
#include "DataModelTestDataCommon/C.h"
#include "DataModelTestDataCommon/CAuxContainer.h"
#include "DataModelTestDataCommon/CTrigAuxContainer.h"
#include "DataModelTestDataRead/GVec.h"
#include "DataModelTestDataRead/G.h"
#include "DataModelTestDataRead/GAuxContainer.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthenaKernel/errorcheck.h"
#include <memory>
#include <sstream>


namespace DMTest {


/**
 * @brief Constructor.
 * @param name The algorithm name.
 * @param svc The service locator.
 */
xAODTestRead::xAODTestRead (const std::string &name,
                            ISvcLocator *pSvcLocator)
  : AthAlgorithm (name, pSvcLocator),
    m_count(0)
{
  declareProperty ("ReadPrefix",  m_readPrefix);
  declareProperty ("WritePrefix", m_writePrefix);
}
  

/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestRead::initialize()
{
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestRead::execute()
{
  ++m_count;
  ATH_MSG_INFO (m_count);

  static const C::Accessor<int> anInt2 ("anInt2");
  static const C::Accessor<int> anInt10 ("anInt10");
  static const C::Accessor<int> dInt1 ("dInt1");
  static const C::Accessor<int> dInt100 ("dInt100");
  static const C::Accessor<int> dInt150 ("dInt150");
  static const C::Accessor<int> dInt200 ("dInt200");

  // Ordering of auxid is not reliable.  Sort by name.
  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  std::vector<std::string> names;

  const CVec* ctrig = 0;
  CHECK( evtStore()->retrieve (ctrig, m_readPrefix + "ctrig") );

  names.clear();
  for (SG::auxid_t auxid : ctrig->getAuxIDs())
    names.push_back (r.getName(auxid));
  std::sort (names.begin(), names.end());
  std::ostringstream ost1;
  ost1 << "ctrig aux items: ";
  for (const std::string& n : names)
    ost1 << n << " ";
  ATH_MSG_INFO (ost1.str());

  for (const C* c : *ctrig) {
    std::ostringstream ost;
    ost << " anInt1 " << c->anInt()
        << " aFloat: " << c->aFloat()
        << " anInt2: " << anInt2(*c)
        << " dInt1: " << dInt1(*c);
    if (dInt100.isAvailable(*c))
      ost << " dInt100: " << dInt100(*c);
    if (dInt150.isAvailable(*c))
      ost << " dInt150: " << dInt150(*c);
    if (dInt200.isAvailable(*c))
      ost << " dInt200: " << dInt200(*c);
    if (anInt10.isAvailable(*c))
      ost << " anInt10: " << anInt10(*c);
    ATH_MSG_INFO (ost.str());
  }

  const GVec* gvec = 0;
  if (evtStore()->contains<GVec> (m_readPrefix + "gvec")) {
    CHECK( evtStore()->retrieve (gvec, m_readPrefix + "gvec") );

    names.clear();
    for (SG::auxid_t auxid : gvec->getAuxIDs())
      names.push_back (r.getName(auxid));
    std::sort (names.begin(), names.end());
    std::ostringstream ost3;
    ost3 << "gvec aux items: ";
    for (const std::string& n : names)
      ost3 << n << " ";
    ost3 << "\n";
    for (const G* g : *gvec) {
      ost3 << " anInt " << g->anInt();
      ost3 << "\n";
    }
    ATH_MSG_INFO (ost3.str());
  }

  if (!m_writePrefix.empty()) {
    // Passing this as the third arg of record will make the object const.
    bool LOCKED = false;
    auto ctrignew = std::make_unique<CVec>();
    auto trig_store = std::make_unique<CTrigAuxContainer>();
    ctrignew->setStore (trig_store.get());
    for (size_t i = 0; i < ctrig->size(); i++) {
      ctrignew->push_back (new C);
      *ctrignew->back() = *(*ctrig)[i];
    }
    CHECK (evtStore()->record (std::move(ctrignew), m_writePrefix + "ctrig", LOCKED));
    CHECK (evtStore()->record (std::move(trig_store), m_writePrefix + "ctrigAux.", LOCKED));

    if (gvec) {
      auto gvecnew = std::make_unique<GVec>();
      auto gstore = std::make_unique<GAuxContainer>();
      gvecnew->setStore (gstore.get());
      for (size_t i = 0; i < gvec->size(); i++) {
        gvecnew->push_back (new G);
        *gvecnew->back() = *(*gvec)[i];
      }
      CHECK (evtStore()->record (std::move(gvecnew), m_writePrefix + "gvec", LOCKED));
      CHECK (evtStore()->record (std::move(gstore), m_writePrefix + "gvecAux.", LOCKED));
    }
  }

  CHECK( read_cvec_with_data() );
  //CHECK( read_cview() );

  return StatusCode::SUCCESS;
}


/**
 * @brief Test reading container with additional data.
 */
StatusCode xAODTestRead::read_cvec_with_data() const
{
  const CVecWithData* vec = 0;
  CHECK( evtStore()->retrieve (vec, m_readPrefix + "cvecWD") );

  static const C::Accessor<int> anInt10 ("anInt10");
  std::ostringstream ost;
  ost << m_readPrefix << "cvecWD " << vec->meta1 << ":";
  for (const C* c : *vec) {
    ost << " " << c->anInt();
    if (anInt10.isAvailable(*c))
      ost << "(" << anInt10(*c) << ")";
  }
  ATH_MSG_INFO (ost.str());

  if (!m_writePrefix.empty()) {
    // Passing this as the third arg of record will make the object const.
    bool LOCKED = false;

    auto vecnew = std::make_unique<CVecWithData>();
    auto store = std::make_unique<CAuxContainer>();
    vecnew->setStore (store.get());
    for (size_t i = 0; i < vec->size(); i++) {
      vecnew->push_back (new C);
      *vecnew->back() = *(*vec)[i];
    }
    CHECK (evtStore()->record (std::move(vecnew), m_writePrefix + "cvecWD", LOCKED));
    CHECK (evtStore()->record (std::move(store), m_writePrefix + "cvecWDAux.", LOCKED));
  }

  return StatusCode::SUCCESS;
}


#if 0
/**
 * @brief Test reading view container.
 */
StatusCode xAODTestRead::read_cview() const
{
  if (!evtStore()->contains<CView> (m_readPrefix + "cview")) {
    ATH_MSG_INFO( "(No " << m_readPrefix << "cview view container.)" );
    return StatusCode::SUCCESS;
  }

  const CView* cview = nullptr;
  static const C::Accessor<int> anInt10 ("anInt10");
  CHECK( evtStore()->retrieve (cview, m_readPrefix + "cview") );
  std::ostringstream ost;
  ost << m_readPrefix << "cview:";
  for (const C* c : *cview) {
    ost << " " << c->anInt();
    if (anInt10.isAvailable(*c))
      ost << "(" << anInt10(*c) << ")";
  }
  ATH_MSG_INFO (ost.str());

  if (!m_writePrefix.empty()) {
    bool LOCKED = false;
    CHECK (evtStore()->record (std::make_unique<CView> (*cview),
                               m_writePrefix + "cview", LOCKED));
  }

  return StatusCode::SUCCESS;
}
#endif


/**
 * @brief Algorithm finalization; called at the end of the job.
 */
StatusCode xAODTestRead::finalize()
{
  return StatusCode::SUCCESS;
}


} // namespace DMTest


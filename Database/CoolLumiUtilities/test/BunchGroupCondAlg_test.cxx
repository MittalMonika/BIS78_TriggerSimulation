/*
 * Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CoolLumiUtilities/test/BunchGroupCondAlg_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2019
 * @brief Unit test for BunchGroupCondAlg.
 */


#undef NDEBUG
#include "../src/BunchGroupCondAlg.h"
#include "CoolLumiUtilities/BunchGroupCondData.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "PersistentDataModel/AthenaAttributeList.h"
#include "TestTools/initGaudi.h"
#include "CoralBase/Blob.h"
#include <iostream>
#include <cassert>


class TestRCUSvc
  : public Athena::IRCUSvc
{
public:
  virtual StatusCode remove (Athena::IRCUObject* /*obj*/) override
  {
    return StatusCode::SUCCESS;
  }
  virtual size_t getNumSlots() const override { return 1; }
  virtual void add (Athena::IRCUObject* /*obj*/) override
  { }

  virtual unsigned long addRef() override { std::abort(); }
  virtual unsigned long release() override { std::abort(); }
  virtual StatusCode queryInterface(const InterfaceID &/*ti*/, void** /*pp*/) override { std::abort(); }
};


EventIDBase timestamp (int t)
{
  return EventIDBase (EventIDBase::UNDEFNUM,  // run
                      EventIDBase::UNDEFEVT,  // event
                      t);
}


void test1 (ISvcLocator* svcloc)
{
  std::cout << "test1\n";

  EventContext ctx;
  ctx.setExtension (Atlas::ExtendedEventContext());
  EventIDBase eid (0, 0, 0, 0);
  ctx.setEventID (eid);

  BunchGroupCondAlg alg ("BunchGroupCondAlg", svcloc);
  alg.addRef();
  assert( alg.sysInitialize().isSuccess() );

  TestRCUSvc rcu;
  DataObjID id1 ("testBG");
  auto cc1 = std::make_unique<CondCont<AthenaAttributeList> > (rcu, id1);
  DataObjID id2 ("BunchGroupCondData");
  
  coral::Blob blob (BunchGroupCondData::NBCID);
  uint8_t* bcids = static_cast<uint8_t*> (blob.startingAddress());
  bcids[3] = 0xa1;
  bcids[5] = 0x65;
  bcids[7] = 0x9c;
  auto attrs = std::make_unique<AthenaAttributeList>();
  attrs->extend ("BunchCode", "blob");
  (*attrs)["BunchCode"].setValue (blob);

  const EventIDRange range (timestamp (0), timestamp (100));
  assert( cc1->insert (range, std::move(attrs), ctx).isSuccess() );

  ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "test");
  assert( conditionStore->record (std::move (cc1), "testBG") );

  assert( alg.execute (ctx).isSuccess() );

  CondCont<BunchGroupCondData>* cc2 = nullptr;
  assert( conditionStore->retrieve (cc2, "BunchGroupCondData").isSuccess() );
  const BunchGroupCondData* data = 0;
  const EventIDRange* range2p = nullptr;
  assert (cc2->find (eid, data, &range2p));
  assert (range2p->start().time_stamp() == timestamp(0).time_stamp());
  assert (range2p->stop().time_stamp() == timestamp(100).time_stamp());

  assert (data->bunchGroup(0) == (std::vector<unsigned int>{3, 5}));
  assert (data->bunchGroup(1) == (std::vector<unsigned int>{}));
  assert (data->bunchGroup(2) == (std::vector<unsigned int>{5, 7}));
  assert (data->bunchGroup(3) == (std::vector<unsigned int>{7}));
  assert (data->bunchGroup(4) == (std::vector<unsigned int>{7}));
  assert (data->bunchGroup(5) == (std::vector<unsigned int>{3, 5}));
  assert (data->bunchGroup(6) == (std::vector<unsigned int>{5}));
  assert (data->bunchGroup(7) == (std::vector<unsigned int>{3, 7}));
}


int main()
{
  std::cout << "CoolLumiUtilities/BunchGroupCondAlg\n";

  ISvcLocator* svcloc = nullptr;
  if (!Athena_test::initGaudi("CoolLumiUtilities/BunchGroupCondAlg_test.txt", svcloc)) {
    return 1;
  }


  test1 (svcloc);
  return 0;
}

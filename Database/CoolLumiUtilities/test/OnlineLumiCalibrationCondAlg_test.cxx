/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file CoolLumiUtilities/test/OnlineLumiCalibrationCondAlg_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2018
 * @brief Unit test for OnlineLumiCalibrationCondAlg.
 */


#undef NDEBUG
#include "../src/OnlineLumiCalibrationCondAlg.h"
#include "CoolLumiUtilities/OnlineLumiCalibrationCondData.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/Blob.h"
#include "TestTools/initGaudi.h"
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


coral::AttributeList makeAL (float muToLumi,
                             float p0, float p1, float p2)
{
  coral::AttributeList attr;
  attr.extend ("NumOfParameters", "unsigned int");
  attr.extend ("Function", "string");
  attr.extend ("MuToLumi", "float");
  attr.extend ("Parameters", "blob");

  attr["MuToLumi"].setValue (muToLumi);
  attr["Function"].setValue (std::string ("Polynomial"));

  coral::Blob blob (9 * sizeof(float));
  float* p = static_cast<float*> (blob.startingAddress());
  p[0] = 1;
  p[1] = 0;
  p[2] = 100;
  p[3] = p0;
  p[4] = p1;
  p[5] = p2;
  p[6] = 0;
  p[7] = 0;
  p[8] = 0;
  attr["Parameters"].setValue (blob);
  attr["NumOfParameters"].setValue (9u);

  return attr;
}


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

  OnlineLumiCalibrationCondAlg alg ("OnlineLumiCalibrationCondAlg", svcloc);
  alg.addRef();
  assert( alg.sysInitialize().isSuccess() );

  TestRCUSvc rcu;
  DataObjID id1 ("testcalib");
  auto cc1 = std::make_unique<CondCont<CondAttrListCollection> > (rcu, id1);
  DataObjID id2 ("OnlineLumiCalibrationCondData");
  
  auto attrs = std::make_unique<CondAttrListCollection> (false);
  attrs->add (1, makeAL (10.5, 1, 2, 3));
  attrs->add (7, makeAL (13.5, 3, 2, 1));
  const EventIDRange range (timestamp (0), timestamp (100));
  assert( cc1->insert (range, std::move(attrs), ctx).isSuccess() );

  ServiceHandle<StoreGateSvc> conditionStore ("ConditionStore", "test");
  assert( conditionStore->record (std::move (cc1), "testcalib") );

  assert( alg.execute (ctx).isSuccess() );

  CondCont<OnlineLumiCalibrationCondData>* cc2 = nullptr;
  assert( conditionStore->retrieve (cc2, "OnlineLumiCalibrationCondData").isSuccess() );
  const OnlineLumiCalibrationCondData* data = 0;
  assert (cc2->find (eid, data));

  assert (data->getMuToLumi(1) == 10.5);
  assert (data->getMuToLumi(7) == 13.5);
  assert (data->getMuToLumi(5) == -1);

  float x = 0;
  assert (data->calibrateMu(1, 2, x));
  assert (x == (1 + 2*2 + 3*4));
  assert (data->calibrateMu(7, 2, x));
  assert (x == (3 + 2*2 + 1*4));
  assert (!data->calibrateMu(5, 2, x));

  assert (data->calibrateLumi(1, 2, x));
  assert (x == (1 + 2*2 + 3*4) * 10.5);
  assert (data->calibrateLumi(7, 2, x));
  assert (x == (3 + 2*2 + 1*4) * 13.5);
  assert (!data->calibrateLumi(5, 2, x));
}


int main()
{
  std::cout << "CoolLumiUtilities/OnlineLumiCalibrationCondAlg\n";

  ISvcLocator* svcloc;
  if (!Athena_test::initGaudi("OnlineLumiCalibrationCondAlg_test.txt", svcloc)) {
    return 1;
  }


  test1 (svcloc);
  return 0;
}

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file D3PDMakerTest/src/FillerAlg.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2010
 * @brief Put objects into StoreGate to test D3PD maker.
 */


#include "FillerAlg.h"
#include "D3PDMakerTest/Obj1.h"
#include "D3PDMakerTest/Obj3.h"
#include "D3PDMakerTest/Obj4.h"
#include "ParticleEvent/SelectedParticles.h"
#include "NavFourMom/INavigable4MomentumCollection.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/errorcheck.h"
#include <stdint.h>


using CLHEP::GeV;


namespace D3PDTest {


// Dufus-quality RNG, using LCG.  Constants from numerical recipies.
// I don't particularly care about RNG quality here, just about
// getting something that's reproducible.
uint32_t seed = 1;
uint32_t rngmax = static_cast<uint32_t> (-1);
uint32_t rng()
{
  seed = (1664525*seed + 1013904223);
  return seed;
}

double randf (double rmax, double rmin = 0)
{
  return static_cast<double>(rng()) / rngmax * (rmax-rmin) + rmin;
}




/**
 * @brief Standard Gaudi algorithm constructor.
 * @param name The algorithm name.
 * @param svcloc The service locator.
 */
FillerAlg::FillerAlg (const std::string& name,
                      ISvcLocator* svcloc)
  : AthAlgorithm (name, svcloc),
    m_count (0)
{
  declareProperty ("SGKeyObj1Cont", m_sgkeyObj1cont = "obj1container");
  declareProperty ("SGKeyObj1Sel",  m_sgkeyObj1sel  = "obj1sel");
  declareProperty ("SGKeyObj1",     m_sgkeyObj1     = "obj1");
  declareProperty ("SGKeyObj3Cont", m_sgkeyObj3cont = "obj3container");
  declareProperty ("SGKeyObj4Cont", m_sgkeyObj4cont = "obj4container");
}


/**
 * @brief Standard Gaudi @c initialize method.
 */
StatusCode FillerAlg::initialize()
{
  Obj1Container::dvlinfo(); // TEMP
  m_count = 0;
  return AthAlgorithm::initialize();
}


/**
 * @brief Standard Gaudi @c finalize method.
 */
StatusCode FillerAlg::finalize()
{
  return AthAlgorithm::finalize();
}


/**
 * @brief Fill collections involving Obj1/2.
 */
StatusCode FillerAlg::fillObj1Collections()
{
  Obj1Container* c = new Obj1Container;
  SelectedParticles* s = new SelectedParticles;
  for (unsigned i = 0; i < 10; i++) {
    Obj1* o1 = new Obj1 (10*m_count + i);
    for (unsigned int j=0; j < i; j++)
      o1->m_vo2.push_back (Obj2 (10*m_count + i + 100*j));
    c->push_back (o1);

    if (i % (m_count+1) == 0)
      s->SetBit (i);
  }
  CHECK( this->evtStore()->record (c, m_sgkeyObj1cont) );
  CHECK( this->evtStore()->record (s, m_sgkeyObj1sel) );

  Obj1* o1 = new Obj1;
  for (int i = 0; i < m_count; i++)
    o1->m_vo2.push_back (Obj2 (i + 23*m_count));
  for (int i = 0; i < 2*m_count; i++)
    o1->m_vo3.push_back (Obj2 (i + 27*m_count));
  c = new Obj1Container;
  c->push_back (o1);
  CHECK( this->evtStore()->record (c, m_sgkeyObj1) );

  return StatusCode::SUCCESS;
}


/**
 * @brief Fill collections involving Obj3.
 */
StatusCode FillerAlg::fillObj3Collections()
{
  INavigable4MomentumCollection* c = new INavigable4MomentumCollection;
  for (unsigned i = 0; i < 10; i++) {
    Obj3* o3 = new Obj3 (randf(100*GeV),
                         randf(5, -5),
                         randf(-M_PI, M_PI),
                         randf(10*GeV));
    c->push_back (o3);
  }
  CHECK( this->evtStore()->record (c, m_sgkeyObj3cont) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Fill collections involving Obj4.
 */
StatusCode FillerAlg::fillObj4Collections()
{
  Obj4Container* c = new Obj4Container;
  for (unsigned i = 0; i < 10; i++) {
    Obj4* o4 = new Obj4 (30*m_count + i);
    c->push_back (o4);
  }
  CHECK( this->evtStore()->record (c, m_sgkeyObj4cont) );

  return StatusCode::SUCCESS;
}


/**
 * @brief Standard Gaudi @c execute method.
 */
StatusCode FillerAlg::execute()
{
  CHECK( fillObj1Collections() );
  CHECK( fillObj3Collections() );
  CHECK( fillObj4Collections() );

  ++m_count;
  return StatusCode::SUCCESS;
}


} // namespace D3PDTest

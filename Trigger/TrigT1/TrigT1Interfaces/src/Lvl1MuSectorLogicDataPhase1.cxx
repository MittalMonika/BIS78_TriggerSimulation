/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#include <cstdio>
#include <iostream>
#include <iomanip>

#include "TrigT1Interfaces/Lvl1MuSectorLogicDataPhase1.h"
#include "TrigT1Interfaces/Lvl1MuSectorLogicConstantsPhase1.h"

namespace LVL1MUONIF {

  Lvl1MuSectorLogicDataPhase1::Lvl1MuSectorLogicDataPhase1()
      : 
    m_2candidatesInSector ( false ), 
    m_roi(NULL),
    m_ovl(NULL),
    m_pt(NULL),
    m_2candidates(NULL),
    m_bcid( 0 ),
    m_charge(NULL),
    m_id_system( -1 )
  {
  }
  Lvl1MuSectorLogicDataPhase1::~Lvl1MuSectorLogicDataPhase1()
  {
    delete m_roi;
    delete m_ovl;
    delete m_pt;
    delete m_charge;
    delete m_2candidates;
  }

  void Lvl1MuSectorLogicDataPhase1::initialize()
  {
    if (m_id_system == -1) return;

    size_t ncand = NCAND[m_id_system];
    m_roi = new int[ncand];
    m_ovl = new int[ncand];
    m_pt = new int[ncand];
    m_charge = new int[ncand];
    m_2candidates = new bool[ncand];
    for ( size_t i = 0; i < ncand; ++i ) {
      m_roi[ i ] = -1;
      m_ovl[ i ] = -1;
      m_pt[ i ] = -1;
      m_charge[ i ] = + 1;
      m_2candidates[ i ] = false;
    }
  }


  Lvl1MuSectorLogicDataPhase1::Lvl1MuSectorLogicDataPhase1( const Lvl1MuSectorLogicDataPhase1& right ) {
    *this = right;
  }

  Lvl1MuSectorLogicDataPhase1& Lvl1MuSectorLogicDataPhase1::operator=( const Lvl1MuSectorLogicDataPhase1& right ) {
    if ( this != &right ) {
      m_id_system = right.m_id_system;
      m_bcid = right.m_bcid;
      m_2candidatesInSector = right.m_2candidatesInSector;
      for ( size_t i = 0; i < NCAND[m_id_system]; ++i ) {
        m_roi[ i ] = right.m_roi[ i ];
        m_ovl[ i ] = right.m_ovl[ i ];
        m_pt[ i ] = right.m_pt[ i ];
        m_charge[ i ] = right.m_charge[ i ];
        m_2candidates[ i ] = right.m_2candidates[ i ];
      }
    }
    return *this;
  }

  std::ostream& operator<<( std::ostream& out, const Lvl1MuSectorLogicDataPhase1& right ) {

    right.printOut( out );
    return out;

  }

  void Lvl1MuSectorLogicDataPhase1::clear() {
    m_2candidatesInSector = false;
    m_bcid = 0;
    for ( size_t i = 0; i < NCAND[m_id_system]; ++i ) {
      m_roi[ i ] = -1;
      m_ovl[ i ] = -1;
      m_pt[ i ] = -1;
      m_charge[ i ] = -1;
      m_2candidates[ i ] = false;
    }
  }

} // namespace LVL1MUONIF

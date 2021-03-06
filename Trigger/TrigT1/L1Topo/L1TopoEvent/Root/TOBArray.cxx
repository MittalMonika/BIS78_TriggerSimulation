/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
//  TOBArray.cxx
//  TopoCore
//  Created by Joerg Stelzer on 11/17/12.

#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/CompositeTOB.h"
#include "L1TopoEvent/InputTOBArray.h"
#include "L1TopoEvent/JetTOBArray.h"
#include "L1TopoEvent/ClusterTOBArray.h"

#include <algorithm>

void TCS::TOBArray::print(std::ostream &o) const {
   for(const_iterator tob = m_data.begin(); tob != m_data.end(); ++tob) {
      if( tob!=begin() ) o << std::endl;
      o << **tob;
   }
}

void
TCS::TOBArray::push_back(const TCS::CompositeTOB& tob) {
   m_data.push_back(CompositeTOB::createOnHeap(tob));
}

void
TCS::TOBArray::sort(sort_fnc fnc) {
   std::sort(m_data.begin(), m_data.end(), fnc);
}

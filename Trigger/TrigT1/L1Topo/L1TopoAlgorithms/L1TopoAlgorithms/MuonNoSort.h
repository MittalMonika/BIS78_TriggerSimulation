/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
//  MuonNoSort.h
//  TopoCore
//  Created by Joerg Stelzer on 11/10/12.

#ifndef TCS__MuonNoSort
#define TCS__MuonNoSort

#include "L1TopoInterfaces/SortingAlg.h"
#include "L1TopoEvent/TOBArray.h"

#include <iostream>
#include <vector>

namespace TCS {
   
   class MuonNoSort : public SortingAlg {
   public:
      
      // constructor
      MuonNoSort(const std::string & name);

      // destructor
      virtual ~MuonNoSort();

      virtual TCS::StatusCode initialize();

      virtual TCS::StatusCode sort(const InputTOBArray & input, TOBArray & output);
      
   private:

      parType_t      m_numberOfMuons = { 0 };

   };

} // end of namespace TCS

#endif /* defined(__TopoCore__SortingAlg__) */

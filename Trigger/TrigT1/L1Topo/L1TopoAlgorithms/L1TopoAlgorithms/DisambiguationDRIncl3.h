//  DisambiguationDRIncl3.h
//  TopoCore
//  Created by Joerg Stelzer on 11/16/12.
//  Copyright (c) 2012 Joerg Stelzer. All rights reserved.

#ifndef __TopoCore__DisambiguationDRIncl3__
#define __TopoCore__DisambiguationDRIncl3__

#include <iostream>
#include "L1TopoInterfaces/DecisionAlg.h"

namespace TCS {
   
   class DisambiguationDRIncl3 : public DecisionAlg {
   public:
      DisambiguationDRIncl3(const std::string & name);
      virtual ~DisambiguationDRIncl3();

      virtual StatusCode initialize();

      
      virtual StatusCode process( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison );
      

   private:

      parType_t      p_NumberLeading1 = { 0 };
      parType_t      p_NumberLeading2 = { 0 };
      parType_t      p_DisambDR[2] = {0, 0};
      parType_t      p_MinET1 = { 0 };
      parType_t      p_MinET2 = { 0 };
      parType_t      p_MinET3 = { 0 };
      parType_t      p_EtaMin1 = { 0 };
      parType_t      p_EtaMax1 = { 0 };
      parType_t      p_EtaMin2 = { 0 };
      parType_t      p_EtaMax2 = { 0 };
      parType_t      p_EtaMin3 = { 0 };
      parType_t      p_EtaMax3 = { 0 };
      parType_t      p_DRCutMin = { 0 };
      parType_t      p_DRCutMax = { 0 };


   };
   
}

#endif

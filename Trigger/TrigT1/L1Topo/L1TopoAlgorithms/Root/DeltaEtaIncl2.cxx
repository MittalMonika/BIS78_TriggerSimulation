/*********************************
 * DeltaEtaIncl2.cpp
 * Created by Joerg Stelzer on 11/16/12.
 * Modified V Sorin 2014
 * Copyright (c) 2012 Joerg Stelzer. All rights reserved.
 *
 * @brief algorithm calculates the eta-distance between two lists and applies delta-eta criteria
 *
 * @param NumberLeading
**********************************/

#include <cmath>

#include "L1TopoAlgorithms/DeltaEtaIncl2.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"

REGISTER_ALG_TCS(DeltaEtaIncl2)

using namespace std;

// not the best solution but we will move to athena where this comes for free
#define LOG cout << fullname() << ":     "


namespace {
   unsigned int
   calcDeltaEta(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
      double deta = fabs( tob1->eta() - tob2->eta() );
      return deta;
   }
}


TCS::DeltaEtaIncl2::DeltaEtaIncl2(const std::string & name) : DecisionAlg(name)
{
   defineParameter("InputWidth1", 0);
   defineParameter("InputWidth2", 0); 
   defineParameter("NumResultBits", 1);
   defineParameter("MinEt1",0);
   defineParameter("MinEt2",0);
   defineParameter("MinDeltaEta",  0, 0);
   defineParameter("MaxDeltaEta", 127, 0);
   setNumberOutputBits(1);
}

TCS::DeltaEtaIncl2::~DeltaEtaIncl2(){}


TCS::StatusCode
TCS::DeltaEtaIncl2::initialize() {
   p_NumberLeading1 = parameter("InputWidth1").value();
   p_NumberLeading2 = parameter("InputWidth2").value();
   for(int i=0; i<1; ++i) {
      p_DeltaEtaMin[i] = parameter("MinDeltaEta", i).value();
      p_DeltaEtaMax[i] = parameter("MaxDeltaEta", i).value();
   }
   p_MinET1 = parameter("MinEt1").value();
   p_MinET2 = parameter("MinEt2").value();


   TRG_MSG_INFO("NumberLeading1 : " << p_NumberLeading1);  // note that the reading of generic parameters doesn't work yet
   TRG_MSG_INFO("NumberLeading2 : " << p_NumberLeading2);
   TRG_MSG_INFO("DeltaEtaMin0   : " << p_DeltaEtaMin[0]);
   TRG_MSG_INFO("DeltaEtaMax0   : " << p_DeltaEtaMax[0]);
   TRG_MSG_INFO("MinET1          : " << p_MinET1);
   TRG_MSG_INFO("MinET2          : " << p_MinET2);
   TRG_MSG_INFO("number output : " << numberOutputBits());
   
   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::DeltaEtaIncl2::process( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decison )
{

      
   if( input.size() == 2) {
      
      for( TOBArray::const_iterator tob1 = input[0]->begin(); 
           tob1 != input[0]->end() && distance(input[0]->begin(), tob1) <= p_NumberLeading1;
           ++tob1)
         {

            if( parType_t((*tob1)->Et()) <= p_MinET1) continue; // ET cut

            for( TCS::TOBArray::const_iterator tob2 = input[1]->begin(); 
                 tob2 != input[1]->end() && distance(input[1]->begin(), tob2) <= p_NumberLeading2;
                 ++tob2) {

               if( parType_t((*tob2)->Et()) <= p_MinET2) continue; // ET cut

               // test DeltaEtaMin, DeltaEtaMax
               unsigned int deltaEta = calcDeltaEta( *tob1, *tob2 );

               bool accept[1];
               for(unsigned int i=0; i<1; ++i) {
                  accept[i] = deltaEta >= p_DeltaEtaMin[i] && deltaEta <= p_DeltaEtaMax[i];
                  if( accept[i] ) {
                     decison.setBit(i, true);
                     output[i]->push_back(TCS::CompositeTOB(*tob1, *tob2));
                  }
               }
	       TRG_MSG_DEBUG("DeltaEta = " << deltaEta << " -> " 
                             << (accept[0]?"pass":"fail"));

            }
         }

   } else {

      TCS_EXCEPTION("DeltaEtaIncl2 alg must have 2 inputs, but got " << input.size());

   }
   return TCS::StatusCode::SUCCESS;
}


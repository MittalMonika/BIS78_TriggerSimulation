/*********************************
 * MinDeltaPhiIncl2.cpp
 * Created by Veronica Sorin  on 14/8/14.
 *
 * @brief algorithm calculates the min phi-distance between  two lists and applies delta-phi criteria
 *
 * @param NumberLeading
**********************************/

#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "TH1F.h"


#include "L1TopoAlgorithms/MinDeltaPhiIncl2.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"

REGISTER_ALG_TCS(MinDeltaPhiIncl2)

using namespace std;

// not the best solution but we will move to athena where this comes for free
#define LOG cout << "TCS::MinDeltaPhiIncl2:     "




namespace {
   unsigned int
   calcDeltaPhi(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
      double dphi = fabs( tob1->phiDouble() - tob2->phiDouble() );
      if(dphi>M_PI)
         dphi = 2*M_PI - dphi;
      
      return round( 10 * dphi );
   }
}


TCS::MinDeltaPhiIncl2::MinDeltaPhiIncl2(const std::string & name) : DecisionAlg(name)
{
   defineParameter("InputWidth1", 9);
   defineParameter("InputWidth2", 9);
   defineParameter("MaxTob1", 0); 
   defineParameter("MaxTob2", 0); 
   defineParameter("NumResultBits", 3);
   defineParameter("DeltaPhiMin",  0, 0);
   defineParameter("DeltaPhiMin",  0, 1);
   defineParameter("DeltaPhiMin",  0, 2);
   defineParameter("MinET1",1);
   defineParameter("MinET2",1);
   setNumberOutputBits(3);
}

TCS::MinDeltaPhiIncl2::~MinDeltaPhiIncl2(){}


TCS::StatusCode
TCS::MinDeltaPhiIncl2::initialize() {
   p_NumberLeading1 = parameter("InputWidth1").value();
   p_NumberLeading2 = parameter("InputWidth2").value();
   if(parameter("MaxTob1").value() > 0) p_NumberLeading1 = parameter("MaxTob1").value();
   if(parameter("MaxTob2").value() > 0) p_NumberLeading2 = parameter("MaxTob2").value();

   for(int i=0; i<3; ++i) {
      p_DeltaPhiMin[i] = parameter("DeltaPhiMin", i).value();
   }
   p_MinET1 = parameter("MinET1").value();
   p_MinET2 = parameter("MinET2").value();
   TRG_MSG_INFO("NumberLeading1 : " << p_NumberLeading1);
   TRG_MSG_INFO("NumberLeading2 : " << p_NumberLeading2);
   for(int i=0; i<3; ++i) {
      TRG_MSG_INFO("DeltaPhiMin"<<i<<"   : " << p_DeltaPhiMin[i]);
   }
   TRG_MSG_INFO("MinET1         : " << p_MinET1);
   TRG_MSG_INFO("MinET2         : " << p_MinET2);
   TRG_MSG_INFO("nummber output : " << numberOutputBits());

   // create strings for histogram names
   ostringstream MyAcceptHist[numberOutputBits()];
   ostringstream MyRejectHist[numberOutputBits()];
   
   for (unsigned int i=0;i<numberOutputBits();i++) {
     MyAcceptHist[i] << "Accept" << p_DeltaPhiMin[i]  << "MinDPhi2"; 
     MyRejectHist[i] << "Reject" << p_DeltaPhiMin[i]  << "MinDPhi2";
   }

   for (unsigned int i=0; i<numberOutputBits();i++) {
     char MyTitle1[100];
     char MyTitle2[100];
     string Mys1 = MyAcceptHist[i].str();
     string Mys2 = MyRejectHist[i].str();
     std::strcpy(MyTitle1,Mys1.c_str());
     std::strcpy(MyTitle2,Mys2.c_str());
     
     registerHist(m_histAcceptMinDPhi2[i] = new TH1F(MyTitle1,MyTitle1,100,0,3.5));
     registerHist(m_histRejectMinDPhi2[i] = new TH1F(MyTitle2,MyTitle2,100,0,3.5));
   }

   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::MinDeltaPhiIncl2::process( const std::vector<TCS::TOBArray const *> & input,
                            const std::vector<TCS::TOBArray *> & output,
                            Decision & decison )
{

   // mindphi 
   unsigned int mindphi = *min_element(begin(p_DeltaPhiMin),end(p_DeltaPhiMin));
   bool firstphi = true;

   // declare iterator for the tob with min dphi
   TCS::TOBArray::const_iterator tobmin1,tobmin2;  
      
   if (input.size() == 2) {
   
      for( TOBArray::const_iterator tob1 = input[0]->begin(); 
           tob1 != input[0]->end() && distance(input[0]->begin(), tob1) < p_NumberLeading1;
           ++tob1)
         {

            if( parType_t((*tob1)->Et()) <= p_MinET1) continue; // ET cut

            for( TCS::TOBArray::const_iterator tob2 = input[1]->begin(); 
                 tob2 != input[1]->end() && distance(input[1]->begin(), tob2) < p_NumberLeading2;
                 ++tob2) {

               if( parType_t((*tob2)->Et()) <= p_MinET2) continue; // ET cut

               // test DeltaPhiMin, DeltaPhiMax
               unsigned int deltaPhi = calcDeltaPhi( *tob1, *tob2 );

               if (firstphi) {
                  mindphi = deltaPhi;
                  tobmin1=tob1;
                  tobmin2=tob2;
                  firstphi = false;
               }

               if ( deltaPhi < mindphi ) {
                  
                  mindphi = deltaPhi;
                  tobmin1=tob1;
                  tobmin2=tob2;

               }

            }
         }

      bool accept[3];
      for(unsigned int i=0; i<numberOutputBits(); ++i) {
         accept[i] = mindphi > p_DeltaPhiMin[i] ;
         if( accept[i] ) {
            decison.setBit(i, true);
            output[i]->push_back(TCS::CompositeTOB(*tobmin1, *tobmin2));
	    m_histAcceptMinDPhi2[i]->Fill((float)mindphi/10.);

            TRG_MSG_DEBUG("Decision " << i << ": " << (accept[i]?"pass":"fail") << " mindphi = " << mindphi << "phi1= " << (*tobmin1)->phiDouble()<< "phi2= " <<(*tobmin2)->phiDouble() );
         }
	 else
	   m_histRejectMinDPhi2[i]->Fill((float)mindphi/10.);

         TRG_MSG_DEBUG("Decision " << i << ": " << (accept[i]?"pass":"fail"));
      }


   } else {

      TCS_EXCEPTION("MinDeltaPhiIncl2 alg must have 2 inputs, but got " << input.size());

   }

   return TCS::StatusCode::SUCCESS;
}

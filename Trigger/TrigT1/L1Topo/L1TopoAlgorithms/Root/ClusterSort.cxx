//  ClusterSort.cxx
//  TopoCore
//  Created by Joerg Stelzer on 11/10/12.
//  Copyright (c) 2012 Joerg Stelzer. All rights reserved.
//  algorithm to create sorted lists for clusters, et order applied
//
#include "L1TopoAlgorithms/ClusterSort.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/ClusterTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(ClusterSort)

bool SortByEtLargest(TCS::GenericTOB* tob1, TCS::GenericTOB* tob2)
{
   return tob1->Et() > tob2->Et();
}


// constructor
TCS::ClusterSort::ClusterSort(const std::string & name) : SortingAlg(name) {
   defineParameter( "InputWidth", 120 ); // for FW
   defineParameter( "InputWidth1stStage", 30 ); // for FW
   defineParameter( "OutputWidth", 6);
   defineParameter( "IsoMask", 0);
   defineParameter( "MinEta", 0);
   defineParameter( "MaxEta", 63);
}


// destructor
TCS::ClusterSort::~ClusterSort() {}


TCS::StatusCode
TCS::ClusterSort::initialize() {
   m_numberOfClusters = parameter("OutputWidth").value();
   m_iso = parameter("IsoMask").value();
   m_minEta = parameter("MinEta").value();
   m_maxEta = parameter("MaxEta").value();
   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TCS::ClusterSort::sort(const InputTOBArray & input, TOBArray & output) {

   const ClusterTOBArray & clusters = dynamic_cast<const ClusterTOBArray&>(input);

   // fill output array with GenericTOB buildt from clusters
   for(ClusterTOBArray::const_iterator cl = clusters.begin(); cl!= clusters.end(); ++cl ) {
      const GenericTOB gtob(**cl);

      if (parType_t(fabs((*cl)-> eta())) < m_minEta) continue; 
      if (parType_t(fabs((*cl)-> eta())) > m_maxEta) continue;
      // isolation cut
      if (m_iso != 0 ) {
	unsigned int isobit(0x1 << (m_iso-1));
        if((parType_t((*cl)->isolation()) & isobit) != isobit ) continue;
      }
      
      output.push_back( gtob );
   }

   // sort
   output.sort(SortByEtLargest);
   

   // keep only max number of clusters
   int par = m_numberOfClusters;
   unsigned int maxNumberOfClusters = (unsigned int)(par<0?0:par);
   if(maxNumberOfClusters>0) {
      while( output.size()> maxNumberOfClusters ) {
         output.pop_back();
      }
   }
   return TCS::StatusCode::SUCCESS;
}


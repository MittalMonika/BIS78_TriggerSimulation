/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCaloEventTPCnv/TrigEMClusterCnv_tlp2.h"


//constructor
TrigEMClusterCnv_tlp2::TrigEMClusterCnv_tlp2(){

  // add the "main" base class converter (ie. TrigEMClusterCnv)
  addMainTPConverter();

  // add all converters needed in the top level converter
  // do not change the order of adding converters
  addTPConverter( &m_trigCaloClusterCnv );
}

void TrigEMClusterCnv_tlp2::setPStorage( TrigEMCluster_tlp2* storage ){


   //for the base class converter
   setMainCnvPStorage( &storage->m_trigEMCluster );

   //for all other converters defined in the base class
   m_trigCaloClusterCnv.              setPStorage( &storage->m_trigCaloCluster );
}

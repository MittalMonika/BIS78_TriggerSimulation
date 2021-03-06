/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonEventTPCnv/MuonMeasurementsCnv_tlp3.h"

MuonMeasurementsCnv_tlp3::MuonMeasurementsCnv_tlp3()
{  
   // // Add all converters defined in this top level converter:
   // // never change the order of adding converters!  
   addTPConverter( &m_cscClustersOTCnv );   
   addTPConverter( &m_rpcClustersOTCnv );     
   addTPConverter( &m_tgcClustersOTCnv );     
   addTPConverter( &m_mdtDriftCirclesOTCnv );
   addTPConverter( &m_muonSegmentsCnv );  
   addTPConverter( &m_muonSegmentQualitiesCnv );
   addTPConverter( &m_muonCompetingROTsCnv );
   addTPConverter( &m_mmClustersOTCnv );
   addTPConverter( &m_stgcClustersOTCnv );
}

void MuonMeasurementsCnv_tlp3::setPStorage( TPCnv::MuonMeasurements_tlp3 *storage )
{
   m_cscClustersOTCnv.       setPStorage( &storage->m_cscClustersOT  );
   m_rpcClustersOTCnv.       setPStorage( &storage->m_rpcClustersOT );
   m_tgcClustersOTCnv.       setPStorage( &storage->m_tgcClustersOT );
   m_mdtDriftCirclesOTCnv.   setPStorage( &storage->m_mdtDriftCirclesOT );
   m_muonSegmentsCnv.        setPStorage( &storage->m_muonSegments );   
   m_muonSegmentQualitiesCnv.setPStorage( &storage->m_muonSegmentQualities );  
   m_muonCompetingROTsCnv.   setPStorage( &storage->m_muonCompetingROT ); 
   m_mmClustersOTCnv.        setPStorage( &storage->m_mmClustersOT ); 
   m_stgcClustersOTCnv.      setPStorage( &storage->m_stgcClustersOT ); 
}   
 

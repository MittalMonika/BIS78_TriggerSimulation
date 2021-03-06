/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VXCONTAINER_CNV_TLP1_TRK_H
#define VXCONTAINER_CNV_TLP1_TRK_H

//-----------------------------------------------------------------------------
//
// file:   VxContainerCnv_tlp1.h
// @brief  Top Level converter for persistent Vertex Collection
//
//-----------------------------------------------------------------------------
#include "AthenaPoolCnvSvc/AthenaPoolTopLevelTPConverter.h"

#include "VxContainer_tlp1.h"

//-----------------------------------------
// VxVertex
//-----------------------------------------
#include "TrkEventTPCnv/VxVertex/VxContainerCnv_p1.h"   
#include "TrkEventTPCnv/VxVertex/VxCandidateCnv_p1.h"   
#include "TrkEventTPCnv/VxVertex/ExtendedVxCandidateCnv_p1.h"     
#include "TrkEventTPCnv/VxVertex/VxTrackAtVertexCnv_p1.h"
#include "TrkEventTPCnv/VxVertex/VxTrackAtVertexCnv_p2.h"
#include "TrkEventTPCnv/VxVertex/RecVertexCnv_p1.h"
#include "TrkEventTPCnv/VxVertex/VertexCnv_p1.h"

//-----------------------------------------------------------------------------
// Trk Parameters
//-----------------------------------------------------------------------------
#include "TrkEventTPCnv/TrkParameters/TrackParametersCnv_p1.h"
#include "TrkEventTPCnv/TrkParameters/PerigeeCnv_p1.h"
#include "TrkEventTPCnv/TrkParameters/MeasuredPerigeeCnv_p1.h"

//-----------------------------------------------------------------------------
// Surfaces and related
//-----------------------------------------------------------------------------
#include "TrkEventTPCnv/TrkSurfaces/PerigeeSurfaceCnv_p1.h"  

//-----------------------------------------------------------------------------
// TrkEventPrimitives
//-----------------------------------------------------------------------------
#include "TrkEventTPCnv/TrkEventPrimitives/FitQualityCnv_p1.h"
#include "TrkEventTPCnv/TrkEventPrimitives/ErrorMatrixCnv_p1.h"
#include "TrkEventTPCnv/TrkEventPrimitives/LocalPositionCnv_p1.h"

//-----------------------------------------------------------------------------
// TrkTrack
//-----------------------------------------------------------------------------
#include "TrkEventTPCnv/TrkTrack/TrackCnv_p1.h"

//-----------------------------------------------------------------------------
// Element Link for track collection
//-----------------------------------------------------------------------------
#include "DataModelAthenaPool/ElementLinkCnv_p1.h"
#include "DataModel/ElementLink.h" 
#include "TrkTrack/TrackCollection.h"

class VxContainerCnv_tlp1 : public AthenaPoolTopLevelTPConverter< VxContainerCnv_p1, Trk::VxContainer_tlp1 >
{

 public:
 
  VxContainerCnv_tlp1();
  virtual ~VxContainerCnv_tlp1() {}
  
//default methods  
  virtual void	setPStorage( Trk::VxContainer_tlp1 *storage );

// return list of tokens to extending persistent objects
// it is stored in m_tokenList member of the Track persistent rep
  virtual TPCnvTokenList_p1* 	getTokenListVar() { return &(getTLPersObject()->m_tokenList); }

// all TP converters for types used in the Vertex package 
 protected:

//VxVertex
   VxCandidateCnv_p1          m_vxCandidateCnv;
   ExtendedVxCandidateCnv_p1  m_extendedVxCandidateCnv;
   VxTrackAtVertexCnv_p1      m_vxTrackAtVertexCnv;
   // VxTrackAtVertexCnv_p2      m_vxTrackAtVertexCnv_p2;
   RecVertexCnv_p1            m_recVertexCnv;
   VertexCnv_p1               m_VertexCnv;

//TrkTrack 
   TrackCnv_p1                m_trackCnv;
   
//TrkParameters   
   TrackParametersCnv_p1      m_trackParametersCnv;
   PerigeeCnv_p1              m_perigeeCnv;
   MeasuredPerigeeCnv_p1      m_measuredPerigeeCnv;
   
//Surfaces and related   
   PerigeeSurfaceCnv_p1       m_perigeeSurfacesCnv;
   
//TrkEventPrimitives
   FitQualityCnv_p1	      m_fitQualityCnv;
   ErrorMatrixCnv_p1	      m_errorMatrixCnv;
   LocalPositionCnv_p1	      m_localPositionCnv;
//element link 
   
};//end of class definitions


#include "TrkEventTPCnv/MVFVxContainerCnv_tlp1.h"
template <>
class T_TPCnv<VxContainer, Trk::VxContainer_tlp1 >
  : public VxContainerCnv_tlp1
{
public:
  T_TPCnv();

  // ??? Maybe this should be moved up to AthenaPoolTopLevelTPConverter...
  virtual void persToTrans (const Trk::VxContainer_tlp1* pers,
                            VxContainer* trans,
                            MsgStream& msg);

private:
  MVFVxContainerCnv_tlp1 m_MVFVxContainerCnv_tlp1;
};


#endif //VXCONTAINER_CNV_TLP1_TRK_H

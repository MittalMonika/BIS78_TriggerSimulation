///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// P4ImplPxPyPzECnv_p1.cxx 
// Implementation file for class P4ImplPxPyPzECnv_p1
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes

// Framework includes
#include "GaudiKernel/MsgStream.h"

// FourMom includes
#include "FourMom/Lib/P4ImplPxPyPzE.h"

// EventCommonTPCnv includes
#include "EventCommonTPCnv/P4ImplPxPyPzECnv_p1.h"

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////

// Destructor
///////////////

/////////////////////////////////////////////////////////////////// 
// Const methods: 
///////////////////////////////////////////////////////////////////

void 
P4ImplPxPyPzECnv_p1::persToTrans( const P4ImplPxPyPzE_p1* persObj, 
				  P4ImplPxPyPzE* transObj, 
				  MsgStream &/*msg*/ )
{
  transObj->m_px = persObj->m_px;
  transObj->m_py = persObj->m_py;
  transObj->m_pz = persObj->m_pz;
  transObj->m_e  = persObj->m_ene;
  return;
}

void 
P4ImplPxPyPzECnv_p1::transToPers( const P4ImplPxPyPzE* transObj, 
				  P4ImplPxPyPzE_p1* persObj, 
				  MsgStream &/*msg*/ )
{
  persObj->m_px  = transObj->m_px;
  persObj->m_py  = transObj->m_py;
  persObj->m_pz  = transObj->m_pz;
  persObj->m_ene = transObj->m_e;
  return;
}

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////////////// 
// Protected methods: 
/////////////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////////////// 
// Const methods: 
/////////////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#ifndef MMPREPDATA_p1_TRK_H
#define MMPREPDATA_p1_TRK_H

//-----------------------------------------------------------------------------
//
// file:   MMPrepData_p1.h
//
//-----------------------------------------------------------------------------
#include "AthenaPoolUtilities/TPObjRef.h"
#include "Identifier/IdentifierHash.h"

namespace Muon
{
  /** 
  We don't write out (from Trk::PrepRawData) 
     * m_indexAndHash (can be recomputed), 
     * m_clusId (can be recomputed - well, it's basically stored in Muon::MdtPRD_Container_p1).
  */
    class MMPrepData_p1
    {
    public:
    MMPrepData_p1() : m_locX(0.0), m_errorMat(0.0), m_time(0), m_charge(0), m_angle(0.0), m_chisqProb(0.0) {}
        
        std::vector< signed char > 	m_rdoList; //!< Store offsets
        
        /// @name Data from Trk::PrepRawData
        //@{
        float               m_locX; //!< Equivalent to localPosition (locX) in the base class.
        float               m_errorMat; //!< 1-d ErrorMatrix in the base class.

	int                 m_time;      // for single-strip PRD, that's the time measured
	int                 m_charge;    // for single-strip PRD, that's the charge measured

	float               m_angle;          ///
	float               m_chisqProb;      /// these are the parameters of the muTPC reconstruction
        
        //@}
        
        /// @name Data from Muon::MMPrepData
        //@{
        //@}  
    };
}

#endif 
